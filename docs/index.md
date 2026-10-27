# Dokumentasi Teknis: nRF52840 Real-time High-Density iBeacon Gateway

Proyek ini adalah implementasi firmware berkinerja tinggi berbasis **Zephyr RTOS** (nRF Connect SDK) yang berjalan pada **Seeed Studio XIAO BLE / Super52840** dan **Pro Micro nRF52840** (Nordic nRF52840 SoC). Firmware ini bertindak sebagai jembatan cerdas (*gateway*) berlatensi ultra-rendah yang menangkap sinyal ribuan paket beacon per detik, memfilternya, dan mentransmisikannya langsung ke ESP32 melalui kabel serial perangkat keras (UARTE) berkecepatan 1.000.000 bps (1 Mbps).

---

## 1. Spesifikasi Sistem & Perangkat Keras

- **Mikrokontroler**: Nordic Semiconductor nRF52840 (ARM Cortex-M4F @ 64 MHz, FPU, 256 KB RAM, 1 MB Flash).
- **Board Target yang Didukung**:
  1. **Seeed Studio XIAO BLE / Sense / Super52840** (`xiao_ble/nrf52840`)
  2. **Pro Micro nRF52840 / Nice!Nano** (`promicro_nrf52840/nrf52840/uf2`)
- **Sistem Operasi**: Zephyr RTOS v4.4.2 (Nordic nRF Connect SDK v3.4.1).
- **Protokol Radio**: Bluetooth Low Energy 5.0 (Continuous Passive Scanning, 100% Duty Cycle).
- **Target Beban**: 300+ unit beacon aktif dengan interval 200-300 ms (kapasitas pengolahan 1.200+ paket per detik).

---

## 2. Pemetaan Pinout Fisik ke ESP32

### A. Seeed XIAO BLE / Super52840
| Label Pin XIAO | Nomor Pin SoC nRF52840 | Arah Sinyal | Terhubung ke ESP32 | Keterangan |
| :--- | :--- | :--- | :--- | :--- |
| **D6 (TX)** | `P1.11` | Output | **RX (GPIO ESP32)** | Jalur transmisi biner frame 25-byte @ 1.000.000 bps |
| **D7 (RX)** | `P1.12` | Input | **TX (GPIO ESP32)** | Jalur kendali start/stop scan dari ESP32 |
| **USB-C** | Native USB D+/D- | Bi-directional | **Port USB PC / Laptop** | Virtual COM Port (CDC ACM) untuk monitor metrik performa |
| **LED Hijau** | `P0.30` (led1) | Output | Indikator Internal | Heartbeat blink setiap 1 detik |

### B. Pro Micro nRF52840
| Label Pin Pro Micro | Nomor Pin SoC nRF52840 | Arah Sinyal | Terhubung ke ESP32 | Keterangan |
| :--- | :--- | :--- | :--- | :--- |
| **TX (D1)** | `P0.06` | Output | **RX (GPIO ESP32)** | Jalur transmisi biner frame 25-byte @ 1.000.000 bps |
| **RX (D0)** | `P0.08` | Input | **TX (GPIO ESP32)** | Jalur kendali start/stop scan dari ESP32 |
| **USB-C** | Native USB D+/D- | Bi-directional | **Port USB PC / Laptop** | Virtual COM Port (CDC ACM) untuk monitor metrik performa |
| **LED Merah** | `P0.15` (led0) | Output | Indikator Internal | Heartbeat blink setiap 1 detik |

---

## 3. Protokol Biner 25-Byte (Komunikasi ke ESP32)

Setiap paket beacon yang lolos filter dikemas dalam sebuah struktur biner padat (*packed binary frame*) berukuran tetap **25 byte**.

### Diagram Struktur Frame (25 Byte)

```text
Byte:   0      1      2..17              18..19   20..21   22     23        24
     +------+------+-------------------+--------+--------+------+---------+----------+
     | 0xAA | Code | Proximity UUID    | Major  | Minor  | RSSI | Battery | Checksum |
     |      |      | (16 Byte)         | (2 B)  | (2 B)  | (1 B)| (1 B)   | (1 B)    |
     +------+------+-------------------+--------+--------+------+---------+----------+
```

### Rincian Field Frame

| Byte Offset | Panjang | Nama Field | Tipe Data | Deskripsi |
| :--- | :--- | :--- | :--- | :--- |
| **0** | 1 Byte | `header` | `uint8_t` | Karakter sinkronisasi tetap: `0xAA` |
| **1** | 1 Byte | `code` | `uint8_t` | Tipe beacon: `0x00` (Apple iBeacon) atau `0x01` (Minew Format B) |
| **2 .. 17** | 16 Byte | `uuid` | `uint8_t[16]` | 128-bit Proximity UUID (Big-Endian raw bytes) |
| **18 .. 19** | 2 Byte | `major` | `uint16_t` | Nilai Major ID beacon (Format Big-Endian / Network Order) |
| **20 .. 21** | 2 Byte | `minor` | `uint16_t` | Nilai Minor ID beacon (Format Big-Endian / Network Order) |
| **22** | 1 Byte | `rssi` | `int8_t` | Kekuatan sinyal diterima dalam satuan dBm (-128 s.d +127) |
| **23** | 1 Byte | `battery` | `uint8_t` | Persentase baterai 0-100% (bernilai 0 untuk Apple iBeacon) |
| **24** | 1 Byte | `checksum` | `uint8_t` | XOR dari byte offset 1 sampai dengan byte offset 23 |

---

## 4. Cara Build dan Flash Multi-Board

Kedua board menggunakan basis kode C yang sama persis (`src/main.c`), namun memiliki script build dan folder hasil kompilasi masing-masing sehingga tidak saling mengganggu:

### Untuk Seeed XIAO BLE:
```powershell
# Kompilasi firmware:
.\build_xiao.ps1

# Flashing via ST-Link:
.\flash_xiao.ps1
```
Hasil biner tersimpan di: `build_xiao/zephyr/zephyr.uf2`

### Untuk Pro Micro nRF52840:
```powershell
# Kompilasi firmware:
.\build_promicro.ps1

# Flashing via ST-Link:
.\flash_promicro.ps1
```
Hasil biner tersimpan di: `build_promicro/zephyr/zephyr.uf2`

*Catatan Flashing via Bootloader USB (UF2): Cukup tekan tombol reset dua kali (double-tap), lalu seret (drag-and-drop) file `zephyr.uf2` ke drive USB yang muncul di komputer.*
