/*
 * Copyright (c) 2026
 * nRF52840 Real-time High-Density iBeacon Gateway (Zephyr RTOS)
 * Target: Seeed Studio XIAO BLE / Super52840 & Pro Micro nRF52840
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

/* =================================================================
 * 1. KONFIGURASI SISTEM, BATCHING, & FILTER
 * ================================================================= */
#define DEBUG_MODE          true
#define ENABLE_UUID_FILTER  true
#define FILTER_UUID_STR     "CB10023F-A318-3394-4199-A8730C7C1AEC"

/* Kapasitas antrean dangkal untuk menjamin latensi ultra rendah */
#define QUEUE_CAPACITY      100

/* Batas usia toleransi paket (TTL). Paket > 50 ms langsung didrop (Zero Stale Data) */
#define MAX_PACKET_AGE_MS   50

/* Kapasitas maksimal batching per transmisi */
#define MAX_BATCH_FRAMES    8
#define FRAME_SIZE          25

/* Indikator LED Heartbeat: led1 (Hijau di Xiao BLE / Merah di Pro Micro via alias) */
#define LED_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

/* Hardware UART0 menuju ESP32 @ 1 Mbps */
#define ESP32_UART_NODE DT_NODELABEL(uart0)
static const struct device *const esp32_uart = DEVICE_DT_GET(ESP32_UART_NODE);

/* USB CDC ACM Console untuk Serial Monitor Laptop */
static const struct device *const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/* Hardware Watchdog 0 (Timeout 5 Detik) */
#define WDT_NODE DT_ALIAS(watchdog0)
static const struct device *const wdt_dev = DEVICE_DT_GET(WDT_NODE);
static int wdt_channel_id = -1;

/* =================================================================
 * 2. STRUKTUR DATA PROTOKOL BINER (25 BYTE)
 * ================================================================= */
struct __attribute__((packed)) BeaconBinaryFrame {
	uint8_t  header;    /* Byte 0      : Selalu 0xAA (Sync Byte) */
	uint8_t  code;      /* Byte 1      : 0x00 (iBeacon) atau 0x01 (Minew B) */
	uint8_t  uuid[16];  /* Byte 2..17  : 16-byte raw Proximity UUID */
	uint16_t major;     /* Byte 18..19 : Major ID (Big-Endian) */
	uint16_t minor;     /* Byte 20..21 : Minor ID (Big-Endian) */
	int8_t   rssi;      /* Byte 22     : Kekuatan sinyal dBm */
	uint8_t  battery;   /* Byte 23     : Baterai % (0 jika iBeacon) */
	uint8_t  checksum;  /* Byte 24     : XOR byte 1..23 */
};

/* Tipe Beacon Internal */
enum BeaconType {
	BEACON_TYPE_IBEACON = 0,
	BEACON_TYPE_MINEW_B = 1,
};

/* Struktur Item Antrean FIFO Memori */
struct BeaconPacket {
	uint32_t timestamp; /* Waktu kedatangan paket (k_uptime_get_32) */
	uint8_t  type;
	uint8_t  mac[6];
	int8_t   rssi;
	uint8_t  uuid[16];
	uint16_t major;
	uint16_t minor;
	int8_t   tx_power;
	uint8_t  battery;
	bool     has_battery;
};

/* Antrean Pesan Zero-Copy (k_msgq) */
K_MSGQ_DEFINE(beacon_msgq, sizeof(struct BeaconPacket), QUEUE_CAPACITY, 4);

/* Metrik Kinerja (Atomic untuk Thread-Safety) */
static atomic_t total_received = ATOMIC_INIT(0);
static atomic_t total_sent     = ATOMIC_INIT(0);
static atomic_t total_dropped  = ATOMIC_INIT(0);
static atomic_t total_expired  = ATOMIC_INIT(0);

/* Buffer UUID Target yang Disimpan dalam Bentuk Biner */
static uint8_t target_uuid_bytes[16];

/* =================================================================
 * 3. HELPER FUNCTIONS (PARSER UUID & QUEUE DROP-OLDEST)
 * ================================================================= */
