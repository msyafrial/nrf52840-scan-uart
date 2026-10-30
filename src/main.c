/*
 * Copyright (c) 2026
 * nRF52840 Universal Real-time Raw BLE Gateway (Zephyr RTOS)
 * Target: Seeed Studio XIAO BLE / Super52840 & Pro Micro nRF52840
 * Mode  : Promiscuous Sniffer - Fixed 42-Byte Binary Frame @ 1 Mbps UART
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

/* =================================================================
 * 1. KONFIGURASI SISTEM & BATCHING
 * ================================================================= */
#define DEBUG_MODE true

/* Kapasitas antrean memori untuk menjamin penyerapan burst tanpa lag */
#define QUEUE_CAPACITY 100

/* Batas usia toleransi paket (TTL). Paket > 50 ms langsung didrop (Zero Stale
 * Data) */
#define MAX_PACKET_AGE_MS 50

/* Kapasitas maksimal batching per transmisi (5 frame * 42 byte = 210 byte <=
 * 256B FIFO) */
#define MAX_BATCH_FRAMES 5
#define FRAME_SIZE 42

/* Indikator LED Heartbeat: led1 (Hijau di Xiao BLE / Merah di Pro Micro via
 * alias) */
#define LED_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

/* Hardware UART0 menuju ESP32 @ 1 Mbps */
#define ESP32_UART_NODE DT_NODELABEL(uart0)
static const struct device *const esp32_uart = DEVICE_DT_GET(ESP32_UART_NODE);

/* USB CDC ACM Console untuk Serial Monitor Laptop */
static const struct device *const console_dev =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/* Hardware Watchdog 0 (Timeout 5 Detik) */
#define WDT_NODE DT_ALIAS(watchdog0)
static const struct device *const wdt_dev = DEVICE_DT_GET(WDT_NODE);
static int wdt_channel_id = -1;

/* =================================================================
 * 2. STRUKTUR DATA PROTOKOL BINER MENTAH (42 BYTE)
 * ================================================================= */
struct __attribute__((packed)) RawBleBinaryFrame {
  uint8_t header;      /* Byte 0      : Selalu 0xAA (Sync Byte) */
  uint8_t mac[6];      /* Byte 1..6   : MAC Address (MSB ke LSB) */
  uint8_t adv_type;    /* Byte 7      : Tipe PDU Radio (ADV_IND, etc.) */
  int8_t rssi;         /* Byte 8      : Kekuatan sinyal dBm */
  uint8_t payload_len; /* Byte 9      : Panjang data AD asli (0..31 byte) */
  uint8_t payload[31]; /* Byte 10..40 : Raw AD Structures (0-padded jika len <
                          31) */
  uint8_t checksum;    /* Byte 41     : XOR byte 1..40 */
};

BUILD_ASSERT(sizeof(struct RawBleBinaryFrame) == FRAME_SIZE,
             "RawBleBinaryFrame size must be exactly 42 bytes");

/* Struktur Item Antrean FIFO Memori */
struct RawBlePacket {
  uint32_t timestamp;  /* Waktu kedatangan paket (k_uptime_get_32) */
  uint8_t mac[6];      /* MAC Address */
  uint8_t adv_type;    /* Tipe PDU */
  int8_t rssi;         /* Signal strength dBm */
  uint8_t payload_len; /* Panjang data AD */
  uint8_t payload[31]; /* Data AD mentah */
};

/* Antrean Pesan Zero-Copy (k_msgq) */
K_MSGQ_DEFINE(beacon_msgq, sizeof(struct RawBlePacket), QUEUE_CAPACITY, 4);

/* Metrik Kinerja (Atomic untuk Thread-Safety) */
static atomic_t total_received = ATOMIC_INIT(0);
static atomic_t total_sent = ATOMIC_INIT(0);
static atomic_t total_dropped = ATOMIC_INIT(0);
static atomic_t total_expired = ATOMIC_INIT(0);

/* =================================================================
 * 3. HELPER QUEUE (DROP-OLDEST FIFO)
 * ================================================================= */
static inline void q_push_drop_oldest(const struct RawBlePacket *pkt) {
  atomic_inc(&total_received);

  /* Coba masukkan paket ke antrean tanpa blocking */
  if (k_msgq_put(&beacon_msgq, pkt, K_NO_WAIT) != 0) {
    /* Antrean PENUH (100 item): Drop paket terlama (Drop-Oldest FIFO) */
    struct RawBlePacket dummy;
    k_msgq_get(&beacon_msgq, &dummy, K_NO_WAIT);
    atomic_inc(&total_dropped);

    /* Masukkan paket baru */
    k_msgq_put(&beacon_msgq, pkt, K_NO_WAIT);
  }
}

