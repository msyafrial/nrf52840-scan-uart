/*
 * Copyright (c) 2026
 * nRF52840 Universal Real-time Raw BLE Gateway (Zephyr RTOS) - v3.0 Ultra-High Throughput
 * Target: Seeed Studio XIAO BLE / Super52840 & Pro Micro nRF52840
 * Mode  : Promiscuous Sniffer - Fixed 42-Byte Binary Frame @ 1 Mbps UART
 * Full Duplex Control Protocol (5-Byte Downlink, 4-Byte Uplink)
 * Single-Owner DMA TX, Offset-8 Aligned Checksum, Dynamic Dual-Prime
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
#include <zephyr/sys/reboot.h>

/* =================================================================
 * 1. KONFIGURASI SISTEM & PROTOKOL KONTROL
 * ================================================================= */
#define DEBUG_MODE true

/* Status Pemindaian Radio BLE */
#define SCAN_STATE_STOPPED 0x00
#define SCAN_STATE_PASSIVE 0x01
#define SCAN_STATE_ACTIVE  0x02
#define SCAN_STATE_ERROR   0xEE

/* Protokol Perintah Downlink dari ESP32 (5 Byte Frame) */
#define CMD_HEADER_BYTE    0x55
#define CMD_SET_SCAN_MODE  0x01
#define CMD_GET_STATUS     0x02
#define CMD_SOFT_REBOOT    0x03
#define REBOOT_MAGIC_PARAM 0x55

/* Protokol Balasan Uplink ke ESP32 (4 Byte Frame) */
#define RSP_HEADER_BYTE    0xBB
#define RSP_TYPE_ACK       0x81
#define RSP_TYPE_STATUS    0x82
#define RSP_TYPE_NACK      0x8F

/* Dynamic Dual-Prime Scanning Parameters */
#define PRIME_PASSIVE_INTERVAL 29 /* 29 * 0.625 ms = 18.125 ms (55.17 hop/s) */
#define PRIME_PASSIVE_WINDOW   29 /* 100% Duty Cycle */
#define PRIME_ACTIVE_INTERVAL  37 /* 37 * 0.625 ms = 23.125 ms (43.24 hop/s) */
#define PRIME_ACTIVE_WINDOW    37 /* 100% Duty Cycle */

/* Kapasitas antrean memori ultra-high throughput (512 item * 52B = ~26.6 KB SRAM) */
#define QUEUE_CAPACITY 512

/* Batas usia toleransi paket (TTL) dinaikkan ke 200 ms untuk micro-burst handling */
#define MAX_PACKET_AGE_MS 200

/* Batching EasyDMA dinaikkan ke 24 frame (1008 Byte) */
#define DMA_BATCH_FRAMES 24
#define FRAME_SIZE 42
#define DMA_TX_BUF_SIZE (DMA_BATCH_FRAMES * FRAME_SIZE) /* 1008 Byte */
#define DMA_RX_BUF_SIZE 32
#define DMA_RX_TIMEOUT_US 2000 /* 2 ms inactivity timeout */

/* =================================================================
 * INDIKATOR STATUS LED (PORTABLE RGB / SINGLE LED)
 * - Xiao BLE: Red = P0.26 (led0), Green = P0.30 (led1), Blue = P0.06 (led2)
 * - Pro Micro: Single Red = P0.15 (led0)
 * ================================================================= */
#define HAS_RGB_LEDS DT_NODE_EXISTS(DT_ALIAS(led2))

#if DT_NODE_EXISTS(DT_ALIAS(led0))
static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
#endif

#if DT_NODE_EXISTS(DT_ALIAS(led1))
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
#endif

#if DT_NODE_EXISTS(DT_ALIAS(led2))
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
#endif

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
  uint8_t payload[31]; /* Byte 10..40 : Raw AD Structures (0-padded jika len < 31) */
  uint8_t checksum;    /* Byte 41     : XOR byte 1..40 */
};

BUILD_ASSERT(sizeof(struct RawBleBinaryFrame) == FRAME_SIZE,
             "RawBleBinaryFrame size must be exactly 42 bytes");