static int hex_char_to_val(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static bool parse_uuid_string(const char *uuid_str, uint8_t *out_bytes)
{
	int byte_idx = 0;
	int high_nibble = -1;

	while (*uuid_str && byte_idx < 16) {
		char c = *uuid_str++;
		if (c == '-') continue;

		int val = hex_char_to_val(c);
		if (val < 0) return false;

		if (high_nibble < 0) {
			high_nibble = val;
		} else {
			out_bytes[byte_idx++] = (uint8_t)((high_nibble << 4) | val);
			high_nibble = -1;
		}
	}
	return (byte_idx == 16 && high_nibble == -1);
}

static inline bool is_target_uuid(const uint8_t *uuid)
{
#if ENABLE_UUID_FILTER
	return (memcmp(uuid, target_uuid_bytes, 16) == 0);
#else
	return true;
#endif
}

static inline void q_push_drop_oldest(const struct BeaconPacket *pkt)
{
	atomic_inc(&total_received);

	/* Coba masukkan paket ke antrean tanpa blocking */
	if (k_msgq_put(&beacon_msgq, pkt, K_NO_WAIT) != 0) {
		/* Antrean PENUH (100 item): Drop paket terlama (Drop-Oldest FIFO) */
		struct BeaconPacket dummy;
		k_msgq_get(&beacon_msgq, &dummy, K_NO_WAIT);
		atomic_inc(&total_dropped);

		/* Masukkan paket baru */
		k_msgq_put(&beacon_msgq, pkt, K_NO_WAIT);
	}
}

/* =================================================================
 * 4. BLE ADVERTISING PARSER (ZERO-COPY)
 * ================================================================= */
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	const uint8_t *p_data = ad->data;
	const uint16_t data_len = ad->len;
	uint16_t idx = 0;

	while (idx < data_len) {
		uint8_t ad_len = p_data[idx];
		if (ad_len == 0 || (idx + 1 + ad_len) > data_len) {
			break;
		}

		uint8_t ad_type = p_data[idx + 1];
		const uint8_t *ad_payload = &p_data[idx + 2];
		uint8_t payload_len = ad_len - 1;

		/* A. MINEW FORMAT B (AD Type 0x16: Service Data 16-bit UUID + Baterai) */
		if (ad_type == 0x16 && payload_len >= 22) {
			const uint8_t *extracted_uuid = &ad_payload[0];

			if (is_target_uuid(extracted_uuid)) {
				struct BeaconPacket pkt;
				pkt.timestamp = k_uptime_get_32();
				pkt.type = BEACON_TYPE_MINEW_B;
				for (int i = 0; i < 6; i++) {
					pkt.mac[i] = addr->a.val[5 - i];
				}
				pkt.rssi = rssi;
				memcpy(pkt.uuid, extracted_uuid, 16);
				pkt.major = ((uint16_t)ad_payload[16] << 8) | ad_payload[17];
				pkt.minor = ((uint16_t)ad_payload[18] << 8) | ad_payload[19];
				pkt.tx_power = (int8_t)ad_payload[20];
				pkt.battery = ad_payload[21];
				pkt.has_battery = true;

				q_push_drop_oldest(&pkt);
			}
		}
		/* B. APPLE iBEACON (AD Type 0xFF: Manufacturer Specific) */
		else if (ad_type == 0xFF && payload_len >= 25) {
			if (ad_payload[0] == 0x4C && ad_payload[1] == 0x00 &&
			    ad_payload[2] == 0x02 && ad_payload[3] == 0x15) {

				const uint8_t *uuid_ptr = &ad_payload[4];

				if (is_target_uuid(uuid_ptr)) {
					struct BeaconPacket pkt;
					pkt.timestamp = k_uptime_get_32();
					pkt.type = BEACON_TYPE_IBEACON;
					for (int i = 0; i < 6; i++) {
						pkt.mac[i] = addr->a.val[5 - i];
					}
					pkt.rssi = rssi;
					memcpy(pkt.uuid, uuid_ptr, 16);
					pkt.major = ((uint16_t)ad_payload[20] << 8) | ad_payload[21];
					pkt.minor = ((uint16_t)ad_payload[22] << 8) | ad_payload[23];
					pkt.tx_power = (int8_t)ad_payload[24];
					pkt.battery = 0;
					pkt.has_battery = false;

					q_push_drop_oldest(&pkt);
				}
			}
		}

		idx += (ad_len + 1);
	}
}

/* =================================================================
 * 5. ADAPTIVE BATCHING TRANSMITTER KE ESP32 (UART 1 MBPS)
 * ================================================================= */
static inline void assemble_frame(uint8_t *dest, const struct BeaconPacket *pkt)
{
	struct BeaconBinaryFrame *frame = (struct BeaconBinaryFrame *)dest;
	frame->header = 0xAA;
	frame->code = (pkt->type == BEACON_TYPE_MINEW_B) ? 0x01 : 0x00;
	memcpy(frame->uuid, pkt->uuid, 16);
	frame->major = sys_cpu_to_be16(pkt->major);
	frame->minor = sys_cpu_to_be16(pkt->minor);
	frame->rssi = pkt->rssi;
	frame->battery = pkt->has_battery ? pkt->battery : 0;

	/* XOR Checksum: byte 1 ('code') s/d byte 23 ('battery') */
	uint8_t crc = 0;
	const uint8_t *raw = (const uint8_t *)frame;
	for (size_t i = 1; i < FRAME_SIZE - 1; i++) {
		crc ^= raw[i];
	}
	frame->checksum = crc;
}

