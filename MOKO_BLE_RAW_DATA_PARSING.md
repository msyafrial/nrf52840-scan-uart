# Panduan Lengkap Parsing Raw Data BLE MOKO MK Button (BXP-B Series V1.0)

Dokumen ini berisi spesifikasi teknis dan panduan parsing *byte-by-byte* untuk seluruh data siaran BLE (**Primary Advertising / `ADV_IND`**) dan data respons pemindaian (**Scan Response / `SCAN_RSP`**) pada emulator **ESP32-S3 MK Button (BXP-B Series)** yang kompatibel dengan aplikasi **BeaconX Pro**.

---

## 1. Diagram Alur Identifikasi Paket BLE (Parser Decision Flowchart)

Diagram alur berikut digunakan oleh parser receiver (misal nRF52840 atau ESP32) untuk mengidentifikasi dan mem-parsing tipe paket yang masuk:

```mermaid
flowchart TD
    Start(["Terima Paket BLE Raw Byte Stream"]) --> CheckADType["Iterasi Struktur AD (Length + AD Type)"]

    CheckADType --> CheckType{"Cek AD Type"}

    %% 1. Flags
    CheckType -->|0x01 Flags| ParseFlags["Parse BLE Flags (e.g. 0x06)"]

    %% 2. Manufacturer Data
    CheckType -->|0xFF Mfg Data| CheckApple{"Company ID == 0x004C & SubType == 0x02?"}
    CheckApple -->|Ya| ParseIBeacon["Parse Apple iBeacon:\n- UUID (16B)\n- Major (2B)\n- Minor (2B)\n- RSSI@1m (1B)"]
    CheckApple -->|Bukan| IgnoreMfg["Abaikan / Format Lain"]

    %% 3. Service Data (0x16)
    CheckType -->|0x16 Service Data| CheckUUID{"Cek 16-bit Service UUID"}

    %% 3a. FEE0 Alarm
    CheckUUID -->|0xFEE0 (E0 FE)| ParseAlarm["Parse Slot Alarm Frame:\n- Frame Type (0x20/0x21/0x22)\n- Status Flag (Bit 0 Password, Bit 1 Trigger)\n- Trigger Count (2B)\n- Device ID (1..6B)\n- FW Type & RFU"]

    %% 3b. EA00 Device Info
    CheckUUID -->|0xEA00 (00 EA)| ParseDevInfo["Parse Device Info (Scan Response):\n- Accel X, Y, Z (signed 16-bit)\n- Suhu (0.1°C)\n- Ranging Data (1B)\n- Baterai (mV atau %)\n- MAC Address (6B)"]

    %% 3c. FEAB iBeacon Info
    CheckUUID -->|0xFEAB (AB FE)| ParseFEAB["Parse iBeacon Scan Response:\n- Frame 0x50\n- RSSI@1m, Adv Interval\n- UUID, Major, Minor"]

    %% 3d. FEAA Eddystone UID
    CheckUUID -->|0xFEAA (AA FE)| ParseUID["Parse Eddystone-UID:\n- Frame 0x00\n- RSSI@0m\n- Namespace (10B)\n- Instance (6B)"]

    %% 4. Local Name (0x09)
    CheckType -->|0x09 Name| ParseName["Parse Complete Local Name\n(e.g. BXP-B-D_E5)"]

    %% 5. Tx Power (0x0A)
    CheckType -->|0x0A Tx Power| ParseTx["Parse Tx Power Level (signed int8 dBm)"]

    ParseFlags --> NextAD{"Masih ada AD block berikutnya?"}
    ParseIBeacon --> NextAD
    ParseAlarm --> NextAD
    ParseDevInfo --> NextAD
    ParseFEAB --> NextAD
    ParseUID --> NextAD
    ParseName --> NextAD
    ParseTx --> NextAD

    NextAD -->|Ya| CheckType
    NextAD -->|Selesai| End(["Paket Selesai Di-parse"])
```

---

## 2. Struktur Dasar Elemen AD (Advertising Data Structure)