/* Struktur Item Antrean FIFO Memori (Offset 8 Aligned untuk Cortex-M4 LDMIA) */
struct __aligned(4) QueuedBleFrame {
  uint32_t timestamp;             /* Offset 0..3 (4 Byte) */
  uint8_t _align_pad[3];          /* Offset 4..6 (3 Byte Padding Penyelaras) */
  struct RawBleBinaryFrame frame; /* Offset 7..48 (42 Byte) */
                                  /* Offset 7: header (0xAA) */
                                  /* Offset 8..47: mac[0]..payload[30] (100% 4-Byte Aligned!) */
                                  /* Offset 48: checksum */
  uint8_t _tail_pad[3];           /* Offset 49..51 (Total 52 Byte, kelipatan 4) */
};

BUILD_ASSERT(sizeof(struct QueuedBleFrame) == 52,
             "QueuedBleFrame size must be exactly 52 bytes");

/* Antrean Pesan Zero-Copy (k_msgq) */
K_MSGQ_DEFINE(beacon_msgq, sizeof(struct QueuedBleFrame), QUEUE_CAPACITY, 4);

/* Metrik Kinerja (Atomic untuk Thread-Safety) */
static atomic_t total_received = ATOMIC_INIT(0);
static atomic_t total_sent = ATOMIC_INIT(0);
static atomic_t total_dropped = ATOMIC_INIT(0);
static atomic_t total_expired = ATOMIC_INIT(0);
static atomic_t tx_thread_heartbeat = ATOMIC_INIT(0);

/* Status Operasional Radio BLE Saat Ini */
static volatile uint8_t current_scan_state = SCAN_STATE_STOPPED;

/* Struktur Frame Perintah dari ESP32 (5 Byte) */
struct nrf_cmd_frame {
  uint8_t raw[5];
};

/* Message queue untuk menampung perintah dari ISR ke thread worker */
K_MSGQ_DEFINE(cmd_msgq, sizeof(struct nrf_cmd_frame), 8, 4);

/* =================================================================
 * DEFINISI BUFFER SRAM EASYDMA & SINGLE-OWNER TX SIGNALS (v3.0)
 * ================================================================= */
/* Wajib di SRAM & 4-byte aligned untuk nRF52840 EasyDMA */
static uint8_t __aligned(4) dma_tx_buf_ping[DMA_TX_BUF_SIZE];
static uint8_t __aligned(4) dma_tx_buf_pong[DMA_TX_BUF_SIZE];
static uint8_t __aligned(4) dma_rsp_buf[4];

static uint8_t __aligned(4) dma_rx_buf[2][DMA_RX_BUF_SIZE];
static uint8_t rx_next_buf_idx = 1;

/* Gatekeeper Semaphore Tunggal untuk TX DMA (Initial = 1, Limit = 1) */
static K_SEM_DEFINE(dma_tx_sem, 1, 1);

/* Semaphore pemicu thread TX (Binary Event Flag: 0 atau 1 untuk eliminasi ghost wakeups) */
static K_SEM_DEFINE(tx_trigger_sem, 0, 1);

/* Flag atomik untuk balasan kontrol (Single-Owner DMA TX, 0ms blocking di Workqueue) */
static atomic_t has_pending_ctrl_rsp = ATOMIC_INIT(0);

/* State-Machine Sliding Window Parser RX */
static uint8_t rx_cmd_buf[5];
static uint8_t rx_cmd_idx = 0;
static uint32_t last_cmd_rx_time = 0;

/* Work item untuk rekonfigurasi Bluetooth di System Work Queue thread */
static struct k_work scan_reconfig_work;

/* =================================================================
 * 3. FAST CHECKUM 32-BIT PARALLEL FOLDING (100% 4-BYTE ALIGNED)
 * ================================================================= */
static inline uint8_t compute_xor_40bytes(const uint8_t *aligned_mac_ptr) {
  const uint32_t *w = (const uint32_t *)aligned_mac_ptr;
  uint32_t x = w[0] ^ w[1] ^ w[2] ^ w[3] ^ w[4] ^
               w[5] ^ w[6] ^ w[7] ^ w[8] ^ w[9];
  x ^= (x >> 16);
  x ^= (x >> 8);
  return (uint8_t)(x & 0xFF);
}