static void esp32_tx_thread_entry(void *p1, void *p2, void *p3)
{
	struct BeaconPacket pkt;
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
static void format_count(uint32_t val, char *buf, size_t sz)
{
	if (val >= 1000000) {
		snprintf(buf, sz, "%.1fM", (double)val / 1000000.0);
	} else if (val >= 10000) {
		snprintf(buf, sz, "%.1fK", (double)val / 1000.0);
	} else {
		snprintf(buf, sz, "%u", val);
	}
}

static void debug_stats_thread_entry(void *p1, void *p2, void *p3)
{
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
				uint32_t cur_exp  = atomic_get(&total_expired);

				uint32_t delta_ms = now - last_time;
				uint32_t pps = (delta_ms > 0) ? ((cur_recv - last_received) * 1000 / delta_ms) : 0;
				last_received = cur_recv;
				last_time = now;

				format_count(cur_recv, str_in, sizeof(str_in));
				format_count(cur_sent, str_sent, sizeof(str_sent));

				printf(">>> [PERF STATS] Rate: %u pkt/s | In: %s | Sent ESP32: %s | Expired: %u | Dropped: %u <<<\r\n",
				       pps, str_in, str_sent, cur_exp, cur_drop);
			}
		}
	}
}
K_THREAD_DEFINE(debug_stats_tid, 2048, debug_stats_thread_entry, NULL, NULL, NULL,
		K_PRIO_PREEMPT(10), 0, 0);

/* =================================================================
 * 7. MAIN FUNCTION
 * ================================================================= */
int main(void)
{
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
			if (dtr) break;
			k_msleep(100);
		}
	}

	printf("\r\n==================================================\r\n");
	printf(">>> nRF52840 Realtime Beacon Gateway (Zephyr RTOS) <<<\r\n");
	printf("==================================================\r\n");

	/* Periksa Penyebab Reboot Sebelumnya (Hardware Diagnostics) */
	uint32_t reset_cause = 0;
	if (hwinfo_get_reset_cause(&reset_cause) == 0) {
		if (reset_cause & RESET_WATCHDOG) {
			printf("[SYSTEM] PERINGATAN: Sistem pulih dari WATCHDOG TIMEOUT!\r\n");
		} else {
			printf("[SYSTEM] Sistem booting normal (Reset Cause: 0x%08X)\r\n", reset_cause);
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
				printf("[WDT] Hardware Watchdog aktif (Timeout: 5 detik, Auto-Recovery)\r\n");
			} else {
				printf("[WDT] Setup Watchdog GAGAL (err %d)\r\n", err);
			}
		} else {
			printf("[WDT] Install timeout GAGAL (err %d)\r\n", wdt_channel_id);
		}
	} else {
		printf("[WDT] Device Watchdog tidak siap!\r\n");
	}

#if ENABLE_UUID_FILTER
	if (parse_uuid_string(FILTER_UUID_STR, target_uuid_bytes)) {
		printf("[CONFIG] Filter UUID AKTIF: %s\r\n", FILTER_UUID_STR);
	} else {
		printf("[ERROR] Format FILTER_UUID_STR tidak valid!\r\n");
	}
#else
	printf("[CONFIG] Filter UUID NONAKTIF: Mengirim SEMUA beacon!\r\n");
#endif

	printf("[UART] UARTE aktif di 1.000.000 bps menuju ESP32 (D6=TX, D7=RX)\r\n");

	/* Inisialisasi Subsystem Bluetooth Low Energy */
	err = bt_enable(NULL);
	if (err) {
		printf("[BLE] Inisialisasi Bluetooth GAGAL (err %d)\r\n", err);
		return 0;
	}
	printf("[BLE] Bluetooth Controller aktif & siap.\r\n");

	/* Scanning Parameter: 100% Continuous Passive Scanning (Interval = Window = 160 / 100ms) */
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = 160,
		.window     = 160,
		.timeout    = 0,
		.interval_coded = 0,
		.window_coded   = 0,
	};

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printf("[BLE] Memulai scan GAGAL (err %d)\r\n", err);
		return 0;
	}

	printf("[BLE] Continuous Passive Scanner berjalan pada 100%% duty cycle.\r\n");
	printf("----------------------------------------------------------------\r\n");

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