Setiap siaran paket BLE (maksimal 31 byte) tersusun atas satu atau lebih blok **AD Structure** dengan format baku:
$$\mathbf{[Length]} + \mathbf{[AD\ Type]} + \mathbf{[Payload\ Data]}$$
* **Length (1 byte):** Panjang sisa blok (yakni $1\text{ byte } [AD\ Type] + N\text{ byte } [Payload]$).
* **AD Type (1 byte):** Jenis data (misal: `0x01` = Flags, `0x16` = Service Data, `0xFF` = Manufacturer Data, `0x09` = Local Name, `0x0A` = Tx Power).

---

## 3. SLOT 0: Customized - Alarm Mode (`0xFEE0`)

### A. Primary Advertising Packet (`ADV_IND`)
Disiarkan saat giliran Slot 0 (atau saat terjadi event klik tombol / alarm burst).

* **Contoh Raw Hex Data:**  
  `02 01 06 0C 16 E0 FE 20 00 00 07 00 01 01 00 00 0B 09 42 58 50 2D 42 2D 44 5F 45 35`

| Offset Byte | Field Name | Ukuran | Contoh Hex | Tipe Data | Keterangan & Rumus Parsing |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **0** | AD1 Length | 1B | `0x02` | uint8 | Panjang blok Flags (2 byte data setelahnya) |
| **1** | AD1 Type | 1B | `0x01` | uint8 | `0x01` = Flags |
| **2** | Flags Value | 1B | `0x06` | uint8 | `0x06` (LE General Discoverable + BR/EDR Not Supported) |
| **3** | AD2 Length | 1B | `0x0C` | uint8 | Panjang blok Service Data (12 byte data setelahnya) |
| **4** | AD2 Type | 1B | `0x16` | uint8 | `0x16` = Service Data 16-bit UUID |
| **5 - 6** | Service UUID | 2B | `E0 FE` | uint16 (LE) | **`0xFEE0`** (MOKO Button Service UUID, Little Endian) |
| **7** | Frame Type | 1B | `0x20` | uint8 | `0x20`: Single press mode<br>`0x21`: Double press mode<br>`0x22`: Long press mode<br>`0x23`: Abnormal inactivity mode |
| **8** | Status Flag | 1B | `0x00` | uint8 | Bitmask Status:<br>• **Bit 0** (`& 0x01`): Password Verification (`0`=Mati, `1`=Aktif)<br>• **Bit 1** (`& 0x02`): Main Button Trigger Status (`0`=Standby, `1`=Alarm Triggered)<br>• **Bit 2** (`& 0x04`): Sub Button Status (`0`=Normal, `1`=Triggered)<br>• **Bit 3..7**: RFU |
| **9 - 10** | Trigger Count | 2B | `00 07` | uint16 (BE) | Counter jumlah klik tombol.<br>Rumus: $\text{Count} = (\text{Byte}[9] \ll 8) \mid \text{Byte}[10]$ (Contoh: `0x0007` = 7) |
| **11 - 13** | Device ID | 3B (s.d 6B)| `00 01 01` | Hex Bytes | ID Perangkat. Format hex string: `"000101"` |
| **14** | Firmware Type | 1B | `0x00` | uint8 | Tipe firmware bawaan (`0x00`) |
| **15** | RFU | 1B | `0x00` | uint8 | Reserved for Future Use (`0x00`) |
| **16** | AD3 Length | 1B | `0x0B` | uint8 | Panjang blok Device Name (11 byte) |
| **17** | AD3 Type | 1B | `0x09` | uint8 | `0x09` = Complete Local Name |
| **18 - 27** | Device Name | 10B | `42 58 50...`| ASCII | String nama perangkat (ASCII: `"BXP-B-D_E5"`) |

---

### B. Scan Response Packet (`SCAN_RSP`) untuk Slot 0
Dibalas oleh beacon saat scanner mengirimkan `SCAN_REQ`. Berisi data sensor dan kelistrikan lengkap.

* **Contoh Raw Hex Data:**  
  `18 16 00 EA 00 00 00 10 00 28 FF C2 03 DC 00 FA C5 0B B8 48 27 E2 1A CA E5 02 0A 00`