/* =================================================================
 * 4. BLE ADVERTISING CALLBACK (ZERO-OVERHEAD PROMISCUOUS CAPTURE)
 * Ditempatkan di Code RAM (__ramfunc) untuk eksekusi 0 Wait-State
 * ================================================================= */
static __ramfunc void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                                   struct net_buf_simple *ad) {
  if (current_scan_state == SCAN_STATE_STOPPED) {
    return;
  }

  struct QueuedBleFrame q;
  q.timestamp = k_uptime_get_32();
  q._align_pad[0] = 0;
  q._align_pad[1] = 0;
  q._align_pad[2] = 0;

  q.frame.header = 0xAA;
  /* Format Big-Endian MAC (MSB ke LSB) - unrolled direct assignment */
  q.frame.mac[0] = addr->a.val[5];
  q.frame.mac[1] = addr->a.val[4];
  q.frame.mac[2] = addr->a.val[3];
  q.frame.mac[3] = addr->a.val[2];
  q.frame.mac[4] = addr->a.val[1];
  q.frame.mac[5] = addr->a.val[0];

  q.frame.adv_type = type;
  q.frame.rssi = rssi;

  uint8_t len = (ad != NULL && ad->len <= 31) ? (uint8_t)ad->len : (ad != NULL ? 31 : 0);
  q.frame.payload_len = len;

  if (len > 0) {
    memcpy(q.frame.payload, ad->data, len);
  }
  if (len < 31) {
    memset(&q.frame.payload[len], 0, 31 - len);
  }

  /* Checksum 32-bit single-cycle load dari Offset 8 (100% 4-Byte Aligned) */
  q.frame.checksum = compute_xor_40bytes(&q.frame.mac[0]);

  /* Dorong ke antrean memori FIFO dengan Drop-Oldest failsafe */
  atomic_inc(&total_received);
  if (k_msgq_put(&beacon_msgq, &q, K_NO_WAIT) != 0) {
    struct QueuedBleFrame dummy;
    k_msgq_get(&beacon_msgq, &dummy, K_NO_WAIT);
    atomic_inc(&total_dropped);
    k_msgq_put(&beacon_msgq, &q, K_NO_WAIT);
  }

  /* Berikan sinyal ke thread TX */
  k_sem_give(&tx_trigger_sem);
}

/* =================================================================
 * UNIFIED ASYNC UART CALLBACK (TX + RX + SELF-HEALING)
 * ================================================================= */
static void uart_async_callback(const struct device *dev,
                                struct uart_event *evt,
                                void *user_data) {
  switch (evt->type) {

  /* --- 1. TX SELESAI ATAU DIBATALKAN: Buka kunci Gatekeeper --- */
  case UART_TX_DONE:
  case UART_TX_ABORTED:
    k_sem_give(&dma_tx_sem);
    break;

  /* --- 2. DATA RX MASUK: Proses dengan Sliding Window Parser --- */
  case UART_RX_RDY: {
    const uint8_t *data = &evt->data.rx.buf[evt->data.rx.offset];
    size_t len = evt->data.rx.len;

    uint32_t now = k_uptime_get_32();
    if (rx_cmd_idx > 0 && (now - last_cmd_rx_time > 10)) {
      rx_cmd_idx = 0; /* Reset jika jeda antar-byte dalam 1 paket > 10ms */
    }
    last_cmd_rx_time = now;

    for (size_t i = 0; i < len; i++) {
      uint8_t ch = data[i];
      if (rx_cmd_idx == 0) {
        if (ch == CMD_HEADER_BYTE) { /* 0x55 */
          rx_cmd_buf[rx_cmd_idx++] = ch;
        }
      } else {
        rx_cmd_buf[rx_cmd_idx++] = ch;
        if (rx_cmd_idx == 5) {
          uint8_t expected_crc = rx_cmd_buf[1] ^ rx_cmd_buf[2] ^ rx_cmd_buf[3];
          if (rx_cmd_buf[4] == expected_crc) {
            struct nrf_cmd_frame cmd;
            memcpy(cmd.raw, rx_cmd_buf, 5);
            k_msgq_put(&cmd_msgq, &cmd, K_NO_WAIT);
            k_work_submit(&scan_reconfig_work);
          }
          rx_cmd_idx = 0;
        }
      }
    }
    break;
  }

  /* --- 3. PERMINTAAN BUFFER RX BARU (Double Buffering Seamless) --- */
  case UART_RX_BUF_REQUEST:
    uart_rx_buf_rsp(dev, dma_rx_buf[rx_next_buf_idx], DMA_RX_BUF_SIZE);
    rx_next_buf_idx ^= 1; /* Toggle 0 <-> 1 */
    break;

  /* --- 4. ERROR LINE / GLITCH: Reset Parser State --- */
  case UART_RX_STOPPED:
    rx_cmd_idx = 0;
    break;

  /* --- 5. HARDWARE RX IDLE: Self-Healing Re-Enable --- */
  case UART_RX_DISABLED:
    rx_cmd_idx = 0;
    rx_next_buf_idx = 1;
    uart_rx_enable(dev, dma_rx_buf[0], DMA_RX_BUF_SIZE, DMA_RX_TIMEOUT_US);
    break;

  default:
    break;
  }
}