/* =================================================================
 * 4. BLE ADVERTISING CALLBACK (ZERO-OVERHEAD PROMISCUOUS CAPTURE)
 * ================================================================= */
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad) {
  struct RawBlePacket pkt;

  pkt.timestamp = k_uptime_get_32();
  pkt.adv_type = type;
  pkt.rssi = rssi;

  /* Konversi format alamat MAC ke Big-Endian (MSB ke LSB) */
  for (int i = 0; i < 6; i++) {
    pkt.mac[i] = addr->a.val[5 - i];
  }

  /* Batasi dan salin payload AD mentah (maks 31 byte) */
  uint8_t len =
      (ad != NULL && ad->len <= 31) ? (uint8_t)ad->len : (ad != NULL ? 31 : 0);
  pkt.payload_len = len;

  if (len > 0) {
    memcpy(pkt.payload, ad->data, len);
  }
  if (len < 31) {
    memset(&pkt.payload[len], 0, 31 - len);
  }

  /* Langsung dorong ke antrean memori tanpa overhead parsing */
  q_push_drop_oldest(&pkt);
}

/* =================================================================
 * 5. ADAPTIVE BATCHING TRANSMITTER KE ESP32 (UART 1 MBPS)
 * ================================================================= */
static inline void assemble_frame(uint8_t *dest,
                                  const struct RawBlePacket *pkt) {
  struct RawBleBinaryFrame *frame = (struct RawBleBinaryFrame *)dest;
  frame->header = 0xAA;
  memcpy(frame->mac, pkt->mac, 6);
  frame->adv_type = pkt->adv_type;
  frame->rssi = pkt->rssi;
  frame->payload_len = pkt->payload_len;
  memcpy(frame->payload, pkt->payload, 31);

  /* XOR Checksum: byte 1 s/d byte 40 */
  uint8_t crc = 0;
  const uint8_t *raw = (const uint8_t *)frame;
  for (size_t i = 1; i < FRAME_SIZE - 1; i++) {
    crc ^= raw[i];
  }
  frame->checksum = crc;
}

static void esp32_tx_thread_entry(void *p1, void *p2, void *p3) {
  struct RawBlePacket pkt;
  uint8_t batch_buf[MAX_BATCH_FRAMES * FRAME_SIZE];

  while (1) {
    /* 1. Tunggu minimal 1 paket pertama (blocking, 0% CPU saat idle) */
    k_msgq_get(&beacon_msgq, &pkt, K_FOREVER);

    int frame_count = 0;

    /* Cek usia paket pertama (TTL) */
    uint32_t age_ms = k_uptime_get_32() - pkt.timestamp;
    if (age_ms <= MAX_PACKET_AGE_MS) {
      assemble_frame(&batch_buf[0], &pkt);
      frame_count++;
    } else {
      atomic_inc(&total_expired);
    }

    /* 2. ADAPTIVE BATCHING: Jika ada paket berikutnya yang sudah antre,
     *    kuras langsung secara non-blocking hingga MAX_BATCH_FRAMES */
    while (frame_count < MAX_BATCH_FRAMES &&
           k_msgq_get(&beacon_msgq, &pkt, K_NO_WAIT) == 0) {
      age_ms = k_uptime_get_32() - pkt.timestamp;
      if (age_ms <= MAX_PACKET_AGE_MS) {
        assemble_frame(&batch_buf[frame_count * FRAME_SIZE], &pkt);
        frame_count++;
      } else {
        atomic_inc(&total_expired);
      }
    }

    /* 3. Kirim seluruh batch kontigu ke UART */
    if (frame_count > 0) {
      size_t total_bytes = frame_count * FRAME_SIZE;
      for (size_t i = 0; i < total_bytes; i++) {
        uart_poll_out(esp32_uart, batch_buf[i]);
      }
      atomic_add(&total_sent, frame_count);
    }
  }
}
K_THREAD_DEFINE(esp32_tx_tid, 2048, esp32_tx_thread_entry, NULL, NULL, NULL,
                K_PRIO_PREEMPT(4), 0, 0);

/* =================================================================
 * 6. THREAD DEBUG & STATS MONITOR (USB CDC ACM) - LOW PRIORITY
 * ================================================================= */
static void format_count(uint32_t val, char *buf, size_t sz) {
  if (val >= 1000000) {
    snprintf(buf, sz, "%.1fM", (double)val / 1000000.0);
  } else if (val >= 10000) {
    snprintf(buf, sz, "%.1fK", (double)val / 1000.0);
  } else {
    snprintf(buf, sz, "%u", val);
  }
}

static void debug_stats_thread_entry(void *p1, void *p2, void *p3) {
  uint32_t last_time = k_uptime_get_32();
  uint32_t last_received = 0;
  uint32_t dtr = 0;
  char str_in[16];
  char str_sent[16];

  while (1) {
    k_msleep(2000);

    if (DEBUG_MODE && device_is_ready(console_dev)) {
      uart_line_ctrl_get(console_dev, UART_LINE_CTRL_DTR, &dtr);
      if (dtr) {
        uint32_t now = k_uptime_get_32();
        uint32_t cur_recv = atomic_get(&total_received);
        uint32_t cur_sent = atomic_get(&total_sent);
        uint32_t cur_drop = atomic_get(&total_dropped);
        uint32_t cur_exp = atomic_get(&total_expired);

        uint32_t delta_ms = now - last_time;
        uint32_t pps =
            (delta_ms > 0) ? ((cur_recv - last_received) * 1000 / delta_ms) : 0;
        last_received = cur_recv;
        last_time = now;

        format_count(cur_recv, str_in, sizeof(str_in));
        format_count(cur_sent, str_sent, sizeof(str_sent));

        printf(">>> [RAW BLE STATS] Rate: %u pkt/s | In: %s | Sent ESP32: %s | "
               "Expired: %u | Dropped: %u <<<\r\n",
               pps, str_in, str_sent, cur_exp, cur_drop);
      }
    }
  }
}
K_THREAD_DEFINE(debug_stats_tid, 2048, debug_stats_thread_entry, NULL, NULL,
                NULL, K_PRIO_PREEMPT(10), 0, 0);