| Offset Byte | Field Name | Ukuran | Contoh Hex | Tipe Data | Keterangan & Rumus Parsing |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **0** | AD1 Length | 1B | `0x18` | uint8 | Panjang blok Service Data (24 byte / 0x18) |
| **1** | AD1 Type | 1B | `0x16` | uint8 | `0x16` = Service Data 16-bit UUID |
| **2 - 3** | Service UUID | 2B | `00 EA` | uint16 (LE) | **`0xEA00`** (MOKO General Device Info UUID, Little Endian) |
| **4** | Frame Type | 1B | `0x00` | uint8 | `0x00` = General Device Information Frame |
| **5** | Accel Full-scale | 1B | `0x00` | uint8 | `0x00`: ±2g, `0x01`: ±4g, `0x02`: ±8g, `0x03`: ±16g |
| **6 - 7** | Motion Threshold | 2B | `00 10` | uint16 (BE) | Ambang batas gerak. `0x0010` = 16 mg |
| **8 - 9** | Accel X-axis | 2B | `00 28` | int16 (BE) | Percepatan sumbu X (signed int16).<br>Contoh: `0x0028` = $+40\text{ mg}$ |
| **10 - 11** | Accel Y-axis | 2B | `FF C2` | int16 (BE) | Percepatan sumbu Y (signed int16).<br>Contoh: `0xFFC2` = $-62\text{ mg}$ |
| **12 - 13** | Accel Z-axis | 2B | `03 DC` | int16 (BE) | Percepatan sumbu Z (signed int16).<br>Contoh: `0x03DC` = $+988\text{ mg}$ ($\approx 1\text{g}$ gravitasi bumi) |
| **14 - 15** | Temperature | 2B | `00 FA` | int16 (BE) | Suhu perangkat.<br>Rumus: $T = \text{Value} \times 0.1\ ^\circ\text{C}$<br>Contoh: `0x00FA` = $250 \times 0.1 = \mathbf{25.0^\circ\text{C}}$ |
| **16** | Ranging Data | 1B | `C5` | int8 | RSSI kalibrasi pada jarak 0m / 1m.<br>Contoh: `0xC5` (Two's complement) = $\mathbf{-59\text{ dBm}}$ |
| **17 - 18** | Battery Voltage / % | 2B | `0B B8` | uint16 (BE) | **Aturan Parsing:**<br>• Jika nilai $> 100$: Tegangan dalam $\text{mV}$.<br>Contoh: `0x0BB8` = $\mathbf{3000\text{ mV}}$ ($3.0\text{V}$)<br>• Jika nilai $\le 100$: Persentase baterai $\%$. |
| **19 - 24** | MAC Address | 6B | `48 27 E2 1A CA E5` | 6 Byte MAC | Alamat MAC fisik Bluetooth perangkat (`48:27:E2:1A:CA:E5`) |
| **25** | AD2 Length | 1B | `0x02` | uint8 | Panjang blok Tx Power (2 byte) |
| **26** | AD2 Type | 1B | `0x0A` | uint8 | `0x0A` = Tx Power Level |
| **27** | Tx Power | 1B | `0x00` | int8 | Daya pancar radio dalam $\text{dBm}$. Contoh: `0x00` = $\mathbf{0\text{ dBm}}$ |

---

## 4. SLOT 1: Customized - iBeacon

### A. Primary Advertising Packet (`ADV_IND`)
Paket standar Apple Inc. Proximity iBeacon.

* **Contoh Raw Hex Data:**  
  `02 01 06 1A FF 4C 00 02 15 CB 10 02 3F A3 18 33 94 41 99 A8 73 0C 7C 1A EC 27 12 03 EA C5`

| Offset Byte | Field Name | Ukuran | Contoh Hex | Tipe Data | Keterangan & Rumus Parsing |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **0** | AD1 Length | 1B | `0x02` | uint8 | Panjang blok Flags (2 byte) |
| **1** | AD1 Type | 1B | `0x01` | uint8 | `0x01` = Flags |
| **2** | Flags Value | 1B | `0x06` | uint8 | `0x06` (Discoverable) |
| **3** | AD2 Length | 1B | `0x1A` | uint8 | Panjang blok Mfg Data (26 byte / 0x1A) |
| **4** | AD2 Type | 1B | `0xFF` | uint8 | `0xFF` = Manufacturer Specific Data |
| **5 - 6** | Company ID | 2B | `4C 00` | uint16 (LE) | **`0x004C`** (Apple Inc., Little Endian) |
| **7** | Beacon Type | 1B | `0x02` | uint8 | `0x02` = Proximity Beacon Sub-type |
| **8** | Beacon Length | 1B | `0x15` | uint8 | `0x15` = 21 byte data iBeacon setelahnya |
| **9 - 24** | Proximity UUID | 16B | `CB 10 02 3F...` | 16 Bytes | UUID 128-bit iBeacon:<br>`CB10023F-A318-3394-4199-A8730C7C1AEC` |
| **25 - 26** | Major | 2B | `27 12` | uint16 (BE) | Major ID. `0x2712` = $\mathbf{10002}$ |
| **27 - 28** | Minor | 2B | `03 EA` | uint16 (BE) | Minor ID. `0x03EA` = $\mathbf{1002}$ |
| **29** | Measured RSSI@1m | 1B | `C5` | int8 | RSSI terkalibrasi pada jarak 1 meter:<br>`0xC5` = $\mathbf{-59\text{ dBm}}$ |

---

### B. Scan Response Packet (`SCAN_RSP`) untuk Slot 1 (`0xFEAB`)
Paket respon MOKO khusus yang memancarkan ulang info slot iBeacon agar aplikasi BeaconX Pro dapat membaca interval dan parameter tanpa melakukan pairing GATT.

* **Contoh Raw Hex Data:**  
  `02 0A 00 1A 16 AB FE 50 C5 0A CB 10 02 3F A3 18 33 94 41 99 A8 73 0C 7C 1A EC 27 12 03 EA`

| Offset Byte | Field Name | Ukuran | Contoh Hex | Tipe Data | Keterangan & Rumus Parsing |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **0** | AD1 Length | 1B | `0x02` | uint8 | Panjang blok Tx Power (2 byte) |
| **1** | AD1 Type | 1B | `0x0A` | uint8 | `0x0A` = Tx Power Level |
| **2** | Tx Power | 1B | `0x00` | int8 | $0\text{ dBm}$ |
| **3** | AD2 Length | 1B | `0x1A` | uint8 | Panjang blok Service Data (26 byte / 0x1A) |
| **4** | AD2 Type | 1B | `0x16` | uint8 | `0x16` = Service Data 16-bit UUID |
| **5 - 6** | Service UUID | 2B | `AB FE` | uint16 (LE) | **`0xFEAB`** (MOKO iBeacon Service Data UUID) |
| **7** | Frame Type | 1B | `0x50` | uint8 | `0x50` = MOKO iBeacon Parameter Frame |
| **8** | RSSI@1m | 1B | `C5` | int8 | Kalibrasi RSSI 1 meter (`-59 dBm`) |
| **9** | Adv Interval | 1B | `0x0A` | uint8 | Interval siaran slot.<br>Satuan: $100\text{ ms}/\text{digit}$<br>Contoh: `0x0A` = $10 \times 100\text{ ms} = \mathbf{1000\text{ ms}}$ |
| **10 - 25** | iBeacon UUID | 16B | `CB 10 02 3F...` | 16 Bytes | Salinan Proximity UUID (sama dengan iBeacon) |
| **26 - 27** | Major | 2B | `27 12` | uint16 (BE) | Salinan Major ID (`10002`) |
| **28 - 29** | Minor | 2B | `03 EA` | uint16 (BE) | Salinan Minor ID (`1002`) |

---

## 5. SLOT 2: Eddystone-UID

### A. Primary Advertising Packet (`ADV_IND`)
Paket resmi Google Eddystone-UID dengan Service UUID `0xFEAA`.

* **Contoh Raw Hex Data:**  
  `02 01 06 03 03 AA FE 17 16 AA FE 00 EC ED D1 E0 01 02 03 04 05 06 07 01 02 03 04 05 03 00 00`

| Offset Byte | Field Name | Ukuran | Contoh Hex | Tipe Data | Keterangan & Rumus Parsing |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **0** | AD1 Length | 1B | `0x02` | uint8 | Panjang blok Flags (2 byte) |
| **1** | AD1 Type | 1B | `0x01` | uint8 | `0x01` = Flags |
| **2** | Flags Value | 1B | `0x06` | uint8 | `0x06` (Discoverable) |
| **3** | AD2 Length | 1B | `0x03` | uint8 | Panjang blok Service List (3 byte) |
| **4** | AD2 Type | 1B | `0x03` | uint8 | `0x03` = Complete List of 16-bit Service UUIDs |
| **5 - 6** | Service UUID List| 2B | `AA FE` | uint16 (LE) | **`0xFEAA`** (Google Eddystone UUID) |
| **7** | AD3 Length | 1B | `0x17` | uint8 | Panjang blok Service Data (23 byte / 0x17) |
| **8** | AD3 Type | 1B | `0x16` | uint8 | `0x16` = Service Data 16-bit UUID |
| **9 - 10** | Service UUID | 2B | `AA FE` | uint16 (LE) | **`0xFEAA`** (Google Eddystone UUID) |
| **11** | Frame Type | 1B | `0x00` | uint8 | `0x00` = Eddystone-UID Frame |
| **12** | Calibrated RSSI@0m | 1B | `EC` | int8 | RSSI terkalibrasi pada jarak 0 meter.<br>Contoh: `0xEC` = $\mathbf{-20\text{ dBm}}$ |
| **13 - 22** | Namespace ID | 10B | `ED D1 E0 01 02 03 04 05 06 07` | 10 Bytes | 10 Byte Namespace ID (`0xEDD1E001020304050607`) |
| **23 - 28** | Instance ID | 6B | `01 02 03 04 05 03` | 6 Bytes | 6 Byte Instance ID (`0x010203040503`) |
| **29 - 30** | RFU | 2B | `00 00` | 2 Bytes | Reserved for future use (`0x0000`) |

---

### B. Scan Response Packet (`SCAN_RSP`) untuk Slot 2
Memancarkan Nama Perangkat.

| Offset Byte | Field Name | Ukuran | Contoh Hex | Tipe Data | Keterangan & Rumus Parsing |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **0** | Length | 1B | `0x0B` | uint8 | Panjang data (11 byte) |
| **1** | AD Type | 1B | `0x09` | uint8 | `0x09` = Complete Local Name |
| **2 - 11** | Device Name | 10B | `42 58 50...` | ASCII | `"BXP-B-D_E5"` |

---

## 6. Contoh Kode Parser C/C++ Siap Pakai

Berikut adalah implementasi struktur C/C++ dan fungsi parser yang dapat langsung Anda gunakan pada proyek **`esp32_nrf_parsing`** atau gateway scanner nRF52840 Anda:

```cpp
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Struktur Data Hasil Parsing Lengkap MOKO MK Button
typedef struct {
    // Info Paket
    bool has_alarm_frame;
    bool has_device_info;
    bool has_ibeacon;
    bool has_uid;

    // Slot 0 Alarm (0xFEE0)
    uint8_t frame_type;         // 0x20=Single, 0x21=Double, 0x22=Long
    bool password_enabled;      // Bit 0
    bool is_triggered;          // Bit 1
    uint16_t trigger_count;
    char device_id[13];         // Hex string (e.g. "000101")
    char device_name[32];

    // Slot 0 Device Info / Scan Response (0xEA00)
    int16_t accel_x_mg;
    int16_t accel_y_mg;
    int16_t accel_z_mg;
    float temperature_c;
    int8_t ranging_data;
    uint16_t battery_mv;        // jika > 100 mV, jika <= 100 %
    uint8_t mac[6];
    int8_t tx_power_dbm;

    // Slot 1 iBeacon
    uint8_t ibeacon_uuid[16];
    uint16_t ibeacon_major;
    uint16_t ibeacon_minor;
    int8_t ibeacon_rssi_1m;

    // Slot 2 Eddystone-UID
    uint8_t uid_namespace[10];
    uint8_t uid_instance[6];
    int8_t uid_rssi_0m;
} MokoParsedData;

// Fungsi Parser Utama MOKO BLE Raw Payload
void parse_moko_ble_payload(const uint8_t *raw, size_t len, MokoParsedData *out) {
    if (!raw || !out || len < 2) return;

    size_t idx = 0;
    while (idx < len) {
        uint8_t block_len = raw[idx];
        if (block_len == 0 || (idx + 1 + block_len) > len) break;

        uint8_t ad_type = raw[idx + 1];
        const uint8_t *data = &raw[idx + 2];
        uint8_t data_len = block_len - 1;

        // 1. Service Data 16-bit UUID (0x16)
        if (ad_type == 0x16 && data_len >= 2) {
            uint16_t uuid16 = data[0] | (data[1] << 8); // Little Endian

            // 1a. FEE0: Alarm Frame
            if (uuid16 == 0xFEE0 && data_len >= 8) {
                out->has_alarm_frame = true;
                out->frame_type = data[2];
                out->password_enabled = (data[3] & 0x01) != 0;
                out->is_triggered = (data[3] & 0x02) != 0;
                out->trigger_count = (data[4] << 8) | data[5];

                // Device ID (bytes 6..N-2)
                size_t dev_id_bytes = (data_len >= 10) ? (data_len - 8) : 3;
                if (dev_id_bytes > 6) dev_id_bytes = 6;
                char hex_tmp[16] = {0};
                for (size_t i = 0; i < dev_id_bytes; i++) {
                    snprintf(&hex_tmp[i*2], 3, "%02X", data[6 + i]);
                }
                strncpy(out->device_id, hex_tmp, sizeof(out->device_id) - 1);
            }
            // 1b. EA00: Device Info (Scan Response)
            else if (uuid16 == 0xEA00 && data_len >= 21) {
                out->has_device_info = true;
                out->accel_x_mg = (int16_t)((data[4] << 8) | data[5]);
                out->accel_y_mg = (int16_t)((data[6] << 8) | data[7]);
                out->accel_z_mg = (int16_t)((data[8] << 8) | data[9]);

                int16_t raw_temp = (int16_t)((data[10] << 8) | data[11]);
                out->temperature_c = raw_temp * 0.1f;

                out->ranging_data = (int8_t)data[12];
                out->battery_mv = (data[13] << 8) | data[14];
                memcpy(out->mac, &data[15], 6);
            }
            // 1c. FEAA: Eddystone-UID
            else if (uuid16 == 0xFEAA && data_len >= 18 && data[2] == 0x00) {
                out->has_uid = true;
                out->uid_rssi_0m = (int8_t)data[3];
                memcpy(out->uid_namespace, &data[4], 10);
                memcpy(out->uid_instance, &data[14], 6);
            }
        }
        // 2. Manufacturer Specific Data (0xFF) -> Apple iBeacon
        else if (ad_type == 0xFF && data_len >= 25) {
            uint16_t mfg_id = data[0] | (data[1] << 8);
            if (mfg_id == 0x004C && data[2] == 0x02 && data[3] == 0x15) {
                out->has_ibeacon = true;
                memcpy(out->ibeacon_uuid, &data[4], 16);
                out->ibeacon_major = (data[20] << 8) | data[21];
                out->ibeacon_minor = (data[22] << 8) | data[23];
                out->ibeacon_rssi_1m = (int8_t)data[24];
            }
        }
        // 3. Complete Local Name (0x09)
        else if (ad_type == 0x09 && data_len > 0) {
            size_t cpy_len = (data_len < sizeof(out->device_name) - 1) ? data_len : (sizeof(out->device_name) - 1);
            memcpy(out->device_name, data, cpy_len);
            out->device_name[cpy_len] = '\0';
        }
        // 4. Tx Power Level (0x0A)
        else if (ad_type == 0x0A && data_len >= 1) {
            out->tx_power_dbm = (int8_t)data[0];
        }

        idx += (1 + block_len);
    }
}
```