/* =================================================================
 * BALASAN KONTROL NON-BLOCKING (SINGLE-OWNER DMA TX ARCHITECTURE)
 * ================================================================= */
static void send_control_response(uint8_t rsp_type, uint8_t scan_state) {
  dma_rsp_buf[0] = RSP_HEADER_BYTE;
  dma_rsp_buf[1] = rsp_type;
  dma_rsp_buf[2] = scan_state;
  dma_rsp_buf[3] = rsp_type ^ scan_state;

  atomic_set(&has_pending_ctrl_rsp, 1);
  k_sem_give(&tx_trigger_sem);
}

/* =================================================================
 * 5. THREAD TRANSMISI SERIAL TUNGGAL (SINGLE-OWNER DMA TX)
 * ================================================================= */
static void esp32_tx_thread_entry(void *p1, void *p2, void *p3) {
  struct QueuedBleFrame q;
  uint8_t *cur_buf = dma_tx_buf_ping;

  while (1) {
    atomic_inc(&tx_thread_heartbeat);
    /* 1. Tunggu sinyal paket atau sinyal kontrol (maks 2 ms jika antrean kosong) */
    if (k_msgq_num_used_get(&beacon_msgq) == 0 && !atomic_get(&has_pending_ctrl_rsp)) {
      k_sem_take(&tx_trigger_sem, K_MSEC(2));
    }

    /* 2. Prioritas Tertinggi: Kirim Respon Kontrol (ACK/STATUS/NACK) jika ada */
    if (atomic_get(&has_pending_ctrl_rsp)) {
      if (k_sem_take(&dma_tx_sem, K_MSEC(25)) == 0) {
        atomic_set(&has_pending_ctrl_rsp, 0);
        int err = uart_tx(esp32_uart, dma_rsp_buf, 4, SYS_FOREVER_US);
        if (err != 0) {
          k_sem_give(&dma_tx_sem);
        }
      }
    }

    /* 3. Kuras Paket BLE dari Antrean (Zero-Reassembly Direct Copy) */
    uint32_t now = k_uptime_get_32();
    int frame_count = 0;
    while (frame_count < DMA_BATCH_FRAMES &&
           k_msgq_get(&beacon_msgq, &q, K_NO_WAIT) == 0) {
      if ((now - q.timestamp) <= MAX_PACKET_AGE_MS) {
        memcpy(&cur_buf[frame_count * FRAME_SIZE], &q.frame, FRAME_SIZE);
        frame_count++;
      } else {
        atomic_inc(&total_expired);
      }
    }

    /* 4. Luncurkan Transmisi EasyDMA Batch jika ada paket valid */
    if (frame_count > 0) {
      if (k_sem_take(&dma_tx_sem, K_MSEC(25)) != 0) {
        uart_tx_abort(esp32_uart);
        k_sem_take(&dma_tx_sem, K_MSEC(5));
      }

      /* Top-Up: gabungkan paket baru yang masuk selama menunggu DMA sebelumnya selesai */
      if (frame_count < DMA_BATCH_FRAMES) {
        now = k_uptime_get_32();
        while (frame_count < DMA_BATCH_FRAMES &&
               k_msgq_get(&beacon_msgq, &q, K_NO_WAIT) == 0) {
          if ((now - q.timestamp) <= MAX_PACKET_AGE_MS) {
            memcpy(&cur_buf[frame_count * FRAME_SIZE], &q.frame, FRAME_SIZE);
            frame_count++;
          } else {
            atomic_inc(&total_expired);
          }
        }
      }

      size_t total_bytes = frame_count * FRAME_SIZE;
      int err = uart_tx(esp32_uart, cur_buf, total_bytes, SYS_FOREVER_US);
      if (err == 0) {
        atomic_add(&total_sent, frame_count);
        cur_buf = (cur_buf == dma_tx_buf_ping) ? dma_tx_buf_pong : dma_tx_buf_ping;
      } else {
        k_sem_give(&dma_tx_sem);
      }
    }
  }
}
K_THREAD_DEFINE(esp32_tx_tid, 2048, esp32_tx_thread_entry, NULL, NULL, NULL,
                K_PRIO_PREEMPT(4), 0, 0);

