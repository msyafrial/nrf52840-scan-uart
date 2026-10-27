# Dokumentasi Kode Sumber & Referensi Teknis (`src/main.c`)

Dokumen ini menjelaskan secara rinci seluruh konstanta, variabel yang dapat dikonfigurasi, struktur data, dan fungsi-fungsi yang ada di dalam berkas [src/main.c](file:///c:/Users/syafr/OneDrive/Documents/nrfConnect/Projects/nrf52840_ibeacon/src/main.c), beserta cara penggunaan dan skenario modifikasinya.

---

## 1. Daftar Variabel & Konstanta Konfigurasi

Semua parameter operasional utama dikelompokkan di bagian atas berkas `src/main.c` (Bagian 1) agar mudah diubah tanpa perlu menyentuh logika inti program:

| Nama Konstanta / Macro | Tipe / Nilai Default | Lokasi Baris | Deskripsi & Fungsi | Dampak Modifikasi |
| :--- | :--- | :--- | :--- | :--- |
| **`DEBUG_MODE`** | `true` | Baris ~20 | Mengaktifkan/menonaktifkan cetak teks log metrik performa ke port USB CDC ACM laptop. | Ubah ke `false` jika board sudah dideploy permanen di lapangan tanpa PC untuk menghemat siklus CPU dan daya. |
| **`ENABLE_UUID_FILTER`** | `true` | Baris ~21 | Saklar filter Proximity UUID 128-bit. | - `true`: Hanya paket dengan UUID yang cocok dengan `FILTER_UUID_STR` yang diproses.<br>- `false`: **Mode Promiscuous**, mengirim SEMUA beacon yang tertangkap antena ke ESP32. |
| **`FILTER_UUID_STR`** | `"CB10023F-A318-..."` | Baris ~22 | String UUID target sepanjang 36 karakter (format standar 8-4-4-4-12 heksadesimal). | Ganti dengan UUID beacon milik Anda yang ingin dilacak. Karakter strip `-` diabaikan otomatis oleh parser. |
| **`QUEUE_CAPACITY`** | `100` (atau `64`) | Baris ~25 | Kapasitas maksimal slot antrean pesan kernel Zephyr (`k_msgq`). | Menentukan berapa banyak paket beacon yang dapat ditampung di memori RAM saat terjadi *burst*. Pada 1 Mbps, 100 paket merepresentasikan jeda aliran data maksimal sekitar 25 ms. |
| **`MAX_PACKET_AGE_MS`** | `50` (milidetik) | Baris ~28 | Ambang batas kadaluwarsa toleransi paket (*Time-To-Live / TTL*). | Paket di antrean yang usianya melebihi nilai ini akan **langsung dibuang (drop)** agar ESP32 tidak menerima data usang saat terjadi lag atau reboot. |
| **`MAX_BATCH_FRAMES`** | `8` | Baris ~31 | Batas maksimal jumlah frame yang dirakit menjadi satu semburan buffer DMA UART. | Pada trafik padat, hingga 8 frame (200 byte) disatukan dalam 1 blok transmisi kontigu tanpa jeda. |
| **`FRAME_SIZE`** | `25` (byte) | Baris ~32 | Ukuran tetap setiap frame biner protokol komunikasi ke ESP32. | **Jangan diubah** kecuali protokol di sisi ESP32 juga disesuaikan. |

---

## 2. Struktur Data (Data Structures)

### 2.1 `struct __attribute__((packed)) BeaconBinaryFrame`
Struktur memori biner padat tanpa padding byte yang dikirim langsung melalui jalur kabel serial UART ke ESP32:

```c
struct __attribute__((packed)) BeaconBinaryFrame {
    uint8_t  header;    /* Byte 0      : Selalu bernilai 0xAA (Sync Byte) */
    uint8_t  code;      /* Byte 1      : 0x00 (iBeacon) atau 0x01 (Minew B) */
    uint8_t  uuid[16];  /* Byte 2..17  : 16-byte raw Proximity UUID */
    uint16_t major;     /* Byte 18..19 : Major ID (Format Big-Endian / Network Order) */
    uint16_t minor;     /* Byte 20..21 : Minor ID (Format Big-Endian / Network Order) */
    int8_t   rssi;      /* Byte 22     : Kekuatan sinyal dBm (-128 s.d +127) */
    uint8_t  battery;   /* Byte 23     : Persentase baterai 0-100% (0 jika iBeacon) */
    uint8_t  checksum;  /* Byte 24     : XOR Checksum dari byte 1 s.d byte 23 */
};
```

### 2.2 `enum BeaconType`
Mendefinisikan klasifikasi jenis paket beacon yang dideteksi oleh firmware:
- `BEACON_TYPE_UNKNOWN` (`0`): Tipe tidak dikenal.
- `BEACON_TYPE_MINEW_B` (`1`): Format Minew B (AD Type `0x16`, membawa informasi baterai).
- `BEACON_TYPE_IBEACON` (`2`): Format Apple iBeacon standar (AD Type `0xFF`, prefix `0x004C0215`).

### 2.3 `struct BeaconPacket`
Struktur penampung internal di memori RAM sebelum data diserialisasi menjadi `BeaconBinaryFrame`:
- `timestamp`: Waktu penerimaan paket dalam milidetik kernel (`k_uptime_get_32()`).
- `mac[6]`: Alamat fisik Bluetooth MAC address (Big-Endian).
- `rssi`, `tx_power`: Nilai kekuatan sinyal pemancar dan penerima.
- `battery`, `has_battery`: Status dan persentase baterai.
- `major`, `minor`, `uuid[16]`: Identitas unik beacon.

---

## 3. Variabel Internal & Sinkronisasi Kernel

- **`beacon_msgq`**: Antrean pesan kernel Zephyr (`k_msgq`) yang menyimpan data bertipe `struct BeaconPacket`. Dibuat dengan alignment 4-byte dan bersifat *lock-free* serta *thread-safe*.
- **`total_received`**: Counter atomik 32-bit (`atomic_t`) menghitung total kumulatif paket beacon yang berhasil ditangkap oleh antena radio.
- **`total_sent`**: Counter atomik menghitung total frame biner yang berhasil ditembakkan ke port UART ESP32.
- **`total_dropped`**: Counter atomik menghitung paket terlama yang dibuang karena antrean penuh (*Drop-Oldest*).
- **`total_expired`**: Counter atomik menghitung paket yang dibuang karena usianya di antrean melewati batas `MAX_PACKET_AGE_MS` (50 ms).
- **`target_uuid_bytes[16]`**: Array biner 16-byte hasil konversi `FILTER_UUID_STR` saat fungsi `main()` pertama kali berjalan.

---

## 4. Referensi Lengkap Fungsi-Fungsi

### 4.1 Utilitas & Helper

#### `bool parse_uuid_string(const char *str, uint8_t out[16])`
- **Tujuan**: Mengonversi teks UUID heksadesimal standar (misalnya `"CB10023F-A318-3394-4199-A8730C7C1AEC"`) menjadi representasi biner murni 16 byte.
- **Parameter**:
  - `str`: String UUID input (mengabaikan tanda minus `-`).
  - `out`: Buffer array 16-byte untuk menampung hasil konversi.
- **Nilai Balik**: `true` jika berhasil mengonversi tepat 16 byte, `false` jika format teks tidak valid atau kurang panjang.

#### `inline bool is_target_uuid(const uint8_t *uuid_bytes)`
- **Tujuan**: Memvalidasi apakah UUID paket yang masuk cocok dengan target filter.
- **Parameter**: `uuid_bytes` adalah pointer ke 16 byte UUID yang baru diekstrak dari payload radio.
- **Nilai Balik**: `true` jika cocok (atau jika `ENABLE_UUID_FILTER` diset `false`), `false` jika tidak cocok.
- **Optimasi**: Menggunakan `memcmp` 16-byte yang dioptimasi oleh compiler GCC menjadi 4 instruksi pembandingan kata 32-bit (~62 nanodetik pada Cortex-M4).

#### `void format_count(uint32_t val, char *buf, size_t sz)`
- **Tujuan**: Meringkas tampilan angka besar di serial monitor agar tidak memakan tempat di layar:
  - Nilai $\ge 1.000.000$ ditampilkan dalam format `1.2M`.
  - Nilai $\ge 10.000$ ditampilkan dalam format `25.4K`.
  - Nilai $< 10.000$ ditampilkan apa adanya (misal `1427`).

---

### 4.2 Jalur Penerimaan Data Radio (BLE Ingress)

#### `void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad)`
- **Konteks Eksekusi**: *Interrupt Context / Bluetooth Controller Thread* (Prioritas Tertinggi).
- **Tugas Utama**:
  1. Membaca pointer payload `ad->data` secara *zero-copy* tanpa alokasi memori dinamis.
  2. Melakukan loop membedah struktur data TLV (Type-Length-Value):
     - **Minew Format B**: Mendeteksi AD Type `0x16`, mengekstrak UUID 16-byte, Major, Minor, Tx Power, dan Baterai.
     - **Apple iBeacon**: Mendeteksi AD Type `0xFF` dengan prefix Apple `0x004C0215`, mengekstrak UUID 16-byte, Major, Minor, dan Tx Power.
  3. Jika UUID lolos filter `is_target_uuid()`, fungsi memanggil `q_push_drop_oldest(&pkt)`.

#### `inline void q_push_drop_oldest(const struct BeaconPacket *pkt)`
- **Tujuan**: Memasukkan paket ke dalam antrean kernel `beacon_msgq` dengan strategi *Drop-Oldest FIFO*:
  1. Menaikkan counter `total_received`.
  2. Mencoba `k_msgq_put(..., K_NO_WAIT)`.
  3. Jika antrean penuh (`ret != 0`), fungsi **membuang 1 paket terlama** menggunakan `k_msgq_get(..., K_NO_WAIT)`, lalu memasukkan paket terbaru ke antrean, dan menaikkan counter `total_dropped`.

---

### 4.3 Jalur Pengiriman Data Serial (UART Egress)

#### `inline void assemble_frame(uint8_t *dest, const struct BeaconPacket *pkt)`
- **Tujuan**: Merakit 1 frame biner 25-byte ke dalam buffer memori:
  1. Mengisi header `0xAA` dan kode tipe beacon.
  2. Menyalin 16 byte UUID.
  3. Mengonversi Major dan Minor ke format Big-Endian menggunakan macro hardware `sys_cpu_to_be16()`.
  4. Menghitung XOR checksum dari byte ke-1 (`code`) hingga byte ke-23 (`battery`) dan menyimpannya di byte ke-24.

#### `void esp32_tx_thread_entry(void *p1, void *p2, void *p3)`
- **Prioritas Thread**: `K_PRIO_PREEMPT(4)` (Prioritas Tinggi).
- **Logika Adaptive Batching**:
  1. **Tunggu Event**: Menunggu minimal 1 paket pertama tiba di antrean (`k_msgq_get(..., K_FOREVER)`). Thread tidur pulas saat tidak ada paket (0% beban CPU).
  2. **Cek Batas Usia (TTL)**: Menghitung selisih waktu `k_uptime_get_32() - pkt.timestamp`. Jika usianya $> 50\text{ ms}$, paket dibuang dan counter `total_expired` bertambah.
  3. **Kuras Antrean (Burst Batching)**: Melakukan loop non-blocking `k_msgq_get(..., K_NO_WAIT)` untuk merakit paket-paket lain yang sudah siap antre ke dalam satu buffer kontigu hingga maksimal `MAX_BATCH_FRAMES` (8 frame = 200 byte).
  4. **Tembak ke Hardware UART**: Mengirim seluruh blok memori batch sekaligus melalui driver UARTE perangkat keras pada kecepatan 1.000.000 bps.

---

### 4.4 Jalur Monitoring & Diagnostik (Debug Egress)

#### `void debug_stats_thread_entry(void *p1, void *p2, void *p3)`
- **Prioritas Thread**: `K_PRIO_PREEMPT(10)` (Prioritas Terendah).
- **Tugas Utama**:
  1. Bangun secara periodik setiap 2.000 milidetik (`k_msleep(2000)`).
  2. Memeriksa saluran DTR pada port USB:
     `uart_line_ctrl_get(console_dev, UART_LINE_CTRL_DTR, &dtr);`
  3. Jika DTR aktif (artinya pengguna membuka Serial Monitor di laptop), fungsi menghitung kecepatan kedatangan paket (*Packets Per Second / PPS*):
     $$\text{PPS} = \frac{(\text{cur\_recv} - \text{last\_received}) \times 1000}{\text{delta\_ms}}$$
  4. Mencetak baris laporan ringkas:
     ```text
     >>> [PERF STATS] Rate: 20 pkt/s | In: 1.4K | Sent ESP32: 1.4K | Expired: 0 | Dropped: 0 <<<
     ```

---

### 4.5 Inisialisasi Sistem (`main`)

#### `int main(void)`
- Mengonfigurasi LED status (Pin `P0.30`).
- Memastikan perangkat keras UART0 siap (`device_is_ready(esp32_uart)`).
- Menunggu koneksi awal USB monitor selama maksimal 2 detik secara *non-blocking*.
- Mengonversi string `FILTER_UUID_STR` ke bentuk biner `target_uuid_bytes`.
- Mengaktifkan stack Bluetooth dengan `bt_enable(NULL)`.
- Mengonfigurasi scanner pasif 100% duty cycle:
  - `interval = 160` (100 ms).
  - `window = 160` (100 ms).
  - Memulai pemindaian dengan `bt_le_scan_start(&scan_param, device_found)`.
- Menjalankan loop utama yang membalikkan status LED (*toggle blink*) setiap 1 detik sebagai indikator detak jantung (*heartbeat*) sistem.


---

## 4.5. Hardware Watchdog Timer (WDT) & Diagnostik Sistem

Pada bagian awal `main()`, sistem secara otomatis mengaktifkan fitur keandalan industri:
1. **Pemeriksaan Penyebab Reboot (`hwinfo_get_reset_cause`)**:
   - Jika sistem mengalami restart akibat Watchdog timeout, log konsol USB akan menampilkan:
     `[SYSTEM] PERINGATAN: Sistem pulih dari WATCHDOG TIMEOUT!`
   - Jika booting normal (colok adaptor atau tombol reset), akan menampilkan:
     `[SYSTEM] Sistem booting normal (Reset Cause: 0x...)`
2. **Inisialisasi Watchdog (`wdt_install_timeout` & `wdt_setup`)**:
   - Batas waktu timeout: **5.000 ms (5 detik)**.
   - Opsi: `WDT_FLAG_RESET_SOC` (restart sistem penuh jika terjadi hang).
   - Opsi Debug: `WDT_OPT_PAUSE_HALTED_BY_DBG` (jeda otomatis saat debugger aktif).
3. **Feeding Otomatis di Loop Utama**:
   - Fungsi `wdt_feed(wdt_dev, wdt_channel_id)` dipanggil setiap 1 detik bersamaan dengan kedipan LED heartbeat.

---
## 5. Panduan Kustomisasi Praktis (How-To Recipes)

### Resep 1: Mengganti UUID Target
Jika Anda memiliki kelompok beacon baru dengan UUID berbeda:
Buka `src/main.c`, ubah baris:
```c
#define FILTER_UUID_STR "E2C56DB5-DFFB-48D2-B060-D0F5A71096E0"
```
Simpan file, lalu jalankan `.\build.ps1` dan `.\flash.ps1`.

### Resep 2: Menerima SEMUA Beacon (Mode Promiscuous)
Jika Anda ingin menangkap seluruh beacon di sekitar tanpa mempedulikan UUID-nya:
Ubah baris:
```c
#define ENABLE_UUID_FILTER false
```

### Resep 3: Menyesuaikan Toleransi Data Basi (TTL)
- Untuk tracking gerak sangat cepat (drone / atletik): Set `MAX_PACKET_AGE_MS` ke `30` (30 ms).
- Untuk tracking aset normal: Set `MAX_PACKET_AGE_MS` ke `100` (100 ms).

### Resep 4: Mengubah Kecepatan Baudrate Serial ke ESP32
Jika Anda ingin mengubah baudrate dari 1 Mbps ke kecepatan lain (misal 115.200 bps):
1. Buka [boards/<board>.overlay](file:///c:/Users/syafr/OneDrive/Documents/nrfConnect/Projects/nrf52840_ibeacon/boards/<board>.overlay), ubah:
   ```dts
   &uart0 {
       status = "okay";
       current-speed = <115200>;
   };
   ```
2. Pastikan kode di sisi ESP32 juga disesuaikan ke `115200`.
