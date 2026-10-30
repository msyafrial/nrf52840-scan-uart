# Dokumentasi Kode Sumber & Referensi Teknis (`src/main.c`)

Dokumen ini menjelaskan secara rinci seluruh konstanta, struktur data, fungsi, dan arsitektur komunikasi pada firmware **Universal Raw BLE Gateway (Promiscuous Sniffer 42-Byte @ 1 Mbps)** di dalam berkas [src/main.c](file:///c:/Users/syafr/OneDrive/Documents/nrfConnect/Projects/nrf52840_ibeacon/src/main.c).

---

## 1. Daftar Variabel & Konstanta Konfigurasi

Semua parameter operasional utama dikelompokkan di bagian atas berkas `src/main.c` (Bagian 1):

| Nama Konstanta / Macro | Tipe / Nilai Default | Lokasi Baris | Deskripsi & Fungsi | Dampak Modifikasi |
| :--- | :--- | :--- | :--- | :--- |
| **`DEBUG_MODE`** | `true` | Baris ~20 | Mengaktifkan/menonaktifkan cetak log performa ke port USB CDC ACM laptop. | Ubah ke `false` jika board dideploy permanen di lapangan tanpa PC untuk efisiensi daya. |
| **`QUEUE_CAPACITY`** | `100` | Baris ~23 | Kapasitas maksimal slot antrean pesan kernel Zephyr (`k_msgq`). | Menentukan berapa banyak paket mentah yang dapat ditampung di memori RAM saat terjadi *burst*. Pada 1.200 pkt/s, 100 paket mencakup buffer ~83 ms. |
| **`MAX_PACKET_AGE_MS`** | `50` (milidetik) | Baris ~26 | Batas toleransi usia paket (*Time-To-Live / TTL*). | Paket di antrean yang usianya melebihi 50 ms langsung didrop (*Zero Stale Data*) agar ESP32 tidak menerima data usang. |
| **`MAX_BATCH_FRAMES`** | `5` | Baris ~29 | Batas maksimal jumlah frame per semburan transmisi UART. | 5 frame $\times$ 42 byte = 210 byte (sangat aman di bawah buffer FIFO hardware UART 256 byte). |
| **`FRAME_SIZE`** | `42` (byte) | Baris ~30 | Ukuran tetap setiap frame biner protokol komunikasi ke ESP32. | Ukuran baku (fixed-size) 42-byte. |

---

## 2. Struktur Data Protokol Biner Mentah (42 Byte)

### 2.1 `struct __attribute__((packed)) RawBleBinaryFrame`
Struktur memori biner padat tanpa padding byte yang dikirim langsung melalui jalur kabel serial UART ke ESP32:

```c
struct __attribute__((packed)) RawBleBinaryFrame {
    uint8_t  header;       /* Byte 0      : Selalu bernilai 0xAA (Sync Byte) */
    uint8_t  mac[6];       /* Byte 1..6   : MAC Address pengirim (MSB ke LSB) */
    uint8_t  adv_type;     /* Byte 7      : Tipe PDU Radio (ADV_IND, etc.) */
    int8_t   rssi;         /* Byte 8      : Kekuatan sinyal dBm (-128 s.d +127) */
    uint8_t  payload_len;  /* Byte 9      : Panjang data AD asli (0..31 byte) */
    uint8_t  payload[31];  /* Byte 10..40 : Raw AD Structures (0-padded jika len < 31) */
    uint8_t  checksum;     /* Byte 41     : XOR Checksum dari byte 1 s.d byte 40 */
};
```

### 2.2 `struct RawBlePacket`
Struktur penampung internal di memori RAM Zephyr (`k_msgq`) sebelum data diserialisasi menjadi `RawBleBinaryFrame`:
- `timestamp`: Waktu kedatangan paket dalam milidetik kernel (`k_uptime_get_32()`).
- `mac[6]`: Alamat Bluetooth MAC address (Big-Endian).
- `adv_type`: Tipe PDU radio BLE.
- `rssi`: Nilai kekuatan sinyal penerimaan radio hardware (dBm).
- `payload_len`: Panjang data advertising aktual.
- `payload[31]`: Buffer data mentah 31 byte.

---

## 3. Variabel Internal & Sinkronisasi Kernel

- **`beacon_msgq`**: Antrean pesan kernel Zephyr (`k_msgq`) bertipe `struct RawBlePacket` dengan kapasitas 100 item. Dibuat dengan alignment 4-byte dan bersifat *lock-free* serta *thread-safe*.
- **`total_received`**: Counter atomik 32-bit (`atomic_t`) menghitung total kumulatif paket yang diterima radio.
- **`total_sent`**: Counter atomik menghitung total frame yang dikirim ke UART ESP32.
- **`total_dropped`**: Counter atomik menghitung paket yang dibuang karena antrean penuh (*Drop-Oldest FIFO*).
- **`total_expired`**: Counter atomik menghitung paket yang dibuang karena usianya melewati batas `MAX_PACKET_AGE_MS` (50 ms).

---

## 4. Referensi Lengkap Fungsi-Fungsi

### 4.1 Jalur Penerimaan Data Radio (BLE Ingress)

#### `void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad)`
- **Konteks Eksekusi**: *Bluetooth Controller Thread / High Priority*.
- **Keunggulan**: **Zero-overhead promiscuous capture**. Tidak ada lagi loop parsing string UUID atau pencarian tipe paket di context radio.
- **Operasi**:
  1. Catat `timestamp`, `adv_type`, dan `rssi`.
  2. Konversi format alamat MAC ke Big-Endian (`addr->a.val[5 - i]`).
  3. Salin pointer raw payload `ad->data` hingga batas 31 byte, sisa byte dibersihkan dengan `0x00`.
  4. Langsung dorong ke `beacon_msgq` via `q_push_drop_oldest(&pkt)`.

#### `inline void q_push_drop_oldest(const struct RawBlePacket *pkt)`
- **Tujuan**: Memasukkan paket ke antrean `beacon_msgq` dengan strategi *Drop-Oldest*:
  - Jika antrean penuh (100 item), sistem membuang 1 paket terlama via `k_msgq_get()` lalu memasukkan paket terbaru. Ini memastikan antrean tidak pernah tersumbat.

### 4.2 Jalur Pengiriman Data Serial (UART Egress)

#### `inline void assemble_frame(uint8_t *dest, const struct RawBlePacket *pkt)`
- Merakit frame biner 42-byte:
  1. Mengisi byte `0xAA`.
  2. Menyalin MAC (6B), adv_type (1B), rssi (1B), payload_len (1B), payload (31B).
  3. Menghitung XOR checksum dari byte 1 s.d byte 40 dan meletakkannya di byte 41.

#### `void esp32_tx_thread_entry(void *p1, void *p2, void *p3)`
- **Prioritas**: `K_PRIO_PREEMPT(4)`.
- Menggunakan **Adaptive Batching**: Menunggu paket pertama, lalu menguras antrean hingga 5 frame (210 byte), mengecek TTL 50 ms, dan mengirim seluruh batch ke port hardware UARTE 1.000.000 bps.

### 4.3 Hardware Watchdog Timer (WDT)
- Timeout: **5.000 ms (5 detik)** dengan aksi reset instan (`WDT_FLAG_RESET_SOC`).
- Di-feed di loop utama setiap 1 detik beriringan dengan toggle LED heartbeat.

---

## 5. Panduan & Contoh Kode Parser di Sisi ESP32 (ESP-IDF / Arduino)

Berikut contoh implementasi pembacaan frame 42-byte di ESP32:

```c
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define FRAME_SIZE 42

struct __attribute__((packed)) RawBleBinaryFrame {
    uint8_t  header;       /* Byte 0      : 0xAA */
    uint8_t  mac[6];       /* Byte 1..6   : MAC Address */
    uint8_t  adv_type;     /* Byte 7      : Tipe PDU */
    int8_t   rssi;         /* Byte 8      : RSSI (dBm) */
    uint8_t  payload_len;  /* Byte 9      : Panjang data AD */
    uint8_t  payload[31];  /* Byte 10..40 : Raw AD Payload */
    uint8_t  checksum;     /* Byte 41     : XOR Checksum */
};

/* Verifikasi XOR Checksum */
bool verify_checksum(const struct RawBleBinaryFrame *frame) {
    uint8_t crc = 0;
    const uint8_t *raw = (const uint8_t *)frame;
    for (int i = 1; i <= 40; i++) {
        crc ^= raw[i];
    }
    return (crc == frame->checksum);
}

/* Parser Serbaguna: iBeacon, Eddystone, dan Sensor */
void parse_raw_ble_frame(const struct RawBleBinaryFrame *frame) {
    if (frame->header != 0xAA || !verify_checksum(frame)) {
        return; /* Frame korup atau tidak sinkron */
    }

    uint8_t idx = 0;
    while (idx < frame->payload_len) {
        uint8_t ad_len = frame->payload[idx];
        if (ad_len == 0 || (idx + 1 + ad_len) > frame->payload_len) break;

        uint8_t ad_type = frame->payload[idx + 1];
        const uint8_t *ad_data = &frame->payload[idx + 2];
        uint8_t payload_len = ad_len - 1;

        /* 1. APPLE iBEACON (Manufacturer Data 0xFF) */
        if (ad_type == 0xFF && payload_len >= 25) {
            if (ad_data[0] == 0x4C && ad_data[1] == 0x00 &&
                ad_data[2] == 0x02 && ad_data[3] == 0x15) {
                
                uint16_t major = ((uint16_t)ad_data[20] << 8) | ad_data[21];
                uint16_t minor = ((uint16_t)ad_data[22] << 8) | ad_data[23];
                int8_t tx_power = (int8_t)ad_data[24];
                
                // UUID ada di &ad_data[4] sepanjang 16 byte
                // Proses iBeacon di sini...
            }
        }
        /* 2. GOOGLE EDDYSTONE (Service Data 0x16 dengan UUID 0xFEAA) */
        else if (ad_type == 0x16 && payload_len >= 3) {
            if (ad_data[0] == 0xAA && ad_data[1] == 0xFE) {
                uint8_t frame_type = ad_data[2];
                if (frame_type == 0x00 && payload_len >= 18) {
                    // Eddystone-UID: Namespace (10B) & Instance (6B)
                } else if (frame_type == 0x20 && payload_len >= 14) {
                    // Eddystone-TLM:
                    uint16_t batt_mv = ((uint16_t)ad_data[4] << 8) | ad_data[5];
                    int16_t temp_raw = ((int16_t)ad_data[6] << 8) | ad_data[7];
                    float temp_c = (float)temp_raw / 256.0f;
                    // Proses data suhu & baterai Eddystone di sini...
                }
            }
        }

        idx += (ad_len + 1);
    }
}
```