/* =================================================================
 * 6. PROTOKOL KONTROL 2-ARAH (RX ISR & WORK QUEUE)
 * ================================================================= */
static void scan_reconfig_work_handler(struct k_work *work) {
  struct nrf_cmd_frame cmd;

  while (k_msgq_get(&cmd_msgq, &cmd, K_NO_WAIT) == 0) {
    uint8_t cmd_id = cmd.raw[1];
    uint8_t param  = cmd.raw[2];

    switch (cmd_id) {
    case CMD_SET_SCAN_MODE:
      if (param == SCAN_STATE_STOPPED) {
        if (current_scan_state == SCAN_STATE_STOPPED) {
          send_control_response(RSP_TYPE_ACK, current_scan_state);
        } else {
          int err = bt_le_scan_stop();
          if (err == 0 || err == -EALREADY) {
            current_scan_state = SCAN_STATE_STOPPED;
            k_msgq_purge(&beacon_msgq); /* Purge antrean seketika - 0 paket hantu! */
            send_control_response(RSP_TYPE_ACK, current_scan_state);
          } else {
            current_scan_state = SCAN_STATE_ERROR;
            send_control_response(RSP_TYPE_NACK, current_scan_state);
          }
        }
      } else if (param == SCAN_STATE_PASSIVE) {
        if (current_scan_state == SCAN_STATE_PASSIVE) {
          send_control_response(RSP_TYPE_ACK, current_scan_state);
        } else {
          if (current_scan_state != SCAN_STATE_STOPPED) {
            bt_le_scan_stop();
          }
          struct bt_le_scan_param scan_param = {
              .type = BT_LE_SCAN_TYPE_PASSIVE,
              .options = BT_LE_SCAN_OPT_NONE,
              .interval = PRIME_PASSIVE_INTERVAL, /* 29 unit = 18.125 ms (55.17 hop/s) */
              .window = PRIME_PASSIVE_WINDOW,     /* 29 unit = 18.125 ms (100% Duty Cycle) */
              .timeout = 0,
              .interval_coded = 0,
              .window_coded = 0,
          };
          int err = bt_le_scan_start(&scan_param, device_found);
          if (err == 0) {
            current_scan_state = SCAN_STATE_PASSIVE;
            send_control_response(RSP_TYPE_ACK, current_scan_state);
          } else {
            current_scan_state = SCAN_STATE_ERROR;
            send_control_response(RSP_TYPE_NACK, current_scan_state);
          }
        }
      } else if (param == SCAN_STATE_ACTIVE) {
        if (current_scan_state == SCAN_STATE_ACTIVE) {
          send_control_response(RSP_TYPE_ACK, current_scan_state);
        } else {
          if (current_scan_state != SCAN_STATE_STOPPED) {
            bt_le_scan_stop();
          }
          struct bt_le_scan_param scan_param = {
              .type = BT_LE_SCAN_TYPE_ACTIVE,
              .options = BT_LE_SCAN_OPT_NONE,
              .interval = PRIME_ACTIVE_INTERVAL, /* 37 unit = 23.125 ms (43.24 hop/s) */
              .window = PRIME_ACTIVE_WINDOW,     /* 37 unit = 23.125 ms (100% Duty Cycle) */
              .timeout = 0,
              .interval_coded = 0,
              .window_coded = 0,
          };
          int err = bt_le_scan_start(&scan_param, device_found);
          if (err == 0) {
            current_scan_state = SCAN_STATE_ACTIVE;
            send_control_response(RSP_TYPE_ACK, current_scan_state);
          } else {
            current_scan_state = SCAN_STATE_ERROR;
            send_control_response(RSP_TYPE_NACK, current_scan_state);
          }
        }
      } else {
        /* Parameter tidak valid */
        send_control_response(RSP_TYPE_NACK, current_scan_state);
      }
      break;

    case CMD_GET_STATUS:
      send_control_response(RSP_TYPE_STATUS, current_scan_state);
      break;

    case CMD_SOFT_REBOOT:
      if (param == REBOOT_MAGIC_PARAM) {
        send_control_response(RSP_TYPE_ACK, current_scan_state);
        k_msleep(20);
        sys_reboot(SYS_REBOOT_COLD);
      } else {
        send_control_response(RSP_TYPE_NACK, current_scan_state);
      }
      break;

    default:
      send_control_response(RSP_TYPE_NACK, current_scan_state);
      break;
    }
  }
}