/* =================================================================
 * 7. MAIN FUNCTION
 * ================================================================= */
int main(void) {
  int err;

  /* Inisialisasi LED Indikator */
  if (gpio_is_ready_dt(&led)) {
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  }

  /* Inisialisasi UART0 ke ESP32 */
  if (!device_is_ready(esp32_uart)) {
    return 0;
  }

  /* Tunggu host membuka Serial Monitor USB maks 2 detik (Non-blocking) */
  if (device_is_ready(console_dev)) {
    uint32_t dtr = 0;
    for (int i = 0; i < 20; i++) {
      uart_line_ctrl_get(console_dev, UART_LINE_CTRL_DTR, &dtr);
      if (dtr)
        break;
      k_msleep(100);
    }
  }

  printf("\r\n=======================================================\r\n");
  printf(">>> nRF52840 Universal Raw BLE Gateway (Zephyr RTOS) <<<\r\n");
  printf("=======================================================\r\n");

  /* Periksa Penyebab Reboot Sebelumnya (Hardware Diagnostics) */
  uint32_t reset_cause = 0;
  if (hwinfo_get_reset_cause(&reset_cause) == 0) {
    if (reset_cause & RESET_WATCHDOG) {
      printf("[SYSTEM] PERINGATAN: Sistem pulih dari WATCHDOG TIMEOUT!\r\n");
    } else {
      printf("[SYSTEM] Sistem booting normal (Reset Cause: 0x%08X)\r\n",
             reset_cause);
    }
    hwinfo_clear_reset_cause();
  }

  /* Inisialisasi Hardware Watchdog Timer (WDT) - 5 Detik Timeout */
  if (device_is_ready(wdt_dev)) {
    struct wdt_timeout_cfg wdt_config = {
        .window.min = 0,
        .window.max = 5000, /* 5000 ms = 5 detik timeout */
        .callback = NULL,   /* Reset SoC langsung jika freeze */
        .flags = WDT_FLAG_RESET_SOC,
    };
    wdt_channel_id = wdt_install_timeout(wdt_dev, &wdt_config);
    if (wdt_channel_id >= 0) {
      err = wdt_setup(wdt_dev, WDT_OPT_PAUSE_HALTED_BY_DBG);
      if (err == 0) {
        printf("[WDT] Hardware Watchdog aktif (Timeout: 5 detik, "
               "Auto-Recovery)\r\n");
      } else {
        printf("[WDT] Setup Watchdog GAGAL (err %d)\r\n", err);
      }
    } else {
      printf("[WDT] Install timeout GAGAL (err %d)\r\n", wdt_channel_id);
    }
  } else {
    printf("[WDT] Device Watchdog tidak siap!\r\n");
  }

  printf("[MODE] Promiscuous Raw Sniffer - Fixed 42-Byte Binary Frame\r\n");
  printf("[UART] UARTE aktif di 1.000.000 bps menuju ESP32 (D6=TX, D7=RX)\r\n");

  /* Inisialisasi Subsystem Bluetooth Low Energy */
  err = bt_enable(NULL);
  if (err) {
    printf("[BLE] Inisialisasi Bluetooth GAGAL (err %d)\r\n", err);
    return 0;
  }
  printf("[BLE] Bluetooth Controller aktif & siap.\r\n");

  /* Scanning Parameter: 100% Continuous Passive Scanning (Interval = Window =
   * 160 / 100ms) */
  struct bt_le_scan_param scan_param = {
      .type = BT_LE_SCAN_TYPE_PASSIVE,
      .options = BT_LE_SCAN_OPT_NONE,
      .interval = 160,
      .window = 160,
      .timeout = 0,
      .interval_coded = 0,
      .window_coded = 0,
  };

  err = bt_le_scan_start(&scan_param, device_found);
  if (err) {
    printf("[BLE] Memulai scan GAGAL (err %d)\r\n", err);
    return 0;
  }

  printf(
      "[BLE] Continuous Passive Scanner berjalan pada 100%% duty cycle.\r\n");
  printf(
      "----------------------------------------------------------------\r\n");

  /* Loop Utama: Heartbeat LED berkedip setiap 1 detik & Feed WDT */
  while (1) {
    if (wdt_channel_id >= 0) {
      wdt_feed(wdt_dev, wdt_channel_id);
    }
    if (gpio_is_ready_dt(&led)) {
      gpio_pin_toggle_dt(&led);
    }
    k_sleep(K_MSEC(1000));
  }

  return 0;
}
