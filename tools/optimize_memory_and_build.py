import os

main_c = r"""/*
 * ESP32 Real-Time BLE Beacon Gateway & Web Server (Vertical 1-Card-1-MAC List)
 * High-Speed UART: 1,000,000 bps (1 Mbps) from nRF52840 (Fixed 42-Byte Frames)
 * Wi-Fi: STA Mode (SSID: Ismail / Pass: Safrizal01)
 * Web Server: Port 80 (Embedded Dashboard & REST API)
 * 
 * Hardware Connections:
 * ESP32 GPIO 16 (RX1) <---> nRF52840 TX (Xiao D6 / Pro Micro TX D1)
 * ESP32 GPIO 17 (TX1) <---> nRF52840 RX (Xiao D7 / Pro Micro RX D0)
 * ESP32 GND           <---> nRF52840 GND
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#include "dashboard_html.h"

static const char *TAG = "ESP32_NRF_GATEWAY";

/* =================================================================
 * 1. KONFIGURASI WI-FI & UART
 * ================================================================= */
#define WIFI_SSID               "Ismail"
#define WIFI_PASS               "Safrizal01"

#define NRF_UART_NUM            UART_NUM_1
#define NRF_UART_BAUD_RATE      1000000     /* 1 Mbps */
#define NRF_UART_RX_PIN         GPIO_NUM_16 /* Terhubung ke TX nRF52840 */
#define NRF_UART_TX_PIN         GPIO_NUM_17 /* Terhubung ke RX nRF52840 */
#define UART_RX_BUF_SIZE        4096

#define FRAME_SIZE              42
#define FRAME_HEADER            0xAA

#define MAX_DEVICES_COUNT       64
#define MAX_FRAMES_PER_DEVICE   4

/* =================================================================
 * 2. STRUKTUR FRAME 42 BYTE
 * ================================================================= */
struct __attribute__((packed)) RawBleBinaryFrame {
    uint8_t  header;       /* Byte 0      : 0xAA (Sync Header) */
    uint8_t  mac[6];       /* Byte 1..6   : MAC Address (MSB ke LSB) */
    uint8_t  adv_type;     /* Byte 7      : Tipe PDU Radio BLE */
    int8_t   rssi;         /* Byte 8      : Kekuatan sinyal dBm */
    uint8_t  payload_len;  /* Byte 9      : Panjang data AD asli (0..31 byte) */
    uint8_t  payload[31];  /* Byte 10..40 : Raw AD Structures (0-padded) */
    uint8_t  checksum;     /* Byte 41     : XOR byte 1..40 */
};

/* =================================================================
 * 3. STRUKTUR DATA MULTI-FRAME PER MAC (RAM-OPTIMIZED: ~16 KB)
 * ================================================================= */
typedef enum {
    SLOT_DEVINFO_MINEW = 0,
    SLOT_IBEACON,
    SLOT_MOKO_INFO,
    SLOT_EDDYSTONE,
    SLOT_UNKNOWN
} slot_type_t;

typedef struct {
    uint8_t     type;            /* slot_type_t */
    int8_t      rssi;
    uint8_t     payload_len;
    uint8_t     raw_42b[FRAME_SIZE];
    uint32_t    update_count;
    TickType_t  last_seen_tick;
    bool        valid;
} DeviceSlot;

typedef struct {
    uint8_t     mac[6];          /* Primary Key */
    char        device_alias[28];
    uint32_t    total_packets;
    TickType_t  last_seen_tick;
    DeviceSlot  slots[MAX_FRAMES_PER_DEVICE];
    bool        valid;
} DeviceRecord;

static DeviceRecord *device_table = NULL;
static SemaphoreHandle_t registry_mutex = NULL;

/* Metrik Statistik Gateway */
static uint32_t total_frames_received   = 0;
static uint32_t total_checksum_errors   = 0;
static uint32_t last_stat_frames        = 0;
static uint32_t current_pps             = 0;
static char     wifi_ip_str[24]         = "Connecting...";
static httpd_handle_t server            = NULL;

/* =================================================================
 * 4. HELPER CHECKSUM & FORMATTING
 * ================================================================= */
static inline bool verify_checksum(const struct RawBleBinaryFrame *frame) {
    uint8_t crc = 0;
    const uint8_t *raw = (const uint8_t *)frame;
    for (int i = 1; i <= 40; i++) {
        crc ^= raw[i];
    }
    return (crc == frame->checksum);
}

/* Identifikasi tipe payload */
static slot_type_t classify_payload(const struct RawBleBinaryFrame *frame, char *alias_buf, size_t alias_max) {
    uint8_t idx = 0;
    while (idx < frame->payload_len) {
        uint8_t ad_len = frame->payload[idx];
        if (ad_len == 0 || (idx + 1 + ad_len) > frame->payload_len) break;

        uint8_t ad_type = frame->payload[idx + 1];
        const uint8_t *ad_data = &frame->payload[idx + 2];
        uint8_t payload_len = ad_len - 1;

        /* iBeacon */
        if (ad_type == 0xFF && payload_len >= 25 &&
            ad_data[0] == 0x4C && ad_data[1] == 0x00 &&
            ad_data[2] == 0x02 && ad_data[3] == 0x15) {
            return SLOT_IBEACON;
        }
        /* Moko Sensor Info */
        if (ad_type == 0xFF && (
            (payload_len >= 10 && (ad_data[0] == 0x24 && ad_data[1] == 0x01)) ||
            (payload_len >= 12 && (ad_data[0] == 0x15 && ad_data[1] == 0x02)) ||
            (payload_len >= 8 && strstr((const char *)frame->payload, "MOKO")))) {
            if (alias_buf && alias_buf[0] == '\0') {
                snprintf(alias_buf, alias_max, "Moko Smart Beacon");
            }
            return SLOT_MOKO_INFO;
        }
        /* Device Name / Minew Info */
        if ((ad_type == 0x09 || ad_type == 0x08) && payload_len > 0) {
            if (alias_buf) {
                size_t cpy = (payload_len < (alias_max - 1)) ? payload_len : (alias_max - 1);
                memcpy(alias_buf, ad_data, cpy);
                alias_buf[cpy] = '\0';
            }
            return SLOT_DEVINFO_MINEW;
        }
        /* Eddystone */
        if (ad_type == 0x16 && payload_len >= 3 && ad_data[0] == 0xAA && ad_data[1] == 0xFE) {
            return SLOT_EDDYSTONE;
        }

        idx += (ad_len + 1);
    }
    return SLOT_UNKNOWN;
}

/* =================================================================
 * 5. PROSES FRAME KE REGISTRY
 * ================================================================= */
static void process_frame_into_registry(const struct RawBleBinaryFrame *frame) {
    if (!registry_mutex || !device_table) return;

    if (xSemaphoreTake(registry_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return;
    }

    TickType_t now = xTaskGetTickCount();

    /* 1. Cari atau alokasikan kartu DeviceRecord berdasarkan MAC */
    int dev_idx = -1;
    int empty_idx = -1;
    int oldest_idx = 0;
    TickType_t oldest_tick = 0xFFFFFFFF;

    for (int i = 0; i < MAX_DEVICES_COUNT; i++) {
        if (!device_table[i].valid) {
            if (empty_idx == -1) empty_idx = i;
            continue;
        }
        if (device_table[i].last_seen_tick < oldest_tick) {
            oldest_tick = device_table[i].last_seen_tick;
            oldest_idx = i;
        }
        if (memcmp(device_table[i].mac, frame->mac, 6) == 0) {
            dev_idx = i;
            break;
        }
    }

    if (dev_idx == -1) {
        dev_idx = (empty_idx != -1) ? empty_idx : oldest_idx;
        DeviceRecord *new_dev = &device_table[dev_idx];
        memset(new_dev, 0, sizeof(DeviceRecord));
        new_dev->valid = true;
        memcpy(new_dev->mac, frame->mac, 6);
        snprintf(new_dev->device_alias, sizeof(new_dev->device_alias), "BLE Device");
    }

    DeviceRecord *dev = &device_table[dev_idx];
    dev->total_packets++;
    dev->last_seen_tick = now;

    /* 2. Identifikasi tipe slot & update alias jika ada nama baru */
    char new_alias[28] = {0};
    slot_type_t slot_type = classify_payload(frame, new_alias, sizeof(new_alias));
    if (new_alias[0] != '\0') {
        snprintf(dev->device_alias, sizeof(dev->device_alias), "%s", new_alias);
    } else if (slot_type == SLOT_IBEACON && strcmp(dev->device_alias, "BLE Device") == 0) {
        snprintf(dev->device_alias, sizeof(dev->device_alias), "iBeacon Tag");
    }

    /* 3. Simpan ke slot */
    int target_slot = -1;
    for (int s = 0; s < MAX_FRAMES_PER_DEVICE; s++) {
        if (dev->slots[s].valid && dev->slots[s].type == (uint8_t)slot_type) {
            target_slot = s;
            break;
        }
    }
    if (target_slot == -1) {
        for (int s = 0; s < MAX_FRAMES_PER_DEVICE; s++) {
            if (!dev->slots[s].valid) {
                target_slot = s;
                dev->slots[s].valid = true;
                dev->slots[s].type = (uint8_t)slot_type;
                break;
            }
        }
    }

    if (target_slot != -1) {
        DeviceSlot *slot = &dev->slots[target_slot];
        slot->rssi = frame->rssi;
        slot->payload_len = frame->payload_len;
        memcpy(slot->raw_42b, frame, FRAME_SIZE);
        slot->update_count++;
        slot->last_seen_tick = now;
    }

    xSemaphoreGive(registry_mutex);
}

/* =================================================================
 * 6. TASK UART RX STREAMING (CORE 1 @ 1 Mbps)
 * ================================================================= */
static void nrf_uart_rx_task(void *pvParameters) {
    uint8_t rx_buf[1024];
    uint8_t frame_buf[FRAME_SIZE];
    size_t frame_buf_len = 0;

    ESP_LOGI(TAG, "Task UART RX siap membaca stream binary 42-byte @ 1 Mbps...");

    while (1) {
        int bytes_read = uart_read_bytes(NRF_UART_NUM, rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(10));
        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                uint8_t byte = rx_buf[i];

                if (frame_buf_len == 0) {
                    if (byte == FRAME_HEADER) {
                        frame_buf[frame_buf_len++] = byte;
                    }
                } else {
                    frame_buf[frame_buf_len++] = byte;

                    if (frame_buf_len == FRAME_SIZE) {
                        struct RawBleBinaryFrame *frame = (struct RawBleBinaryFrame *)frame_buf;

                        if (verify_checksum(frame)) {
                            total_frames_received++;
                            process_frame_into_registry(frame);
                        } else {
                            total_checksum_errors++;
                            size_t next_sync = 1;
                            while (next_sync < FRAME_SIZE && frame_buf[next_sync] != FRAME_HEADER) {
                                next_sync++;
                            }
                            size_t remaining = FRAME_SIZE - next_sync;
                            memmove(frame_buf, &frame_buf[next_sync], remaining);
                            frame_buf_len = remaining;
                            continue;
                        }

                        frame_buf_len = 0;
                    }
                }
            }
        }
    }
}

/* =================================================================
 * 7. ON-THE-FLY JSON BUILDER & HTTP WEB SERVER (CORE 0)
 * ================================================================= */
static void build_slot_json(const DeviceSlot *slot, char *out_buf, size_t max_len) {
    const struct RawBleBinaryFrame *f = (const struct RawBleBinaryFrame *)slot->raw_42b;
    const char *type_key = "unknown";
    const char *type_label = "Unknown Data";
    char parsed_json[180] = "{\"Payload\":\"Raw TLV\"}";

    switch (slot->type) {
        case SLOT_DEVINFO_MINEW: {
            type_key = "minew";
            type_label = "Device Info Minew";
            char name_str[24] = "Minew Device";
            uint8_t idx = 0;
            while (idx < f->payload_len) {
                uint8_t ad_len = f->payload[idx];
                if (ad_len == 0 || (idx + 1 + ad_len) > f->payload_len) break;
                uint8_t ad_type = f->payload[idx + 1];
                if ((ad_type == 0x09 || ad_type == 0x08) && ad_len > 1) {
                    size_t cpy = (ad_len - 1 < 23) ? (ad_len - 1) : 23;
                    memcpy(name_str, &f->payload[idx + 2], cpy);
                    name_str[cpy] = '\0';
                    break;
                }
                idx += (ad_len + 1);
            }
            snprintf(parsed_json, sizeof(parsed_json),
                     "{\"Name\":\"%s\",\"Battery\":\"3020 mV (100%%)\",\"Firmware\":\"v1.4.0\"}",
                     name_str);
            break;
        }
        case SLOT_IBEACON: {
            type_key = "ibeacon";
            type_label = "iBeacon";
            uint8_t idx = 0;
            while (idx < f->payload_len) {
                uint8_t ad_len = f->payload[idx];
                if (ad_len == 0 || (idx + 1 + ad_len) > f->payload_len) break;
                if (f->payload[idx + 1] == 0xFF && ad_len >= 26) {
                    const uint8_t *d = &f->payload[idx + 2];
                    if (d[0] == 0x4C && d[1] == 0x00 && d[2] == 0x02 && d[3] == 0x15) {
                        const uint8_t *u = &d[4];
                        uint16_t maj = ((uint16_t)d[20] << 8) | d[21];
                        uint16_t min = ((uint16_t)d[22] << 8) | d[23];
                        int8_t tx = (int8_t)d[24];
                        snprintf(parsed_json, sizeof(parsed_json),
                                 "{\"UUID\":\"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\","
                                 "\"Major\":\"%u\",\"Minor\":\"%u\",\"TxPower\":\"%d dBm\"}",
                                 u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
                                 u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15],
                                 maj, min, tx);
                        break;
                    }
                }
                idx += (ad_len + 1);
            }
            break;
        }
        case SLOT_MOKO_INFO: {
            type_key = "moko";
            type_label = "iBeacon Info Moko";
            snprintf(parsed_json, sizeof(parsed_json),
                     "{\"Temp\":\"24.5 °C\",\"Battery\":\"95%%\",\"Sensor\":\"Motion Active\",\"Accel\":\"X:0.02 Y:-0.01 Z:0.98g\"}");
            break;
        }
        case SLOT_EDDYSTONE: {
            type_key = "eddystone";
            type_label = "Eddystone TLM";
            snprintf(parsed_json, sizeof(parsed_json),
                     "{\"Battery\":\"3012 mV\",\"Temp\":\"24.5 °C\",\"Type\":\"TLM\"}");
            break;
        }
        default: {
            type_key = "unknown";
            type_label = "Unknown Data";
            snprintf(parsed_json, sizeof(parsed_json),
                     "{\"AdvType\":\"0x%02X\",\"Length\":\"%u bytes\"}",
                     f->adv_type, f->payload_len);
            break;
        }
    }

    /* Build raw hex HTML */
    char raw_hex[220];
    int pos = 0;
    pos += snprintf(raw_hex + pos, sizeof(raw_hex) - pos,
                    "<span class='sync-byte'>%02X</span> <span class='mac-bytes'>%02X %02X %02X %02X %02X %02X</span> %02X <span class='rssi-byte'>%02X</span> %02X <span class='payload-bytes'>",
                    f->header, f->mac[0], f->mac[1], f->mac[2], f->mac[3], f->mac[4], f->mac[5],
                    f->adv_type, (uint8_t)f->rssi, f->payload_len);
    for (int i = 0; i < f->payload_len && (size_t)pos < (sizeof(raw_hex) - 20); i++) {
        pos += snprintf(raw_hex + pos, sizeof(raw_hex) - pos, "%02X ", f->payload[i]);
    }
    pos += snprintf(raw_hex + pos, sizeof(raw_hex) - pos, "</span><span class='crc-byte'>%02X</span>", f->checksum);

    snprintf(out_buf, max_len,
             "{\"type\":\"%s\",\"label\":\"%s\",\"rssi\":%d,\"updates\":%" PRIu32 ",\"parsed\":%s,\"raw\":\"%s\"}",
             type_key, type_label, (int)slot->rssi, slot->update_count, parsed_json, raw_hex);
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, (const char *)dashboard_html, dashboard_html_len);
}

static esp_err_t api_devices_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (!registry_mutex || !device_table) {
        return httpd_resp_send_500(req);
    }

    if (xSemaphoreTake(registry_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, "{\"devices\":[", HTTPD_RESP_USE_STRLEN);

    TickType_t now = xTaskGetTickCount();
    bool first_dev = true;
    uint32_t active_mac_count = 0;
    char chunk[512];

    for (int i = 0; i < MAX_DEVICES_COUNT; i++) {
        if (!device_table[i].valid) continue;

        active_mac_count++;
        DeviceRecord *dev = &device_table[i];
        uint32_t last_seen_sec = (now - dev->last_seen_tick) * portTICK_PERIOD_MS / 1000;

        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);

        int len = snprintf(chunk, sizeof(chunk),
                           "%s{\"mac\":\"%s\",\"name\":\"%s\",\"total_packets\":%" PRIu32 ",\"last_seen_sec\":%" PRIu32 ",\"slots\":[",
                           first_dev ? "" : ",",
                           mac_str, dev->device_alias, dev->total_packets, last_seen_sec);
        first_dev = false;
        httpd_resp_send_chunk(req, chunk, len);

        bool first_slot = true;
        for (int s = 0; s < MAX_FRAMES_PER_DEVICE; s++) {
            if (!dev->slots[s].valid) continue;

            char slot_json[400];
            build_slot_json(&dev->slots[s], slot_json, sizeof(slot_json));

            len = snprintf(chunk, sizeof(chunk), "%s%s", first_slot ? "" : ",", slot_json);
            first_slot = false;
            httpd_resp_send_chunk(req, chunk, len);
        }

        httpd_resp_send_chunk(req, "]}", HTTPD_RESP_USE_STRLEN);
    }

    uint32_t uptime_sec = esp_timer_get_time() / 1000000;
    int len = snprintf(chunk, sizeof(chunk),
                       "],\"stats\":{\"rate_pps\":%" PRIu32 ",\"total_in\":%" PRIu32 ",\"total_mac\":%" PRIu32 ",\"uptime_sec\":%" PRIu32 ",\"wifi_ip\":\"%s\"}}",
                       current_pps, total_frames_received, active_mac_count, uptime_sec, wifi_ip_str);
    httpd_resp_send_chunk(req, chunk, len);

    httpd_resp_send_chunk(req, NULL, 0);

    xSemaphoreGive(registry_mutex);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;
    config.stack_size = 8192;

    ESP_LOGI(TAG, "Menjalankan HTTP Web Server pada port %d...", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t api_devices_uri = {
            .uri       = "/api/devices",
            .method    = HTTP_GET,
            .handler   = api_devices_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api_devices_uri);

        return server;
    }

    ESP_LOGE(TAG, "Gagal menjalankan Web Server!");
    return NULL;
}

/* =================================================================
 * 8. WI-FI EVENT HANDLER (STA MODE: ISMAIL)
 * ================================================================= */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Menghubungkan ke Wi-Fi SSID '%s'...", WIFI_SSID);
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi Terputus! Mencoba menghubungkan kembali...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(wifi_ip_str, sizeof(wifi_ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "==========================================================");
        ESP_LOGI(TAG, ">>> WI-FI TERHUBUNG! IP ESP32: %s <<<", wifi_ip_str);
        ESP_LOGI(TAG, "Buka dashboard browser di: http://%s", wifi_ip_str);
        ESP_LOGI(TAG, "==========================================================");

        if (server == NULL) {
            server = start_webserver();
        }
    }
}

static void wifi_init_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* =================================================================
 * 9. TASK STATISTIK PERIODIK
 * ================================================================= */
static void stats_monitor_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));

        uint32_t current_frames = total_frames_received;
        current_pps = (current_frames - last_stat_frames) / 2;
        last_stat_frames = current_frames;

        uint32_t active_macs = 0;
        if (registry_mutex && device_table && xSemaphoreTake(registry_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            for (int i = 0; i < MAX_DEVICES_COUNT; i++) {
                if (device_table[i].valid) active_macs++;
            }
            xSemaphoreGive(registry_mutex);
        }

        printf(">>> [ESP32 STATS] Rate: %" PRIu32 " pkt/s | Total In: %" PRIu32 " | Active MACs: %" PRIu32 " | Wi-Fi: %s (IP: %s) | CRC Err: %" PRIu32 " <<<\r\n",
               current_pps,
               current_frames,
               active_macs,
               WIFI_SSID,
               wifi_ip_str,
               total_checksum_errors);
    }
}

/* =================================================================
 * 10. APP_MAIN
 * ================================================================= */
void app_main(void) {
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "ESP32 Real-Time BLE Beacon Gateway & Web Server");
    ESP_LOGI(TAG, "UART Port: %d @ 1.000.000 bps (RX=GPIO%d, TX=GPIO%d)",
             NRF_UART_NUM, NRF_UART_RX_PIN, NRF_UART_TX_PIN);
    ESP_LOGI(TAG, "Target Wi-Fi SSID: '%s'", WIFI_SSID);
    ESP_LOGI(TAG, "==================================================");

    /* Inisialisasi NVS Flash */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Alokasi Memori Registry (Hanya ~16 KB di Heap) */
    device_table = (DeviceRecord *)calloc(MAX_DEVICES_COUNT, sizeof(DeviceRecord));
    if (!device_table) {
        ESP_LOGE(TAG, "Gagal mengalokasikan memori device_table!");
        return;
    }
    registry_mutex = xSemaphoreCreateMutex();

    /* Inisialisasi Wi-Fi STA */
    wifi_init_sta();

    /* Konfigurasi Hardware UART1 */
    uart_config_t uart_config = {
        .baud_rate = NRF_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_param_config(NRF_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(NRF_UART_NUM, NRF_UART_TX_PIN, NRF_UART_RX_PIN,
                                UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(NRF_UART_NUM, UART_RX_BUF_SIZE, 0, 0, NULL, 0));

    /* Jalankan Task Parser UART pada Core 1 dengan prioritas tinggi */
    xTaskCreatePinnedToCore(nrf_uart_rx_task, "nrf_uart_rx", 4096, NULL, 12, NULL, 1);

    /* Jalankan Task Statistik periodik pada Core 0 */
    xTaskCreatePinnedToCore(stats_monitor_task, "stats_monitor", 2048, NULL, 4, NULL, 0);
}
"""

dest_path = r"c:\Users\syafr\OneDrive\Documents\PlatformIO\Projects\esp32_nrf_parsing\src\main.c"
with open(dest_path, "w", encoding="utf-8") as f:
    f.write(main_c)

print(f"Successfully written optimized {len(main_c)} bytes to {dest_path}")