/* =================================================================
 * 7. THREAD DEBUG & STATS MONITOR (USB CDC ACM) - LOW PRIORITY
 * Format angka statistik menggunakan Fixed-Point Integer Math (0 emulasi float)
 * ================================================================= */
static void format_count(uint32_t val, char *buf, size_t sz) {
  if (val >= 1000000) {
    snprintf(buf, sz, "%u.%uM", val / 1000000, (val % 1000000) / 100000);
  } else if (val >= 1000) {
    snprintf(buf, sz, "%u.%uK", val / 1000, (val % 1000) / 100);
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
               "Expired: %u | Dropped: %u | Scan State: 0x%02X <<<\r\n",
               pps, str_in, str_sent, cur_exp, cur_drop, current_scan_state);
      }
    }
  }
}
K_THREAD_DEFINE(debug_stats_tid, 2048, debug_stats_thread_entry, NULL, NULL,
                NULL, K_PRIO_PREEMPT(10), 0, 0);

/* =================================================================
 * 8. INDIKATOR STATUS LED (HEARTBEAT 500MS)
 * ================================================================= */
static void led_heartbeat_tick(void) {
#if HAS_RGB_LEDS
  /* Seeed XIAO BLE: Full RGB Indicator */
  switch (current_scan_state) {
  case SCAN_STATE_STOPPED:
    /* Kedip MERAH: matikan hijau & biru, toggle merah */
    if (gpio_is_ready_dt(&led_green)) {
      gpio_pin_set_dt(&led_green, 0);
    }
    if (gpio_is_ready_dt(&led_blue)) {
      gpio_pin_set_dt(&led_blue, 0);
    }
    if (gpio_is_ready_dt(&led_red)) {
      gpio_pin_toggle_dt(&led_red);
    }
    break;

  case SCAN_STATE_ACTIVE:
    /* Kedip HIJAU: matikan merah & biru, toggle hijau */
    if (gpio_is_ready_dt(&led_red)) {
      gpio_pin_set_dt(&led_red, 0);
    }
    if (gpio_is_ready_dt(&led_blue)) {
      gpio_pin_set_dt(&led_blue, 0);
    }
    if (gpio_is_ready_dt(&led_green)) {
      gpio_pin_toggle_dt(&led_green);
    }
    break;

  case SCAN_STATE_PASSIVE:
    /* Kedip BIRU: matikan merah & hijau, toggle biru */
    if (gpio_is_ready_dt(&led_red)) {
      gpio_pin_set_dt(&led_red, 0);
    }
    if (gpio_is_ready_dt(&led_green)) {
      gpio_pin_set_dt(&led_green, 0);
    }
    if (gpio_is_ready_dt(&led_blue)) {
      gpio_pin_toggle_dt(&led_blue);
    }
    break;

  default:
    /* Error: Strobe Merah */
    if (gpio_is_ready_dt(&led_green)) {
      gpio_pin_set_dt(&led_green, 0);
    }
    if (gpio_is_ready_dt(&led_blue)) {
      gpio_pin_set_dt(&led_blue, 0);
    }
    if (gpio_is_ready_dt(&led_red)) {
      gpio_pin_toggle_dt(&led_red);
    }
    break;
  }
#else
  /* Pro Micro: Single Red LED (led0) */
#if DT_NODE_EXISTS(DT_ALIAS(led0))
  if (gpio_is_ready_dt(&led_red)) {
    gpio_pin_toggle_dt(&led_red);
  }
#endif
#endif
}


/* =================================================================
 * 10. MAIN FUNCTION
 * ================================================================= */
int main(void) {
  int err;

  /* Inisialisasi LED Indikator (Semua mati di awal) */
#if DT_NODE_EXISTS(DT_ALIAS(led0))
  if (gpio_is_ready_dt(&led_red)) {
    gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
  }
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led1))
  if (gpio_is_ready_dt(&led_green)) {
    gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
  }
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led2))
  if (gpio_is_ready_dt(&led_blue)) {
    gpio_pin_configure_dt(&led_blue, GPIO_OUTPUT_INACTIVE);
  }
#endif


  /* Inisialisasi System Work Item untuk rekonfigurasi Bluetooth */
  k_work_init(&scan_reconfig_work, scan_reconfig_work_handler);

  /* Inisialisasi UART0 ke ESP32 */
  if (!device_is_ready(esp32_uart)) {
    return 0;
  }

  /* Inisialisasi UART0 Async EasyDMA (Callback terpadu TX + RX + Self-Healing) */
  err = uart_callback_set(esp32_uart, uart_async_callback, NULL);
  if (err) {
    printf("[UART] Setup Async Callback GAGAL (err %d)\r\n", err);
  }

  /* Aktifkan Continuous RX DMA (Buffer 32B ganda, Timeout 2ms) */
  rx_next_buf_idx = 1;
  err = uart_rx_enable(esp32_uart, dma_rx_buf[0], DMA_RX_BUF_SIZE, DMA_RX_TIMEOUT_US);
  if (err) {
    printf("[UART] Aktifkan RX DMA GAGAL (err %d)\r\n", err);
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
  printf("[UART] UARTE aktif di 1.000.000 bps menuju ESP32 (Full-Duplex Control Ready)\r\n");

  /* Inisialisasi Subsystem Bluetooth Low Energy */
  err = bt_enable(NULL);
  if (err) {
    printf("[BLE] Inisialisasi Bluetooth GAGAL (err %d)\r\n", err);
    current_scan_state = SCAN_STATE_ERROR;
    return 0;
  }
  printf("[BLE] Bluetooth Controller aktif & siap.\r\n");

  /* Status Awal: STOPPED (Menunggu perintah mulai dari ESP32) */
  current_scan_state = SCAN_STATE_STOPPED;
  printf("[BLE] Radio siaga dalam mode STOPPED (Menunggu perintah ESP32).\r\n");
  printf("----------------------------------------------------------------\r\n");

  /* Loop Utama: Heartbeat LED berkedip setiap 500ms & Feed WDT */
  uint32_t last_tx_hb = (uint32_t)atomic_get(&tx_thread_heartbeat);
  uint32_t tx_stall_ticks = 0;
  while (1) {
    uint32_t cur_tx_hb = (uint32_t)atomic_get(&tx_thread_heartbeat);
    if (cur_tx_hb != last_tx_hb) {
      last_tx_hb = cur_tx_hb;
      tx_stall_ticks = 0;
    } else {
      tx_stall_ticks++;
    }

    if (wdt_channel_id >= 0 && tx_stall_ticks < 6) {
      wdt_feed(wdt_dev, wdt_channel_id);
    }
    led_heartbeat_tick();
    k_sleep(K_MSEC(500));
  }

  return 0;
}
