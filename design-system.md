# IOTARU ESP32 Gateway Design System

## 1. Color Palette & Semantic Tokens
- **Background Deep**: `#060b13` / `#0b1322`
- **Surface Bento Card**: `rgba(15, 23, 42, 0.75)` with `backdrop-filter: blur(12px)`
- **Border**: `rgba(255, 255, 255, 0.08)` / Focused: `#38bdf8` (`rgba(56, 189, 248, 0.35)`)
- **Primary Accent**: `#38bdf8` (Cyan Neon), `#0284c7` (Sky Deep)
- **Text Primary**: `#f8fafc` (White)
- **Text Muted**: `#94a3b8` (Slate 400)
- **Success**: `#22c55e` (Emerald)
- **Warning**: `#fbbf24` (Amber)
- **Danger**: `#ef4444` (Rose)

## 2. Typography
- **UI Font**: system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", sans-serif
- **Mono / Numeric Font**: `Consolas`, `Fira Code`, `SF Mono`, `monospace` (Tabular figures)
- **Section Headers**: `1.05rem`, font-weight `700`, letter-spacing `0.3px`
- **Labels**: `0.85rem` - `0.88rem`, font-weight `600`, `#94a3b8`

## 3. Bento Card Layout
- `display: flex; flex-direction: column; gap: 20px;` on `#settingsValidation`.
- Mobile padding: `padding: 16px 14px calc(var(--bottom-nav-h) + 85px) 14px;` ensuring sticky dock never obscures content.
- Clean hierarchy: No redundant `.bento-sub` sub-headers or repetitive `<small>` field hints.

## 4. Subtopic Architecture (Lock to Device ID)
- Replaced manual `id_module` text input with an immutable, glowing subtopic pill: `beacon/%device_id%`.
- Subtopic Prefix: `#94a3b8` (`beacon/`), Hardware Device ID: `#38bdf8` (`1a2b3c4d`).

## 5. Daily Reboot Schedule Toggle
- iOS tactile toggle switch (`#restartScheduleToggle`) coupled with container `#restartScheduleContainer`.
- When toggled OFF: container gains `.dimmed` (`opacity: 0.35; filter: grayscale(0.7); pointer-events: none;`) and submits empty string `""` so ESP32 clears scheduled reboot.

## 6. RSSI Minimum Threshold & Slider Control (2026 Modern Ergonomics)
- **Unified Seamless Input Pill (`.rv` / `#rssiPill`)**:
  - **Single Outer Container**: Berperan sebagai wadah tunggal dengan `background: var(--input-bg, #090d16)`, `border: 1px solid var(--input-border, #1a2333)`, `border-radius: 8px`, `height: 38px`, dan padding horizontal `12px`.
  - **FOUC & Specificity Conflict Resolution**: Input `#rssi_input_abs` diberikan reset absolut (`all: unset !important; background: transparent !important; border: none !important; outline: none !important; box-shadow: none !important; padding: 0 !important;`) serta didefinisikan pada Critical Pre-paint CSS (`settings.html`) dan `app.css`. Hal ini mencegah rule umum `input[type="text"]` (yang memiliki spesifisitas sama `0-1-1`) menimpa input dan memunculkan kotak nested "box-in-a-box" yang terkesan seperti FOUC/glitch.
  - **Active State / Focus-within**: Ketika input `#rssi_input_abs` aktif/fokus, seluruh kapsul (`.rv:focus-within`) menyala dengan ring cyan lembut (`border-color: #38bdf8; box-shadow: 0 0 0 2px rgba(56, 189, 248, 0.25)`), tanpa ada outline ganda pada input internal.
  - **Prefix & Suffix Alignment**: Tanda minus (`&minus;`) berwarna cyan `#38bdf8` (`font-weight: 700`), digit angka monospaced (`IBM Plex Mono` / `Consolas`), dan unit `<small>dBm</small>` tersusun sejajar secara vertikal (`align-items: center; gap: 4px;`) membentuk satu kontrol numerik utuh berstandar enterprise.
  - Monospace numeric input dengan dynamic character width (`style.width = len + 'ch'`) memastikan angka pas tanpa ruang kosong berlebih.
- **Range Slider Bounds**:
  - `min="30" max="100" step="1"`.
  - Native track content-box override: `padding: 0 !important; margin: 8px 0 4px 0 !important; border: none !important; height: 8px !important;` to eliminate Chromium padding bleed.
  - Thumb: `20px` diameter circular pill with `#38bdf8` background and `2px solid #ffffff` border, perfectly centered vertically (`margin-top: -6px`).
  - At `val = 30`: Thumb touches the exact left boundary directly over `-30 dBm`.
  - At `val = 100`: Thumb touches the exact right boundary directly over `-100 dBm` with 0px background bleed.
- **Clean Ticks**:
  - Display strictly `-30`, `-65`, `-100`. Removed redundant `(Dekat)` / `(default)` and `(Maksimal)` text.
  - `#rssiCatBadge` completely removed.
- **Strict Auto-Clamping**:
  - If user types `< 30` (e.g. `20`, `15`): instantly clamped to `30`. Single digits or `10` clamped to `30` on blur/debounce.
  - If user types `> 100` (e.g. `120`, `150`): instantly clamped back to `100`.
  - Valid range `30..100`: synced real-time to slider, hidden input (`-val` for ESP32 backend), and active fill gradient.

## 7. Hybrid Save Experience (Primary Action Button & Reactive Floating Save Dock)
- **Primary Bottom Action Button (`#btnMainSave`)**:
  - Posisi: Terletak di bagian bawah formulir setelah Bento Card terakhir (`.form-actions-bottom`).
  - Styling: `.btn .btn-info` dengan warna aksen cyan `var(--primary)`, ikon disket simpan, dan bayangan glow.
  - Tujuan: Menjamin pengguna selalu melihat tombol simpan yang jelas dan permanen tanpa kebingungan saat menggulir ke akhir halaman pengaturan.
- **Reactive Floating Save Dock (`#floatingSaveDock`)**:
  - Posisi: Sticky/fixed dock melayang di atas bottom navigation bar (`bottom: 74px`, z-index 990).
  - Dirty tracking reaktif: Muncul secara otomatis (animasi dockSlideUp) saat input/form mengalami perubahan dibandingkan nilai awal (`isFormDirty() = true`).
  - Elemen: Indikator pulsing amber dot (*Belum disimpan*), tombol *Batal* (`#btnCancelChanges`) untuk me-rollback form, dan tombol *Simpan* (`#btnDockSave` dengan atribut HTML5 `form="settingsValidation"`).
- **Sinkronisasi Event**:
  - Kedua tombol memicu submit event ke form `#settingsValidation` yang divalidasi dan dikirim via AJAX `handleFormSubmit` ke `/api/v1/config/settings`.

## 8. Reboot Workflow & Session Lifecycle Security
- **Reboot Feedback Modal**:
  - Saat perintah reboot dikirim ke ESP32 (`/api/v1/system/reboot`), antarmuka memunculkan SweetAlert:
    - **Title**: `Me-reboot...`
    - **Text**: `Perangkat sedang memulai ulang`
    - **Icon**: `info`
    - **Action Button**: `OK` (`#0284c7`)
  - Ketika pengguna menekan **OK**, browser langsung dialihkan (`window.location.href`) ke halaman login (`/login.html`).
- **Active Session Route Guard (Proteksi Halaman Login)**:
  - **Server-Side Protection (`src/main.cpp`)**: Rute `/login` dan `/login.html` memvalidasi cookie sesi (`validateSession(request)`). Jika sesi pengguna masih aktif (`role != ROLE_NONE`), server otomatis me-redirect (HTTP 302) pengguna ke halaman Overview (`/`).
  - **Client-Side Protection (`data/login.html`)**: Ditambahkan pre-check asynchronous (`/api/v1/auth/check`). Jika sesi terdeteksi valid (HTTP 200), halaman seketika melakukan `window.location.replace("/")` sebelum form login sempat digunakan.

## 9. Filter Major Tag Component & Numeric Bounds Validation
- **Komponen Kontrol**:
  - **Saklar Taktil ON/OFF (`#filterMajorToggle`)**:
    - Default state: **OFF** (tidak aktif).
    - Ketika switch OFF: Kontainer input `#majorContainer` diberi class `.dimmed` (opasitas 0.5, pointer-events none), input Target Major secara fisik dinonaktifkan (`disabled = true`, styling `cursor: not-allowed; opacity: 0.55`), tidak bersifat wajib (`required = false`), dan gateway menerima semua paket tanpa filter Major. Ketika saklar dinyalakan (ON), input otomatis di-enable kembali (`disabled = false`).
    - Ketika switch ON: Kontainer input aktif normal (`required = true`).
  - **Input Target Major (`#filter_major`)**:
    - **Placeholder**: `"65535"`.
    - **Max Length**: Dibatasi maksimal 5 digit (`maxlength="5"`, `inputmode="numeric"`).
    - **Rentang Angka & Auto-Clamp**: Nilai valid `0` s/d `65535`. Jika pengguna memasukkan nilai di atas `65535` (misal `70000`), sistem langsung melakukan auto-clamp kembali ke `65535`.
    - **Bebas Angka Nol di Depan**: Pengguna dapat langsung mengetik `1` untuk mencocokkan tag Major `00001` atau `1`.
  - **Badge Validasi Dinamis (`#majorBadge`)**:
    - Status kosong/default: `"uuid-badge neutral"` (*"0 - 65535"*).
    - Status valid: `"uuid-badge valid"` (*"Major [nilai] Valid"*).
    - Status melebihi batas: `"uuid-badge invalid"` (*"Maks 65535"*).
- **Backend NVS Persistence**:
  - Namespace: `"gateway"`.
  - Kunci: `major_en` (u32, default 0), `major` (string, default "").

## 10. Three-Tier System Versioning Architecture
Pemisahan arsitektur versioning gateway agar setiap layer dapat di-update secara independen:
1. **Device Version (Hardware / Model Profile)**:
   - **Penyimpanan**: Tertanam di **NVS** (Preferences namespace `"gateway"`, kunci `dev_ver`).
   - **Provisioning**: Didefinisikan pada `nvs_config.csv` (`dev_ver,data,string,v1.0.0`) dan dimuat saat boot (`config.device_version = prefs.getString("dev_ver", "v1.0.0");`).
2. **Firmware Version (Core Engine)**:
   - **Penyimpanan**: Variabel di kode **C++** (`const char *FIRMWARE_VERSION = "v1.0.0";` pada `src/main.cpp`).
   - **Kompilasi**: Terikat langsung dengan build binary firmware `.bin`.
3. **UI Version (Frontend Asset)**:
   - **Penyimpanan**: File JSON di filesystem **LittleFS** (`data/version.json` dengan struktur `{"version": "v1.0.0"}`).
   - **Pemuatan**: Dibaca secara dinamis saat boot via `loadUiVersion()` menggunakan `ArduinoJson`.
   - **Independensi OTA**: Update antarmuka web melalui OTA LittleFS filesystem seketika memperbarui nomor versi UI tanpa memerlukan flashing ulang firmware C++.
4. **REST API Endpoint**:
   - `GET /api/v1/system/version` mengembalikan payload JSON lengkap:
     ```json
     {
       "device_version": "v1.0.0",
       "firmware_version": "v1.0.0",
       "ui_version": "v1.0.0"
     }
     ```

## 6. Hardware LED Indicator Design System (WS2812B)
- **LED 1 [SYS] (Status Sistem & Firmware):**
  - `SYS_LED_NORMAL`: `#22c55e` (Hijau Solid) — Sistem dan RTOS berjalan stabil.
  - `SYS_LED_BOOTING`: `#eab308` (Kuning Kedip 2 Hz) — Inisialisasi hardware dan NVS.
  - `SYS_LED_OTA`: `#f97316` (Oranye Fast Strobe 10 Hz) — Flashing partisi firmware OTA.
  - `SYS_LED_FAULT`: `#ef4444` (Merah Solid) — Kerusakan NVS/LittleFS atau proses reset.
- **LED 2 [COM] (Status Jaringan & Komunikasi):**
  - `COM_LED_NORMAL`: `#22c55e` (Hijau Solid + 35ms Blip Padam) — Jaringan & MQTT terhubung normal.
  - `COM_LED_SOFTAP`: `#3b82f6` (Biru Kedip 1 Hz) — **Mode Hotspot Setup / Config AP Aktif** (dipicu via Tombol GPIO 14 > 5s atau SSID kosong).
  - `COM_LED_MQTT_OFFLINE`: `#06b6d4` (Cyan Kedip 1 Hz) — Jaringan lokal terhubung, tetapi broker MQTT offline.
  - `COM_LED_CONNECTING`: `#eab308` (Kuning Kedip 2 Hz) — Mencari WiFi atau DHCP Ethernet.
  - `COM_LED_DISCONNECTED`: `#ef4444` (Merah Double Blink) — Sambungan jaringan terputus (Watchdog aktif).
- **Spesifikasi Driver Hardware (Bitbang Presisi Tinggi Direct Register):**
  - Driver: Memory-mapped register IO langsung (`GPIO_OUT_W1TS_REG` & `GPIO_OUT_W1TC_REG`).
  - Bit 0: T0H = 84 siklus CPU (~350 ns), T0L = 204 siklus CPU (~850 ns).
  - Bit 1: T1H = 192 siklus CPU (~800 ns), T1L = 96 siklus CPU (~400 ns).
  - Reset Latch: Pulsa LOW > 350 µs (`esp_rom_delay_us(350)`).
  - Isolasi Core: Dijalankan di Core 1 dalam critical section interrupt `portMUX_TYPE`.


## 7. Hotspot AP Config Mode Banner & Controls
- **Card Container (`.ap-banner-card`):**
  - Background: `linear-gradient(135deg, rgba(15, 23, 42, 0.9) 0%, rgba(30, 41, 59, 0.8) 100%)`
  - Border: `1px solid rgba(59, 130, 246, 0.35)`
  - Backdrop Blur: `blur(12px)`
  - Glow Shadow: `0 8px 24px rgba(2, 132, 199, 0.12)`
  - Border Radius: `12px`
- **Pulsing Status Indicator (`.ap-pulse-ring`, `.ap-pulse-dot`):**
  - Dot: `#0284c7` dengan outer glow `#38bdf8`
  - Pulse Wave: `#38bdf8` (skala 0.95 -> 2.4, opacity 0.8 -> 0) irama 1.6s
- **Countdown Badge (`.ap-timer-pill`):**
  - Font: `monospace` 11.5px semi-bold
  - Background: `rgba(56, 189, 248, 0.15)`
  - Color: `#38bdf8`
  - Border: `1px solid rgba(56, 189, 248, 0.3)`
- **Action Button (`.btn-ap-stop`):**
  - Warning Accent: `rgba(249, 115, 22, 0.15)`
  - Border: `rgba(249, 115, 22, 0.4)`
  - Text: `#fb923c`
  - Hover: `rgba(249, 115, 22, 0.28)`, glow `0 4px 12px rgba(249, 115, 22, 0.25)`

### Live Test Monitor Beacon Badges
- Standard iBeacon Badge: `.badge-beacon.ibeacon` (Cyan `#38bdf8`, background `rgba(6, 182, 212, 0.15)`)
- Extended iBeacon Badge: `.badge-beacon.ibeacon_ext` (Purple `#c084fc`, background `rgba(168, 85, 247, 0.15)`)
- Legacy Fallback: `.badge-beacon.minew` (Mirrors `.badge-beacon.ibeacon_ext`)

### Network & MQTT Status Indicators (Overview Dashboard)
- Status Online / Connected: `.badge-tech.conn`, `.badge-tech.mqtt-online`
  - Background: `rgba(16, 185, 129, 0.15)`
  - Text Color: `#34d399` (Emerald 400)
  - Border: `1px solid rgba(16, 185, 129, 0.3)`
- Status Offline / Disconnected: `.badge-tech.offline`, `.badge-tech.mqtt-offline`
  - Background: `rgba(239, 68, 68, 0.15)`
  - Text Color: `#f87171` (Rose 400)
  - Border: `1px solid rgba(239, 68, 68, 0.3)`

## 7. Dynamic Device Model Branding (%model% & %ap_ssid%)
- **Template Processor Tokens**:
  - `%model%`: Dynamically injected from NVS `dev_model` (e.g. `SG-10`), with fallback to `EMPTY`. Used in `<title>`, `.machine-title`, `.login-title`, and footer copyright.
  - `%ap_ssid%`: Dynamically composed as `<device_model>-<device_id>` (e.g. `SG-10-bb5c7bcc`).
  - Web UI pages (`index.html`, `login.html`, `settings.html`, `test.html`, `firmware.html`, `auth_admin.html`, `auth_teknisi.html`) are 100% decoupling from hardcoded model strings.

## 8. Industrial Network Security & DNS Configuration Components
- **Primary & Secondary DNS Controls:**
  - Token IDs: `#w_dns`, `#w_dns2` (Wi-Fi), `#eth_dns`, `#eth_dns2` (Ethernet).
  - Primary DNS Fallback: Jika pengguna mengosongkan DNS pada mode statis, sistem menetapkan IP Gateway secara transparan.
  - Secondary DNS Optional: Placeholder `Kosongkan jika tanpa DNS sekunder` / `Kosongkan jika pakai DNS jaringan`.
- **Hotspot Mode Konfigurasi (SoftAP) Bento Card (Admin Only):**
  - Card Icon Accent: Indigo (`.bento-icon.indigo`, `rgba(99, 102, 241, 0.15)`, `#818cf8`)
  - SSID Display (`#ap_ssid_display`): Read-only styled with 75% opacity and disabled cursor.
  - WPA2 Password Control (`#ap_password`): Password input with toggle show/hide eye button, enforce `minlength="8"` for standard WPA2 PSK security compliance.
  - Access Control: Field hanya dapat dikonfigurasi melalui sesi Administrator (`auth_admin.html`) dan diverifikasi di sisi backend C++ (`ROLE_ADMIN`).

## 9. Industrial Ethernet Diagnostics & Telemetry Indicators
- **Running Mode Badge (#badgeConnMode):**
  - Pada mode Ethernet aktif dan terhubung, badge bertransformasi menampilkan format: \Ethernet (100M Full)\ atau \Ethernet (10M Half)\.
  - Status kelas: \.badge-tech.conn\ (Hijau / Emerald) saat IP diperoleh, \.badge-tech.offline\ (Merah / Rose) saat kabel terputus.
- **Dual-Interface Network Specs Grid Layout (Wi-Fi & Ethernet Indipenden):**
  - **IP Address (Wi-Fi) Tile (`#valWifiIp`):**
    - Warna: `#34d399` (Emerald 400), Font: Monospace.
    - Menampilkan alamat IP koneksi Wi-Fi saat terhubung (`192.168.x.x`) atau `-` saat terputus.
  - **MAC Address (Wi-Fi) Tile (`#valWifiMac`):**
    - Warna: `#38bdf8` (Sky 400), Font: Monospace.
    - Menampilkan alamat MAC fisik antarmuka nirkabel Wi-Fi dari eFuse ESP32.
  - **IP Address (Ethernet) Tile (`#valEthIp`, Container `#tileEthIp`):**
    - Warna: `#34d399` (Emerald 400) saat terhubung, Muted Gray `#94a3b8` saat kabel terputus.
    - Otomatis adaptif: Menampilkan IP Router LAN (`192.168.1.x`), IP Direct PC (`192.168.4.1`), atau status `Kabel Terputus (Link Down)`.
    - Auto-Hide: Kontainer tile otomatis disembunyikan (`display: none;`) jika hardware kontroler Ethernet W5500 tidak terdeteksi.
  - **MAC Address (Ethernet) Tile (`#valEthMac`, Container `#tileEthMac`):**
    - Warna: `#38bdf8` (Sky 400), Font: Monospace.
    - Menampilkan alamat MAC perangkat keras Ethernet W5500.
    - Auto-Hide: Kontainer tile disembunyikan bersamaan jika hardware Ethernet uninstalled.
- **Ethernet Physical Link Tile (#labelSignalStrength, #valSignalStrength):**
  - Menggantikan tampilan RSSI nirkabel dengan telemetri kabel fisik RJ-45: 100 Mbps (Full Duplex) atau Kabel Terputus (Link Down).
  - Memberikan visibilitas langsung kepada teknisi IT/OT pabrik untuk mendeteksi kabel rusak atau Duplex Mismatch switch.

## 10. Industrial Staging & Pending Reboot Architecture
- **Floating Pending Reboot Banner (.pending-reboot-banner):**
  - Subtle Amber/Gold alert banner with dark glassmorphism:
    - Background: `linear-gradient(135deg, rgba(245, 158, 11, 0.15) 0%, rgba(217, 119, 6, 0.1) 100%)`
    - Border: `1px solid rgba(245, 158, 11, 0.35)`
    - Box Shadow: `0 8px 24px rgba(217, 119, 6, 0.15)`
    - Text: `#fde68a` (Amber 200) dengan icon peringatan `#fbbf24` (Amber 400).
  - Tombol Tindakan Cepat:
    - `.btn-banner-reboot`: Emerald green `#10b981` (hover `#059669`) untuk eksekusi restart bersih (`POST /api/v1/system/reboot`).
    - `.btn-banner-discard`: Rose tint `rgba(239, 68, 68, 0.15)` (hover `rgba(239, 68, 68, 0.28)`) untuk rollback pembatalan (`POST /api/v1/config/network/discard`).
- **Transparansi Pembatalan (Rollback Modal Dialog):**
  - Modal konfirmasi SweetAlert menampilkan rincian: Mode Jaringan dan SSID yang dikembalikan ke konfigurasi aktif.
  - Penegasan visual eksplisit: Kredensial akun master/teknisi dan MQTT tetap tersimpan aman tanpa ikut terhapus.

## 11. Emergency Recovery UI (PROGMEM Self-Contained System)
- **Zero-Dependency Architectural Constraint:**
  - Antarmuka darurat mandiri (*100% self-contained*) tersimpan di C++ Flash `PROGMEM` (`src/web/EmergencyRecoveryPage.h`).
  - Tidak memuat pustaka eksternal (CDN/Google Fonts/SweetAlert/FontAwesome) maupun aset LittleFS (`/assets/...`) karena partisi LittleFS diasumsikan rusak, korup, atau belum terpasang.
  - Ikon menggunakan SVG inline murni, font mengandalkan *native system UI font stack*, dan komunikasi HTTP menggunakan Vanilla JavaScript `XMLHttpRequest`.

- **Color Tokens & Aesthetic Hierarchy:**
  - **Background Gradient:** `radial-gradient(ellipse at 50% 0%, #172554 0%, #070c14 75%)` (Nuansa industri darurat/dark cybernetic).
  - **Emergency Banner Card:**
    - Background: `rgba(239, 68, 68, 0.08)`
    - Border: `1px solid rgba(239, 68, 68, 0.35)`
    - Box Shadow: `0 4px 20px rgba(239, 68, 68, 0.15)`
    - Accent: Danger Rose `#ef4444` & Amber Warning `#fbbf24`.
  - **Bento Card Surface:**
    - Background: `rgba(15, 23, 42, 0.85)` dengan `backdrop-filter: blur(16px)`
    - Border: `1px solid rgba(255, 255, 255, 0.08)`
    - Radius: `16px`
  - **Telemetri Badges (`.tele-badge`):**
    - Background: `rgba(255, 255, 255, 0.06)`
    - Label Muted: `#94a3b8`, Value Bright: `#f8fafc` (Monospace untuk IP & ID perangkat).

- **Form & Interactive Components:**
  - **Input File Upload Container (`.file-dropzone`):**
    - Border: `2px dashed rgba(56, 189, 248, 0.3)`
    - Hover / Active: `rgba(56, 189, 248, 0.6)` dengan glow halus.
  - **Primary Action Button (Restore LittleFS):**
    - Background: `linear-gradient(135deg, #06b6d4 0%, #0284c7 100%)`
    - Text: `#ffffff` bold, Shadow: `0 4px 14px rgba(6, 182, 212, 0.3)`
  - **Secondary Action Button (Flash Firmware Bin):**
    - Background: `linear-gradient(135deg, #475569 0%, #334155 100%)`
    - Border: `1px solid rgba(255, 255, 255, 0.1)`
  - **Real-Time Progress Bar (`.progress-bar`):**
    - Track: `rgba(255, 255, 255, 0.08)`
    - Fill Gradient: `linear-gradient(90deg, #06b6d4 0%, #10b981 100%)` dengan transisi halus `width 0.2s ease`.
  - **Emergency Log Console (`#recoveryLog`):**
    - Background: `#030712` (OLED Pure Black)
    - Font: `Consolas, Menlo, Monaco, monospace`
    - Teks: Emerald `#4ade80` untuk log sukses, Amber `#fde047` untuk progres, Rose `#f87171` untuk kegagalan.

- **Security & Authentication Bridge:**
  - Form darurat dilengkapi verifikasi password Administrator (`admin_password`).
  - Sebelum transfer binary diinisiasi via endpoint OTA `/api/v1/ota/upload`, JS melakukan *pre-authentication handshake* ke `/api/v1/auth/login` untuk mendapatkan cookie sesi admin resmi (`IOTARU_SESSION`), mencegah pengunggahan binary oleh pihak tidak berwenang saat perangkat dalam mode pemulihan.

## 12. Industrial Bento Drag-and-Drop File Upload Components (.bento-dropzone)
- **Aesthetic Consistency & Unified Blue Theme Accent:**
  - Kedua kotak upload berkas biner (Firmware program dan Filesystem LittleFS) diselaraskan menggunakan aksen warna **Biru Tema IOTARU** yang identik (`#38bdf8` / `#0284c7`), selaras dengan identitas visual bento dark-glassmorphism.
  - Border normal: `2px dashed rgba(56, 189, 248, 0.35)` dengan background `rgba(15, 23, 42, 0.55)`.
  - Border radius: `14px`, padding: `24px 18px`.

- **Interactive Hover & Dragover States:**
  - **Hover State:** Border bertransformasi menjadi solid glowing `#38bdf8`, background `rgba(56, 189, 248, 0.08)`, elevasi `translateY(-2px)`, dan bayangan `box-shadow: 0 0 20px rgba(56, 189, 248, 0.15)`.
  - **Dragover State (`.dragover`):** Dipicu saat kursor menyeret file ke atas zona drop. Background menguat menjadi `rgba(56, 189, 248, 0.14)`, border menyala terang, dan skala membesar halus `scale(1.01)`.
  - **Dropzone Icon (.dropzone-icon):** Kotak ikon 44x44px berlatar `rgba(56, 189, 248, 0.12)` dengan warna `#38bdf8`, membesar secara dinamis saat hover/dragover.

- **Dynamic File Selection Chip (.dropzone-selected-box):**
  - Menggantikan tampilan teks idle dengan kotak status berkas terpilih:
    - Background: `rgba(56, 189, 248, 0.1)`
    - Border: `1px solid rgba(56, 189, 248, 0.4)`
    - Radius: `10px`, animasi masuk `dropzoneFadeIn 0.3s ease`.
    - Menampilkan nama file (`.dropzone-filename`, teks putih tebal, ellipsis untuk nama panjang) dan ukuran berkas (`.dropzone-filesize`, monospace font).
    - Tombol Hapus/Ganti (`.dropzone-remove-btn`): Ikon silang yang berubah menjadi merah (`#ef4444`) saat di-hover untuk membatalkan berkas terpilih secara instan.

- **Unified Pre-Flight Binary Signature & Hardware Identity Verification (Firmware & LittleFS):**
  - **Dual Hardware Identity Validation (MODEL@BOARD_VERSION):**
    - Field `project_name` pada offset `0x50` (32 bytes) memuat format gabungan `MODEL@BOARD_VERSION` (misal: `SG-10@v1.0.0`).
    - **Model Mismatch Rejection:** Berkas ditolak seketika jika model pada biner tidak sama dengan model perangkat aktif (`CURRENT_DEVICE_MODEL`).
    - **Board Revision Mismatch Rejection:** Berkas ditolak seketika jika revisi board/PCB pada biner tidak kompatibel dengan revisi board fisik perangkat (`CURRENT_DEVICE_VERSION`).
  - **Pre-Flash Metadata Confirmation Dialog (SweetAlert Industrial Bento):**
    - Saat tombol "Unggah & Update Firmware" diklik, sistem menampilkan dialog konfirmasi yang merinci:
      - Device Model (misal: `SG-10`)
      - Board Version (misal: `v1.0.0`)
      - Firmware Version (misal: `v1.0.0-rc2`)
      - Tanggal Firmware (misal: `Mar 23 2026`, diekstrak dari offset `0x80`, tanpa jam).
  - **Partition Table Rejection (partitions.bin):** Offset 0x00..0x01 diperiksa terhadap magic number tabel partisi ESP32 `0x50AA` (`0xAA 0x50`) dan ditolak seketika.
  - **Firmware App Image Validation (firmware.bin vs bootloader.bin):**
    - Offset 0x00 mutlak `0xE9`.
    - Offset 0x20..0x23 mutlak `0xABCD5432` (`esp_app_desc_t`).
    - Berkas `bootloader.bin` ditolak seketika.
  - **LittleFS Filesystem Positive Validation (littlefs.bin):**
    - Menolak jika diawali `0xE9` atau `0x50AA`.
    - Offset 0x08..0x0F mutlak bernilai teks ASCII `"littlefs"`.
  - **Backend C++ Hard-Rejection Guard (`WebManager.cpp`):** Verifikasi identik di level C++ pada byte paket pertama (`index == 0`) untuk memblokir stream paket yang salah model, salah revisi board, atau corrupt.

## 11. Test Page Live Beacon Filter & Unique Beacon Table Pool
- **Input Filter Realtime (`#tagFilterInput`)**:
  - **Placeholder**: `Filter Major, Minor, atau Format (mis: 1-2, 100)...` (menghilangkan janji pencarian MAC address yang tidak didukung frame radio nRF).
  - **Smart Separator Matching**:
    - Mendukung format pasangan Major-Minor dengan pemisah `-`, `:`, atau `/` (contoh: `1-2`, `10-20`).
    - Mengonversi masing-masing bagian ke integer sehingga bebas dari distorsi *zero-padding* 5 digit (`00001-00002`).
    - Mendukung filter separuh pasangan (contoh: `1-` untuk Major 1 saja, atau `-2` untuk Minor 2 saja).
    - Mendukung angka tunggal (contoh: `5` mencocokkan Major 5 ATAU Minor 5).
    - Mendukung pencarian teks format/protokol (`ibeacon`, `ext`, `ibeacon_ext`).
- **Unique Beacon Pool Architecture (`test.js`)**:
  - Menggantikan raw stream array overwrite (`maxHistory = 200 raw packets`) dengan **Unique Beacon Pool** (`Map<tag, item>` dan `Array<item>`).
  - Setiap paket baru memperbarui metrik RSSI, Battery, dan `lastSeen` milik beacon bersangkutan secara in-place, lalu memindahkannya ke posisi teratas (most recently active).
  - Mencegah fenomena beacon hilang seketika dalam 1–2 detik saat frekuensi transmisi nRF tinggi (100–300 pkt/s).
  - Saat filter aktif, tabel menampilkan hingga 10 beacon unik paling mutakhir yang lolos kriteria filter.
  - **Kolom Waktu / Timestamp (Client-Side JS)**:
    - Ditambahkan kolom `Waktu` (`<th>Waktu</th>`) di posisi paling kanan tabel.
    - Diformat sebagai `HH:mm:ss` secara eksklusif di JavaScript browser (`formatTime(b.lastSeen)` menggunakan `new Date()`).
    - **Nol beban pada ESP32**: Tidak membutuhkan sinkronisasi NTP, modul RTC, maupun payload tambahan pada WebSocket gateway. Setiap paket yang diterima langsung dicatat waktu kedatangannya di browser pengguna.
  - **Arsitektur Continuous Raw Packet Stream (FIFO Maks. 30 Paket)**:
    - Mode deduplikasi unik dinonaktifkan atas kebutuhan inspeksi raw streaming beruntun.
    - Setiap paket yang masuk dari modul nRF dicatat sebagai entri independen baru lengkap dengan timestamp presisi milidetik (`HH:mm:ss.SSS`).
    - Kapasitas buffer browser dibatasi tepat **maksimal 30 paket**.
    - **Filter Bidireksional**:
      - Menyaring paket riwayat yang sudah ada di buffer (maks. 30).
      - Menyaring paket baru yang sedang mengalir masuk: paket yang cocok langsung disisipkan di baris teratas (#1), sedangkan paket yang tidak cocok disimpan di buffer latar belakang tanpa menggeser tampilan filter pengguna.
      - Menghapus input filter seketika menampilkan 10 paket terbaru dari buffer 30 riwayat.
  - **Pruning & Discard Data Non-Matching pada Riwayat Buffer**:
    - Ketika input filter diaktifkan (`filterVal !== ""`), seluruh data riwayat lama di `packetStreamHistory` yang tidak cocok seketika dibuang (`filter()`), mengosongkan slot riwayat hanya untuk paket yang dicari pengguna.
    - Setiap paket baru yang mengalir masuk via WebSocket saat filter aktif akan dicek: jika **tidak cocok**, paket tersebut **langsung dibuang (discard) dan tidak disimpan ke riwayat**.
    - Hasil: Seluruh kuota 30 slot riwayat buffer dialokasikan 100% khusus untuk paket yang lolos filter tanpa risiko terdorong keluar oleh paket lain.
## 12. Throughput HUD 8-Card Grid & 60-Second Realtime Performance Chart (6-Series)
- **8-Card Responsive Throughput HUD (`.throughput-hud`)**:
  - Menyajikan 8 metrik performa kritis gateway secara terpadu:
    1. **Tag Read Speed (`#stat_read_speed`)**: Laju penerimaan paket radio nRF (`pkt/s`).
    2. **Unique Beacons (`#stat_unique_tags`)**: Akumulasi jumlah beacon fisik unik (`Set<tagId>`) yang terbaca sejak sesi pengujian dimulai. Otomatis di-reset ke 0 saat 'Start Test' ditekan.
    3. **MQTT Publish Speed (`#stat_mqtt_speed`)**: Laju pengiriman payload JSON/Binary batch (`batch/s`).
    4. **Queue RTOS Load (`#stat_queue_load`)**: Utilisasi buffer antrian FreeRTOS (`load / 300`).
    5. **Antrian Buffer Batch (`#stat_batch_load`)**: Utilisasi penampung buffer pengiriman batch MQTT (`batch_load / batch_max`, misal `18 / 100`).
    6. **Pkt per Batch (`#stat_batch_pkt_size`)**: Jumlah paket beacon aktual yang berhasil dibundel dan dikirim dalam tiap batch terakhir (`statLastBatchPktCount`, misal `85 pkt` atau `100 pkt`).
    7. **Net Bandwidth (`#stat_net_bandwidth`)**: Kecepatan throughput trafik keluar (egress) ke broker MQTT dalam kilobyte per detik (`KB/s`). Dihitung murni untuk seluruh frame data keluar (Data Batch + Heartbeat/Status + RPC Command + PINGREQ Keepalive + TCP ACK) lengkap dengan enkapsulasi Layer 2 Ethernet (18B) atau Wi-Fi (36B) dan Layer 3/4 TCP/IP (40B).
    8. **Free Heap RAM (`#stat_free_heap`)**: Sisa memori internal SRAM ESP32 (`KB`).
  - **Tata Letak Adaptif / Responsive Grid**:
    - Layar Ultra-Wide (`>= 1320px`): 8 kartu sejajar dalam 1 baris (`grid-template-columns: repeat(8, 1fr)`).
    - Laptop / Desktop Standar (`>= 1024px`): 4 kolom x 2 baris proporsional (`repeat(4, 1fr)`).
    - Tablet (`>= 640px`): 4 kolom x 2 baris (`repeat(4, 1fr)`).
    - Mobile (`< 640px`): 2 kolom x 4 baris (`repeat(2, 1fr)`).
    - Desain kartu: Glassmorphism gelap (`background: rgba(15, 23, 42, 0.7)`), aksen border bercahaya, badge icon svg neon, dan tabular numerals semibold.

- **60-Second Realtime Analysis Chart (Windows Task Manager Style - 6 Multi-Series)**:
  - **Sumbu Waktu Windows Task Manager**:
    - Menampilkan label teks elegan di sumbu horizontal bawah: `'60 seconds'` di ujung kiri dan `'0'` di ujung kanan.
    - Menghilangkan label detik berulang yang padat (seperti `-50s`, `-40s`, dst.), digantikan oleh 6 grid vertikal halus (`rgba(255, 255, 255, 0.05)`) dan 4 grid horizontal penanda ketinggian (0%, 25%, 50%, 75%, 100%).
  - **Visualisasi 6 Metrik Multi-Series**:
    - **Tag Read Speed**: Neon Cyan (`#22d3ee`, glow blur 4px), auto-scale kecepatan (`pkt/s`).
    - **MQTT Publish Speed**: Neon Emerald (`#34d399`), laju transmisi (`batch/s`).
    - **RTOS Queue Load**: Neon Amber (`#fbbf24`), utilisasi buffer antrian (0 - 300).
    - **Batch Buffer Load**: Neon Rose (`#f43f5e`), utilisasi buffer batch (0 - 100).
    - **Pkt / Batch**: Neon Violet (`#818cf8`, fill `rgba(129, 140, 248, 0.10)`), volume paket yang dikirim per batch (0 - `batch_max`).
    - **Net Bandwidth**: Electric Sky (`#38bdf8`, fill `rgba(56, 189, 248, 0.08)`), laju transfer jaringan ke broker MQTT (`KB/s`).
  - **High-DPI Retina Rendering**:
    - Canvas diukur berdasarkan bounding rect CSS dan diskalakan dengan `window.devicePixelRatio` (2x / 3x) untuk ketajaman visual maksimal di layar Retina dan AMOLED tanpa blur.
  - **Interaktivitas Crosshair & Floating Tooltip 6 Baris**:
    - Event `mousemove` memetakan koordinat pointer ke indeks riwayat 60 detik (`-60s` s/d `0s`).
    - Garis crosshair vertikal putih transparan (`rgba(255, 255, 255, 0.4)`) dan titik penanda (bullet dot) di setiap kurva aktif.
    - Floating Tooltip glassmorphism gelap melayang mengikuti kursor, menampilkan detail nilai 6 metrik pada detik bersangkutan secara presisi (termasuk baris `Net Bandwidth: ... KB/s`).
  - **Siklus Hidup (Lifecycle Engine)**:
    - Di-reset dan mulai melakukan *sampling* 1 Hz saat tombol 'Mulai Pengujian' (Start Test) ditekan.
    - Pembekuan visual (*freeze state*) saat 'Hentikan Pengujian' (Stop Test) ditekan untuk memudahkan teknisi melakukan analisa statis.

## 13. Carrier-Grade OTA Lifecycle & Visual Feedback System
- **Status LED SYS Saat Flash OTA**:
  - **SYS_LED_OTA (Oranye Berkedip / Orange Pulse)**: Aktif seketika saat chunk pertama diterima (index == 0) dan partisi flash diinisialisasi via Update.begin().
  - **Auto-Recovery ke SYS_LED_NORMAL (Hijau)**: Jika terjadi kegagalan transmisi, koneksi putus, atau *inactivity watchdog* (10 detik) terpicu, Update.abort() dipanggil dan status LED otomatis dipulihkan kembali ke hijau tanpa restart.
- **OTA Concurrency Lock (HTTP 409 Conflict)**:
  - Mencegah request upload simultan dari beberapa tab atau user berbeda (otaInProgress guard). Request kedua ditolak seketika sehingga stream flash tidak mengalami korupsi.
- **Total Data Silencing Selama OTA (Zero Contention / Zero OOM)**:
  - **UART nRF Ingestion Ditangguhkan**: Alokasi paket beacon ke antrean FreeRTOS distop, buffer FIFO hardware di-drain cepat tanpa memory leak.
  - **MQTT Batch Publisher Ditangguhkan**: Task MQTT di Core 1 di-pause (TaskDelay(200)), mencegah benturan soket TCP dan alokasi buffer.
  - **WebSocket Live Telemetry Ditangguhkan**: Pengiriman array beacon JSON dihentikan sementara, menyisakan jalur transmisi murni untuk progress bar firmware OTA (ota_progress).
- **ESP-IDF Native Hardware Rollback**:
  - Partisi baru berstatus `ESP_OTA_IMG_PENDING_VERIFY` pasca-flash.
  - Hanya jika boot `app_main()` berhasil tuntas hingga akhir, `esp_ota_mark_app_valid_cancel_rollback()` dipanggil.
  - Mencegah *hard-brick* atau *bootloop*: jika firmware baru mengalami crash/panic saat boot, bootloader otomatis kembali ke partisi firmware lama pada reboot berikutnya.

## 14. Native ESP-IDF v6 Architecture & Hardware Pinout Specification
- **Hardware Bus & Pinout Assignment**:
  - **Ethernet W5500 (SPI DMA Hardware Acceleration)**:
    - Host: SPI2 (VSPI) Controller dengan DMA Channel Auto (`SPI_DMA_CH_AUTO`).
    - Bus Speed: 25 MHz (`SPI_MASTER_FREQ_25M`).
    - Pinout: `CS: GPIO 26, SCK: GPIO 18, MISO: GPIO 19, MOSI: GPIO 23, RST: GPIO 15, INT: GPIO 14`.
    - Stack: Native `esp_eth` MAC/PHY driver terintegrasi dengan `esp_netif` lwIP.
  - **BLE Ingestion UART (nRF52840 Co-Processor)**:
    - Port: UART2 pada `1,000,000 bps` (1 Mbps), 8N1.
    - Buffer: 4096-byte Hardware FIFO ring-buffer dengan FreeRTOS Event Queue.
    - Pinout: `RX: GPIO 16, TX: GPIO 17`.
    - Affinity: Terkunci khusus ke **Core 0** (PRO_CPU) untuk latensi sub-mikrodetik tanpa interferensi stack TCP/IP.
  - **WS2812B NeoPixel Multi-LED Indicator (2-Pixel Daisy-Chain on GPIO 13)**:
    - **Timing Engine**: Optimized direct register bitbang cycle-counted (< 60 µs latch, IRAM-resident) dengan FreeRTOS task 50 FPS di Core 1.
    - **Pixel 0: LED SYS (Status Operasional Sistem)**:
      - *Identify Mode*: Biru Murni Strobe 4 Hz (125ms On / 125ms Off) — Prioritas 1
      - *OTA Firmware Flash*: Oranye Tajam Strobe 10 Hz (50ms On / 50ms Off) — Prioritas 2
      - *Config Mode (SoftAP / Direct PC LAN)*: **Biru Murni Kedip 1 Hz (500ms On / 500ms Off)** — Prioritas 3
      - *Hardware Fault*: Merah Murni Solid — Prioritas 4
      - *Booting / Init*: Kuning Hangat Kedip 2 Hz (250ms On / 250ms Off) — Prioritas 5
      - *Normal Running*: Hijau Murni Solid — Prioritas 6
    - **Pixel 1: LED NET / COM (Status Komunikasi Jaringan Murni)**:
      - *Link Down (Kabel RJ45 lepas / Wi-Fi disconnected)*: Merah Murni Double Blink (100ms On, 100ms Off, 100ms On, 700ms Off) — Prioritas 1
      - *Link Up, Menunggu IP DHCP*: Kuning Hangat Kedip 2 Hz (250ms On / 250ms Off) — Prioritas 2
      - *IP Acquired, MQTT Offline*: Cyan (Biru Muda) Kedip 1 Hz (500ms On / 500ms Off) — Prioritas 3
      - *Normal Operasional (IP & MQTT Online)*: **Hijau Murni Solid + Traffic Blip Padam 90ms** saat frame beacon terkirim — Prioritas 4
  - **Recovery / Factory Reset Button**:
    - Pin: `GPIO 34` (Active Low, 10k external pull-up).
    - Polling: Zero-boot synchronous evaluation (`< 5s` Batal, `5-10s` Mode AP Hotspot, `> 10s` Reset Pabrik).
  - **Partition Layout (ESP32 4MB Flash)**:
    - NVS (`nvs`): `0x9000` (24 KB)
    - OTA Data (`otadata`): `0xF000` (8 KB)
    - PHY Init (`phy_init`): `0x11000` (4 KB)
    - App Slot 0 (`app0`): `0x20000` (1.43 MB / 1464 KB)
    - App Slot 1 (`app1`): `0x190000` (1.43 MB / 1464 KB)
    - Filesystem LittleFS (`storage`): `0x300000` (1 MB / 1024 KB)

- **Asymmetric Multi-Core Affinity (Core 0 vs Core 1)**:
  - **Core 0 (PRO_CPU) — Dedicated Ingestion Engine**:
    - Melayani UART2 ISR dan ring-buffer ingestion stream secara eksklusif.
    - Eksekusi evaluasi XOR CRC 25-byte biner frame nRF.
    - Hardware filter in-line: Filter RSSI Minimum (`filter_rssi`), Target UUID 16-byte, dan Target Major (0–65535).
    - Penyaluran data ke antrean FreeRTOS `beaconQueue` (300 slot *drop-oldest*) dan `CircularBuffer` (50 slot live monitor) menggunakan lock-free C++11 `std::atomic`.
  - **Core 1 (APP_CPU) — Application, Network & UI Stack**:
    - lwIP TCP/IP stack execution & Ethernet W5500 SPI interrupt handler.
    - MQTT Publisher Task: Batch buffering hingga 4096B (format biner 6B & 7B), timer interval flush (100–1000ms), LWT, dan downlink RPC.
    - Native HTTP Server (port 80): REST API `/api/v1/*`, Static Web Assets dengan kompresi GZIP transparan.
    - WebSocket Live Telemetry Broadcast: Broadcaster task interval 100ms via `httpd_ws_send_frame_async()`.

- **Zero-FOUC Native Chunked Streaming Template Engine**:
  - Mengeliminasi alokasi dinamis string Arduino (`String`) yang menyebabkan fragmentasi heap.
  - Streaming berkas langsung dari LittleFS VFS (`/storage`) menggunakan chunk HTTP 512-byte (`httpd_resp_send_chunk()`).
  - *In-flight dynamic token replacement* untuk tag `%model%`, `%ap_ssid%`, `%device_id%`, `%firmware_version%`, `%ui_version%`, dan `%device_version%` tanpa me-load seluruh file HTML ke RAM.

- **Memori & Efisiensi Silikon**:
  - Total RAM SRAM terpakai: **14.8%** (~48.6 KB dari 327.6 KB), menyisakan **~279 KB RAM bebas** untuk stabilitas operasional industri 24/7.
  - Flash ROM terpakai: **78.9%** (~1.18 MB dari 1.43 MB alokasi partisi OTA).


## 13. Authentication Security, Transport Protocol & NVS Fault Tolerance
- **Dual-Mode Form Parameter Extraction (`get_form_param` & `urldecode_s`)**:
  - **Memory Safety**: Parser URL-decode (`urldecode_s`) menerapkan bounds checking ketat (`written + 1 < dst_len`) untuk mencegah potensi buffer overflow pada buffer tujuan.
  - **Dukungan Dual-Format Otomatis**:
    1. `multipart/form-data`: Mendeteksi header MIME `name="key"` dan batas boundary `\r\n\r\n` ... `\r\n--`, membersihkan trailing CR/LF secara otomatis.
    2. `application/x-www-form-urlencoded`: Mendeteksi pasangan `key=value` dipisah `&` dengan URL decoding aman.
  - **Standardisasi Frontend**: Frontend ([login.html](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/login.html), [app.js](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/assets/js/app.js), [emergency_page.cpp](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/src/web/emergency_page.cpp)) membungkus payload menggunakan `new URLSearchParams(formData)` dengan header `Content-Type: application/x-www-form-urlencoded` untuk efisiensi transfer data.

- **Mitigasi Serangan Otentikasi (Defense-in-Depth)**:
  - **Timing Side-Channel Protection**: Menggunakan `constant_time_eq()` untuk pencocokan kredensial username dan password secara bitwise konstan, mencegah penyerang menganalisa panjang atau kecocokan karakter melalui variasi latensi respon CPU.
  - **Anti-Brute Force Delay**: Penundaan `vTaskDelay(pdMS_TO_TICKS(1800))` saat kredensial salah untuk membatasi throughput serangan kamus / brute-force.
  - **Rate Limiting**: Maksimal 5 percobaan gagal berturut-turut memicu penguncian sementara selama 60 detik (HTTP 429 Too Many Requests).

- **Arsitektur NVS Zero-Lockout**:
  - Jika partisi master NVS (`nvs.bin`) belum di-flash ke alamat `0x9000`, `ConfigStore::initNvsConfiguration()` otomatis menulis kredensial default (`admin`/`admin` & `user`/`user`) langsung ke namespace operasional `cfg_auth`.
  - Fallback memori RAM di `Session::authenticate()` dan `ConfigStore::loadConfig()` menjamin kredensial tidak pernah kosong/blank meskipun flash NVS baru saja di-format atau mengalami kegagalan baca.


## 14. MQTT Topic Hierarchy & mDNS Discovery Protocol
- **Hierarki Topik MQTT Terstandarisasi (`<dev_model>/<action>/<device_id>`)**:
  - **MQTT Client ID**: Menggunakan murni Hardware ID lowercase: `<device_id>` (contoh: `b3462cf9`).
  - **Uplink Data**: `<dev_model>/data/<device_id>` (contoh: `SG-10/data/b3462cf9`).
  - **Status / Telemetri / LWT**: `<dev_model>/info/<device_id>/status` (contoh: `SG-10/info/b3462cf9/status`).
  - **Downlink Command RPC**: `<dev_model>/cmd/<device_id>` (contoh: `SG-10/cmd/b3462cf9`).
  - **Command Response RPC**: `<dev_model>/res/<device_id>` (contoh: `SG-10/res/b3462cf9`).
  - **Konsistensi Kode**: Variabel di seluruh layer C++ terstandarisasi menggunakan pasangan nama `dev_model` dan `device_id` dengan penjaminan karakter hex lowercase.

- **Protokol Auto-Discovery mDNS (Multicast DNS)**:
  - **Sanitasi Model & Format Hostname**: Semua karakter selain huruf dan angka pada `dev_model` dibersihkan dan diubah menjadi lowercase (contoh: `"SG-10"` $\rightarrow$ `"sg10"`). Format hostname mDNS adalah `<clean_model>-<device_id>` (contoh: `sg10-b3462cf9`), sehingga dapat diakses via URL: `http://sg10-<device_id>.local`.
  - **Service Registration**: Layanan web HTTP port 80 diiklankan secara otomatis melalui protokol `_http._tcp` dengan TXT records: `model`, `firmware`, dan `device_id`.
  - **Inisialisasi**: Aktif otomatis saat stack IP jaringan (baik Ethernet W5500 maupun Wi-Fi Station) menerima konfigurasi IP (`IP_EVENT_*_GOT_IP`).


## 15. Industrial-Grade WebSocket & Live Telemetry Architecture
- **Kepatuhan Protokol RFC 6455 (Two-Step Ingestion)**:
  - **Header & Opcode Extraction**: Pembacaan frame WebSocket diawali dengan panggilan `httpd_ws_recv_frame(req, &ws_pkt, 0)` untuk membaca panjang payload dan tipe frame secara presisi tanpa risiko buffer overflow atau parser desync di level lwIP/httpd.
  - **Penanganan Otomatis Frame Kontrol**:
    - `HTTPD_WS_TYPE_PING`: Dijawab seketika dengan `HTTPD_WS_TYPE_PONG` (`final = true`) untuk menjaga integritas socket keep-alive tanpa membebani thread aplikasi.
    - `HTTPD_WS_TYPE_CLOSE`: Menutup koneksi dan membersihkan slot socket secara tertib (*clean graceful shutdown*).
  - **Integritas Frame Broadcast (`final = true`)**: Seluruh frame teks JSON yang disiarkan menyertakan flag `ws_pkt.final = true`, menjamin browser klien langsung memicu event `ws.onmessage` tanpa fragmentasi buffer internal browser yang menggantung.

- **State Management Berbasis Slot Sesi (`WsClientSlot`)**:
  - Mengeliminasi ketergantungan pada global atomic counter (`s_active_ws_subscribers`).
  - Menggunakan tabel slot sesi terproteksi mutex (`WsClientSlot s_ws_slots[4]`) yang melacak `fd`, status langganan `is_subscribed`, peran otentikasi `role`, dan timestamp aktivitas `last_activity_ms`.
  - Bebas dari *counter drift*: Refresh halaman, multiple tabs, atau koneksi terputus tiba-tiba ditangani secara otomatis pada level deskriptor socket masing-masing.

- **Adaptive Broadcaster & Proteksi DRAM Heap**:
  - **Zero-Allocation saat Idle**: Broadcaster memeriksa keberadaan klien yang aktif *subscribed* sebelum melakukan serialisasi JSON. Jika tidak ada subscriber aktif, task langsung kembali tidur tanpa menyentuh alokasi memori heap (`cJSON_CreateObject`), mencegah fragmentasi memori DRAM saat berjalan 24/7.
  - **Cadence Broadcasting Optimal**: Interval broadcast diatur pada 250 ms (4 Hz) yang adaptif, menghasilkan visualisasi data yang fluid pada UI Live Test tanpa membebani Core 1.
  - **Resiliensi TCP Backpressure**: Toleransi terhadap status socket sibuk sementara (`EAGAIN`/`EWOULDBLOCK`), hanya melepaskan slot jika socket terkonfirmasi hangus (`ESP_ERR_INVALID_STATE` atau `ESP_FAIL`).


## 16. Multi-Device Concurrent Session Control & Sliding Window Lifecycle
- **Kapasitas Koneksi & Alokasi Soket TCP (3 Klien Simultan)**:
  - **lwIP Maximum Sockets**: Dikonfigurasi menjadi 24 (`CONFIG_LWIP_MAX_SOCKETS=24` pada `sdkconfig.defaults` dan `sdkconfig.esp32doit-devkit-v1`), menyediakan kapasitas headroom untuk 3 laptop, Ethernet/WiFi stack, MQTT client, mDNS, dan background tasks.
  - **ESP-HTTPD Server Sockets**: `config.max_open_sockets = 16`, mampu melayani aset statis HTTP bersamaan dengan koneksi WebSocket pada 3 laptop.
- **Kebijakan Batas Login Klien Ganda (Maksimal 2 Perangkat)**:
  - **Maksimal Sesi Aktif**: `MAX_CONCURRENT_SESSIONS = 2` pada `session_mgr.hpp`.
  - **Penolakan Perangkat Ketiga (HTTP 409 Conflict)**: Jika slot otentikasi penuh (2 perangkat berbeda telah aktif login), percobaan login dari perangkat ke-3 ditolak dengan status HTTP 409 dan kode JSON `session_limit`.
  - **Modal Notifikasi SweetAlert2 (Frontend UX)**: Pada `login.html`, respons HTTP 409 memunculkan modal peringatan amber:
    - *Title*: "Sesi Penuh (Maks. 2 Perangkat)"
    - *Message*: "Kapasitas login telah mencapai batas maksimal (2 perangkat aktif). Silakan logout terlebih dahulu di salah satu perangkat yang sedang aktif sebelum masuk."
    - *Icon*: "warning"
- **Sliding Window Session Timeout (15 Menit / 900 Detik)**:
  - **Durasi Sesi**: `SESSION_TIMEOUT_MS = 900000` (15 menit) dengan atribut cookie `Max-Age=900`.
  - **Sliding Renewal Otomatis**: Setiap interaksi API (`validate()`, `touch()`) memperbarui timestamp aktivitas (`last_activity_ms = now`), sehingga pengguna yang aktif bekerja tidak akan terputus sesi loginnya.
  - **Keepalive Saat Uji Diagnostik ("Start Test")**:
    - Backend: Broadcaster WebSocket memanggil `Session::touch(token)` secara berkala selama klien terdaftar dalam status berlangganan uji (`is_subscribed == true`).
    - Frontend: Halaman `/test` menjalankan interval keepalive heartbeat setiap 60 detik (`/api/v1/auth/check`) selama pengetesan aktif, menjamin sesi teknisi tetap terjaga selama sesi monitoring jangka panjang.
- **Client IP Tracking & Deduplikasi Sesi**:
  - Pelacakan IP klien via socket `getpeername()` mencegah pemborosan slot otentikasi jika satu laptop melakukan re-login atau me-refresh browser.


## 17. Resilient File Upload UI/UX & Live Telemetry HUD
- **Ergonomi Dropzone Pembaruan Firmware & LittleFS**:
  - **Isolasi Event Bubbling**: Menghapus `onclick` ganda dari markup HTML dan menambahkan `e.stopPropagation()` pada file input click handler, mengeliminasi pembatalan dialog pemilihan file Chromium pada OS Windows.
  - **State Tampilan Terisolasi**: Area file terpilih (`#dropzoneFirmwareSelected` dan `#dropzoneFSSelected`) disembunyikan secara eksplisit (`display: none;`) saat inisialisasi awal hingga file binary `.bin` valid dipilih.
  - **Penghapusan Pemeriksaan Magic Byte 0xE9**:
    - Validasi kaku terhadap byte `0xE9` dihapus sepenuhnya dari `ota_service.cpp`, `emergency_page.cpp`, dan frontend `firmware.js`.
    - Pengecekan diserahkan kepada subsistem OTA bawaan ESP-IDF (`esp_ota_begin`, `esp_ota_write`, dan `esp_ota_end`) yang memverifikasi integritas checksum SHA256 serta keabsahan image binary secara native.
  - **Transmisi Biner Murni (Raw Octet-Stream Streaming)**:
    - Frontend mengirim file langsung via `xhr.send(file)` dengan header `Content-Type: application/octet-stream`, menggantikan `FormData` (multipart) yang sebelumnya menyisipkan header `WebKitFormBoundary` dan merusak offset biner firmware pada partisi flash.
  - **Validasi Binary Pre-Flight Toleran**: Pemeriksaan model perangkat keras pada header image binary disesuaikan untuk mengenali identitas proyek build PlatformIO (`SG-10`, `esp32_ibeacon_espidf`, `arduino-lib-builder`), mencegah penolakan palsu (*false rejection*) sebelum proses OTA dikirim ke mikrokontroler.
- **Minimalisme HUD Card MQTT Publish Speed**:
  - Penyelarasan tampilan kartu telemetri agar seragam dengan HUD Card lainnya (hanya menampilkan angka kecepatan utama `#stat_mqtt_speed` dan satuan `batch/s`).
  - Baris sub-metrik `Tot: X pkt` dihapus dari antarmuka visual demi estetika dashboard yang bersih dan simetris, sementara penghitung metrik internal tetap aktif di backend untuk telemetri log sistem.

## 18. Hardware Status LED (Dual WS2812B NeoPixel) & Traffic Dynamics
- **Arsitektur Hardware Visual**:
  - Dua unit LED WS2812B terhubung daisy-chain pada **GPIO 13** (`Pins::LED_WS2812`), dikelola oleh FreeRTOS task khusus di Core 1 (50 FPS / 20ms refresh).
  - **LED 1 (SYS)**:
    - *Hijau Solid*: Operasi normal dan stabil (`SysLedState::NORMAL`).
    - *Kuning Kedip (2 Hz)*: Fase booting & inisialisasi awal hardware (`SysLedState::BOOTING`).
    - *Oranye Strobo Cepat (10 Hz)*: Proses penulisan OTA Flash berlangsung (`SysLedState::OTA`).
    - *Biru Kedip (4 Hz)*: Mode Identifikasi perangkat aktif selama 3 detik (`triggerIdentify(3000)`).
    - *Merah Solid*: System Fault / Panic.
  - **LED 2 (COM) - Hirarki Status Komunikasi 5 Tahap & Penegakan Strict Priority**:
    - *Hirarki Mutlak (Status di Bawahnya DILARANG Menyala)*:
      1. **Level 1 (Tertinggi)**: `ComLedState::SOFTAP` (*Biru Kedip 1 Hz*). Aktif jika `s_configMode` atau `s_softApActive`. Memblokir 100% status Merah, Kuning, Cyan, dan Hijau terlepas dari event kabel LAN lepas atau MQTT putus.
      2. **Level 2**: `ComLedState::DISCONNECTED` (*Merah Double Blink*). Fisik kabel LAN Ethernet tercabut (Link Down) atau sinyal Wi-Fi STA terputus saat SoftAP tidak aktif.
      3. **Level 3**: `ComLedState::CONNECTING` (*Kuning Kedip 2 Hz*). Fisik kabel LAN tercolok (Link Up) dan sedang menunggu negosiasi IP DHCP dari router. Merah dilarang menyala.
      4. **Level 4**: `ComLedState::MQTT_OFFLINE` (*Cyan Kedip 1 Hz*). Alamat IP valid didapat (LAN/Wi-Fi up), namun broker MQTT belum tersambung. Merah dan Kuning dilarang menyala.
      5. **Level 5 (Terendah)**: `ComLedState::NORMAL` (*Hijau Solid + Traffic Blip 90ms*). Seluruh lapisan jaringan dan MQTT 100% normal. Padam sekejap selama 90 ms (~4-5 frame @ 50 FPS) setiap kali batch iBeacon berhasil terkirim via `triggerTrafficBlip()`.
    - *Strict Priority Guard*: Fungsi `setComState()` menolak modifikasi status jika sistem berada pada Level 1 (SoftAP/Config Mode).

## 19. Boot Recovery Button, Zero-Task Isolation, & Wi-Fi Scanning Architecture
- **Zero-Task Isolation pada Boot Recovery (`checkBootRecovery`)**:
  - Inisialisasi pin GPIO LED WS2812B (`LedManager::init`) dipisahkan secara ketat dari peluncuran task FreeRTOS (`LedManager::startTask`).
  - Selama evaluasi penekanan tombol recovery fisik (`Pins::BUTTON_RESET`, GPIO 34), Core 0 memiliki kendali eksklusif 100% tanpa adanya background task di Core 1 yang dapat menimpa bit data WS2812B:
    - **0 – 5 Detik**: Merah Solid (`0xFF0000`). Melepaskan tombol membatalkan recovery dan melanjutkan boot normal.
    - **5 – 10 Detik**: Biru Solid (`0x0000FF`). Memberikan umpan balik visual yang tegas bahwa sistem siap masuk ke Mode Hotspot Konfigurasi SoftAP saat dilepas.
    - **> 10 Detik**: Padam Total (`0x000000`). Indikasi visual bahwa perangkat siap melakukan Factory Reset (NVS wipe) saat tombol dilepas.
  - Task FreeRTOS `WS2812_Task` baru diluncurkan setelah fungsi `checkBootRecovery()` selesai, menjamin warna solid tidak pernah berkedip atau tertimpa pola animasi booting.
- **Global Lazy Initialization Wi-Fi Driver (`ensureWifiDriverInitialized`)**:
  - Pada konfigurasi default Ethernet (`ConnectionMode::ETHERNET`), inisialisasi Wi-Fi STA sengaja dilewati untuk menghemat RAM (320 KB heap optimization).
  - Ditambahkan helper terpusat `NetworkManager::ensureWifiDriverInitialized()` yang menginisialisasi kernel Wi-Fi ESP-IDF (`esp_wifi_init`) dan netif secara *on-demand*.
  - Mengeliminasi crash `ESP_ERR_WIFI_NOT_INIT` (`0x3001`) saat memanggil `startConfigAP()` maupun endpoint `/api/v1/wifi/scan` di lingkungan berorientasi Ethernet.
- **Arsitektur State Machine Wi-Fi Scanner (`/api/v1/wifi/scan`) & Passive Station Mode**:
  - **Pencegahan Auto-Connect Loop (`shouldWifiStaConnect`)**: Pada `net_manager.cpp`, event `WIFI_EVENT_STA_START` dan `WIFI_EVENT_STA_DISCONNECTED` diproteksi secara ketat. ESP32 hanya memanggil `esp_wifi_connect()` dan memperbarui status LED COM (`ComLedState::CONNECTING` / `DISCONNECTED`) jika mode aktif adalah Wi-Fi (`ConnectionMode::WIFI`), Ethernet tidak aktif, bukan mode SoftAP, dan SSID terkonfigurasi.
  - **Isolasi Mode Ethernet**: Saat gateway beroperasi di mode Ethernet (`ConnectionMode::ETHERNET`), radio Wi-Fi Station beroperasi dalam kondisi IDLE/pasif murni saat scanning. Tidak ada pemanggilan koneksi otomatis ke SSID lama yang menyebabkan banjir log `NO_AP_FOUND (201)` dan tabrakan state machine Wi-Fi (`ESP_ERR_WIFI_STATE` `0x3008`).
  - **Auto-Elevation Mode Wi-Fi**: Jika mode Wi-Fi saat ini `NULL`, otomatis diaktifkan ke `WIFI_MODE_STA`. Jika mode saat ini `AP` (SoftAP aktif), dinaikkan ke `WIFI_MODE_APSTA` sehingga pemindaian saluran nirkabel dapat berlangsung tanpa memutus koneksi hotspot pengguna.
  - **Sinkronisasi Selesai Seluruh Saluran (`WIFI_EVENT_SCAN_DONE`)**:
    - Backend TIDAK PERNAH membaca hasil prematur via `esp_wifi_scan_get_ap_num` sebelum pemindaian seluruh saluran selesai. Sinyal kesiapan scan hanya di-flag saat event resmi ESP-IDF `WIFI_EVENT_SCAN_DONE` terpicu.
    - Menghilangkan anomali pemindaian "terlalu cepat" (<1 detik) yang disebabkan oleh polling pertama membaca buffer AP parsial dari 1-2 channel awal saja.
  - **Dukungan Frekuensi Penuh Channel 1 s.d. 13 (Regulasi ID)**:
    - Ditambahkan inisialisasi `wifi_country_t` dengan kode negara `"ID"`, rentang saluran `schan=1` hingga `nchan=13`, serta kebijakan `WIFI_COUNTRY_POLICY_MANUAL`. Memastikan access point di frekuensi atas (mis. channel 11-13) tidak terlewatkan.
  - **Pencegahan Bentrok Polling & Watchdog Timeout**: Backend melacak flag `s_scan_in_progress` dan `s_scan_start_millis`. Polling periodik 700 ms dari modal UI `app.js` tidak akan memicu ulang scan baru, melainkan mengecek kesiapan hasil via `NetworkManager::isWifiScanDone()`. Jika pemindaian tertahan > 8 detik, sistem secara defensif menghentikan pemindaian lama via `esp_wifi_scan_stop()`.
  - **Defensive State Retry**: Jika driver merespons `ESP_ERR_WIFI_STATE` saat memulai scan, handler secara otomatis mengeksekusi `esp_wifi_disconnect()`, memberi jeda 50 ms untuk transisi driver ke state IDLE, lalu mengulangi pemindaian secara transparan.
  - **Pengurutan Kualitas Sinyal & Deduplikasi SSID**: Ketika pemindaian selesai, daftar Access Point diurutkan berdasarkan kekuatan sinyal RSSI (*descending* / sinyal terkuat di atas), dan SSID ganda dideduplikasi secara otomatis untuk menyajikan daftar jaringan yang bersih dan profesional.

## 20. Resiliensi WS2812B Driver & Anti-Dimming Engine (Web Concurrency)
- **Eksekusi IRAM Murni (`IRAM_ATTR writeRawPixels`)**:
  - Rutinitas bit-bang nanodetik WS2812B dipindahkan sepenuhnya dari Flash SPI ke Internal IRAM (`IRAM_ATTR`).
  - Mengeliminasi *Instruction Cache Miss* dan stall CPU ketika Web Server (`httpd`) membaca puluhan file static HTML/CSS/JS dari partisi LittleFS pada SPI Flash. Timing WS2812B kebal dari *premature latch reset*, mencegah fenomena LED meredup sekejap saat halaman web dibuka.
- **Transmisi Berbasis Dirty-Checking**:
  - Sistem melacak frame terakhir (`s_lastColorSys`, `s_lastColorCom`, `s_lastBrightness`).
  - Pulsa sinyal fisik ke GPIO 13 HANYA dipancarkan saat nilai warna atau kecerahan berubah.
  - Saat status statis (misal Hijau Solid atau fase ON 500ms), jalur data GPIO 13 idle sepenuhnya. Mengeliminasi 98% kemungkinan tabrakan bus dengan I/O web server serta menghemat beban CPU interrupt lockup.
- **Prioritas Task Adaptif di Core 1**:
  - Prioritas task `WS2812_Task` dinaikkan ke Priority 3 untuk menjamin konsistensi evaluasi frame tanpa terhambat oleh background telemetry worker.

## 21. Industrial Cyberpunk Telemetry HUD & Live Test Monitor 2.0 (`/test`)
- **Header Control Bar & Session Tracking (`.test-control-card`)**:
  - **Status & Actions Header**:
    - **Header Title Area**: Dilengkapi icon box neon cyan (`.test-header-icon`, 38x38px), judul utama *"Pengujian BLE Gateway"*, dan subjudul *"Streaming real-time inspeksi beacon"*.
    - **Status Pill Reaktif (`#testStatusBadge`)**: Menampilkan badge ringkas `STANDBY` (saat berhenti) atau pulsating cyan `STREAMING LIVE` (saat aktif).
  - **Aksi Kontrol Responsif**:
    - **Start / Stop Test (`#btnStartTest` / `#btnStopTest`)**: Tombol utama dengan warna cyan neon `#22d3ee` teks gelap `#042436` (Start) atau rose crimson `#f43f5e` (Stop). Pada mobile (<640px), tombol ini otomatis melebar (`flex: 1`, tinggi 44px) mengisi baris kontrol.
    - **Export CSV (`#btnExportCsv`)**: Tombol dark slate berbatas halus. Pada mobile (<640px), teks label disembunyikan sehingga berubah menjadi tombol ikon persegi kompak (`44x44px`) berdampingan dengan Start Test (`[ ▶ Start Test ] [ ↓ ]`).
  - **Sub-Baris Metrik Sesi Pengujian Realtime (`.test-session-stats`)**:
    - Tampil sebagai kapsul/pill bar gelap (`background: rgba(2, 6, 23, 0.65)`, border 1px subtle).
    - **Elapsed (`#sessionElapsedTime`)**: Stopwatch aktif dengan format `HH:MM:SS`.
    - **Rx (`#sessionPacketsReceived`)**: Akumulator total paket beacon yang diterima gateway secara simetris dihitung langsung dari selisih register hardware chip ESP32 (`msg.beacons_read - sessionStartBeaconsRead`). Mengeliminasi disparitas pembacaan client-side.
    - **Tx (`#sessionPacketsSent`)**: Akumulator total paket beacon yang berhasil dipublikasikan ke broker MQTT (`msg.mqtt_published - sessionStartMqttPublished`).
    - **Pembersihan Pre-Start Buffer (`flushLiveBeacons`)**: Saat tombol *Start Test* ditekan, firmware ESP32 otomatis mengosongkan buffer live FIFO (`s_liveBeaconCount = 0`), mencegah bocornya 50 paket lama dari sesi pra-start sehingga Rx dan Tx bergerak serempak dari angka 0 dengan selisih minimal (0-5 paket in-flight).
    - **Drop (`#sessionDropRate`)**: Rasio kegagalan transmisi realtime dalam format `Jumlah Drop (Persentase dari Terkirim)` (contoh: `0 (0.00%)`).
- **8-Card Cyberpunk Telemetry HUD Grid (`.hud-card-v2`)**:
  - **Tata Letak Responsif 2 Kolom Mobile**:
    - Standar mobile & tablet (`< 960px`): Menggunakan format **2 Kolom** penuh (`grid-template-columns: repeat(2, 1fr); gap: 8px s/d 10px`).
    - Desktop (`>= 960px`): Melebar menjadi **4 Kolom** (`grid-template-columns: repeat(4, 1fr); gap: 14px`).
  - **Matriks Posisi 2 Kolom Mobile (Sesuai Referensi Visual)**:
    - **Baris 1**: Kolom 1 = **Tag Read Speed** (`card-cyan`) | Kolom 2 = **Unique Beacons** (`card-lavender`)
    - **Baris 2**: Kolom 1 = **MQTT Publish Speed** (`card-emerald`) | Kolom 2 = **Queue FreeRTOS** (`card-amber`)
    - **Baris 3**: Kolom 1 = **Antrian Buffer Batch** (`card-sky`) | Kolom 2 = **Net Bandwidth** (`card-cyan`)
    - **Baris 4**: Kolom 1 = **Pkt per Batch** (`card-indigo`) | Kolom 2 = **Free Heap RAM** (`card-lavender`)
  - **Detail Kartu Telemetri**:
    1. **Tag Read Speed (`card-cyan`)**:
       - Nilai utama: `#stat_read_speed` (`pkt/s`, font cyan 1.85rem / 1.25rem mobile monospace).
       - Rolling sub-stats: Rata-rata sesi (`#stat_read_avg` `Avg: X pkt/s`) dan kecepatan puncak (`#stat_read_max` `Max: Y pkt/s`).
       - Tanpa progress bar & tanpa badge nominal demi presisi numerik murni.
    2. **Unique Beacons (`card-lavender` - Opsi 3)**:
       - Nilai utama: `#stat_unique_tags` (`tag`, font lavender `#c084fc`).
       - Badge protokol: Lavender pill `[iBeacon]` (`#stat_beacon_type`).
       - Sub-baris metrik: Rata-rata kekuatan sinyal (`#stat_beacon_rssi` `Sinyal: -XX dBm (Avg)`).
       - Tanpa progress bar demi estetika bersih.
    3. **MQTT Publish Speed (`card-emerald`)**:
       - Nilai utama: `#stat_mqtt_speed` (`pkt/s`, font emerald `#34d399`).
       - Kecepatan murni paket per detik (`pkt/s`).
       - Rolling sub-stats: Rata-rata throughput (`#stat_mqtt_avg` `Avg: X pkt/s`) dan puncak (`#stat_mqtt_max` `Max: Y pkt/s`).
       - Tanpa progress bar & tanpa badge nominal.
    4. **Queue FreeRTOS (`card-amber`)**:
       - Nilai utama: `#stat_queue_load` (`0 / 300`, font putih tebal).
       - Badge status: `#stat_queue_badge` (`0% Load Safe` hijau atau `Warning` amber).
       - Progress fill line: `#bar_queue_load` (`height: 3px`).
    5. **Antrian Buffer Batch (`card-sky`)**:
       - Nilai utama: `#stat_batch_load` (`0 / 100`, font putih tebal).
       - Badge kapasitas: `#stat_batch_badge` (`0% Buffer` cyan).
       - Progress fill line: `#bar_batch_load`.
    6. **Net Bandwidth (`card-cyan`)**:
       - Nilai utama: `#stat_net_bandwidth` (`KB/s`, font cyan).
       - Badge status: `Optimum` hijau.
       - Sub-baris status interface: `#stat_net_link_text` menampilkan indikator visual `■■■ LAN W5500 100M Full` atau `■■■ WiFi 5GHz -XXdBm`.
    7. **Pkt per Batch (`card-indigo`)**:
       - Nilai utama: `#stat_batch_pkt_size` (`/ 100 pkt`, font putih tebal).
       - Badge informasi: `#stat_batch_pkt_badge` (`Avg payload` slate).
       - Progress fill line: `#bar_batch_pkt`.
    8. **Free Heap RAM (`card-lavender`)**:
       - Nilai utama: `#stat_free_heap` (`KB`, font putih).
       - Badge memori: `#stat_heap_badge` (`XX% Left` hijau).
       - Progress fill line: `#bar_free_heap` ungu (`fill-purple`).
- **Ekspor Telemetri CSV (`exportToCsv`)**:
  - Format output `.csv` dengan UTF-8 Byte Order Mark (`\uFEFF`) untuk kompatibilitas penuh dengan Microsoft Excel dan Google Sheets.
  - Kolom data: `No, Waktu, Tipe Beacon, Tag / MAC, RSSI (dBm), Baterai (%)`.
  - Penamaan file otomatis: `ble_gateway_telemetry_YYYYMMDDHHMMSS.csv`.

## 22. Navigation Guard & Active Test Warning UX (`/test`)
- **Proteksi Status Uji Aktif (`isTesting === true`)**:
  - Mencegah terputusnya stream telemetri secara tak sengaja atau hilangnya rekaman live beacon saat teknisi sedang mengamati performa gateway.
  - Jika pengguna mengklik link menu navigasi apa pun (sidebar desktop, mobile bottom nav, header logo, atau action sheet), aksi default navigasi ditahan (`e.preventDefault()`, `e.stopPropagation()`).
- **Modal Dialog Peringatan Cyberpunk (`Swal.fire` / `swal-lite.js`)**:
  - **Tampilan Dark Glassmorphic**: Latar belakang `#0f172a` dengan linear gradient ke `#1e293b`, border subtle `rgba(255, 255, 255, 0.1)`, serta backdrop blur 8px (`rgba(2, 6, 23, 0.75)`).
  - **Ikon Status**: `warning` amber `#f59e0b` dengan lingkaran pulsing semi-transparan `rgba(245, 158, 11, 0.15)`.
  - **Judul**: `"Sesi Pengujian Sedang Berjalan"` (Font `#f8fafc` tebal).
  - **Penjelasan Eksplisit**:
    > *"Pengujian live beacon sedang aktif. Jika Anda berpindah halaman, koneksi pengujian akan dihentikan dan seluruh data sesi uji yang belum diekspor akan hilang.<br><br>Yakin ingin tetap berpindah halaman?"*
  - **Tombol Konfirmasi ("Lanjut")**:
    - Styling: Danger/Warning Rose `#ef4444` (`#ef4444` dengan teks putih).
    - Tindakan: Memanggil `stopTest()` untuk mengirimkan frame `{ action: "stop" }` ke WebSocket ESP32, menghentikan timer, membersihkan flag, lalu mengeksekusi `window.location.href = targetUrl`.
  - **Tombol Pembatalan ("Batal")**:
    - Styling: Slate Muted `rgba(51, 65, 85, 0.7)` dengan border `rgba(255, 255, 255, 0.08)`.
    - Tindakan: Menutup modal, membatalkan perpindahan halaman, membiarkan pengujian dan grafik telemetri tetap berjalan normal.
- **Proteksi Tingkat Browser (`window.onbeforeunload`)**:
  - Mencegah reload (F5 / Ctrl+R) atau penutupan tab secara tidak sengaja saat pengujian sedang aktif.
  - Memanggil `e.preventDefault()` dan menyetel pesan peringatan standar browser.

## 23. 2026 Enterprise Segmented Tab & Live Unique Beacon Monitor (`/test`)
- **Session Telemetry Sub-Stats Bar (`.test-session-stats`)**:
  - Bar statistik sesi horizontal di bawah tombol kontrol sesi pengujian:
    `Elapsed: 00:00:00 | Rx: 0 | Tx: 0 | Drop: 0 (0.00%) | Bandwidth: 0.00 KB`
  - **Total Bandwidth (`#sessionTotalBandwidth`, `.session-stat-val.sky`)**: Menghitung akumulasi byte egress nyata (Ethernet L2/WiFi + TCP/IP + MQTT header + Payload) secara presisi dari firmware (`s_statTotalWireBytes`), ditransmisikan via WebSocket telemetry (`total_bytes`), dan diformat otomatis (`KB`, `MB`, `GB`).
- **Segmented Control Rail (Apple HIG / Linear / Vercel Standards)**:
  - **Track / Rail**: Kontainer inset gelap `rgba(2, 6, 23, 0.7)` dengan border `1px solid rgba(255, 255, 255, 0.08)`, border-radius `12px`, dan padding `4px`.
  - **Floating Active Tile**: Segmen aktif berpendar cyan neon (`background: linear-gradient(...)`, border `rgba(56, 189, 248, 0.4)`, glow `rgba(56, 189, 248, 0.15)`, shadow `0 4px 14px rgba(0, 0, 0, 0.45)`).
  - **Clean Minimalist Design**:
    - **Tab Buffer FIFO (`#tabFifo`)**: Memiliki pulsing live stream dot cyan (`.segment-live-dot`) dan label teks "Buffer FIFO" (tanpa badge pil "10 Pkt").
    - **Tab Unique Beacons (`#tabUnique`)**: Dilengkapi ikon radio pulse SVG dan label "Unique Beacons" (tanpa badge pil angka), menghasilkan tampilan ultra-clean dan lapang.
- **Tab Body Toolbar (Search Field Placement & Standard UI/UX Width)**:
  - Kotak filter pencarian (`.filter-search-box.compact`) diletakkan **di bawah tab segmen dan di atas tabel**, diposisikan di sebelah **KANAN** (`.tab-body-toolbar.right-aligned`, `justify-content: flex-end`).
  - Lebar proporsional standar enterprise UI/UX: **250px** di desktop, **200px** di mobile (tidak lagi melar full-width).
- **Tabel Unique Beacons (Datadog Telemetry Style)**:
  - **Struktur 6 Kolom (Tanpa Kolom Format)**:
    `#` (40px) | `Major-Minor` | `RSSI` | `Battery` | `Terakhir Dilihat` | `Total Pkt`.
    Kolom `Format` dihilangkan agar ruang identitas Major-Minor dan metrik sinyal/waktu/baterai lebih lapang.
  - **Urutan Stabil (First-Seen Insertion Order / No Jumping Rows)**:
    Data baris tidak saling bertukar posisi (*no flickering/row jumping*) saat paket dari berbagai tag masuk silih berganti. Baris #1, #2, #3, ... tetap permanen berdasarkan urutan kedatangan pertama.
  - **In-Place Live Update**: Nilai **Waktu Terakhir**, **RSSI**, **Baterai**, dan **Total Pkt (Hits)** diperbarui secara *live in-place* pada baris masing-masing.
  - **Persistensi Baterai**: Status baterai dari paket `ibeacon_ext` tetap tersimpan dan tidak hilang meskipun paket berikutnya berupa `iBeacon` standar.
  - **Signal Strength Coding**: RSSI diberi kode warna: hijau neon (`good`, `>= -65 dBm`), kuning amber (`mid`, `-66 s/d -80 dBm`), merah rose (`weak`, `< -80 dBm`).
- **Optimasi Responsif 360x800 Mobile (Android Standard)**:
  - Segmented rail mengisi lebar kartu dengan alokasi tombol seimbang `flex: 1` dan tinggi touch-target **42px**.
  - Kotak pencarian rata kanan dengan lebar 200px di layar mobile.
  - Kontainer tabel memiliki batas ketinggian vertikal `max-height: 360px` dengan *smooth touch scroll* horizontal dan custom dark scrollbar.

## 24. Dropped Data Telemetry & Diagnostic Tab (`/test`)
- **Tujuan & Arsitektur Diagnosis Real-time**:
  - Menyediakan visibilitas penuh terhadap paket beacon yang ditolak (*drop*) atau gagal dipublikasikan oleh ESP32 gateway selama pengujian live.
  - Setiap paket drop diklasifikasikan ke dalam 6 kategori alasan mutlak:
    1. `TTL Expired` (Value 0): Paket beacon berada di antrian buffer melebihi batas waktu TTL (Time-To-Live).
    2. `RSSI < Min` (Value 1): Kekuatan sinyal beacon berada di bawah ambang batas minimal filter RSSI.
    3. `UUID Mismatch` (Value 2): Nilai Proximity UUID paket beacon tidak cocok dengan UUID yang diizinkan pada pengaturan filter.
    4. `Major Mismatch` (Value 3): Nilai Major beacon tidak memenuhi filter target Major.
    5. `Queue Full` (Value 4): Antrian FreeRTOS penuh saat beban throughput spike (drop-oldest overflow).
    6. `MQTT Fail` (Value 5): Kegagalan saat mempublikasikan payload ke broker MQTT (koneksi putus/buffer socket penuh).
- **Segmented Control Rail Tab ke-3 (`#tabDropped`)**:
  - Ditambahkan segmen ketiga dengan ikon block/circle-slash SVG dan label "Dropped Data" pada `.segmented-control`.
  - Berpindah tab secara instan tanpa reload browser, mengaktifkan tampilan visual grid kartu dan tabel dropped data.
- **Ringkasan Total Alasan Drop (Desktop Symmetric 6-Column Grid Layout)**:
  - Kontainer `.drop-summary-grid` (`#dropSummaryGrid`) diletakkan tepat di atas tabel data dan di bawah segmented tab rail.
  - **Tata Letak Desktop (`> 960px`)**:
    - Disusun simetris dan seragam dalam **1 baris 6 kolom horizontal** (`repeat(6, 1fr)`), dengan gap 10px.
    - Setiap kartu memiliki *top-accent border* berwarna khas (2px) untuk memperjelas kategori:
      - TTL Expired: Rose Crimson (`#f43f5e`, `.card-ttl`)
      - RSSI < Min: Amber Warning (`#f59e0b`, `.card-rssi`)
      - UUID Mismatch: Violet Purple (`#a855f7`, `.card-uuid`)
      - Major Mismatch: Yellow Gold (`#eab308`, `.card-major`)
      - Queue Full: Sky Blue (`#0284c7`, `.card-queue`)
      - MQTT Fail: Red Alert (`#ef4444`, `.card-mqtt`)
    - Header kartu menampilkan pulsing dot berwarna dan nama alasan.
    - Body kartu menampilkan nilai angka tebal monospace 1.25rem dengan satuan `pkt`.
  - **Tata Letak Responsif Mobile (360x800)**:
    - Ditata dalam grid 3 kolom x 2 baris (`repeat(3, 1fr)`), nilai angka disesuaikan menjadi 1.05rem, padding 8px 6px.
- **Balanced Toolbar Layout (`#tabBodyToolbar`)**:
  - Menggunakan `justify-content: space-between` saat tab Dropped Data aktif:
    - **Sisi Kiri (`#toolbarLeftInfo`)**: Menampilkan pill badge amber `.drop-total-badge` dengan pulsing dot: `Total Ditolak: X pkt`.
    - **Sisi Kanan**: Kotak filter pencarian `.filter-search-box.compact` dengan lebar standar 250px (desktop) atau 200px (mobile).
  - Saat tab Buffer FIFO atau Unique Beacons aktif, elemen sisi kiri disembunyikan (`display: none;`), dan kotak pencarian tetap rata kanan secara elegan via `margin-left: auto`.
- **Tabel Log Dropped Data Tepat 5 Kolom**:
  - Kolom tabel disederhanakan tanpa kolom redundant:
    `#` (40px) | `Tag / Identitas` | `RSSI` | `Alasan Drop` | `Waktu Drop`.
  - **Kolom #**: Penomoran indeks baris (1 s.d. 50).
  - **Kolom Tag / Identitas**: Teks Major-Minor atau identifier beacon dengan font monospace cyan `#38bdf8`.
  - **Kolom RSSI**: Nilai sinyal numerik dalam format `-XX dBm` dengan badge sinyal berwarna.
  - **Kolom Alasan Drop (`.badge-drop`)**: Badge pill berlatar transparan dengan border tipis dan warna sesuai jenis kegagalan:
    - `.drop-ttl`: Rose background (`#fb7185`)
    - `.drop-rssi`: Amber background (`#fbbf24`)
    - `.drop-uuid`: Purple background (`#c084fc`)
    - `.drop-major`: Yellow background (`#fde047`)
    - `.drop-queue`: Sky background (`#38bdf8`)
    - `.drop-mqtt`: Red background (`#f87171`)
  - **Kolom Waktu Drop**: Timestamp presisi milidetik `HH:MM:SS.mmm`.
- **Sinkronisasi Firmware & Reset Status**:
  - Telemetri drop dipancarkan via WebSocket setiap detik dalam objek `drop_summary` (6 field counter) dan array `dropped_beacons` (ring buffer 50 paket terbaru).
  - Saat pengetesan dihentikan atau dimulai ulang (`Start Test`), daftar dropped data di-reset ke 0 untuk memastikan sesi uji baru bersih.

---

## 32. Komponen Banner Hotspot SoftAP & Keepalive Lifecycle (`#apModeBanner`)
- **Tujuan Desain**:
  - Memberikan indikator visual yang jelas kepada teknisi/administrator ketika perangkat sedang beroperasi dalam mode darurat/on-demand Hotspot SoftAP (`192.168.4.1`).
  - Menyajikan informasi SSID aktif, countdown timer *auto-shutdown idle*, dan tombol aksi langsung untuk mematikan AP secara manual.
- **Spesifikasi Anatomi & Gaya (`.ap-banner-card`)**:
  - **Latar & Bingkai**: Latar transparan gelap dengan aksen peringatan amber industri `rgba(245, 158, 11, 0.08)`, border solid tipis `1px solid rgba(245, 158, 11, 0.25)`, border-radius `10px`, padding `12px 18px`.
  - **Indikator Pulsing (`.ap-pulse-ring`)**: Animasi riak cincin amber dengan titik inti solid `#f59e0b` yang berdenyut lembut (`animation: apPulse 2s infinite`).
  - **Tipografi**:
    - Judul: Teks tebal semibold 0.85rem dengan warna amber `#fbbf24`.
    - Subtitle: Teks sekunder slate `#94a3b8` 0.78rem dengan penekanan SSID `#f8fafc`.
    - Badge Timer (`.ap-timer-pill`): Pill badge gelap monospace dengan latar `rgba(15, 23, 42, 0.6)`, border `rgba(245, 158, 11, 0.3)`, warna teks kuning amber terang `#fef08a`.
  - **Tombol Aksi (`.btn-ap-stop`)**: Tombol kompak dengan ikon antena terputus, latar merah rose transparan `rgba(244, 63, 94, 0.15)`, border `rgba(244, 63, 94, 0.35)`, teks `#fda4af`. Pada hover bertransisi ke latar rose pekat dengan efek glow.
- **Siklus Hidup Keepalive & Auto-Reset Interaktif**:
  - **Backend Global Activity Hook**: Setiap request HTTP (halaman, REST API, static assets) dan frame WebSocket secara otomatis memanggil `NetworkManager::recordApActivity()` jika mode AP aktif.
  - **Dedicated Touch API**: Endpoint ringan `POST /api/v1/system/config-ap/touch` mengembalikan `{ status: "ok", ap_active: true, ap_remain_sec: 60 }`.
  - **Frontend User-Activity Throttling**:
    - Mendengarkan event interaksi pengguna (`mousedown`, `keydown`, `input`, `touchstart`, `scroll`).
    - Mereset counter tampilan secara instan kembali ke 60s untuk responsivitas UX maksimal.
    - Mengirim ping touch ke backend dengan throttle 12 detik dan heartbeat periodik 20 detik selama tab/halaman browser aktif.
  - **Safety Auto-Off**: Jika pengguna benar-benar meninggalkan browser tanpa aktivitas selama 60 detik, SoftAP akan dimatikan otomatis oleh backend untuk keamanan jaringan industri dan penghematan daya radio RF.

---

## 33. Mekanisme Sliding Expiration Session & Global Keepalive
- **Tujuan Keamanan & Pengalaman Pengguna**:
  - Mencegah teknisi atau administrator terputus sesi loginnya secara mendadak saat sedang melakukan pemantauan live test beacon dalam durasi panjang (>15-20 menit) maupun saat sedang mengisi konfigurasi formulir yang kompleks.
  - Mempertahankan batas keamanan sesi di sisi server (kedaluwarsa otomatis jika perangkat benar-benar ditinggalkan idle tanpa aktivitas selama 15 menit).
- **Spesifikasi Teknis**:
  - **Cookie Lifetime Buffer**: Cookie otentikasi `token` diterbitkan dengan atribut `Max-Age=3600` (buffer 1 jam) dan `SameSite=Lax; HttpOnly`.
  - **Sliding Renewal via HTTP Heartbeat**:
    - Setiap respons otentikasi sukses pada `/api/v1/auth/check` atau `/api/v1/system/config-ap/touch` menyertakan kembali header `Set-Cookie` yang diperbarui untuk me-reset hitungan mundur cookie di browser kembali ke 3600 detik.
    - Script global [app.js](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/assets/js/app.js) menjalankan interval heartbeat setiap 45 detik selama tab browser aktif.
    - Script [test.js](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/assets/js/test.js) menjalankan ping keepalive khusus setiap 30 detik saat sesi pengujian live aktif.
  - **Proteksi Navigasi Bersih**:
    - Event penutupan WebSocket `ws.onclose` hanya melakukan verifikasi status/redirect jika pengetesan sedang berjalan aktif, mencegah navigasi sukarela pengguna antar halaman terputus secara keliru.

---

## 34. Arsitektur Pusat Log Sistem & Drop Data (`/log` - Unified System & Data Logs Hub)
- **Tujuan Arsitektur**:
  - Menyediakan pusat audit terpadu bagi teknisi dan administrator terhadap riwayat konektivitas jaringan (Wi-Fi STA, Ethernet W5500, MQTT Broker, SoftAP, Boot) serta **50 riwayat paket beacon yang terbuang/gagal (Dropped Beacon Data)** pada gateway IOTARU SG-10.
  - Memudahkan audit dan diagnosa pemutusan jaringan maupun alasan filtering data beacon (RSSI terlalu rendah, UUID/Major mismatch, antrean penuh, gagal publish MQTT) dalam satu halaman fokus.
- **Dual-Engine Log Storage Architecture**:
  1. **Log Jaringan (`NetLogService`) - Persisten di LittleFS**:
     - Database dibatasi pada **maksimal 50 log terbaru** (FIFO Ring Buffer di RAM dan LittleFS `/storage/net_events.log`).
     - Aman dari wear-out karena event jaringan jarang terjadi dan dilindungi debouncing 2 detik.
     - Persisten: Data tetap utuh saat gateway restart atau mati listrik.
  2. **Log Drop Data Beacon (`DropLogService`) - In-RAM Static Rolling FIFO (Zero Flash Wear-out)**:
     - Dibatasi secara presisi pada **50 data paket drop terbaru** (`MAX_DROP_LOGS = 50`) yang disimpan **100% di memori RAM** (~1.6 KB alokasi statis).
     - **Flash Longevity Guard**: Menghindari kerusakan chip SPI flash akibat tingginya frekuensi drop paket beacon (bisa puluhan hingga ratusan per detik di lingkungan padat radio). Flash LittleFS sama sekali tidak ditulis saat drop terjadi.
     - Thread-safe: Dilindungi FreeRTOS Mutex dengan *non-blocking try-lock* (2 ms timeout), menjamin Core 0 UART ingestion dan throughput paket tetap berjalan dengan kecepatan mikrodetik tanpa latency atau packet loss.
- **Kebijakan Navigasi**:
  - URL Utama: `/log` (dengan alias `/log.html` dan backward-compatibility redirect `/net_log`).
  - Label Sidebar Desktop: `System Logs` dengan icon terminal/audit log.
  - Bebas Polusi Navigasi Mobile: Tetap menjaga kebersihan dock bottom nav mobile (`.bottom-nav`).
- **Anatomi Visual & Desain UI (`log.html` & `log.js`)**:
  - **Master Tab Switcher (Segmented Control Utama)**:
    - *Tab 1: Log Jaringan & Sistem* dengan live dot Cyan dan counter 50 data.
    - *Tab 2: Log Drop Data Beacon* dengan live dot Rose dan counter 50 data.
  - **Adaptive 4 KPI Status Cards**:
    - Mode Net Log: Wi-Fi Station, Ethernet W5500, MQTT Broker, Kapasitas LittleFS (`x / 50 Data`).
    - Mode Drop Log: Total Drop Sesi (akumulasi), Filter RSSI Min, Filter Target (UUID/Major mismatch), Status Buffer RAM (`x / 50 Data`).
  - **Control Toolbar Dinamis**:
    - Segmented Filter Kategori otomatis beralih:
      - Tab Net Log: `Semua`, `Wi-Fi`, `Ethernet`, `MQTT`, `SoftAP`, `System`.
      - Tab Drop Log: `Semua`, `RSSI Min`, `UUID`, `Major`, `Overflow`, `TTL`, `MQTT Fail`.
    - Input Pencarian Cepat real-time untuk kedua mode.
    - Toggle Auto-Refresh 5 detik dengan animasi pulse green dot.
    - Tombol **Export CSV Cerdas**: Otomatis mengunduh `net_log_sg10.csv` saat di Tab Jaringan atau `drop_log_sg10.csv` saat di Tab Drop Data.
    - Tombol **Hapus Log Kontekstual**: Modal konfirmasi SweetAlert Lite yang disesuaikan ("Hapus Log Jaringan?" vs "Hapus Log Drop Data?").
  - **Anatomi Tabel Drop Log 6 Kolom Presisi (Tanpa Kolom Keterangan)**:
    1. `#`: ID urut numerik monospace `#50`.
    2. `Waktu`: Format kalender aktual WIB `YYYY-MM-DD HH:MM:SS` atau uptime `+HH:MM:SS`.
    3. `Tipe`: Badge tipe frame (`iBeacon` Sky, `ibeacon_ext` Purple, `MQTT` Rose).
    4. `Tag / Major-Minor`: Monospace pill `10001-00042` atau `10 pkt`.
    5. `RSSI`: Indikator sinyal dBm berwarna (Hijau > -65, Kuning > -80, Merah ≤ -80).
    6. `Alasan Drop`: Semantic badge berwarna (`TTL Expired` Slate, `RSSI Min` Amber, `UUID/Major Mismatch` Orange, `Queue Overflow/MQTT Fail` Rose).
    *(Kolom Keterangan dieliminasi agar tampilan data tampil ringkas, bersih, dan berfokus pada audit cepat).*
- **REST API Endpoints**:
  - `GET /log` & `GET /log.html`: Streaming halaman `/storage/log.html`.
  - `GET /net_log`: Kompatibilitas link lama menuju `/storage/log.html`.
  - `GET /api/v1/system/net-logs`, `POST .../clear`, `GET .../export`: Endpoint audit log jaringan.
  - `GET /api/v1/system/drop-logs`: JSON ringkasan counter dan 50 data log drop.
  - `POST /api/v1/system/drop-logs/clear`: Menghapus riwayat 50 log drop data di memori RAM.
  - `GET /api/v1/system/drop-logs/export`: Mengunduh berkas CSV `drop_log_sg10.csv` (6 kolom presisi).

## 32. Simulator 2: RDR Hex Protocol & Gateway Heartbeat (`res/rdr`)
- **Tujuan Arsitektur**:
  - Mengemulasikan 4 reader gateway (`a4c13801`, `ee100101`, `c0100101`, `e1a00103`) untuk protokol biner 34-byte (68 karakter hex) pada port web `3334`.
  - Memancarkan transmisi tag pada topik `rdr/<id_gateway>` tiap 200 ms dan heartbeat periodik tiap 5000 ms (5 detik) ke topik `res/rdr`.
- **Spesifikasi Heartbeat**:
  - **Topik**: `res/rdr`
  - **Interval**: 5000 ms (5 detik).
  - **Payload**: String plain text `<id_module>` dari gateway aktif (mis. `"a4c13801"`), tanpa pembungkus JSON atau biner.
- **HUD & Visual Telemetry**:
  - **HUD Overlay Glassmorphism**:
    - `TOTAL MESSAGES`: Counter akumulasi pesan MQTT (Tag + Heartbeat).
    - `HEARTBEAT (5s)` (`#hud-heartbeats`): Counter emisi heartbeat gateway dengan token warna `#c084fc` (Purple Violet Neon).
    - `TOTAL PACKETS`: Counter paket biner deteksi tag.
    - `EGRESS BYTES`: Total throughput keluar dalam format KB.
  - **Real-time Log Box (`#log-box`)**:
    - Menampilkan paket transmisi hex 68 karakter: `📤 [rdr/<gw>] <tag>: <rssi> dBm...`
    - Menampilkan heartbeat gateway tiap 5 detik: `💓 [res/rdr] Heartbeat: "<id_module>" (<gw_name>)`
  - **Freeze Protection & Background Optimization**:
    - `Page Visibility API`: Mencegah antrian render canvas dan manipulasi DOM berlebihan saat browser berada di background/tab lain.
    - Log buffer dibatasi maksimal 25 item terbaru (`children.length > 25`).
    - Drag-and-drop mouse move di-throttle dengan interval 40 ms (`requestAnimationFrame`).

## 33. Firmware ESP32 Gateway Heartbeat Subsystem (Topik `CRHC3048`)
- **Tujuan**:
  - Mengirimkan sinyal detak jantung (*heartbeat*) periodik langsung dari gateway ESP32 ke broker MQTT pada topik independen `CRHC3048`.
- **Spesifikasi Kanal**:
  - **Topik**: `CRHC3048` (string konstan)
  - **Interval**: 5000 ms (5 detik)
  - **Payload**: `device_id` (plain text 8-karakter hex lowercase, misal: `"299cbddb"`), bersumber dari `Core::ConfigStore::getDeviceId()`.
  - **QoS**: `0` (At-most-once)
  - **Trigger Awal**: Emisi perdana dieksekusi seketika saat event `MQTT_EVENT_CONNECTED` diterima dari broker.
- **Integrasi Throughput & Bandwidth Monitoring**:
  - Setiap paket heartbeat dihitung secara akurat menggunakan `recordWireBytes(calculateWireBytes(strlen(s_topicHeartbeat), payloadLen, 0))`.
  - Transmisi heartbeat otomatis tercermin pada indikator live bandwidth dashboard Web UI (`#stat_net_bandwidth`) dan log supervisor diagnostik FreeRTOS (`[MONITOR] MQTT: ... BW: x.xx KB/s`).
- **Thread Safety**:
  - Berjalan langsung di dalam loop `mqttPublisherTask` (Core 1, Prioritas 6) untuk mencegah multithreading race condition dengan antrean batch data BLE.

## 34. AT24C256 I2C EEPROM Wear-Leveling & Zero Flash-Wear Protection
- **Latar Belakang & Filosofi Desain**:
  - Penulisan log berulang ke Flash SPI internal (LittleFS) dapat memicu keausan memori (*flash wear-out* / bad sectors).
  - Sistem beralih ke penyimpanan eksternal berbasis EEPROM I2C **AT24C256** (32 KB / 256 Kbit) dengan ketahanan tulis 1.000.000 kali per siklus.
- **Hardware Auto-Detection**:
  - I2C Bus: `I2C_NUM_0` dengan pin SDA = GPIO 21, SCL = GPIO 22, Clock = 100 kHz.
  - Alamat I2C IC: `0x50`.
  - Saat booting, driver `Bsp::At24c256::init()` melakukan *non-destructive probe* ke alamat `0x50`.
  - **Fallback RAM-Only**: Jika IC AT24C256 tidak terdeteksi, gateway beroperasi dalam mode RAM-Only (FIFO 50 entri). Seluruh penulisan ke LittleFS `/storage/net_events.log` sepenuhnya dihentikan untuk menjamin nol keausan Flash (*zero flash wear*).
  - Berkas lama di LittleFS dibiarkan tetap ada dan tidak dimodifikasi.
- **Arsitektur Rolling Circular Wear-Leveling (Zero Memory Shifting)**:
  - **Ukuran Slot**: 128 bytes (tepat 2 halaman AT24C256 @ 64 bytes).
  - **Total Kapasitas**: 200 slot fisik (menggunakan 25.600 bytes dari total 32.768 bytes).
  - **Struktur Slot Data (128B Packed)**:
    - `magic` (4B): `0xA55A1074` (Marker validitas slot).
    - `seq_id` (4B): ID monontonik terus bertambah tanpa reset.
    - `uptime_sec` (4B): Uptime saat kejadian log.
    - `time_str` (24B): Kalender nyata WIB atau uptime.
    - `category` (1B): Kategori jaringan (`NetCategory`).
    - `event` (1B): Jenis event (`NetEvent`).
    - `detail` (80B): Keterangan kontekstual (IP, SSID, Broker).
    - `checksum` (2B): XOR checksum integritas data.
    - `reserved` (8B): Padding keselarasan byte.
  - **Distribusi Keausan (Wear-Leveling)**:
    - Setiap penulisan log hanya menargetkan slot berikutnya secara melingkar: `nextSlot = (headSlot + 1) % 200`.
    - **Nol Pergeseran Fisik (Zero Shifting)**: Tidak ada pemindahan data slot lama di EEPROM (`memmove` hanya dilakukan di buffer RAM).
    - **Ketahanan Operasional**: $200 \times 1.000.000 = 200.000.000$ siklus penulisan (lebih dari 380 tahun operasi kontinu pada 1 log/menit).
  - **Headerless Slot Discovery Saat Booting**:
    - Booting membaca 8-byte header (`magic` + `seq_id`) dari 200 slot secara cepat.
    - Slot dengan `seq_id` tertinggi ditetapkan sebagai `headSlot`.
    - Menghindari penulisan berulang pada 1 blok header statis di alamat `0x0000`.
- **Integrasi REST API**:
  - Endpoint `GET /api/v1/system/net-logs` menyajikan informasi diagnostik:
    - `"storage_mode"`: `"AT24C256 EEPROM (200 Slots Rolling, 50 FIFO)"` atau `"RAM-Only (Zero Flash Wear)"`.
    - `"eeprom_detected"`: `true` / `false`.
    - `"flash_protected"`: `true`.

## 35. Security Hardening, Multi-Core Concurrency & Anti-Replay Subsystem
- **Latar Belakang & Ruang Lingkup**:
  - Peningkatan menyeluruh mencakup proteksi konkurensi dual-core Xtensa, integritas memori stack FreeRTOS, keamanan otentikasi REST & WebSocket, proteksi berkas partisi OTA, serta otentikasi dinamis anti-replay MQTT.
- **1. Multi-Core Atomic Statistics (`DropLogService`)**:
  - Anggota counter statistik `DropLogService` (`s_statTotalDropped`, `s_statTtlExpired`, dll.) diubah menjadi `std::atomic<uint32_t>` internal dengan order `std::memory_order_relaxed`.
  - Struktur publik `DropSummaryStats` tetap bertipe Plain Old Data (POD) struct untuk mempertahankan kompatibilitas penuh dengan REST API tanpa modifikasi antarmuka eksternal.
- **2. Stack Optimization & Dual-Checksum EEPROM Logging (`NetLogService`)**:
  - Alokasi pemindaian slot EEPROM `validSlots[200]` dipindahkan dari stack ke segmen static BSS (`s_scanSlots`), memangkas 1.6 KB beban stack FreeRTOS saat boot.
  - Perhitungan checksum slot disesuaikan ke tepat 118 bytes (`offsetof(Bsp::EepromSlotData, checksum)`).
  - Mekanisme pembacaan boot menerapkan *dual-checksum verification* (118B format baru atau 114B format lama), menjamin backward-compatibility penuh terhadap log yang sudah tersimpan di EEPROM fisik.
  - Pembersihan log (`clearLogs()`) melakukan reset instan pada counter RAM dan menerapkan *cooperative yielding* 1 tick (`vTaskDelay(1)`) antar slot EEPROM guna mencegah starvation task lain.
- **3. Thread-Safe Session Management & Anti-DoS (`SessionManager`)**:
  - Mutex FreeRTOS `s_sessionMutex` melindungi manipulasi tabel sesi dan tracking percobaan login dari akses konkuren multi-socket.
  - Delay pemblokiran `vTaskDelay(1800)` dihapus sepenuhnya dari thread server HTTP.
  - Rate limiting diubah menjadi pelacakan berbasis IP client (hingga 6 slot IP aktif) dengan masa penahanan 60 detik setelah 5 kegagalan berturut-turut.
  - Mekanisme re-login hanya mengenali token kriptografi valid (`matches_token`), sepenuhnya menghapus pengenalan via IP NAT (`matches_ip`) untuk mencegah pembajakan sesi di jaringan NAT bersama.
- **4. Defense-in-Depth HTTP Security & WebSocket Session Guard**:
  - Seluruh respon web dan file streaming menyertakan header keamanan:
    - `X-Content-Type-Options: nosniff`
    - `X-Frame-Options: DENY`
    - `Referrer-Policy: no-referrer`
  - Cookie otentikasi ditingkatkan menggunakan atribut `SameSite=Strict`.
  - Alokasi hak akses admin otomatis pada WebSocket (`allocate_ws_slot(fd, UserRole::ADMIN)`) dihapus total; frame WebSocket hanya diproses setelah memvalidasi session cookie atau query parameter `?token=`.
- **5. Filesystem OTA Flash Erase Protection (`OtaService`)**:
  - Pengecekan `esp_littlefs_mounted("storage")` sebelum eksekusi `esp_vfs_littlefs_unregister("storage")` dan proteksi error `0x103` (`ESP_ERR_INVALID_STATE`). Mencegah rusaknya VFS cache saat partisi LittleFS ditimpa, sekaligus menjamin proses upload LittleFS via Emergency Recovery Mode tetap berhasil meskipun LittleFS awalnya belum terpasang atau gagal mount saat cold boot.
- **6. Kredensial NVS Universal Terenkripsi AES-128 Hardware-Bound (`ConfigStore`)**:
  - Mempertahankan satu citra pabrik universal `nvs.bin` untuk seluruh unit ESP32 tanpa memerlukan pembakaran eFuse fisik.
  - Pada boot pertama atau saat penyimpanan, password dienkripsi secara software menggunakan algoritma AES-128-ECB dengan key 16-byte yang diturunkan secara deterministik dari MAC fisik unik ESP32 (`esp_read_mac`) ditambah salt konstan.
  - Password tersimpan di NVS dengan prefiks `enc:`. Data legacy tanpa prefiks tetap terbaca dan secara otomatis dimigrasikan ke bentuk terenkripsi.
  - Password didekripsi ke RAM operasional saat boot agar verifikasi hash dinamis MQTT dapat berjalan tanpa beban dekripsi berulang.
  - Fallback kredensial hardcoded pada broker MQTT dihapus sepenuhnya.
- **7. Otentikasi Dinamis MQTT & 10-Item Rolling FIFO Anti-Replay (`MqttService`)**:
  - **Pencocokan Topik Eksak**: Topik perintah diverifikasi dengan panjang string eksak (`event->topic_len == strlen(s_topicCmd)`), mencegah tabrakan subtopik.
  - **Format Payload JSON**: `{"cmd":"<perintah>", "seq":<angka>, "auth":"<sha256_hex>"}`.
  - **Rumus Otentikasi SHA-256**:
    $$\text{auth} = \text{SHA256}(\text{admin\_username} + \text{":"} + \text{admin\_password\_asli} + \text{":"} + \text{cmd} + \text{":"} + \text{seq})$$
  - **Respon Kode Numerik Terpadu**:
    - `0`: SUCCESS (`{"res":"ok", "code":0, "cmd":"...", "seq":...}`)
    - `1`: BAD_FORMAT (`{"res":"err", "code":1, "seq":...}`)
    - `2`: AUTH_FAILED (`{"res":"err", "code":2, "cmd":"...", "seq":...}`)
    - `3`: DUPLICATE_SEQ (`{"res":"err", "code":3, "cmd":"...", "seq":...}`)
    - `4`: UNKNOWN_CMD (`{"res":"err", "code":4, "cmd":"...", "seq":...}`)
  - **Anti-Replay Rolling FIFO (10 Data Terakhir)**:
    - Sequence number yang sama hanya dapat dieksekusi satu kali. Sequence yang berubah (lebih tinggi maupun lebih rendah) dapat diterima secara valid.
    - Untuk perintah berfrekuensi tinggi (`ping`), rolling FIFO 10 entri dikelola di RAM guna menghemat umur pakai EEPROM.
    - Untuk perintah kritis (`reboot`, `identify`), rolling FIFO 10 entri dipersistensikan ke halaman EEPROM AT24C256 di alamat `0x6400` (64 bytes dengan magic `0x53455131` dan checksum XOR), dengan fallback otomatis ke RAM jika IC EEPROM tidak terpasang.
  - **Delayed Reboot Task**: Eksekusi perintah `reboot` dialihkan ke task FreeRTOS terpisah dengan delay 1000 ms agar paket respon MQTT QoS 1 terkirim dan ter-flush sempurna ke broker sebelum sistem restart.

## 36. Production-Grade Hardening, Zero-Corrupt Password Masking & System Integrity
- **Latar Belakang & Ruang Lingkup**:
  - Pengerasan level produksi industri mencakup mitigasi kerentanan keamanan web, pencegahan silent password corruption pada antarmuka pengguna, penguatan kriptografi NVS ke AES-128-CBC dengan per-device salt, serta optimasi stabilitas multi-core Xtensa.
- **1. Zero-Corrupt UI Password Masking (`web_template.cpp` & `web_routes.cpp`)**:
  - Seluruh field password sensitif (`admin_pass`, `user_pass`, `w_password`, `ap_password`, `mqtt_password`) di-*masking* dengan representasi titik peluru `••••••••` saat di-*render* ke form HTML/template.
  - Endpoint REST API (`api_config_auth_handler`) menerapkan filter predikat `is_masked_or_empty(val)`: jika nilai yang dikirim browser berupa `••••••••`, `********`, atau string kosong, konfigurasi password yang ada di NVS tidak akan ditimpa atau dirusak.
  - Validasi panjang minimal 8 karakter dan larangan karakter `:` diberlakukan untuk kredensial autentikasi.
  - Pembatasan peran (Role-Based Access Control): akun teknisi dibatasi hanya dapat mengubah password teknisi dan tidak diizinkan mengubah kredensial administrator.
- **2. Emergency Recovery Mode Hardening (`emergency_page.cpp`)**:
  - Menghapus atribut default `value="admin"` pada input kata sandi administrator untuk mencegah eskalasi akses tanpa otorisasi.
  - Membersihkan timer pemulihan (`clearInterval`) setelah hitung mundur reboot selesai guna mencegah memory/timer leak di peramban klien.
  - Menghapus isian default `'admin'` pada dialog konfirmasi `prompt()` fungsi reboot darurat.
- **3. Defense-in-Depth HTTP Content-Security-Policy & Path Traversal Guard**:
  - Menambahkan header `Content-Security-Policy: default-src 'self' 'unsafe-inline' data: ws:;` pada setiap respon HTTP untuk memitigasi serangan Cross-Site Scripting (XSS).
  - Melindungi handler aset berkas (`assets_wildcard_handler`) dari eksploitasi direktori traversal (`..` dan `//`).
  - Mengamankan pemanggilan `fopen` untuk berkas statis dan gzip dengan mode biner `"rb"`.
  - Memperluas buffer Set-Cookie HTTP (`cookie_hdr`) menjadi 192 bytes untuk menampung token sesi 64-karakter (256-bit entropy).
- **4. Cryptographic Storage Hardening (`config_store.cpp`)**:
  - Meningkatkan cipher enkripsi password NVS dari AES-128-ECB menjadi **AES-128-CBC** dengan Initialization Vector (IV) 16-byte acak per-penulisan (`esp_fill_random`).
  - Menghasilkan dan memvalidasi salt unik per-unit di partisi NVS (`cfg_sys:k_salt`) sehingga dua gateway dengan MAC identik atau password identik menghasilkan ciphertext yang berbeda.
  - Mendukung panjang kata sandi hingga 63 karakter tanpa risiko buffer truncation.
  - Mempertahankan kompatibilitas penuh backward-compatibility: sistem secara otomatis mendeteksi format lama (`enc:` atau plaintext) dan memigrasikannya ke format terenkripsi CBC ber-prefiks `cbc:`.
- **5. Session Entropy & Timing Attack Prevention (`session_mgr.cpp`)**:
  - Ukuran token sesi diperluas menjadi 64 karakter heksadesimal (256-bit entropy).
  - Mengimplementasikan perbandingan *constant-time* (`constant_time_eq`) pada seluruh tahap verifikasi, pembaharuan, dan pemusnahan token sesi untuk mencegah serangan *timing attack*.
- **6. Multi-Core Concurrency & Deadlock Prevention**:
  - Mengisolasi task reboot dan reset ke Core 1 (`xTaskCreatePinnedToCore(..., 1)`).
  - Menyeragamkan penguncian buffer telemetri beacon (`s_beaconBufMutex`) untuk melindungi buffer live dan buffer drop tanpa fragmentasi mutex.
  - Mengadopsi pola *Copy-Then-Send* pada WebSocket broadcast (`broadcast_ws_text`) untuk mencegah kondisi deadlock antara HTTP socket send dan session lock.
  - Menyesuaikan prioritas `nRF_UART_Task` ke `configMAX_PRIORITIES - 4` untuk menjamin tidak terjadinya kelaparan proses (*starvation*) pada task jaringan dan sistem.
- **7. OTA Integrity & Size Verification (`ota_service.cpp` & `web_routes.cpp`)**:
  - Memverifikasi header biner OTA pada chunk pertama (minimal 112 byte) sebelum melakukan penulisan ke flash.
  - Memvalidasi batas ukuran maksimum berkas OTA (maksimal 4 MB) menggunakan konversi aman `strtoul`.
  - Melakukan verifikasi integritas sistem berkas pasca-flash dengan memeriksa keberadaan dan ukuran file krusial (`index.html` dan `app.js`).
- **8. Intranet Authentication & Browser Password Manager Suppression (`web_routes.cpp` & `login.html`)**:
  - **Cookie Policy (`SameSite=Lax`)**: Seluruh respon header `Set-Cookie` (`api_login_handler`, `api_logout_handler`, `api_auth_check_handler`, `api_touch_ap_handler`, `api_ota_upload_handler`) menggunakan atribut `SameSite=Lax`. Hal ini memastikan cookie sesi HTTP pada alamat IP intranet/lokal selalu disertakan oleh peramban berbasis Chromium (Edge, Chrome) pada navigasi top-level tanpa memicu kecurigaan atau isolasi cookie ketat.
  - **DOM Form Credential Sanitization (`login.html`)**:
    - Elemen form menggunakan `autocomplete="off"` dan `onsubmit="return false;"`, serta tombol submit diubah menjadi tombol mandiri `<button type="button" id="btnLogin">` guna mencegah penangkapan form submit standar oleh peramban.
    - Input dilengkapi atribut eksplisit `autocomplete="username"` dan `autocomplete="current-password"` dengan `spellcheck="false"`.
    - Dukungan keyboard `Enter` ditangani melalui event listener independen.
    - Sesaat setelah respon otentikasi `200 OK` diterima dari ESP32, nilai elemen `#password` dan `#username` di DOM langsung dikosongkan (`value = ""`) sebelum `window.location.href = "/"` dieksekusi. Pola ini mencegah Windows Web Account Manager (WAM) dan Microsoft Wallet memicu sinkronisasi akun saat event `beforeunload`/`unload`, sehingga secara efektif mengeliminasi pop-up error Windows `0x8007052e`.
- **9. Overview Spec Grid: Dynamic MAC Address & Realtime Signal Stability (`net_manager.cpp`, `web_routes.cpp`, `web_template.cpp`, & `index.html`)**:
  - **Tile MAC Address Dinamis**: Menambahkan tile spesifikasi dengan label dinamis `%mac_label%` (`MAC Address (LAN)` saat koneksi Ethernet aktif vs `MAC Address (Wi-Fi)` saat koneksi Wi-Fi) dengan tipografi monospace cyan neon `#38bdf8` (`#valMacAddress`).
  - **Abstraksi Hardware MAC (`NetworkManager::getActiveMac`)**: Memilih MAC interface W5500 via IOCTL driver saat Ethernet terpasang, atau eFuse Wi-Fi STA saat mode nirkabel, menjamin pembacaan konsisten tanpa overhead.
  - **Realtime RSSI Polling Stability**: Endpoint `/api/v1/system/version` menyertakan key numerik `wifi_rssi` saat Wi-Fi terhubung, mencegah nilai Signal Strength di Overview berubah menjadi strip `-` pada siklus polling berkala (5 detik).
- **10. Professional System Logging Taxonomy, Audit Trail & Hardware-Level Diagnostics (`net_log_service.*`, `EthernetW5500.cpp`, `mqtt_service.cpp`, & `web_routes.cpp`)**:
  - **Zero-Speculative Logging Taxonomy**:
    - Seluruh terminologi probabilistik (*"kemungkinan"*, *"mungkin"*, atau *"diduga"*) dieliminasi secara ketat dari seluruh subsistem logging.
    - Setiap pesan log menyajikan fakta teknis deterministik berbasis status register hardware, event handler ESP-IDF, dan errno TCP/IP socket standar POSIX/lwIP.
  - **Comprehensive Operational Audit Trail**:
    - Penambahan event audit operasional pada `NetEvent` enum (bertipe `uint8_t` sehingga ukuran `EepromSlotData` tetap tepat 128 bytes dan kompatibel penuh dengan EEPROM fisik):
      - `AUTH_UPDATE`: Pembaruan kredensial username/password administrator atau teknisi (`Kredensial Akun Diperbarui oleh Admin/Teknisi`).
      - `CONFIG_UPDATE`: Perubahan konfigurasi sistem dari antarmuka Web UI (`Pengaturan Sistem Disimpan oleh Admin`).
      - `CONFIG_DISCARD`: Pembatalan konfigurasi tertunda (*staged configuration*) oleh operator (`Perubahan Konfigurasi Jaringan Dibatalkan oleh Pengguna`).
      - `LOG_CLEARED`: Pembersihan histori log sistem pada memori EEPROM (`Seluruh Riwayat Log Sistem Dibersihkan oleh Admin/Teknisi`).
      - `CONFIG_RESET`: Eksekusi factory reset / inisialisasi ulang konfigurasi pabrik (`Konfigurasi Sistem Direset ke Standar Pabrik (Web Admin)`).
  - **Deep-Dive Ethernet Diagnostics (Wiznet W5500)**:
    - **Link Up Auto-Negotiation**: Menanyakan status register PHY via `esp_eth_ioctl(ETH_CMD_G_SPEED)` dan `esp_eth_ioctl(ETH_CMD_G_DUPLEX_MODE)`, mencatat kecepatan aktual (`10/100 Mbps`), mode transmisi (`Full/Half-Duplex`), serta alamat MAC fisik interface LAN.
    - **Physical Link Down**: Mendeteksi pencabutan kabel RJ45 atau port switch nonaktif secara deterministik (`Kabel Fisik RJ45 Terputus / Switch Port Nonaktif (Link Down)`).
    - **Hardware Interface Deactivation**: Pencatatan resmi saat mode switched ke Wi-Fi (`Interface Ethernet Dinonaktifkan (Peralihan ke Mode Wi-Fi)`).
    - **IP Allocation Mode**: Mencatat mode perolehan IP (`DHCP` vs `Static`), subnet mask, dan gateway address (`IP (DHCP/Static): X.X.X.X | GW: Y.Y.Y.Y | Mask: Z.Z.Z.Z`).
    - **Controller Init Guard**: Pencatatan deteksi hardware W5500 pada bus SPI beserta status kegagalan komunikasi jika chip tidak merespon SPI (`Inisialisasi Hardware W5500 SPI Controller Berhasil` vs `Hardware W5500 Tidak Terdeteksi / SPI Gagal`).
  - **Definitive MQTT Error & Disconnect Cause Mapping**:
    - **Errno Translator**: Menerjemahkan `sock_errno` dari transport layer lwIP ke terminologi baku:
      - `ECONNRESET (104)`: `Koneksi Direset oleh Server / Broker (RST)`
      - `ETIMEDOUT (110)`: `Koneksi TCP Timeout (Batas Waktu Terlampaui)`
      - `EHOSTUNREACH (113)`: `Host Broker Tidak Dapat Dijangkau (Unreachable)`
      - `ECONNREFUSED (111)`: `Koneksi Ditolak oleh Broker (Port Tertutup / Service Down)`
      - `EPIPE (32)`: `Jalur Komunikasi TCP Terputus (Broken Pipe)`
      - `ENOTCONN (128)`: `Socket Tidak Terhubung (Socket Not Connected)`
      - `EHOSTDOWN (118)`: `Host Broker Tidak Aktif (Host Down)`
    - **Clean Broker FIN / Keepalive Timeout**: Ketika socket ditutup tanpa errno error (`error_type == NONE`), secara deterministik dicatat sebagai `Sesi Dihentikan oleh Broker (TCP FIN / Keepalive Timeout)`.
    - **Protocol-Level Return Codes**: Menerjemahkan kode error return MQTT connect:
      - `1`: `Protokol Tidak Diterima (Unacceptable Protocol Version)`
      - `2`: `Identifier Ditolak (Identifier Rejected)`
      - `3`: `Server Broker Tidak Tersedia (Server Unavailable)`
      - `4`: `Kredensial Tidak Valid (Bad Username or Password)`
      - `5`: `Akses Ditolak / Tidak Memiliki Izin (Not Authorized)`
    - **Keepalive Optimization**: Interval heartbeat keepalive MQTT dinaikkan dari 15 detik ke 30 detik untuk meningkatkan toleransi latensi jaringan seluler/WAN serta mengurangi pemutusan agresif.
- **11. Beacon Drop Pipeline Hardening, Zero-Loss Telemetry Synchronization & Hardware Observability (`drop_log_service.*`, `beacon_pipeline.*`, `mqtt_service.cpp`, `main.cpp`, `bsp_uart.cpp`)**:
  - **Zero-Loss TTL Pruning Pipeline**:
    - Paket yang melebihi batas waktu hidup `data_ttl` saat ditarik oleh `mqttPublisherTask` dicatat ke sistem drop log sebelum dilewati (`Core::BeaconPipeline::recordDroppedBeacon(packet, DropReason::TTL_EXPIRED)`).
    - Menghidupkan fungsi pembersihan antrean `Core::BeaconPipeline::pruneExpired()` secara berkala pada `supervisorTask` (`src/main.cpp`), menjamin paket kedaluwarsa dibersihkan dan dicatat bahkan saat koneksi MQTT sedang terputus.
  - **Batch Buffer Flush Protection**:
    - Mengeliminasi *silent drop* saat koneksi jaringan atau MQTT putus sesaat. Jika `batchCount > 0` saat koneksi terputus, memanggil `Core::BeaconPipeline::recordMqttFailed(batchCount)` terlebih dahulu sebelum mereset buffer batch ke nol.
  - **Unified Packet-Count Drop Synchronization**:
    - Menambahkan parameter `uint32_t count = 1` pada `DropLogService::record()`. Saat batch berisi $N$ paket gagal, counter akumulatif atomik `s_statTotalDropped` dan `s_statMqttFailed` bertambah sebesar $N$ paket (bukan 1 batch), menyinkronkan metrik antara WebSocket Live Monitor dan kartu KPI Drop Log REST API.
  - **Consolidated Single Source of Truth (`getTotalDropped()`)**:
    - Fungsi `BeaconPipeline::getTotalDropped()` mengagregasi seluruh 6 kategori drop (`TTL + RSSI + UUID + Major + Queue + MQTT`), menjamin kesesuaian nilai total header WebSocket dengan rincian drop summary.
  - **Synchronized Log & Counter Clear**:
    - Operasi penghapusan log drop (`POST /api/v1/system/drop-logs/clear`) mengosongkan tabel riwayat 50 data di RAM dan serentak mereset seluruh variabel atomik sesi (`s_statTotalDropped`, `s_statTtlExpired`, dll.) serta counter pipeline via `BeaconPipeline::resetDropCounters()`.
  - **Edge-Case Hardening & Concurrency Protection**:
    - **TTL Zero Guard**: `BeaconPipeline::pruneExpired()` memvalidasi `g_config.data_ttl <= 0` di awal fungsi guna mencegah pembuangan paket baru secara tidak sengaja saat TTL dinonaktifkan (`data_ttl = 0`).
    - **Queue Contention & Race Prevention**: `supervisorTask` hanya memanggil `pruneExpired()` saat MQTT offline (`!MqttService::isConnected()`), mengeliminasi kontensi antrean dan *preemption race* antara `xQueuePeek` dan `xQueueReceive` dengan `mqttPublisherTask` (Prioritas 6).
    - **Unconditional Atomic Stats Clear**: Reset seluruh variabel atomik `s_stat*` pada `DropLogService::clearLogs()` diposisikan di luar penguncian mutex `s_dropMutex`, menjamin reset metrik kartu KPI 100% instan tanpa risiko terhambat timeout mutex.
- **12. Telemetri Total Reboot ESP32, Disconnect Counters Per-Interface & Persistensi AT24C256 EEPROM / RAM Fallback**:
  - **Pemisahan Domain Metrik Mutlak**:
    - **Total Reboot ESP32**: Didedikasikan 100% untuk menghitung siklus boot hardware chip ESP32 (`boot_count` kali), dilengkapi keterangan alasan reset hardware terakhir (`esp_reset_reason()`: Power-On Cold Boot, Software Reboot Manual, Task Watchdog Timer, Brownout, dll.).
    - **Disconnect Counters Per-Interface**: Metrik pemutusan koneksi dipisahkan secara independen pada kartu masing-masing antarmuka:
      - Wi-Fi Station: `IP: X.X.X.X • DC: <n> kali`
      - Ethernet W5500: `IP: Y.Y.Y.Y • DC: <n> kali`
      - MQTT Broker: `Port: 1883 • DC: <n> kali`
  - **Alokasi EEPROM Mandiri Bebas Benturan (`0x6440`)**:
    - Struktur data `SystemStatsStore` dialokasikan pada alamat `0x6440` (persis 64 byte, 1 halaman AT24C256).
    - Mempertahankan integritas slot 200 log jaringan (`0x0000 - 0x63FF`) dan `MqttSeqStore` (`0x6400 - 0x643F`).
    - Dilindungi checksum **CRC16-LE** untuk ketahanan terhadap pemadaman listrik tiba-tiba (*power-cut resilience*).
    - Mekanisme *throttled commit* (maksimal tulis setiap 3 detik) untuk menjaga ketahanan siklus tulis EEPROM.
    - Fallback otomatis ke variabel static RAM jika modul EEPROM I2C tidak terpasang (*Zero Flash Wear*).
  - **Perbaikan Bug Ethernet W5500 Link Up**:
    - Mengeliminasi kebocoran evaluasi `isLinkUp()` yang sebelumnya mengembalikan `true` saat Wi-Fi aktif.
    - Menambahkan `NetworkManager::isEthernetLinkUp()` yang murni memeriksa status fisik chip SPI W5500 (`s_ethDriver && s_ethDriver->isConnected()`).
    - Jika kabel RJ45 dicabut, kartu Ethernet menampilkan status **LINK DOWN** (badge merah) dan keterangan **Kabel Terputus** secara deterministik.
  - **Penggantian KPI Drop Tab ke-4**:
    - Mengganti kartu redundan *Buffer Riwayat / 50 Data* dengan **`GAGAL PUBLISH MQTT`** (`#kpiDropMqtt`, badge rose `Broker Fail`), terhubung langsung ke akumulasi `summary.mqtt_failed`.
- **13. Komponen Navigasi Paging 10 Data per Halaman (10-Item Table Pagination)**:
  - **Kapasitas Standar**: 10 baris per halaman (`PAGE_SIZE = 10`), diterapkan seragam pada kedua tabel di `/log` (*Log Jaringan & Sistem* dan *Log Drop Data Beacon*).
  - **Anatomi UI Footer (`.log-pagination-wrap`)**:
    - **Info Rentang**: `Menampilkan 1 - 10 dari 50 data` (dinamis mengikuti filter & pencarian).
    - **Navigasi Tombol**: `[‹ Prev] [ 1 ] [ 2 ] [ 3 ] [ 4 ] [ 5 ] [ Next ›]`.
  - **Gaya Desain Dark Cyber**:
    - Tombol `.btn-page`: Background `rgba(255,255,255,0.05)`, border `1px solid rgba(255,255,255,0.1)`, teks slate `#94a3b8`, font monospace, min-width 34px.
    - State Aktif: Background `#38bdf8`, teks gelap `#0f172a`, font-weight 700, cyan glowing shadow `0 0 10px rgba(56,189,248,0.4)`.
    - State Hover: Background `rgba(56, 189, 248, 0.15)`, border `#38bdf8`, teks cerah.
    - State Disabled: Opacity 0.35, kursor `not-allowed`.
  - **Smart UX Auto-Reset**:
    - Mengklik tab kategori segmen atau mengetik di kotak pencarian secara instan menghitung ulang total halaman dan mereset posisi navigasi ke **Halaman 1**.
- **14. Kebijakan Pencatatan Drop Log: Isolasi Kondisi Online (MQTT Connected Only)**:
  - **Prinsip Arsitektur**:
    - Log Drop Data Beacon (`/log?tab=drop`) difokuskan murni untuk memantau performa pipeline dan integritas transmisi data saat sistem dalam kondisi operasional normal (**MQTT Connected**).
    - Kegagalan transmisi selama fase inisialisasi awal (booting), proses negosiasi DHCP Wi-Fi/Ethernet, atau saat koneksi internet terputus dikategorikan sebagai anomali jaringan (*Network Event*) dan dicatat secara akurat di *Log Jaringan & Sistem* (Network Logs) serta counter *Disconnect*.
  - **Mekanisme Eksekusi (Zero False-Drop Enforcement)**:
    - `BeaconPipeline::recordDroppedBeacon()` dan `DropLogService::record()` dilindungi oleh evaluasi predikat: alasan drop pipeline normal (`TTL_EXPIRED`, `RSSI_OUT_OF_RANGE`, `UUID_MISMATCH`, `MAJOR_MISMATCH`, `QUEUE_OVERFLOW`) hanya dicatat saat MQTT Connected.
    - **Pengecualian Khusus Gagal Publish MQTT (`DropReason::MQTT_FAIL`)**: Paket batch yang gagal dipublish ke broker MQTT (akibat socket error, outbox overflow, atau koneksi Wi-Fi/MQTT terputus saat batch siap kirim) **tetap selalu dicatat** ke Log Drop dan counter `GAGAL PUBLISH MQTT` meskipun sistem dalam kondisi disconnect.
    - `BeaconPipeline::pruneExpired()` tetap melakukan pembersihan antrean saat offline agar RAM FreeRTOS tidak jenuh, namun pembersihan ini dilakukan secara *silent* tanpa mencemari Log Drop.
    - `BeaconPipeline::flushQueue()` dieksekusi secara otomatis pada event `MQTT_EVENT_CONNECTED`. Paket beacon usang yang terakumulasi selama jeda koneksi langsung dikosongkan sebelum task publisher mulai bekerja, menjamin tidak ada lonjakan drop TTL palsu sesaat setelah gateway terhubung.

## 37. Fitur Enkripsi MQTTS (MQTT over SSL/TLS) Dinamis & Factory NVS Synchronization
- **Latar Belakang & Spesifikasi**:
  - Menyediakan perlindungan privasi dan keamanan tingkat industri untuk transmisi telemetri beacon, status gateway, dan penerimaan perintah kendali melalui saluran terenkripsi MbedTLS (TLS 1.2 / 1.3).
  - Pilihan protokol bersifat dinamis: pengguna dapat beralih antara **MQTT Plain (TCP)** dan **MQTTS (TLS/SSL)** langsung dari antarmuka Web Admin (`/auth_admin.html`) tanpa perlu kompilasi ulang firmware.
- **1. Master NVS & Factory Default Synchronization (`nvs_config.csv` & `config_store.cpp`)**:
  - **Tabel Partisi Master**: Menambahkan baris `use_ssl,data,u32,0` di bawah namespace `mqtt` pada berkas `nvs_config.csv` (standar default pabrik: Plain TCP `0`).
  - **First-Boot Clone**: Fungsi `initNvsConfiguration()` menyalin `use_ssl` dari partisi master `mqtt` ke working namespace `cfg_mqtt` (`nvsCopyU32(hMaster, hDest, "use_ssl", 0);`).
  - **Factory Reset Safety**: Pada eksekusi factory reset via tombol boot recovery atau Web UI, parameter `use_ssl` dikembalikan ke default `0`.
- **2. Driver & Engine MQTTS ESP-IDF (`mqtt_service.cpp`)**:
  - **Dinamika URI Scheme**: Menggunakan `mqtts://` saat `use_ssl == 1` dan `mqtt://` saat `use_ssl == 0`.
  - **Transport & Verifikasi**: Menggunakan URI Scheme (`mqtt://` vs `mqtts://`) untuk menentukan transport tanpa mengisi `broker.address.transport` secara manual (mencegah `ESP_ERR_INVALID_ARG` akibat konflik skema URI di ESP-IDF). Saat SSL aktif, sistem menyematkan **`esp_crt_bundle_attach`** (kumpulan ~200 Mozilla Root CA resmi ESP-IDF untuk memverifikasi seluruh broker publik secara otomatis seperti PC/MQTT Explorer) serta mengirimkan ekstensi **TLS SNI (Server Name Indication)** secara presisi agar kompatibel penuh dengan arsitektur multi-tenant Cloud Serverless (misal EMQX Cloud Serverless). Pesan Last Will (LWT) tetap mempertahankan `retain = 1` sesuai kebutuhan state presensi gateway.
  - **Task Stack Safety & Reconnect Sequence**: Alokasi `mqtt_cfg.task.stack_size` dinaikkan menjadi `8192` bytes guna menjamin kecukupan memori stack selama proses negosiasi kriptografi TLS Handshake. Fungsi `forceReconnect()` menangani `esp_mqtt_client_start()` secara adaptif pada koneksi jaringan pertama.
- **3. Antarmuka Web & Smart Port Switcher (`auth_admin.html`, `web_template.cpp`, & `web_routes.cpp`)**:
  - **Komponen Switch**: Mengadopsi `.toggle-card-row` dengan switch iOS `.ios-switch` pada Bento Card 4 (*Broker Telemetri MQTT Gateway*).
  - **Smart Port Adjustment**: Script JavaScript secara otomatis mengganti nilai input port menjadi `8883` saat MQTTS diaktifkan (jika sebelumnya `1883` atau `1884`), dan mengembalikannya ke `1884` saat dinonaktifkan.
  - **Integrasi REST API**: Endpoint `POST /api/v1/config/auth` mem-parsing parameter `mqtt_use_ssl` dan menyimpannya secara persisten ke NVS Flash.
- **4. Telemetri & Visualisasi Status (`log.html` & `log.js`)**:
  - Pada Card 3 (*MQTT Broker*), jika mode MQTTS aktif (`data.mqtt_ssl === true`), judul kartu berubah menjadi **MQTTS Broker** dan port menampilkan indikator gembok enkripsi: **`🔒 8883`**.

## 38. Fitur Auto-Detection Direct LAN Peer-to-Peer ke PC & Fallback Micro-DHCP Server
- **Latar Belakang & Spesifikasi**:
  - Memungkinkan gateway terhubung langsung ke PC/laptop via kabel LAN (straight-through atau crossover) tanpa memerlukan router atau server DHCP eksternal.
  - Perangkat otomatis mendeteksi ketiadaan server DHCP (timeout probe 11.5s - 13.5s dengan jitter berbasis MAC) dan secara cerdas mengaktifkan **Embedded Micro-DHCP Server** pada subnet terpadu **`192.168.4.1`**.
  - Laptop teknisi yang berada pada setting default "Obtain an IP address automatically" otomatis menerima IP **`192.168.4.2`** dalam hitungan milidetik tanpa perlu setting IP manual di Windows/macOS/Linux.
- **1. Visual Tokens & Komponen UI (`app.css` & `index.html`)**:
  - **Badge Mode Direct LAN (`.badge-tech.direct`, `.badge-tech.mode-config`)**:
    - Background: `rgba(59, 130, 246, 0.2)`
    - Text: `#60a5fa` (Blue 400)
    - Border: `1px solid rgba(59, 130, 246, 0.4)`
    - Teks Label: `Ethernet (Direct PC 192.168.4.1)` untuk Master Node, atau `Ethernet (Peer Slave 192.168.4.X)` untuk Slave Node.
- **2. Indikasi Fisik LED COM (`bsp_led.cpp`)**:
  - Saat mode Direct LAN (Master maupun Slave) aktif, sistem memanggil `Bsp::LedManager::setConfigMode(true)`.
  - Lampu **LED COM** berkedip **BIRU 1 Hz** (500ms ON, 500ms OFF) identik dengan mode konfigurasi Hotspot, memberikan kepastian visual bahwa gateway sedang dalam mode konfigurasi.
- **3. Zero Internet Blackout Enforcement (RFC 2131)**:
  - Micro-DHCP sengaja **meniadakan Option 3 (Router/Gateway)** dan **Option 6 (DNS)** pada paket penawaran BOOTP.
  - Laptop teknisi hanya membuat rute on-link untuk `192.168.4.0/24`, sehingga koneksi internet Wi-Fi di laptop tetap aktif normal 100%.
- **4. Multi-Device Hub & ARP Anti-Collision**:
  - Mengadopsi ARP Probe (RFC 5227) sebelum menetapkan `192.168.4.1` dengan MAC-seeded timer jitter.
  - Perangkat kedua di hub dummy otomatis menjadi Slave dengan IP unik `192.168.4.X` dan mematikan DHCP server untuk mencegah konflik.
- **5. Discovery Lintas Subnet**:
  - Protokol UDP Discovery Port `30303` (skrip `scripts/scan_gateway.py`) dan mDNS (`http://sg10-<id>.local`) aktif mengumumkan status gateway baik pada mode DHCP, Direct LAN, maupun IP Statis lama.
- **6. Dukungan Dual-Interface (Wi-Fi Online + Direct LAN Maintenance Port)**:
  - **Prinsip Arsitektur**:
    - Port Ethernet W5500 selalu diinisialisasi dalam kondisi siaga (*Maintenance Ready*) baik saat gateway dikonfigurasi pada mode Ethernet maupun Mode Wi-Fi (`ConnectionMode::WIFI`).
    - Kapan pun kabel LAN dicolokkan ke laptop teknisi saat gateway sedang beroperasi di Mode Wi-Fi, port Ethernet otomatis beralih ke mode Direct LAN (`192.168.4.1`) dengan Micro-DHCP aktif.
  - **Proteksi Routing Internet & MQTT Cloud**:
    - Default route internet (`0.0.0.0/0`) tetap dipertahankan 100% pada interface Wi-Fi Station (`wlanif`), karena Ethernet Direct LAN sengaja tidak menetapkan Default Gateway (`gw = 0.0.0.0`).
    - **Proteksi DNS lwIP & Anti-Hijacking**: Mengatasi perilaku internal ESP-IDF lwIP di mana panggilan `esp_netif_set_ip_info()` pada interface Ethernet secara otomatis memanggil `dns_clear_servers(true)` yang menghapus seluruh DNS server secara global. Sistem secara proaktif mengunci `s_staNetif` sebagai default netif dan memulihkan DNS utama (`8.8.8.8`) serta backup (`1.1.1.1`) via `dns_setserver()`, sekaligus mengabaikan `IP_EVENT_ETH_GOT_IP` saat mode Wi-Fi aktif.
    - Transmisi data telemetri beacon via MQTT ke cloud terus berjalan lancar tanpa terputus sama sekali saat teknisi melakukan konfigurasi.
  - **Representasi UI Dashboard (`index.html`)**:
    - Saat dual-interface aktif (Wi-Fi Online + Direct LAN tersambung), badge status koneksi menampilkan label cerdas: **`Wi-Fi + Direct LAN (192.168.4.1)`** dengan styling `.badge-tech.direct` (biru).
    - Tile sinyal otomatis menampilkan **`Ethernet Link (Direct LAN)`** dengan kecepatan link (misal `100 Mbps (Full Duplex)`), dan label MAC beralih ke **`MAC Address (LAN)`**.
  - **Restorasi Otomatis saat Kabel Dicabut**:
    - Ketika teknisi mencabut kabel LAN, event `ETHERNET_EVENT_DISCONNECTED` seketika mematikan Micro-DHCP, mereset IP Ethernet ke `0.0.0.0`, mematikan mode konfigurasi LED COM (`setConfigMode(false)`), dan menyinkronkan kembali COM LED ke status online Wi-Fi (Solid Green) tanpa perlu reboot perangkat.
## 39. Overview Bento Grid & Dual Interface System (2026 Cybernetic Dark)
- **Latar Belakang & Spesifikasi**:
  - Merekonstruksi antarmuka halaman utama Overview (`data/index.html`) mengadopsi struktur visual Bento Grid modern 12 kolom untuk desktop dan 1 kolom bertumpuk untuk mobile.
  - Memberikan visualisasi terpisah yang jelas antara **Wi-Fi Interface** dan **Ethernet Interface** beserta parameter IP, MAC, status link, sinyal, dan telemetri MQTT.
  - Mempertahankan integritas navigasi (Sidebar, Mobile Top Navbar, Mobile Bottom Navigation) dan banner keselamatan (`#pendingRebootBanner` dan `#apModeBanner`) tanpa menyentuh bagian di luar `<section class="content">`.
  - Beroperasi 100% mandiri (*Zero CDN Dependency*) menggunakan Vanilla CSS murni dan Inline SVG, sehingga antarmuka tetap tampil sempurna di lingkungan industri tertutup tanpa akses internet.

- **1. Slim Header Card (`.sg-header-card`)**:
  - **Sisi Kiri**:
    - Ikon Microchip CPU (`#60a5fa`) dalam kontainer rounded dengan latar belakang `rgba(59, 130, 246, 0.12)`.
    - Judul: `SMART GUARD (%model%)` font tebal `700`, titik pemisah `•`, dan subtitle `Industrial Smart BLE Beacon Gateway Controller`.
  - **Sisi Kanan (Status Badges)**:
    - **Desktop (>= 640px)**: Tampil sejajar fleksibel dalam 1 baris.
    - **Mobile (< 640px)**: Tersusun simetris dalam **Grid 2x2** (`grid-template-columns: 1fr 1fr; gap: 7px;`) dengan ukuran teks `11px` (0.72rem), menjamin nol overflow dan tidak saling tindih.
    - 4 Komponen Badge:
      1. **Uptime Live**: Ikon jam cyan + waktu aktif singkat (misal `14h 08j 42m`).
      2. **Running Mode**: Dinamis (`mode-wifi`, `mode-eth`, `mode-smart`) dengan ikon dan teks status.
      3. **MQTT Telemetri**: Ikon pulse + teks status (`Connected` / `Disconnected`).
      4. **Gateway Online**: Dot hijau berdenyut 2s (`.sg-pulse-dot`) + teks `Gateway Online`.

- **2. Panel Kiri: Identitas & Sistem Gateway (`.sg-panel-left`, Desktop 5 Kolom)**:
  - Header dengan ikon Info cyan (`#22d3ee`).
  - Key-Value List vertikal:
    - `MODEL`: `%model%`
    - `MANUFACTURE`: Ikon perisai + `IoT Gateway` (`#38bdf8`)
    - `DEVICE ID (MDNS)`: Chip kode cyan `%device_id%` + tombol salin cepat (`.sg-copy-btn`)
    - `SYSTEM UPTIME`: Ticker live real-time (`#detail-uptime-live`, misal `14 Hari 08:42:19`)
  - Sub-Grid 3 Versi Sistem (Grid 3 Kolom):
    - `DEVICE VER`: `%device_version%`
    - `FIRMWARE`: `%firmware_version%` (aksen indigo)
    - `UI VERSION`: `%ui_version%` (aksen indigo)

- **3. Panel Kanan: Status & Konektivitas Jaringan (`.sg-panel-right`, Desktop 7 Kolom)**:
  - Header kartu dengan ikon network emerald (`#34d399`), deskripsi mode, dan running mode badge pill.
  - **Dual Interface Sub-Cards (`.sg-subcards-grid`)**:
    - **Desktop/Tablet (>= 640px)**: 2 kolom berdampingan (*side-by-side*).
    - **Mobile (< 640px)**: Otomatis bertumpuk 1 kolom secara vertikal untuk memberikan ruang penuh bagi alamat IP dan MAC address monospace.
    - **Sub-Card Wi-Fi (`#card-wifi`)**:
      - Header: Ikon Wi-Fi emerald, label role (`Primary Link (Connected)` / `Offline`), dan badge `Active`.
      - Data: `IP (WI-FI)` dengan tombol salin, `MAC (WI-FI)` dengan tombol salin.
      - Footer: `SIGNAL STRENGTH` dengan visualisator 4-bar sinyal (`.sg-signal-bars`) + nilai dBm tebal.
    - **Sub-Card Ethernet (`#card-ethernet`)**:
      - Header: Ikon RJ45 cable slate/blue, label role (`Kabel Terputus (Link Down)` / `Router LAN Terhubung` / `Direct LAN`), dan badge status (`Link Down` amber / `Active` emerald / `Direct PC` blue).
      - Data: `IP (ETHERNET)` dengan tombol salin dinamis (tersembunyi saat link down), `MAC (ETHERNET)` dengan tombol salin.
      - Footer: `MQTT TELEMETRI` / `LINK SPEED` dengan badge status.
      - Auto-hide dinamis via style `%eth_tiles_style%` jika modul hardware W5500 tidak terdeteksi.

- **4. Toast Notification & Clipboard API (`.sg-toast`)**:
  - Mengambang elegan di atas mobile bottom bar: `bottom: calc(var(--bottom-nav-h, 64px) + 16px)` di mobile, dan `bottom: 24px; right: 24px` di desktop.
  - Transisi slide-up halus dengan durasi tampil 2.4 detik.
  - Fungsi `copyText(text)` cerdas menggunakan `navigator.clipboard.writeText` dengan fallback `document.execCommand('copy')`.

## 40. Pencegahan FOUC (Flash of Unstyled Content) & Sinkronisasi Uptime Hardware Realtime
- **Latar Belakang & Masalah Sebelumnya**:
  - **FOUC (Flash of Unstyled Content)**: Blok tag `<style>` untuk komponen Overview SG-10 sebelumnya terletak di tengah elemen `<body>` (di dalam `<section class="content">`). Saat browser memuat halaman, browser merender elemen HTML sebelum membaca aturan CSS, sehingga memicu pergeseran tampilan mendadak (*layout shift* / FOUC). Selain itu, teks nilai default kartu (seperti `"14h 08j 42m"`, `"mode-wifi"`, `"Kabel Terputus"`) sempat ter-render sebelum ditimpa JavaScript ~100ms kemudian, menyebabkan lonjakan teks dan warna yang tampak berkedip.
  - **Uptime Tidak Sesuai**: Endpoint `/api/v1/system/version` pada `src/web/web_routes.cpp` sebelumnya tidak menyertakan data waktu aktif hardware (`uptime_sec`), sehingga JavaScript dashboard Overview jatuh ke nilai fallback statis mockup `(14 * 86400) + ...` (14 hari) dan tidak pernah mencerminkan waktu boot aktual ESP32.
- **1. Pemindahan Blok Gaya ke `<head>` (`data/index.html`)**:
  - Seluruh blok `<style>` komponen SG-10 Overview dipindahkan ke dalam `<head>` tepat sebelum penutup `</head>`.
  - Browser secara deterministik memblokir proses painting pertama hingga seluruh aturan CSS terurai, menjamin elemen HTML langsung ter-render dengan styling yang presisi sejak frame pertama.
- **2. Server-Side Template Token Pre-Rendering (`src/web/web_template.cpp`)**:
  - Server ESP-IDF C++ kini langsung mengisikan token status dan gaya ke dalam file HTML sebelum dikirimkan ke klien:
    - `%uptime_sec%`: Integer detik dari `esp_timer_get_time() / 1000000ULL`.
    - `%uptime_short%`: Teks waktu aktif format ringkas, contoh `0h 02j 15m`.
    - `%uptime_detail%`: Teks waktu aktif detail, contoh `0 Hari 00:02:15`.
    - `%header_mode_pill_class%`: Class pill status berjalan (`mode-smart`, `mode-eth`, `mode-wifi`).
    - `%wifi_role_label%`, `%wifi_role_class%`, `%wifi_status_text%`, `%wifi_status_class%`, `%wifi_card_class%`, `%wifi_signal_spec%`: Menjamin kartu Wi-Fi menampilkan status dan RSSI nyata saat pertama kali dibuka.
    - `%eth_role_label%`, `%eth_role_class%`, `%eth_status_text%`, `%eth_status_class%`, `%eth_card_class%`, `%eth_ip_display%`, `%eth_ip_class%`, `%eth_copy_style%`, `%eth_footer_label%`, `%eth_footer_value%`: Menjamin kartu Ethernet langsung menampilkan status link fisik (Direct LAN, Router LAN, atau Link Down) tanpa jeda.
- **3. Sinkronisasi Uptime Hardware Akurat (`src/web/web_routes.cpp` & `data/index.html`)**:
  - Endpoint C++ `/api/v1/system/version` menambahkan properti `"uptime_sec": (double)(esp_timer_get_time() / 1000000ULL)`.
  - Inisialisasi JavaScript menggunakan `let uptimeSeconds = parseInt("%uptime_sec%") || 0;`, memastikan ticker detik dimulai tepat dari detik saat halaman dimuat.
  - Panggilan `syncDashboardStatus()` dieksekusi seketika saat inisialisasi script dan berulang setiap interval 5 detik untuk memperbarui sinkronisasi `uptimeSeconds` dan status antarmuka jaringan.

## 41. Eliminasi Redundansi Informasi & Standardisasi Physical Link pada Ethernet Sub-Card
- **Latar Belakang & Identifikasi Redundansi**:
  - **MQTT Telemetri pada Ethernet Card**: Sebelumnya footer kartu Ethernet menampilkan status MQTT Telemetri. Hal ini tidak tepat secara arsitektur model OSI karena MQTT adalah protokol Layer 7 (Aplikasi) yang melayani seluruh gateway, bukan atribut spesifik dari interface fisik Ethernet (Layer 1/2). Pada mode Wi-Fi di mana kabel LAN dicabut, menampilkan "MQTT TELEMETRI: Disconnected" pada Ethernet card menimbulkan kebingungan seolah transmisi data gateway terputus, padahal MQTT aktif berjalan lancar melalui interface Wi-Fi.
  - **Running Mode Badge Ganda**: Di header utama (Slim Header Card) sudah terdapat badge live running mode (`Wi-Fi + Direct LAN` / `Ethernet` / `Wi-Fi`). Keberadaan badge pill kedua di header panel kanan (`#network-mode-badge`) bersifat redundan dan memakan ruang horizontal visual.
- **1. Standarisasi Footer Ethernet Card (`data/index.html` & `src/web/web_template.cpp`)**:
  - Label footer kartu Ethernet kini distandardisasi menjadi **`PHYSICAL LINK / SPEED`** secara simetris dengan footer kartu Wi-Fi (`SIGNAL STRENGTH`).
  - Nilai footer menampilkan parameter tautan fisik nyata:
    - **Direct LAN**: `<span class="sg-subcard-status-badge active">100M FD • Micro-DHCP</span>`
    - **Link Up / Router LAN**: `<span class="sg-subcard-status-badge active">100 Mbps Full-Duplex</span>` (dinamis membaca kecepatan dan duplex hardware W5500)
    - **Link Down (Kabel Terputus)**: `<span class="sg-subcard-status-badge amber">Disconnected (Link Down)</span>`
  - Seluruh referensi MQTT dihilangkan sepenuhnya dari kartu Ethernet. Status MQTT tetap terpusat dan terpantau dengan jelas pada Slim Header Card di bagian atas.
- **2. Pembersihan Header Panel Kanan (`data/index.html`)**:
  - Badge ganda `#network-mode-badge` dihapus dari header panel kanan, menyisakan tata letak yang bersih dan terfokus: Ikon Jaringan, Judul "Status & Konektivitas Jaringan", dan deskripsi mode ringkas.
  - Skrip sinkronisasi `syncDashboardStatus()` dibersihkan dari pembaruan elemen badge redundan tersebut.

## 42. Penyederhanaan Slim Header Card & Integrasi Connection Mode & MQTT pada Card Status Konektivitas
- **Latar Belakang & Rasionalisasi**:
  - Seluruh status pill pada Slim Header Card atas (`#header-uptime-short`, `#header-mode-pill`, `#header-mqtt-pill`, `.online-pill`) dihilangkan untuk menyajikan identitas visual gateway yang bersih dan elegan (*uncluttered branding bar*).
  - Informasi waktu aktif sistem (**System Uptime**) tetap terpantau akurat secara real-time pada panel kiri ("Identitas & Sistem Gateway").
  - Status koneksi spesifik interface (Wi-Fi dan Ethernet) sudah terwakilkan secara mandiri pada masing-masing sub-card interface.
  - Dua parameter tingkat jaringan gateway dialokasikan secara harmonis ke dalam header kartu **Status & Konektivitas Jaringan**:
    1. **Connection Mode** (`WIFI`, `ETHERNET`, `SMART CONNECT`).
    2. **MQTT Connection Status** (`Connected`, `Disconnected`).
- **1. Struktur Visual Slim Header Card (`.sg-header-card`)**:
  - Menyisakan ikon microchip, judul `SMART GUARD (%model%)`, bullet, dan sub-label `Industrial Smart BLE Beacon Gateway Controller`.
  - Tampil bersih dan konsisten pada desktop maupun mobile tanpa risiko wrapping pill.
- **2. Struktur Visual Badges Header Jaringan (`.sg-net-header-badges`)**:
  - **Desktop (>= 640px)**:
    - Terletak di sisi kanan header `sg-card-header` sejajar dengan judul panel.
    - Badges:
      - `MODE: WIFI` (`.mode-wifi`, latar hijau transparan, teks `#34d399`, dot indikator)
      - `MODE: ETHERNET` (`.mode-eth`, latar biru transparan, teks `#60a5fa`, dot indikator)
      - `MODE: SMART CONNECT` (`.mode-smart`, latar ungu transparan, teks `#c084fc`, dot indikator)
      - `MQTT: Connected` (`.mqtt-online`, teks `#34d399`, ikon pulse) / `MQTT: Disconnected` (`.mqtt-offline`, teks `#fbbf24`, ikon pulse)
  - **Mobile (< 640px)**:
    - Header kartu `.sg-card-header` bertumpuk vertikal (`flex-direction: column; align-items: stretch;`).
    - Badges tersusun rapi dalam **Grid 2 Kolom Simetris (50% - 50%)** (`grid-template-columns: 1fr 1fr; gap: 7px;`) dengan teks terpusat (`justify-content: center;`), menjamin nol overflow dan tidak saling bertubrukan.
- **3. Pre-Rendering Server-Side C++ (`src/web/web_template.cpp`)**:
  - Token baru untuk mencegah FOUC:
    - `%conn_mode_text%`: Menghasilkan `"SMART CONNECT"`, `"ETHERNET"`, atau `"WIFI"`.
    - `%conn_mode_class%`: Menghasilkan `"mode-smart"`, `"mode-eth"`, atau `"mode-wifi"`.
    - `%conn_mqtt_class%`: Menghasilkan `"mqtt-online"` atau `"mqtt-offline"`.
    - `%network_mode_desc%`: Deskripsi operasional real-time sesuai mode yang aktif.
## 43. Tipografi Profesional & Penyelarasan Layout Berdasarkan Mockup Referensi Pengguna
- **Latar Belakang & Filosofi Desain**:
  - Merekonstruksi hierarki tipografi dari sebelumnya menggunakan huruf kapital kaku (ALL CAPS) menjadi **Title Case** berstandar enterprise yang lebih elegan, terbaca cepat, dan profesional.
  - Memfokuskan header kartu jaringan pada **Single Connection Mode Badge** (`● MODE: WI-FI`, `● MODE: ETHERNET`, `● MODE: SMART CONNECT`), mengeliminasi elemen sekunder di header agar tampilan lebih lapang dan berfokus pada status arsitektur jaringan.

## 44. Penyempurnaan 100% Visual Parity (Font Size, Family, Spacing, Padding, Card Sizing, & Colors) Berdasarkan Referensi Mockup Pengguna
- **Latar Belakang & Evaluasi Presisi Visual**:
  - Berdasarkan perbandingan piksel langsung dengan gambar referensi pengguna (`user_clipboard_mockup.png` / `media_1792573240281.png`), dilakukan perbaikan mendalam terhadap parameter ukuran font, warna teks, jarak padding antar-elemen, ukuran kontainer kartu, dan proporsi grid.
- **1. Perluasan Kontainer & Proporsi Grid Bento (`.content` & `.sg-bento-grid`)**:
  - **Kontainer Luas (`.content`)**: Ditingkatkan batas lebarnya dari pembatas sempit `1080px` menjadi `max-width: 1440px !important; width: 100% !important; padding: 24px 28px !important;`. Hal ini memberikan napas lega bagi kartu overview untuk mengisi layar secara proporsional layaknya dashboard telemetri industrial high-end.
  - **Proporsi Grid Responsif**:
    - Desktop (`min-width: 1080px`): `grid-template-columns: minmax(340px, 36%) 1fr; gap: 24px;`.
    - Memberikan ruang horizontal ekstra (~750-800px) untuk panel kanan, sehingga kedua subcard (Wi-Fi dan Ethernet) masing-masing mendapatkan lebar ~360-400px.
    - Mengeliminasi 100% masalah pemotongan teks (ellipsis `...`) pada alamat MAC (`CC:7B:5C:BB:B6:D4`), alamat IPv4, dan judul interface (`Ethernet Interface`).
- **2. Penyelarasan Warna & Tipografi Teks (Typography & Color Tokens)**:
  - **IPv4 Address Wi-Fi**: Ditampilkan dalam warna **Putih Murni (`#ffffff`)**, bukan hijau (`#34d399`). Menggunakan font `ui-monospace` tebal (`font-weight: 700; font-size: 0.94rem;`).
  - **MAC Address (Wi-Fi & Ethernet)**: Ditampilkan dalam warna **Slate-200 / Putih Bersih (`#f1f5f9`)**, bukan cyan (`#38bdf8`). Font `ui-monospace` tebal (`font-weight: 700; font-size: 0.90rem;`).
  - **Manufacture**: Teks `IoT Gateway` menggunakan warna **Putih Murni (`#ffffff`)** dengan ikon perisai cyan `#38bdf8` (`<svg stroke="#38bdf8">`), bukan seluruh teks berwarna biru/cyan.
  - **Ethernet IPv4 Unassigned**: Teks `— (Unassigned)` dalam warna slate redup (`#64748b`, `font-size: 0.88rem; font-weight: 400;`). Tombol salin disembunyikan otomatis.
  - **Physical Link**: Teks status `Kabel Terputus` tampil tebal dalam warna **Amber (`#f59e0b` / `#fbbf24`)** (`font-weight: 700; font-size: 0.94rem;`), sejajar bersih dengan baris IPv4 dan MAC tanpa kotak badge status.
  - **Header Card & Subcard Title**:
    - Panel headers (`h3`): `font-size: 1.05rem; font-weight: 700; color: #ffffff;`.
    - Subcard titles (`h4`): `font-size: 0.98rem; font-weight: 700; color: #ffffff;`.
    - Subcard roles: `font-size: 0.80rem; font-weight: 400; color: #8191a6;`.
- **3. Bentuk Kapsul Mode Badge (`.sg-badge-pill`)**:
  - Berubah dari kotak bersudut 8px menjadi **Kapsul Penuh (`border-radius: 9999px; padding: 7px 18px;`)**.
  - Dot indikator hijau solid `●` (`#10b981`, `font-size: 0.78rem`).
  - Teks: `MODE: WI-FI` (`font-size: 0.84rem; font-weight: 700; letter-spacing: 0.04em; color: #10b981;`).
- **4. Eliminasi Garis Pembatas Header yang Kaku (Seamless Modern Cards)**:
  - Dihilangkan garis `border-bottom` pada header panel (`.sg-card-header`), memberikan estetika dark mode yang mulus (*seamless floating content*).
  - Garis `border-top` pada grid versi sistem (`.sg-version-grid`) dihilangkan, menggantinya dengan jarak spasi margin vertikal yang proporsional (`margin-top: 20px;`).
  - Baris data dalam kartu (`.sg-kv-row` dan `.sg-subcard-row`) dipisahkan oleh garis sangat halus `border-bottom: 1px solid rgba(255, 255, 255, 0.04);` dengan padding `14px - 15px 0` yang lega.
- **5. Adaptasi Responsif Mobile (< 640px)**:
  - Padding konten luar diatur ke `16px 12px` dan padding kartu internal disesuaikan menjadi `16px 14px`, mencegah pemborosan ruang horizontal pada layar 375px.
  - Title subcard (`h4`) dan status badge (`Active` / `Link Down`) memiliki ruang 50px+ ekstra, menjamin **nol tumpang-tindih (zero overlap)** dan nol pemotongan teks.
  - Tiga kartu versi sistem (`DEVICE VER`, `FIRMWARE`, `UI VERSION`) tetap tersusun 3 kolom horizontal kompak dengan font `0.94rem`.

## 45. Kartu Telemetri MQTT Broker Ramping (Slim Low-Profile Telemetry Card)
- **Latar Belakang & Keseimbangan Visual (Vertical Bento Alignment)**:
  - Pada layout Bento Overview sebelumnya, panel kiri memiliki ketinggian ~360px karena ditopang oleh 3 tile versi di bagian bawah (`DEVICE VER`, `FIRMWARE`, `UI VERSION`), sedangkan panel kanan hanya memiliki dua kartu interface (~275px), menyisakan ruang kosong vertikal (*empty delta*) sebesar ~85px.
  - Untuk mengisi ruang tersebut secara harmonis dan memenuhi kebutuhan pemantauan uplink cloud, ditambahkan kartu telemetri MQTT broker ramping tepat di bawah `.sg-subcards-grid` pada panel kanan.
- **1. Struktur Visual & Tata Letak Data (3-Column Responsive Grid)**:
  - **Header Kartu Ramping (`.sg-mqtt-header`)**:
    - Ikon Cyan Industrial (`.sg-subcard-icon.cyan`, 26x26px): Background `rgba(56, 189, 248, 0.12)`, border `rgba(56, 189, 248, 0.25)`, warna `#38bdf8`.
    - Judul: `Telemetri MQTT` (`font-size: 0.90rem; font-weight: 700; color: #ffffff;`).
    - Status Peran: `• %mqtt_role_label%` (`font-size: 0.76rem; color: #8191a6;`).
    - Badge Status Realtime: Kapsul hijau emerald `● Connected` (`.sg-subcard-status-badge.active`) atau amber `● Disconnected` (`.sg-subcard-status-badge.amber`).
  - **3 Kolom Data Sejajar Horizontal (`.sg-mqtt-data-grid`)**:
    1. **Broker Destination**: Host URL / IP broker (`g_config.mqtt_host`, misal `mqtt.iotaru.id`) dalam font `ui-monospace` putih 700 (`0.90rem`) dengan tombol salin satu-klik (`copyText`).
    2. **Port Koneksi**: Nomor port TCP (`g_config.mqtt_port`, misal `8883` atau `1883`) dalam font `ui-monospace` putih 700.
    3. **Keamanan (TLS)**: Badge status enkripsi kapsul:
       - TLS Aktif (`mqtt_use_ssl == 1`): Badge hijau dengan ikon gembok `[🔒 TLS Active]`.
       - Plain TCP (`mqtt_use_ssl == 0`): Badge slate dengan ikon terbuka `[🔓 Plain TCP]`.
- **2. Ketinggian Terkalibrasi (Slim Low-Profile Height ~80px)**:
  - Padding kartu diatur kompak `13px 18px` dengan celah antar-kolom `16px`.
  - Pada layar desktop, kartu memiliki tinggi total ~80px, membuat garis bawah panel kanan menutup sejajar rata dengan 3 tile versi di panel kiri.
- **3. Adaptasi Responsif Mobile (< 640px)**:
  - Pada layar ponsel, 3 kolom data otomatis bertransisi menjadi susunan baris fleksibel (`display: flex; align-items: center; justify-content: space-between; gap: 8px;`), label di sebelah kiri dan nilai/tombol salin di sebelah kanan.
  - Subtitle role di header disembunyikan otomatis untuk memaksimalkan ruang judul dan badge status.
- **4. Pre-Rendering C++ Server-Side (Zero FOUC)**:
  - `%mqtt_broker_host%`: Host broker (atau `—` jika belum diatur).
  - `%mqtt_broker_port%`: Nomor port string.
  - `%mqtt_tls_badge%`: Tag HTML badge keamanan (TLS Active / Plain TCP).
  - `%mqtt_conn_badge%`: Tag HTML badge koneksi (Connected / Disconnected).
  - `%mqtt_card_class%`: Class border aktif `active-mqtt` (`border-color: rgba(56, 189, 248, 0.35);`).
  - `%mqtt_role_label%`: Deskripsi status koneksi uplink.
- **5. REST API & Sinkronisasi Polling 5 Detik**:
  - Endpoint `GET /api/v1/system/version` menyertakan key `mqtt_host`, `mqtt_port`, `mqtt_ssl`, dan `mqtt_connected`.
  - Fungsi JavaScript `syncDashboardStatus()` di `data/index.html` memperbarui status badge, broker host, port, dan kapsul TLS secara live tanpa reload halaman.

## 46. Standarisasi Bahasa Inggris 100%, Huruf Kapital Konsisten, Pengisian Ruang Kosong Panel Identitas, Grid 2-Tier Mobile MQTT, dan Semantik Warna Hijau Aktif
- **1. Standarisasi Bahasa Inggris 100% (Full English UI Consistency)**:
  - Seluruh teks antarmuka diubah secara konsisten ke bahasa Inggris:
    - Navbar & Header: `Gateway Overview`, `SMART GUARD (%model%)`, `Industrial Smart BLE Beacon Gateway Controller`, `Logout`.
    - Safety Banners: `Network configuration changes saved to NVS. Reboot required to apply to hardware.`, `Reboot Now`, `Discard`, `Configuration Hotspot Active`, `Disable AP`.
    - Panel Kiri: `Gateway Identity & System`, `Unit identification parameters and live uptime`, `Device Model`, `Manufacturer`, `Device ID (mDNS)`, `System Uptime` (`%u %s %02u:%02u:%02u`, `Day` / `Days`), `System RAM`, `Bandwidth`.
    - Panel Kanan: `Network Status & Connectivity`, `Wi-Fi Interface`, `Ethernet Interface`, `MQTT Telemetry`, `Signal Strength Indicator`, `Physical Link`, `Cable Disconnected`, `Broker Host`, `Port`, `Security (TLS)`.
    - Diagnostic Bento: `Realtime Diagnostic Test`, `High-speed nRF52840 beacon packet monitoring`, `Open Live Test Monitor`.
    - Tooltips & Toasts: `Copy Device ID`, `Copy Wi-Fi IPv4`, `Copy Wi-Fi MAC`, `Copy Ethernet IPv4`, `Copy Ethernet MAC`, `Copy MQTT Broker Host`, `Copied to clipboard!`.
- **2. Aturan Kapitalisasi Text (Title Case & Technical Acronyms)**:
  - **Title Case**: Digunakan untuk seluruh Card Header, Subcard Title, Key-Value Labels, dan Tombol Aksi (`Gateway Identity & System`, `Network Status & Connectivity`, `System Uptime`, `System RAM`, `Bandwidth`, `Broker Host`, `Security (TLS)`, `Reboot Now`).
  - **UPPERCASE (Semua Huruf Kapital)**: Dibatasi secara ketat hanya untuk akronim teknis dan status hardware (`DEVICE VER`, `FIRMWARE`, `UI VERSION`, `MODE: ETHERNET`, `MODE: WI-FI`, `SMART CONNECT`, `IPv4`, `MAC`, `mDNS`, `TLS`, `RAM`, `NVS`, `FD`, `Mbps`).
- **3. Pengisian Ruang Kosong Vertikal Panel Kiri (System RAM & Bandwidth)**:
  - Untuk mengisi ruang kosong vertikal pada Panel Kiri secara proporsional dan menjaga kesetaraan ketinggian dengan Panel Kanan yang memiliki kartu MQTT:
    - **`System RAM`**: Label padat ringkas dengan bullet hijau: `<span style="color:#10b981;">●</span> 184 KB Free`. Sumber data live: `esp_get_free_heap_size() / 1024`.
    - **`Bandwidth`**: Nilai numerik throughput real-time: `14.2 KB/s`. Sumber data live: `Network::MqttService::getBandwidthKBps()`.
  - Kedua baris data ini memperbarui secara instan via pre-render template server (`%free_heap_kb%`, `%net_bandwidth%`) dan sinkronisasi real-time via `/api/v1/system/version`.
- **4. Tampilan Mobile 2-Tier Grid untuk Host Broker MQTT Ramping**:
  - Pada layar mobile (< 680px), host broker yang panjang (misalnya endpoint AWS IoT `a1b2c3d4e5f6-ats.iot.ap-southeast-1.amazonaws.com`) sebelumnya berpotensi terpotong.
  - Diperbarui menjadi **2-Tier Grid**:
    - **Tier 1 (Baris Penuh / `grid-column: 1 / -1; width: 100%;`)**: Menampilkan label `Broker Host` di bagian atas dan nilai URL broker di bawahnya dengan `word-break: break-all; white-space: normal; line-height: 1.35;` serta tombol copy di sampingnya. URL panjang secara mulus melipat ke 2 baris dan 100% terlihat jelas tanpa terpotong (*zero truncation*).
    - **Tier 2 (Grid 2 Kolom 50/50)**: Menampilkan kolom `Port` di sisi kiri dan `Security (TLS)` di sisi kanan.
  - Pada desktop (`min-width: 680px`), grid secara otomatis kembali ke 3 kolom sejajar horizontal (`1.6fr 0.9fr 1.2fr`).
- **5. Semantik Warna Outline & Ikon (Unified Green Active & Gray Inactive)**:
  - **Status Aktif / Online**:
    - Outline kartu antarmuka (`.active-online`, `.active-wifi`, `.active-eth`, `.active-mqtt`): Menggunakan outline hijau `border-color: rgba(16, 185, 129, 0.4);`.
    - Ikon antarmuka: Menggunakan warna hijau emerald (`.sg-subcard-icon.emerald`, `#10b981`, background `rgba(16, 185, 129, 0.12)`). Menghilangkan warna biru pada Ethernet dan cyan pada MQTT saat aktif.
  - **Status Tidak Aktif / Offline / Disconnected**:
    - Outline kartu: Menggunakan abu-abu gelap netral (`#1a2333`).
    - Ikon antarmuka: Menggunakan warna abu-abu netral (`.sg-subcard-icon.slate`, `#8191a6`, background `rgba(30, 41, 59, 0.6)`).
- **6. Penyesuaian Ketinggian Kartu Versi (Elevated & Balanced Version Tiles)**:
  - **Elevasi Tiga Kartu Versi**: Tiga kartu versi (`DEVICE VER`, `FIRMWARE`, `UI VERSION`) dinaikkan posisinya dari bagian bawah panel kiri untuk mengeliminasi kekosongan asimetris.
  - **Penyelarasan Spasi**:
    - Padding baris key-value (`.sg-kv-row`) disesuaikan menjadi `12px 0` yang proporsional.
    - Grid kartu versi (`.sg-version-grid`) diberikan `margin-top: auto; margin-bottom: 6px;` dengan gap `10px`.
    - Padding internal tile (`.sg-version-tile`) dibuat lebih ramping (`10px 6px`) dengan ukuran nilai `1.02rem`.
## 47. Kalibrasi Akurasi Metrik Bandwidth Jaringan (Rolling 1000ms Window & Downlink Accounting)
- **1. Masalah Sampling Asinkron (Under-reporting vs Over-reporting)**:
  - Sebelumnya, fungsi perhitungan bandwidth memanggil `exchange(0)` langsung saat dipanggil.
  - Halaman Live Test (`/test`) membaca via WebSocket setiap 250 ms (0.25 detik), sehingga angka terpecah 4x lebih kecil (~0.33 KB).
  - Halaman Overview (`/`) membaca via polling REST setiap 5 detik, sehingga angka terakumulasi multi-detik (~3.5 KB).
- **2. Arsitektur Perhitungan Independen (Rolling 1000ms Sampling Window)**:
  - Task publisher (`mqttPublisherTask`) sekarang menjalankan sampler internal independen setiap interval waktu nyata $\Delta t \ge 1000\text{ ms}$:
    $$\text{Bandwidth (KB/s)} = \frac{\Delta \text{Bytes Total}}{1024 \times (\Delta t / 1000)}$$
  - Nilai tersimpan di `std::atomic<float> s_statBandwidthKBps`.
  - Fungsi `getBandwidthKBps()` bersifat *read-only* murni tanpa me-reset counter (`load(memory_order_relaxed)`).
  - **Hasil**: Baik Halaman Test (250ms), Overview (5s), maupun Serial Log menampilkan metrik `KB/s` yang sinkron, identik, dan akurat per detik tanpa saling bentrok.
- **3. Integrasi Traffic Downlink Broker (Wire-Level Physical Accuracy)**:
  - Pencatatan byte kabel fisik (`recordWireBytes`) kini memperhitungkan **Uplink (TX)** dan **Downlink (RX)**:
    - **Downlink PUBACK**: Setiap paket QoS 1 yang di-ack oleh broker dicatat pada `MQTT_EVENT_PUBLISHED` sebesar 4 byte MQTT + 40 byte TCP/IP + Frame Layer 2 (18B Ethernet / 36B Wi-Fi).
    - **Downlink Inbound Command**: Perintah masuk dari broker pada `MQTT_EVENT_DATA` dicatat mencakup panjang topik, data payload, TCP/IP, dan header Layer 2.
    - **Downlink SUBACK**: Respon konfirmasi langganan topik pada `MQTT_EVENT_SUBSCRIBED` dicatat sebesar 5 byte MQTT + TCP/IP + Layer 2.
    - **Uplink Command Response**: Respon balik gateway terhadap perintah broker (`s_topicRes`) dicatat secara presisi di kabel jaringan.

## 48. Eliminasi Ruang Kosong Bawah (Bottom Scroll Void) & Penyelarasan Posisi Footer
- **1. Akar Masalah Penumpukan Jarak Bawah (Double/Triple Padding Stack)**:
  - Sebelumnya, `.main-panel` di `app.css` memiliki `padding-bottom: var(--bottom-nav-h);` (64px) yang berlaku tanpa batas di semua ukuran layar (termasuk desktop). Karena `<footer>` berada di dalam `.main-panel`, tercipta rongga kosong 64px persis di bawah garis batas footer.
  - Selain itu, `.content` di `index.html` menyematkan `padding-bottom: calc(var(--bottom-nav-h, 64px) + 28px) !important;` (92px) yang menyebabkan jarak raksasa antara bento card terakhir dan footer.
  - Akibatnya, saat pengguna scroll hingga footer, halaman masih dapat di-scroll turun lebih jauh ke area kosong (*scroll void*).
- **2. Penyelarasan Tata Letak Footer Bebas Rongga Kosong**:
  - **`.main-panel` (`app.css`)**: Dihapus nilai `padding-bottom`-nya (`padding-bottom: 0`) sehingga kontainer utama tidak lagi menambahkan ruang kosong di bawah footer.
  - **`footer` (`app.css`)**:
    - Desktop (`> 860px`): Menggunakan padding standar `padding: 20px;` dengan `margin-top: auto;` sehingga footer pas menempel rapi di dasar halaman dan scroll berhenti tepat di bawah teks hak cipta.
    - Mobile (`<= 860px`): Menggunakan `padding-bottom: calc(var(--bottom-nav-h) + 16px);` sehingga teks hak cipta berada 16px tepat di atas bottom navigation bar mengambang dan tidak tertutup navbar.
  - **`.content` (`index.html`)**: Diubah menjadi `padding: 24px 28px 28px 28px !important;` untuk menyeimbangkan jarak vertikal kartu dengan garis pemisah footer.

## 49. Perbaikan Buffer Template Engine & Integritas Parsing SVG Inline
- **1. Akar Masalah Browser Error `attribute d: Expected moveto path command`**:
  - Buffer lokal placeholder di fungsi `render_template()` (`src/web/web_template.cpp`) awalnya berukuran `char val[256]`.
  - Placeholder status kartu MQTT `%mqtt_tls_badge%` menghasilkan markup HTML badge dengan inline SVG lengkap sepanjang 308 karakter (mode TLS Active) atau 303 karakter (mode Plain TCP).
  - Ketika `snprintf(out_buf, max_len, ...)` dijalankan dengan `max_len = 256`, string terpotong paksa tepat pada karakter ke-255, persis setelah tag pembuka `<path `.
  - Akibatnya, browser menerima potongan HTML tak lengkap `<span ...><rect .../><path \n </div>`. Parser SVG HTML5 pada peramban (Chrome/Edge) mencoba memulihkan tag `<path>` dan menemukan `</div>`, memicu galat konsol browser:
    `Error: <path> attribute d: Expected moveto path command ('M' or 'm'), "… </div\u003E\n …"`.
- **2. Solusi & Mitigasi**:
  - **Ekspansi Buffer Template**: Buffer lokal `char val[256]` di `src/web/web_template.cpp` ditingkatkan menjadi `char val[512]`. Hal ini memberikan kapasitas hingga 511 karakter untuk semua placeholder ber-SVG inline, badge status dinamis, dan token HTML tanpa risiko pemotongan string, dengan tetap sangat aman pada stack task web server (kapasitas stack 8192 byte).
  - **Standardisasi Format SVG Path**: Atribut path SVG pada Plain TCP mode diselaraskan dengan spasi pemisah standar (`d="M7 11V7a5 5 0 0 1 9.9 -1"`) baik di backend template C++ (`web_template.cpp`) maupun frontend live synchronization (`data/index.html`) guna menjamin integritas rendering vektor di seluruh platform browser.

## 50. Perampingan Halaman Overview (Eliminasi Kartu Redundan 'Realtime Diagnostic Test')
- **1. Rasional Pembersihan UI**:
  - Kartu promosi bawah `Realtime Diagnostic Test` (beserta tombol pintas *Open Live Test Monitor*) dihilangkan dari tata letak Overview (`index.html`).
  - Fitur diagnostik transmisi dan pemantauan live packet nRF52840 telah dapat diakses langsung secara permanen melalui item menu sidebar utama (`Live Test Monitor` pada `/test`) dan bottom navigation sheet pada mobile.
  - Penghapusan kartu sekunder ini menghilangkan redundansi tombol ganda, menyatukan ritme visual Grid Bento utama (panel identitas di kiri dan panel jaringan/telemetri MQTT di kanan), serta menghasilkan tampilan dasbor yang bersih dan presisi tanpa scrolling berlebih.

## 51. Mekanisme Status Offline & Eliminasi Angka Live Uptime Saat Perangkat Padam
- **1. Akar Masalah Uptime Berjalan Terus Saat Perangkat Mati**:
  - Ticker waktu `updateUptimeDisplay()` di `data/index.html` dijalankan oleh `setInterval` lokal peramban (`uptimeSeconds++` tiap 1 detik) untuk kelancaran visual antarmuka.
  - Polling status `/api/v1/system/version` berulang tiap 5 detik untuk kalibrasi ulang `uptimeSeconds`. Namun, jika mikrokontroler mati/terputus, blok `.catch()` awalnya dibiarkan kosong tanpa penanda kegagalan koneksi (*silent fail*).
  - Akibatnya, selama tab peramban tetap terbuka, JavaScript di komputer/HP pengguna terus menghitung detik sendiri tanpa batas meskipun perangkat keras fisik sudah padam.
- **2. Solusi Desain & Standarisasi UI/UX**:
  - **Mekanisme Guard `isDeviceOnline` & AbortController**:
    - Polling HTTP dilengkapi `AbortController` dengan batas waktu *timeout* 3500ms agar permintaan jaringan yang menggantung segera diputus jika perangkat mati.
    - Variabel `offlineStreak` melacak kegagalan beruntun. Setelah 2 kali gagal berturut-turut, status beralih menjadi `isDeviceOnline = false`.
    - Di dalam `updateUptimeDisplay()`, penambahan `uptimeSeconds++` langsung dihentikan saat `!isDeviceOnline`.
  - **Komponen Visual & Token Warna (Eliminasi Angka Saat Offline)**:
    - **`#detail-uptime-dot`**: Berubah dari hijau emerald aktif (`#10b981`, `●`) menjadi merah padam (`#ef4444`, `○`) dengan transisi halus `color 0.2s ease`.
    - **`#detail-uptime-live`**: Seluruh angka durasi (seperti `0 Days 00:00:28`) dihilangkan secara tuntas dan digantikan dengan teks statis `"Offline"` berwarna merah (`#ef4444`) berbobot tebal (`font-weight: 700`) sehingga pengguna tidak dibingungkan oleh angka durasi beku (*frozen timestamp*).
    - **`#detail-uptime-badge`**: Disembunyikan (`display: none;`) untuk menghindari redundansi teks ganda ("Offline OFFLINE").
    - **`.sg-badge-pill.mode-offline`**: Badge koneksi utama di panel kanan beralih ke mode offline (`background: rgba(239, 68, 68, 0.12)`, `border: 1px solid rgba(239, 68, 68, 0.45)`, `color: #ef4444`) dengan label `DISCONNECTED` dan deskripsi `Device Unreachable — Gateway offline or connection lost`.
  - **Pemulihan Otomatis (*Auto-Recovery*)**: Begitu perangkat menyala kembali dan request `/api/v1/system/version` sukses, `offlineStreak` di-reset ke 0, dot kembali hijau (`#10b981`, `●`), teks uptime dikembalikan ke angka durasi aktif (`formatUptimeString(uptimeSeconds)`) berwarna putih terang (`#ffffff`), dan ticker live counter 1 detik kembali berjalan sinkron dengan hardware.

## 52. Siklus Sesi Global, Timeout Inaktivitas & Force Auto-Redirect ke Login
- **1. Rasional Keamanan Sesi Industri (Zero Zombie Session)**:
  - Sesi mikrokontroler ESP32 memiliki sliding timeout 15 menit (`SESSION_TIMEOUT_MS = 900000 ms`).
  - Sebelumnya, jika tab dibiarkan terbuka, pemanggilan heartbeat `/api/v1/auth/check` secara buta terus me-refresh sesi di backend dan jika status `401 Unauthorized` terjadi, kode lama hanya mencetak `console.warn` tanpa me-redirect pengguna.
  - Pengguna berpotensi melihat data lama (*stale UI*) dan baru terlempar ke halaman login jika melakukan reload halaman (F5) atau mengklik tautan antar-halaman.
- **2. Arsitektur Otomasi Pengalihan Layar (`app.js`)**:
  - **Pelacak Inaktivitas Nyata Pengguna (*User Activity Tracking*)**:
    - Browser mendengarkan event pengguna nyata: `mousedown`, `keydown`, `touchstart`, dan `scroll`.
    - Batas inaktivitas: `IDLE_TIMEOUT_MS = 15 * 60 * 1000` (15 menit).
    - Jika pengguna diam tanpa interaksi selama 15 menit, perpanjangan sliding session dihentikan dan sistem memicu `redirectToLogin("idle_timeout")`.
  - **Force Redirect Seketika (`window.location.replace`)**:
    - Menggunakan `window.location.replace("/login.html?expired=1")` (bukan `location.href`) untuk menghapus riwayat halaman saat ini sehingga tombol *Back* pada peramban terkunci dan tidak bisa mengakses halaman terproteksi.
  - **Verifikasi Instan saat Kembali ke Tab (`visibilitychange` & `focus`)**:
    - Ketika laptop baru dibuka dari mode sleep atau pengguna kembali mengklik tab setelah lama diminimize, event `visibilitychange` dan `focus` langsung memverifikasi `/api/v1/auth/check` secara instan tanpa perlu menunggu sisa delay interval timer.
  - **Global Fetch Interceptor Guard & Diferensiasi Logout Manual**:
    - Seluruh pemanggilan `window.fetch` ke REST API gateway diproteksi interceptor terpusat. Setiap respons berstatus HTTP `401 Unauthorized` (di luar endpoint auth login/logout) seketika memicu force redirect ke login.
    - **Guard `isManualLoggingOut`**: Saat pengguna mengklik logout secara sengaja (`triggerLogout`) atau memicu reboot/reset, sistem mengaktifkan flag `window.isManualLoggingOut = true`. Seluruh request background (seperti polling 5 detik) yang terputus dengan status 401 setelah logout tidak akan memicu flag `?expired=1`, sehingga pengguna mendarat bersih di `/login.html` tanpa banner sesi kedaluwarsa.
- **3. Banner Notifikasi Ramah di Halaman Login (`login.html`)**:
  - **Komponen `.session-expired-banner`**:
    - Hanya muncul saat sesi benar-benar kedaluwarsa (inactivity timeout) via parameter `?expired=1`.
    - Tidak muncul saat pengguna logout secara sadar/manual.
    - Background: `rgba(239, 68, 68, 0.12)`, Border: `1px solid rgba(239, 68, 68, 0.35)`, Teks: `#fca5a5`, Ikon: `#ef4444`.
    - Animasi halus `fadeInSlide 0.25s ease-out` (`translateY(-4px)` ke `translateY(0)`).
    - Menjelaskan dengan transparan kepada pengguna: *"Sesi Anda telah berakhir demi keamanan. Silakan login kembali."*

## 53. Server-Side Rendering (SSR) Halaman Login, Pemisahan Endpoint Hotspot AP, & Penguncian Telemetri
- **1. Rasional Pemisahan Arsitektur (Separation of Concerns & Least Privilege)**:
  - Sebelumnya, halaman login di-stream sebagai file statis (`stream_static_file`) sehingga tag `%model%` tidak diproses oleh server, memaksa browser melakukan pemanggilan JavaScript tambahan ke `/api/v1/system/version`.
  - Selain itu, status Hotspot Mode AP darurat (`ap_active`, `ap_remain_sec`) ikut "menumpang" di endpoint version, sehingga endpoint version terpaksa dibuka publik dan berisiko membocorkan IP, MAC, MQTT Host, Uptime, dan Heap ke pengguna belum login.
- **2. Standardisasi Server-Side Rendering (SSR) untuk Login**:
  - `login_handler` beralih ke `Template::stream_template_file(req, "/storage/login.html")`.
  - Placeholder `%model%` (`SG-10`), `%ap_ssid%` (`SG-10-xxxx`), dan `%ap_banner_active%` (`1` / `0`) dirender langsung di server sebelum byte pertama dikirim ke browser.
  - Menghilangkan *layout shift* (kedipan teks judul), serta mengeliminasi pemanggilan `fetch("/api/v1/system/version")` dari JavaScript login.
- **3. Endpoint Ringan Khusus Hotspot AP (`GET /api/v1/system/ap-status`)**:
  - Endpoint publik mandiri tanpa autentikasi yang hanya mengembalikan status non-sensitif hotspot darurat:
    ```json
    {"ap_active":true,"ap_ssid":"SG-10-xxxx","ap_remain_sec":55,"reboot_required":false}
    ```
  - **Efisiensi Memori (Zero Heap Allocation)**: Diformat langsung menggunakan `snprintf` pada buffer lokal stack `128 byte` tanpa overhead `cJSON` atau `malloc`.
  - Dipakai secara terpusat oleh `checkLoginApStatus()` di `login.html` dan `initApModeWatcher()` di `app.js`.
- **4. Penguncian Penuh Data Sensitif Telemetri (`GET /api/v1/system/version`)**:
  - Endpoint `/api/v1/system/version` kini dilindungi guard autentikasi ketat:
    `if (Session::validate(req) == UserRole::NONE) return 401 Unauthorized;`
  - Akses anonim tanpa cookie sesi ditolak seketika (menghemat RAM dan CPU cycle ESP32 karena parsing cJSON dilewati).
  - Data sensitif jaringan dan broker MQTT hanya dapat dibaca oleh pengguna terotentikasi (Overview & Test Monitor).

## 54. Arsitektur Validasi Sesi Pasif vs Aktif (Inactivity Timeout 15 Menit)
- **1. Masalah Polling Otomatis Mereset Timer Sesi**:
  - Pada arsitektur sebelumnya, setiap request berautentikasi otomatis mereset waktu `last_activity_ms` ke waktu terkini (sliding expiration).
  - Karena halaman Overview melakukan polling telemetri setiap 5 detik ke `/api/v1/system/version`, timer sesi di ESP32 selalu di-reset ke 0 detik setiap 5 detik, sehingga sesi tidak pernah mencapai batas waktu 15 menit jika layar dibiarkan terbuka.
- **2. Pemisahan Validasi Pasif vs Aktif (`update_sliding`)**:
  - `Session::validate(req, out_token, max_len, bool update_sliding = true)`:
    - **Validasi Pasif (`update_sliding = false`)**: Digunakan untuk request polling otomatis background (seperti telemetri `/api/v1/system/version`, polling log `/api/v1/system/net-logs`, `/api/v1/system/drop-logs`, dan heartbeat `/api/v1/auth/check` tanpa flag aktif). Sesi tetap dicek keabsahannya, namun `last_activity_ms` TIDAK diperbarui.
    - **Validasi Aktif (`update_sliding = true`)**: Digunakan untuk aksi nyata pengguna (navigasi halaman, pengubahan pengaturan/POST konfigurasi, reboot/reset, dan heartbeat saat terdeteksi aktivitas via `/api/v1/auth/check?active=1`).
- **3. Sinkronisasi Deteksi Diam Pengguna (User Inactivity)**:
  - Frontend (`app.js`) mencatat aktivitas nyata: `mousedown`, `keydown`, `touchstart`, `scroll`, dan `mousemove` (dibatasi/throttled tiap 5 detik).
  - Jika pengguna aktif dalam 15 menit, verifikasi background mengirimkan parameter `?active=1` untuk memperbarui sliding session di ESP32.
  - Jika pengguna diam tanpa aktivitas selama >15 menit:
    - Frontend langsung mengarahkan pengguna ke `/login.html?expired=1`.
    - Di sisi backend ESP32, timer `now - last_activity_ms` juga melampaui `SESSION_TIMEOUT_MS = 900000 ms`, sehingga request polling pasif berikutnya langsung menerima respons HTTP `401 Unauthorized`.

## 55. Konsistensi Zona Waktu Booting (WIB) & Pencatatan Event Pre-Reboot
- **1. Inisialisasi Zona Waktu Dini (Zero-Boot Timezone Lock)**:
  - Variabel lingkungan `setenv("TZ", "WIB-7", 1)` dan `tzset()` kini dieksekusi di baris paling awal `app_main()` serta pada awal inisialisasi `NetLogService::init()`.
  - Mencegah *time-warp* / pergeseran zona waktu palsu (dari UTC ke WIB) pada saat gateway me-reboot.
  - Log `SYSTEM BOOT` dan log driver awal kini langsung menggunakan waktu lokal WIB (`YYYY-MM-DD HH:MM:SS`) tanpa jeda transisi UTC.
- **2. Pencatatan Event Pre-Reboot Sistem (`NetEvent::REBOOT`)**:
  - Seluruh jalur eksekusi `esp_restart()` (Jadwal Restart Harian 03:00, Watchdog Diskoneksi Jaringan, Reboot Manual Dashboard, Remote Command MQTT, dan Selesai Update OTA) kini mencatat event eksplisit `SYSTEM REBOOT` sebelum sistem padam.
  - Memastikan rekaman audit sistem memiliki pasangan kronologis yang lengkap:
    - Sebelum padam: `SYSTEM REBOOT` (Badge warning kuning/amber) dengan alasan spesifik.
    - Setelah menyala: `SYSTEM BOOT` (Badge primary biru) dengan verifikasi reset reason.
- **3. Ketahanan Terhadap Ketiadaan Internet (Offline Graceful Fallback)**:
  - Operasi POSIX `setenv/tzset` murni kalkulasi CPU register lokal tanpa request jaringan.
  - Jika internet offline total, sistem log secara mulus beralih ke format waktu relatif (`+HH:MM:SS`), dan `checkDailyRestart()` secara aman standby tanpa memicu restart di jam yang salah.

## 56. Konfigurasi Server Waktu (NTP) & Sistem Probe Validasi Real-Time
- **1. Skema Master NVS & Nilai Default Pabrik**:
  - Tabel master partisi (`nvs_config.csv`) kini menyertakan entri `ntp_server,data,string,pool.ntp.org` di bawah namespace `gateway`.
  - Inisialisasi NVS operasional (`initNvsConfiguration` di `config_store.cpp`) menyalin parameter `ntp_server` dari partisi master ke namespace operasional `cfg_gate`.
  - Fallback aman: Jika partisi master belum di-flash, sistem otomatis menetapkan default string `"pool.ntp.org"`.
  - Struktur data POD firmware (`AppConfig` di `app_config.hpp`) memuat field `char ntp_server[64];` (Zero Dynamic Allocation, hemat RAM ESP32).
  - Pemuatan (`loadConfig`) dan penyimpanan (`saveConfigSettings`) membaca serta menulis key `"ntp_server"` ke NVS.
- **2. Hot-Reload Sinkronisasi SNTP (`updateSntpServer`)**:
  - `NetworkManager::updateSntpServer(const char* server)` memperbarui server NTP primer pada slot 0 stack `esp_sntp` secara *runtime* tanpa mewajibkan gateway reboot.
  - Slot 1 (`time.nist.gov`) dan Slot 2 (`time.google.com`) tetap dipertahankan sebagai lapisan redundansi cadangan global.
- **3. Arsitektur Probe NTP Mandiri (UDP Port 123 Real-Time REST API)**:
  - Endpoint baru: `POST /api/v1/system/test-ntp` (juga mendukung `GET /api/v1/system/test-ntp?server=...`).
  - Mengirim paket SNTP standar RFC 5905 (48-byte, `LI=0, VN=4, Mode=3`) melalui socket UDP ke port 123 target.
  - Dilindungi *socket timeout* 3000 ms (`SO_RCVTIMEO`) untuk menjamin web server tidak pernah terblokir jika host tidak dapat dihubungi.
  - **Karakteristik Diagnostik**:
    - **Aman**: Hanya membaca balasan jam dari server tanpa mengubah jam sistem internal ESP32 secara prematur sebelum disimpan.
    - **Pengukuran Latensi (RTT)**: Menghitung durasi bolak-balik dalam milidetik ($$\Delta t = t_{\text{rx}} - t_{\text{tx}}$$).
    - **Parsing Waktu Nyata**: Menghitung delta epoch NTP (1900) ke Unix (1970) dan memformat jam server ke zona waktu lokal `WIB`.
- **4. Komponen Antarmuka Bento Grid & Status Feedback**:
  - **Bento Card `Server Waktu (NTP)`**: Berada di `/settings.html` dengan ikon jam amber industrial.
  - **Input Host/IP**: Mendukung nama domain (`pool.ntp.org`, `time.google.com`) maupun alamat IP lokal private (`192.168.1.1`).
  - **Tombol Aksi Terintegrasi (`.btn-input-action`)**: Tombol *"Uji Server"* dengan ikon petir di sisi kanan kolom input.
  - **Badge Status (`#ntpTestBadge`)**:
    - Status netral: `.uuid-badge.neutral` (*"Siap diuji"*).
    - Status menguji: `.uuid-badge.testing` (*"Menguji..."*) dengan animasi spinner inline.
    - Status sukses: `.uuid-badge.valid` (*"<RTT> ms"*).
    - Status gagal: `.uuid-badge.invalid` (*"Gagal"*).
  - **Kotak Umpan Balik (`.ntp-result-feedback`)**:
    - Sukses (`.ntp-result-feedback.success`): Latar hijau emerald transparan dengan IP server, RTT ms, dan jam aktual server.
    - Gagal (`.ntp-result-feedback.error`): Latar merah transparan dengan rincian kendala (DNS lookup gagal, port UDP 123 diblokir, atau timeout 3 detik).

## 57. Siklus Hidup Sesi Sliding & Pencegahan Infinite Loop / 302 Bounce
- **1. Pemisahan Heartbeat Pasif dan Perpanjangan Aktif**:
  - Heartbeat berkala (setiap 30 detik di `app.js`) hanya memanggil endpoint `/api/v1/auth/check` secara pasif (tanpa parameter `?active=1`).
  - Server ESP32 memvalidasi sesi tanpa memperbarui timer aktivitas (`update_sliding = false`), sehingga sesi pengguna yang benar-benar ditinggalkan (idle) dapat kedaluwarsa tepat pada 15 menit.
  - Sinyal perpanjangan sliding (`/api/v1/auth/check?active=1`) HANYA dikirimkan jika pengguna melakukan interaksi fisik baru (`mousedown`, `keydown`, `touchstart`, `scroll`, `mousemove`) sejak pemanggilan sebelumnya.
- **2. Pencegahan 302 Bounce Loop pada Halaman Login**:
  - Saat sesi habis (15 menit idle), JavaScript frontend membersihkan cookie `token` secara lokal dan mengarahkan browser ke `/login.html?expired=1`.
  - Di backend ESP32 (`login_handler`), deteksi query parameter `expired=1` memastikan server tidak memantulkan request kembali ke `/` (`302 Found`), melainkan langsung menghanguskan sesi lama di memori ESP32, membersihkan header Set-Cookie, dan menyajikan halaman login.
  - Di frontend `login.html`, IIFE `checkActiveSession()` mengabaikan pengecekan jika URL memiliki `expired=1` dan langsung menampilkan `#sessionExpiredBanner` berwarna amber/danger.
- **3. Standardisasi Cache-Busting Script**:
  - Seluruh halaman web gateway (`index.html`, `settings.html`, `log.html`, `test.html`, `auth_admin.html`, `auth_teknisi.html`, `firmware.html`) kini memanggil script dengan token versi dinamis `app.js?v=%firmware_version%` untuk mencegah browser menahan script lama dalam cache disk.

## 58. Konfigurasi Tingkat Pengiriman Data MQTT (QoS) & Interval Keep Alive
- **1. Pengaturan Tingkat Pengiriman Data (Quality of Service - QoS)**:
  - **Elemen Antarmuka**: Dropdown `<select id="mqtt_qos" name="mqtt_qos">` pada Bento Card 4 (*Broker Telemetri MQTT Gateway*) di `data/auth_admin.html`.
  - **Opsi Seleksi**:
    - `QoS 1 — Delivery Guaranteed (Default)`: Nilai `1`. Broker wajib mengirimkan paket konfirmasi `PUBACK` untuk setiap batch data telemetri. Sangat disarankan untuk integritas data pada jaringan kabel/LAN atau Wi-Fi berkecepatan tinggi.
    - `QoS 0 — Fire & Forget (Hemat Kuota)`: Nilai `0`. Transmisi instan tanpa menunggu konfirmasi `PUBACK`. Mengurangi overhead paket bolak-balik, menghilangkan bottleneck outbox buffer, dan menghemat kuota transmisi (sangat ideal untuk koneksi seluler berkuota / kepadatan >500 beacon/detik).
  - **Proteksi Pesan Kritis**: Mode QoS 0 hanya berlaku untuk aliran paket data beacon pada topik `s_topicData`. Pesan operasional vital gateway (*Birth Message* `status:online`, *Last Will & Testament* `status:offline`, dan *Command Subscription*) tetap terkunci pada `QoS 1` & `Retain 1` demi keandalan monitoring sistem.
- **2. Pengaturan Interval Detak Soket TCP (MQTT Keep Alive)**:
  - **Elemen Antarmuka**: Input numerik terstandardisasi dengan unit visual `.unit-box` dan label satuan `.unit-text` (*Detik*):
    `<input type="number" id="mqtt_keepalive" name="mqtt_keepalive" value="%mqtt_keepalive%" min="5" max="300" placeholder="30" required>`.
  - **Batas Nilai**: Rentang valid `5` s/d `300` detik (Default bawaan pabrik: `30` detik).
  - **Fungsi**: Mengatur frekuensi pengiriman frame `PINGREQ` ke broker saat tidak ada transmisi data agar koneksi soket TCP tidak ditutup sepihak oleh router atau operator seluler (*NAT timeout*).
- **3. Tata Letak Grid & Preservasi NVS**:
  - Disusun berdampingan dalam form grid 2-kolom (`.bento-grid` dengan elemen `.item`) tepat di bawah kredensial password broker.
  - Tersimpan secara persisten pada partisi NVS `cfg_mqtt` (kunci `qos` bertipe `u32` dan `keepalive` bertipe `u32`), terdaftar pada master provisioning `nvs_config.csv`, dan di-*backup* secara otomatis oleh logika inisialisasi pabrik `initNvsConfiguration()`.

## 59. Visualisasi QoS dan Keep Alive pada Kartu Telemetri MQTT Dashboard Overview
- **1. Struktur Grid Responsif 5 Kolom (`.sg-mqtt-data-grid`)**:
  - **Desktop / Tablet Widescreen (`min-width: 768px`)**:
    - Disusun dalam 1 baris grid 5 kolom presisi: `grid-template-columns: 1.8fr 0.75fr 1.15fr 0.85fr 0.8fr; gap: 16px;`.
    - Kolom 1: *Broker Host* (dengan tombol salin clipboard).
    - Kolom 2: *Port* (`%mqtt_broker_port%`).
    - Kolom 3: *Security (TLS)* (`%mqtt_tls_badge%`).
    - Kolom 4: *Delivery QoS* (`#mqtt-qos-val` / `%mqtt_qos_badge%`).
    - Kolom 5: *Keep Alive* (`#mqtt-keepalive-val` / `%mqtt_keepalive%s`).
  - **Mobile Viewport (`< 768px`)**:
    - Bertransformasi menjadi grid 2-kolom 3-tingkat yang simetris dan rapi:
      - Baris 1: Broker Host (membentang penuh: `grid-column: 1 / -1`).
      - Baris 2: Port (kiri) | Security TLS (kanan).
      - Baris 3: Delivery QoS (kiri) | Keep Alive (kanan).
- **2. Status Visual Badge QoS (`#mqtt-qos-val`)**:
  - **QoS 1**: `<span class="sg-subcard-status-badge active">QoS 1</span>` (Aksen hijau emerald transparan `rgba(16, 185, 129, 0.12)` dengan teks `#10b981`), menandakan jaminan pengiriman dengan `PUBACK`.
  - **QoS 0**: `<span class="sg-subcard-status-badge amber">QoS 0</span>` (Aksen amber transparan `rgba(245, 158, 11, 0.12)` dengan teks `#f59e0b`), menandakan transmisi fire-and-forget tanpa konfirmasi outbox.
- **3. Nilai Monospace Keep Alive (`#mqtt-keepalive-val`)**:
  - Ditampilkan dengan tipografi monospace industri: `font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace; font-size: 0.90rem; font-weight: 700; color: #ffffff;`.
  - Dilengkapi akhiran satuan detik (`30s`, `60s`).
- **4. Sinkronisasi Real-Time Dua Arah (SSR & Live Polling)**:
  - **Server-Side Render (SSR)**: Menggunakan template placeholder `%mqtt_qos_badge%` dan `%mqtt_keepalive%` di `src/web/web_template.cpp` sehingga saat HTML pertama kali dimuat oleh browser, nilai sudah langsung terisi tanpa layout shift (*zero CLS*).
  - **Live API Endpoint (`/api/v1/system/version`)**: Mengembalikan field numerik `"mqtt_qos"` dan `"mqtt_keepalive"`.
  - **Client-Side Polling Loop**: Fungsi `syncDashboardStatus()` di `index.html` menyegarkan elemen DOM `#mqtt-qos-val` dan `#mqtt-keepalive-val` secara mulus setiap 5 detik.

## 60. Halaman Dokumentasi Integrasi & Developer API (`data/docs.html`)
- **1. Otorisasi Akses Eksklusif Administrator (RBAC Guard)**:
  - Rute `/docs` dan `/docs.html` dilindungi oleh `docs_handler` di `src/web/web_routes.cpp`.
  - Pengguna tanpa sesi (`UserRole::NONE`) dialihkan seketika via `302 Found` ke `/login.html`.
  - Pengguna role Teknisi (`UserRole::TEKNISI`) dialihkan ke `/auth_teknisi.html`.
  - Hanya peran Administrator (`UserRole::ADMIN`) yang diizinkan mengakses konten dokumentasi integrasi.
- **2. Tata Letak Terstruktur 2-Kolom (Stripe/Supabase API Style)**:
  - **Kolom Kiri (~55%)**:
    - **Header Hero**: Banner judul `Integration & Developer API` dengan badge versi firmware.
    - **Ringkasan Koneksi Broker**: Tile mini parameter aktif (`%mqtt_broker_host%`, `%mqtt_broker_port%`, `%mqtt_tls_badge%`, `%mqtt_qos_badge%`).
    - **Tabel Spesifikasi Payload iBeacon**: Rincian field `mac`, `uuid`, `major`, `minor`, `rssi`, `tx_power`, `distance`, `batt_pct`, `timestamp` beserta tipe data monospace `.docs-type-badge`.
    - **Komparasi QoS**: Callout perbandingan QoS 1 (Guaranteed Delivery) vs QoS 0 (Fire & Forget).
    - **Tabel Local REST API**: Endpoint `/api/v1/system/version`, `/api/v1/system/ap-status`, `/api/v1/system/reboot` dengan method badges (`.docs-method-badge.get`, `.docs-method-badge.post`).
  - **Kolom Kanan (~45%)**:
    - **Sticky Code Terminal**: Kotak konsol kode gelap bernomor baris dengan syntax highlighting (`.ck-kw`, `.ck-fn`, `.ck-str`, `.ck-num`, `.ck-prop`, `.ck-cmt`).
    - **Language Switcher Pills**: Tab minimalis interaktif untuk 6 bahasa/platform: **`Python`**, **`Node.js`**, **`Go`**, **`Rust`**, **`Node-RED`**, dan **`cURL`**.
    - **Aksi Salin Kode (`.docs-copy-action-btn`)**: Tombol salin satu klik dengan teks berubah menjadi "Tersalin!" dan feedback toast.
- **3. Kebijakan Navigasi & Tampilan Mobile**:
  - **Sidebar Desktop**: Item navigasi `Integration Docs` dengan ikon `< / >` (`<polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/>`) ditambahkan secara konsisten pada seluruh halaman admin (`index.html`, `auth_admin.html`, `settings.html`, `firmware.html`, `log.html`, `test.html`).
  - **Top Header Mobile (`.navbar`)**: Konsisten dengan seluruh halaman administratif: Logo IOTARU di kiri, judul halaman (`Integration Docs`), dan tombol logout mobile (`#mobileLogoutBtn`) dengan ikon dan teks bahasa Indonesia (`Keluar`).
  - **Bottom Navbar Mobile (`.bottom-nav`)**: **TIDAK MENAMPILKAN** item Docs demi menjaga kebersihan, kerapian, dan ergonomi jempol pada layar smartphone berlebar 360–414px. Tetap mempertahankan 5 tombol inti standar yang simetris: `Overview`, `Config` (sheet trigger), `Test` (highlight center pill), `Firmware`, dan `Reboot` (power sheet trigger).

## 61. Kustomisasi Field Telemetri MQTT Dinamis (Dynamic Bitmask Protocol v2)
- **1. Deskripsi Komponen UI & Lokasi**:
  - Ditempatkan di dalam Bento Card 4 (*Broker Telemetri MQTT Gateway*) pada halaman `data/auth_admin.html`.
  - Menggunakan grid checkbox responsif (`.telem-checkbox-grid`) dengan tile interaktif (`.telem-chk-card`).
  - Dilengkapi indikator badge interaktif (`#telemByteBadge`) yang menghitung panjang total biner secara real-time via fungsi `recalcTelemMask()`.
- **2. Spesifikasi Token CSS Komponen**:
  - Grid Layout: `display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px;`.
  - Tile Card (`.telem-chk-card`):
    - Background normal: `#090e18`, Border: `1px solid #1a2333`, Border Radius: `9px`, Padding: `10px 14px`.
    - Hover state: Background `#0d1424`, Border `#334155`.
    - Active / Checked state: Background `rgba(14, 165, 233, 0.07)`, Border `rgba(56, 189, 248, 0.45)`.
  - Bobot Byte Badge (`.telem-chk-badge`):
    - Tipografi: `ui-monospace, monospace; font-size: 0.70rem; font-weight: 700;`.
    - Badge state checked: Background `rgba(56, 189, 248, 0.2)`, Text `#38bdf8`.
- **3. Arsitektur Bitmask Biner Protocol v2**:
  - Setiap beacon diawali 1 Byte Header Mask (`uint8_t`):
    - `0x80`: Identifier Protokol Dinamis v2 (selalu 1).
    - `0x01`: Proximity UUID (16 Byte).
    - `0x02`: Major ID (2 Byte, Big-Endian).
    - `0x04`: Minor ID (2 Byte, Big-Endian).
    - `0x08`: RSSI (1 Byte, Signed int8).
    - `0x10`: Battery % (1 Byte, Unsigned uint8).
    - `0x20`: Timestamp (4 Byte, Unsigned uint32 Big-Endian).
  - **Urutan Pengemasan Biner**: `UUID` &rarr; `Major` &rarr; `Minor` &rarr; `RSSI` &rarr; `Battery` &rarr; `Timestamp`.
  - **Universal Client Parser Compatibility**: Subscriber klien (Node.js, Python, Go, Rust) hanya menguji bitmask flag, sehingga perubahan centang di Web UI tidak pernah merusak atau membutuhkan modifikasi script klien.

## 62. Standarisasi Terminal Kode Dokumentasi Integrasi (Integration Docs Code Terminal)
- **1. Sinkronisasi Snippet dengan `test.js`**:
  - Tab `Node.js` ditetapkan sebagai default aktif (`switchSnippet('nodejs')`) pada konsol kode sticky terminal di `data/docs.html`.
  - Logika dekoder biner di dalam snippet mengadopsi secara persis arsitektur fungsi `parseBinaryPayload(buf)` dari `test.js`, meliputi penanganan dinamis Header Bitmask v2 (`0x80 | (mask & 0x3F)`) dan backward compatibility untuk Protokol Legacy v1 (`0x00` Standard 6B dan `0x01` Extended 7B).
- **2. Standarisasi Parameter Koneksi & Topik Subkripsi**:
  - **Topik Subkripsi**: Menggunakan format spesifik per gateway: `SG-10/data/<id-device>`.
  - **Kredensial Autentikasi**: Placeholder kredensial standar pengembang:
    - Username: `"MQTT_USERNAME"`
    - Password: `"MQTT_PASSWORD"`
  - Menghilangkan kredensial hardcoded pada dokumentasi publik untuk menjaga keamanan operasional.
- **3. Urutan Tab Bahasa Pengembang**:
  - `Node.js` (Aktif secara default) &rarr; `Python` &rarr; `Go` &rarr; `Rust` &rarr; `Node-RED` &rarr; `cURL`.
  - Seluruh contoh kode bahasa pemrograman pendukung diperbarui untuk membaca struktur paket biner telemetri iBeacon secara akurat.

## 63. Standarisasi Format Topik MQTT Dinamis (Dynamic Model Prefix)
- **1. Aturan Ekstraksi Prefix Model**:
  - Topik MQTT gateway diekstraksi secara dinamis dari konfigurasi NVS `dev_model` (`g_config.device_model`).
  - **Karakter Sebelum Dash (`-`)**: Mengambil substring sebelum tanda hubung `-` dan mengonversinya menjadi huruf kecil (*lowercase*).
    - Contoh: `AR-27` &rarr; prefix `ar`
    - Contoh: `SGD-268` &rarr; prefix `sgd`
  - **Fail-safe Tanpa Dash**: Jika nilai model tidak memiliki tanda hubung `-`, sistem mengambil maksimal 5 karakter alfanumerik pertama dalam format huruf kecil (*lowercase*).
    - Contoh: `GATEWAY` &rarr; prefix `gatew`
    - Contoh: `SG10` &rarr; prefix `sg10`
  - **Fail-safe Kosong**: Jika string model kosong atau tidak valid, sistem menggunakan default `ar`.
- **2. Format Topik Operasional Aktif**:
  - **Data Telemetri**: `<prefix>/data/<device_id>` (contoh: `ar/data/f5145c3c`)
  - **Status Operasional (Birth & LWT)**: `<prefix>/info/<device_id>/status` (contoh: `ar/info/f5145c3c/status`)
  - **Penerima Perintah (Command Inbound)**: `<prefix>/cmd/<device_id>` (contoh: `ar/cmd/f5145c3c`)
  - **Respon Perintah (Response Outbound)**: `<prefix>/res/<device_id>` (contoh: `ar/res/f5145c3c`)

## 64. Standarisasi Format Payload Status MQTT (Birth Message & LWT)
- **1. Skema JSON Terpadu (Unified Status Schema)**:
  - Digunakan secara konsisten baik saat koneksi awal berhasil (*Birth Message* `status: "online"`) maupun saat perangkat terputus tiba-tiba (*Last Will & Testament* `status: "offline"`).
  - Dikirimkan ke topik status operasional: `<prefix>/info/<device_id>/status` dengan flag `QoS 1` & `Retain 1`.
  - **Daftar Field JSON**:
    - `"status"`: `"online"` atau `"offline"`
    - `"device_id"`: ID perangkat unik berbasis MAC chip (contoh: `"f5145c3c"`)
    - `"model"`: Model perangkat dari NVS `dev_model` (contoh: `"AR-27"`)
    - `"mode"`: Mode koneksi aktif (`"wifi"` atau `"ethernet"`)
    - `"ipwlan"`: Alamat IP antarmuka Wi-Fi Station (atau `""` jika tidak terhubung)
    - `"ipeth"`: Alamat IP antarmuka Ethernet W5500 (atau `""` jika tidak terhubung/kabel lepas)
    - `"firm_ver"`: Versi firmware C++ ESP-IDF (contoh: `"v1.2.1"`)
    - `"ui_ver"`: Versi antarmuka web LittleFS (contoh: `"v1.2.1"`)
    - `"dev_ver"`: Versi board/hardware NVS `dev_ver` (contoh: `"v1.0.0"`)
  - **Contoh Payload**:
    ```json
    {
      "status": "online",
      "device_id": "f5145c3c",
      "model": "AR-27",
      "mode": "wifi",
      "ipwlan": "192.168.8.167",
      "ipeth": "",
      "firm_ver": "v1.2.1",
      "ui_ver": "v1.2.1",
      "dev_ver": "v1.0.0"
    }
    ```


## 65. Arsitektur Ketahanan Wi-Fi & Auto-Recovery Zombie Link (Enterprise Resilience)
- **1. Latar Belakang & Eliminasi False Positive**:
  - Pada lingkungan router komersial atau modem 4G/LTE, kueri DNS UDP port 53 sering kali di-drop oleh firewall lokal router tanpa memberikan respons. Logika lama yang menganggap ketiadaan respons port 53 sebagai link putus memicu gateway melakukan pemutusan radio paksa (`ASSOC_LEAVE: 8`) setiap 1–2 menit (*false positive*).
  - Kasus *silent drop* atau *zombie link* (di mana AP mencoret asosiasi station tanpa mengirimkan frame 802.11 Deauth) di lapangan merupakan anomali langka (rata-rata 1 kali dalam 3 hari).
- **2. Prinsip Kerja & Alur Keputusan Resilien**:
  - **Bypass Total Saat MQTT Online**:
    - Selama koneksi MQTT aktif (`MqttService::isConnected() == true`), fungsi `checkFastLinkHealth()` langsung me-*reset* counter RTO dan keluar (*return*) seketika.
    - Zero probe, nol beban komputasi soket, dan nol transmisi paket ke router gateway selama operasional normal.
  - **Pencegahan Thundering Herd & Broadcast Storm (Multi-Device Matrix)**:
    - **Unicast Layer 2/3 TCP Port 80**: Pemeriksaan integritas router dilakukan langsung ke alamat IP Gateway (`ip_info.gw.addr`) pada TCP port 80 dengan *socket non-blocking* dan *timeout* singkat 300 ms.
    - Jika port 80 terbuka (`so_error == 0` / SYN-ACK) ATAU jika port ditutup oleh firewall router (`so_error == ECONNREFUSED` / RST), kedua respon membuktikan bahwa router lokal hidup dan berkomunikasi lancar di LAN.
    - **MAC-Based Jitter**: Interval pengecekan diacak berbasis octet MAC unik: `(mac[5] % 10) * 1000` ms (rentang 0–9 detik). Mencegah puluhan perangkat di jaringan yang sama mengetuk gateway secara serentak.
    - **Exponential Backoff Saat Kuota Habis / ISP Padam**:
      - Jika router lokal terbukti hidup namun MQTT belum terhubung (kasus kuota seluler modem habis atau ISP pusat padam), gateway **DILARANG MEMUTUSKAN WI-FI**.
      - Web Dashboard tetap 100% dapat diakses oleh teknisi/administrator di IP lokal (`192.168.8.x`).
      - Interval probe secara adaptif digandakan (*exponential backoff*): 15s &rarr; 30s &rarr; 60s &rarr; 120s &rarr; 300s (maksimal 5 menit).
    - **Vonis Zombie Link Nyata**:
      - Hanya jika router gateway gagal merespons selama 2 kali percobaan berturut-turut pada interval terjadwal (menandakan Wi-Fi radio benar-benar terisolasi / *zombie link*), gateway memanggil `forceWifiReassociate("Router Unreachable / Zombie Link")`.
      - Pemutusan sesi radio bersih dilakukan di level driver (`esp_wifi_disconnect()`) dan langsung menyambung kembali dengan memanfaatkan *channel cache* (`WIFI_FAST_SCAN` ~1.5 detik) tanpa restart perangkat keras.
  - **Reset Otomatis Status Backoff**:
    - Ketika Wi-Fi memperoleh IP (`IP_EVENT_STA_GOT_IP`) atau ketika koneksi MQTT berhasil terjalin (`MQTT_EVENT_CONNECTED`), interval backoff dan counter RTO di-reset ke nilai awal (15.000 ms).


## 66. Standarisasi Keandalan EEPROM AT24C256 & Net Log Service (Industrial Grade)
- **1. Pemulihan Bus I2C Otomatis Pasca-Reboot Lunak (`recoverBus`)**:
  - Menyelesaikan fenomena *false RAM-only fallback* saat ESP32 melakukan *soft restart* atau *watchdog reboot* ketika jalur I2C SDA sedang tertahan LOW oleh EEPROM.
  - Sebelum inisialisasi I2C master bus, pin SCL dipulsa hingga 9 kali dan diakhiri kondisi STOP I2C untuk membebaskan bus kembali ke kondisi IDLE (HIGH).
  - Dilengkapi *multi-attempt probe* (3 kali percobaan dengan interval 15 ms) untuk mengakomodasi waktu stabilisasi daya (*power rail settling time*).
- **2. Penguncian Atomik Dua Halaman & Acknowledge Polling (`writeSlot`)**:
  - Penulisan 1 slot log (128 byte) terdiri dari 2 halaman (Page 1: 64B, Page 2: 64B).
  - Mutex `s_i2cMutex` dikunci secara menyeluruh mencakup transmisi Page 1, siklus tulis internal EEPROM ($t_{WR}$ via active *Acknowledge Polling*), transmisi Page 2, dan siklus tulis Page 2.
  - Menghilangkan *race condition* antar-task (MQTT / Web Server) yang sebelumnya memicu kegagalan penulisan Halaman 2 dan korupsi checksum (offset 118).
- **3. Toleransi Jitter Baca (`readBuffer`) & Re-Probe Dinamis Runtime (`isDetected`)**:
  - `readBuffer()` kini menyertakan 1 kali percobaan ulang (*retry*) jika menerima NACK sesaat akibat *noise* atau jitter bus I2C.
  - `At24c256::isDetected()` melakukan *re-probe* dinamis di latar belakang jika EEPROM sempat tidak terdeteksi saat boot awal, memungkinkan pemulihan otomatis dari mode RAM ke EEPROM tanpa restart fisik.
- **4. Jaminan Non-Zero Snapshot Fallback pada Web Dashboard (`getLogs`)**:
  - Web UI mengambil data log 100% dari buffer RAM `s_logs` (cepat, hemat komputasi, dan zero flash/EEPROM wear).
  - Timeout mutex dinaikkan dari 100 ms ke 500 ms.
  - Jika mutex sedang digunakan oleh penulisan EEPROM, fungsi `getLogs()` langsung menyalin memori RAM `s_logs` (RAM snapshot fallback) dan **DILARANG mengembalikan 0 log kosong**. Memastikan tabel log di browser web selalu terisi dan tidak pernah tampak kosong sesaat.


## 67. Standarisasi UI/UX System & Data Logs (`log.html`) - Cybernetic Dark Bento Grid (2026)
- **1. Harmonisasi Visual Hierarchy dengan Overview (`index.html`)**:
  - Container `.content` kini menggunakan konfigurasi industrial standar `max-width: 1440px !important`, dengan padding desktop `24px 28px 28px 28px` dan padding mobile `16px 16px 90px 16px`.
  - Mengadopsi palet warna dan token Bento Grid:
    - Base Dark Background: `#080c14`
    - Card Container: `#0d121f` dengan border `1px solid #1a2333`, border-radius `14px`, dan box-shadow `0 4px 20px rgba(0, 0, 0, 0.35)`.
    - Subcards & Inset Controls: Background `#090e18` dengan border `1px solid #1a2333`.
    - Typography & Monospace: Tipografi UI menggunakan `-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto` dan semua nilai metrik, timestamp, IP, MAC, RSSI, dan counter menggunakan `ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace`.
- **2. Slim Top Header Card (`.sg-header-card`)**:
  - Menggantikan judul `.net-log-header` lama dengan Bento Header Card ramping yang identik dengan Overview:
    - Chip icon 42x42px berlatar `rgba(30, 58, 138, 0.25)` dengan border `rgba(59, 130, 246, 0.35)` dan icon SVG terminal berwarna `#60a5fa`.
    - Judul tebal `SYSTEM & DATA LOGS` (`1.08rem`, font-weight 700, warna `#ffffff`), titik pemisah dot separator (`.sg-title-dot`), dan subtitle deskriptif `"Dual-Engine Realtime Auditor: LittleFS Network Audit Trail & RAM Ring Buffer"`.
    - Capsule badge pill (`.sg-badge-pill`) dengan pulse dot dan label `"ROLLING FIFO (2x50)"`.
- **3. Master Segmented Tab Selector (`.master-tabs`)**:
  - Segmented control bergaya cybernetic bento dengan container `#090e18`, border `1px solid #1a2333`, border-radius `12px`, dan padding `5px`.
  - Tab "Log Jaringan & Sistem" menggunakan pulse dot cyan (`#38bdf8`) dan active glow `rgba(56, 189, 248, 0.14)`.
  - Tab "Log Drop Data Beacon" menggunakan pulse dot rose (`#fb7185`) dan active glow `rgba(244, 63, 94, 0.14)`.
  - Capsule pill counter monospace (`badgeCountNet`, `badgeCountDrop`) untuk status volume log real-time.
- **4. Bento KPI Cards (Dual 4-Column Responsive Grid)**:
  - Grid 4 kolom responsif (`repeat(4, 1fr)` pada desktop, 2 kolom pada tablet, 1 kolom pada mobile).
  - Setiap card dilengkapi Bento Icon Box 34x34px (`#141c2c`, border `#233148`, border-radius `8px`) dengan warna ikon semantik:
    - Wi-Fi Station: Emerald (`#10b981`)
    - Ethernet W5500: Blue (`#60a5fa`)
    - MQTT Broker: Violet (`#c084fc`)
    - ESP32 Reboot: Cyan (`#38bdf8`)
    - Total Drop: Rose (`#fb7185`)
    - Filter RSSI: Amber (`#fbbf24`)
    - Filter Target: Orange (`#fb923c`)
    - MQTT Fail: Red/Rose (`#ef4444`)
  - Nilai metrik angka besar monospace (`1.35rem`, font-weight 700, `#ffffff`).
  - Baris informasi sub-metrik bergaris batas halus `border-top: 1px solid rgba(255, 255, 255, 0.04)` dengan teks tebal monospace kontras tinggi.
  - Mempertahankan 100% ID DOM elemen untuk sinkronisasi JavaScript (`log.js`): `cardKpiWifi`, `kpiWifiBadge`, `dotWifi`, `textWifiStatus`, `kpiWifiSsid`, `kpiWifiIp`, `kpiWifiDc`, `cardKpiEth`, `kpiEthBadge`, `dotEth`, `textEthStatus`, `kpiEthLink`, `kpiEthIp`, `kpiEthDc`, `cardKpiMqtt`, `kpiMqttTitle`, `kpiMqttBadge`, `dotMqtt`, `textMqttStatus`, `kpiMqttHost`, `kpiMqttState`, `kpiMqttPort`, `kpiMqttDc`, `kpiBootStorageBadge`, `kpiBootCount`, `kpiResetReason`, `kpiDropTotal`, `kpiDropTtl`, `kpiDropOvr`, `kpiDropRssi`, `kpiDropFilter`, `kpiDropUuid`, `kpiDropMajor`, `kpiDropMqtt`.
- **5. Toolbar & Action Controls (`.log-toolbar-card`)**:
  - Container Bento `#0d121f` dengan pemisah bersih antara bilah filter/aksi atas dan bilah pencarian bawah.
  - Filter pills bertema segmented control `#090e18` untuk filter kategori jaringan (`Wi-Fi`, `Ethernet`, `MQTT`, `SoftAP`, `System`) dan filter drop beacon (`RSSI Min`, `UUID`, `Major`, `Overflow`, `TTL`, `MQTT Fail`).
  - Action button cluster (`#btnAutoRefresh`, `#btnManualRefresh`, `#btnExportCsv`, `#btnClearLogs`) dengan tombol `#141c2c`, border `#233148`, dan efek hover glowing.
  - Form pencarian terintegrasi dengan ikon search SVG dan counter record real-time.
- **6. Cybernetic Log Table Card (`.net-log-table-wrap`)**:
  - Container bento rapi dengan info header bar `#090e18`.
  - Header kolom berlatar `#090e18` dengan teks uppercase `#64748b` (`0.78rem`, font-weight 700, letter-spacing `0.05em`).
  - Efek hover baris tabel `rgba(56, 189, 248, 0.03)` dengan garis pemisah tipis `rgba(255, 255, 255, 0.04)`.
  - Baris footer pagination modern dengan navigasi halaman monospace (`.btn-page`).


## 68. Standarisasi Tabel Live Test & Drop Log untuk Stream Universal 42-Byte BLE (2026)
- **1. Transisi dari iBeacon Khusus ke Universal Promiscuous Gateway**:
  - Tabel `test.html` (Buffer FIFO) disederhanakan secara presisi untuk throughput tinggi dengan **menghapus kolom PDU Type dan Data Telemetri**.
  - Kolom Buffer FIFO kini berfokus pada:
    1. `#`: Nomor urut stream FIFO.
    2. `Waktu`: Timestamp kedatangan frame (`HH:mm:ss.SSS`).
    3. `MAC Address` (`.mono-mac`): 6-byte hexadecimal monospace (`AA:BB:CC:DD:EE:FF`) dengan background `rgba(56, 189, 248, 0.08)`, border `rgba(56, 189, 248, 0.2)`, dan warna `#38bdf8`.
    4. `Format / Protokol Semantik` (`.proto-badge`): Badge format iBeacon, Minew, MOKO Alarm, Sensor, Eddystone, Universal.
    5. `Indikator RSSI Sinyal` (`.rssi-cell`, `.rssi-bars`): Visual 4-bar sinyal bertingkat (`.rssi-strong` hijau `#10b981`, `.rssi-medium` kuning `#f59e0b`, `.rssi-weak` merah `#f43f5e`) dan dBm.
    6. `Expandable Raw 42-Byte Inspector` (`.btn-raw-hex`, `.raw-drawer`, `.raw-breakdown`): Tombol `[42B Hex ▾]` per baris yang membuka drawer inline berisi dekonstruksi warna biner per byte (Sync `0xAA`, MAC 6B, PDU 1B, RSSI 1B, Len 1B, Payload 31B, CRC 1B) serta string heksadesimal lengkap 42-byte.
- **2. Visualisasi Conditional Filtering pada Paket Ditolak**:
  - Kolom Alasan Drop (`.drop-badge`) memvisualisasikan aturan kondisional:
    - `RSSI_OUT_OF_RANGE`: Paket dari sinyal lemah di bawah batas ambang slider.
    - `UUID_MISMATCH`: Khusus untuk paket yang memiliki field UUID. Paket non-UUID (seperti MOKO Alarm/Sensor) otomatis lolos filter UUID.
    - `MAJOR_MISMATCH`: Khusus untuk paket yang memiliki field Major. Paket tanpa Major otomatis lolos filter Major.
    - `QUEUE_FULL`: Antrean buffer internal gateway penuh.


## 69. Standarisasi Hierarchical Device Card & Nested Frame UI untuk Unique Beacons (2026)
Mengadopsi pola arsitektur kartu hirarkis bertingkat (Device Card & Nested Frame) dari spesifikasi referensi monitor BLE dengan tetap mempertahankan tema warna utama IOTARU Cybernetic Dark (`#060b13` / `#0d121f` / `#141c2c`, cyan neon `#38bdf8`, emerald `#10b981`, rose `#f43f5e`):
- **1. Struktur Kartu Perangkat (`.u-card`)**:
  - **Header Kartu (`.u-card-head`)**:
    - MAC Address fisik (`.u-mac`) dalam font monospace tebal warna `#38bdf8`.
    - Sinyal RSSI terbaik (`.u-rssi-wrap`) dengan visual signal strength bars dan nilai dBm berwarna semantik.
    - Baris metadata (`.u-meta`): Nama perangkat, jumlah tipe frame yang ditangkap (`Frames: X Tipe`), waktu terakhir terlihat (`Last seen: Xs lalu`), dan chevron panah ekspansi (`.u-chev`).
  - **Toolbar Ringkasan (`.u-card-sum`)**:
    - Menampilkan total paket yang diterima oleh MAC ini (`Total packets: X`) dan tombol aksi hapus kartu (`.btn-u-del`).
- **2. Bagian Frame Bertingkat (`.u-frame`)**:
  - Setiap perangkat dapat memuat beberapa tipe frame berbeda yang disiarkan (misalnya satu beacon MOKO menyiarkan frame Alarm `0xFEE0` dan frame Sensor Info `0xEA00` secara bergantian).
  - **Header Frame (`.u-frame-head`)**:
    - Indikator dot semantik (`.u-frame-dot`): MOKO Alarm (`.dot-moko` / `.dot-alert` berkedip), MOKO Sensor (`.dot-sensor` emerald), iBeacon (`.dot-ibc` indigo), Minew (`.dot-minew` cyan), Eddystone (`.dot-eddy` yellow).
    - Judul frame (`.u-frame-title`) dan waktu terakhir frame tersebut terlihat.
  - **Daftar Definisi Telemetri (`.u-dl`)**:
    - Grid responsif dua kolom (`minmax(90px, 130px) 1fr`) yang menampilkan seluruh atribut hasil decode secara detail:
      - MOKO Alarm: Mode klik, Status (`ALARM TRIGGERED` pulsing badge), Count, Device ID, Firmware, Password.
      - MOKO Sensor Info: Battery mV & %, Suhu (°C), Akselerometer 3-Sumbu X/Y/Z (mg / g), Scale, Threshold, Ranging.
      - iBeacon / Minew: UUID, Major, Minor, Baterai, Tx Power, Ranging.
      - Eddystone: Tipe (UID/URL/TLM), Namespace, Instance, URL, Adv count, Uptime.
- **3. Kotak Inspeksi Payload Mentah & Salin Cepat (`.u-rawbox`, `.btn-u-copy`)**:
  - Baris header collapsible berlatar `#141c2c` dengan label `"Raw payload · 42 bytes"`.
  - Tombol ikon salin satu klik (`.btn-u-copy`) yang otomatis menyalin seluruh string raw heksadesimal ke clipboard pengguna disertai notifikasi toast melayang (`.mock-toast`).
  - Tampilan teks heksadesimal (`.u-hex`) berlatar `#040810` dengan monospace font dan pembungkus kata otomatis.


## 70. Standarisasi Kontrol Mode Pemindaian BLE (Scanner Mode) pada Device Settings (2026)
- **1. Konsep Desain Segmented Pill Selector (`.scan-mode-selector`)**:
  - Ditempatkan di bagian atas Bento Card 1 (*Filter BLE Scanner & Sinyal*) pada `settings.html`.
  - Berfungsi mengatur mode kerja radio nRF52840 secara dua arah (*bidirectional UART*).
  - Terdiri dari dua opsi tactile pill:
    1. **Passive Scan (`1` - Default NVS)**:
       - Warna Aksen: Cyan Neon (`#38bdf8`) dengan active background `rgba(56, 189, 248, 0.15)`.
       - Deskripsi: *Mode Pasif bawaan pabrik, hanya mendengarkan paket siaran radio (ADV) tanpa memancarkan sinyal balik (Zero RF emission / stealth).*
    2. **Active Scan (`2`)**:
       - Warna Aksen: Emerald Neon (`#10b981`) dengan active background `rgba(16, 185, 129, 0.15)`.
       - Deskripsi: *Mode Aktif, mengirimkan frame SCAN_REQ untuk meminta SCAN_RSP tambahan (seperti nama lengkap perangkat, data sensor Eddystone/MOKO).*
- **2. Interaktivitas Reaktif & Aturan NVS-Only (Reboot Required)**:
  - Segmented toggle terhubung dengan input tersembunyi `<input type="hidden" id="scan_mode" name="scan_mode" value="%scan_mode%">`.
  - Mengubah opsi memicu dirty state pada form (`#floatingSaveDock` otomatis melayang).
  - **Prinsip Imutabilitas Runtime (Industrial Safety)**: Saat tombol Simpan ditekan, nilai baru **hanya disimpan ke NVS (Flash)**. Sistem **DILARANG KERAS mengubah variabel yang sedang berjalan di RAM atau mengirim perintah langsung ke nRF52840 saat runtime**.
  - Variabel aktif hanya dimuat ke memori saat startup / booting (`saat start saja`). Oleh karena itu, antarmuka memunculkan konfirmasi bahwa perubahan memerlukan **Reboot Perangkat** untuk diaktifkan ke hardware pemindai nRF52840.


## 71. Standarisasi Tabel Riwayat Paket Drop Data (Drop Log Table) 8-Kolom dengan Monospace MAC & Inline Raw Hex Drawer (2026)
- **1. Struktur Kolom Presisi 8-Elemen**:
  - `#`: Nomor urut log drop (auto-increment id).
  - `Waktu`: Waktu kejadian drop (`HH:mm:ss` atau uptime).
  - `MAC Address` (`.mono-mac`): Monospace cyan `#38bdf8` dengan kontras tinggi (`AA:BB:CC:DD:EE:FF`).
  - `Tipe / Format` (`.beacon-type-pill`): Badge protokol BLE (Apple iBeacon, iBeacon Ext, Eddystone, MOKO Alarm, Sensor, MQTT).
  - `Identitas / Tag` (`.tag-mono`): Major-Minor atau data tag beacon.
  - `RSSI` (`.rssi-val`): Nilai kekuatan sinyal dBm dengan pewarnaan semantik (`.rssi-strong` hijau `#10b981`, `.rssi-medium` kuning `#fbbf24`, `.rssi-weak` merah `#f43f5e`).
  - `Alasan Drop` (`.badge-reason`): Label semantik dengan pulse dot (TTL Expired, RSSI < Min, UUID Mismatch, Major Mismatch, Queue Overflow, MQTT Fail).
  - `Raw 42B` (`.btn-raw-hex`): Tombol aksi untuk toggle inline drawer yang menampilkan seluruh stream 42 byte dan tombol salin 1-klik (`Salin Hex`).
- **2. Interaktivitas Inline Drawer & Clipboard**:
  - Baris `.raw-drawer` dibuka/ditutup secara instan tanpa reload halaman (`data-act="toggle-log-raw"`).
  - Menampilkan spaced hex bytes (`AA FE 22 ...`) dan tombol salin clipboard yang memicu toast visual melayang (`#uiToast`).
- **3. Integrasi Ekspor CSV**:
  - Menghasilkan file CSV terstruktur 8 kolom: `No, Waktu, MAC Address, Format / Tipe, Identitas / Tag, RSSI(dBm), Alasan, Raw 42B Hex`.


## 72. Standarisasi Hierarchical Device Card UI pada Tab Unique Beacons (/test.html) & Floating Toast (.ui-toast) (2026)
- **1. Konsep Hierarchical Device Card**:
  - Menggantikan tabel datar konvensional pada tab *Unique Beacons* (`#tabUnique`) dengan kartu perangkat terstruktur bertingkat (`.unique-cards-list`, `.u-card`).
  - **Header Kartu Ringkas (`.u-card-head`)**:
    - Alamat MAC format monospace tebal (`.u-mac`, warna cyan neon `#38bdf8`).
    - Bar indikator RSSI dinamis 4 tingkat (`.u-rssi-wrap`, `.rssi-bars`) disertai angka nilai sinyal dBm bersandi warna (Hijau `>-68dBm`, Kuning `>-80dBm`, Merah `<=-80dBm`).
    - Metadata badge protokol (`.beacon-type-pill`: Apple iBeacon, Eddystone, MOKO, Minew, dll.).
    - Counter akumulasi hit (`Hits: X pkt`) dan waktu deteksi terakhir (`formatTime`).
    - Indikator chevron animasi rotasi 180° (`.u-chev`) saat kartu terbuka.
  - **Bodi Detail Perangkat (`.u-card-body`)**:
    - Ringkasan header internal dengan tombol hapus perangkat (`.btn-u-del` dengan ikon trash) untuk membersihkan entri dari cache unik browser.
    - Sub-kartu frame telemetri (`.u-frame`) dengan status dot indikator warna (`.dot-beacon`, `.dot-eddy`, `.dot-minew`, `.dot-alert`).
    - Grid parameter (`.u-dl`): Format / Tipe, Identitas / Tag (UUID, Major, Minor, atau Namespace/Instance), PDU Type (0x00, 0x02, dll.), Adv Data Len, dan indikator persentase baterai (`.badge-battery`).
    - Accordion laci data mentah 42 byte (`.u-rawbox`, `.u-rawh`, `.btn-u-rawt`) yang dapat diekspansi untuk memeriksa stream biner lengkap yang dipisah spasi (`.u-hex`).
    - Tombol salin 1-klik (`.btn-u-copy`) yang langsung menyalin string heksadesimal mentah ke clipboard sistem.
- **2. Notifikasi Toast Mengambang Global (`.ui-toast`)**:
  - Elemen fixed container di tengah bawah layar (`left: 50%`, `bottom: 84px`, `transform: translateX(-50%)`).
  - Desain *dark translucent glassmorphism* (`background: rgba(15, 23, 42, 0.95)`, `backdrop-filter: blur(12px)`), garis tepi cyan neon (`rgba(56, 189, 248, 0.4)`), dan teks tebal `#38bdf8`.
  - Berpindah status melalui kelas `.ui-toast.show` dengan animasi translateY dan fade transisi `cubic-bezier(0.16, 1, 0.3, 1)`, otomatis hilang setelah durasi 1.5 detik.

## 73. Standarisasi Hybrid Save Action (Permanent Form Action + Floating Save Dock) pada /settings.html (2026)
- **1. Filosofi Desain Hybrid**:
  - Menyediakan dua akses penyimpanan formulir untuk kenyamanan maksimal pengguna:
    1. **Permanent Save Action (`#btnMainSave`)**:
       - Tombol primer terletak statis di bagian bawah card formulir konfigurasi (`.form-actions-bottom`).
       - Memastikan pengguna yang menggulir halaman hingga ke ujung selalu melihat tombol simpan secara intuitif tanpa perlu menunggu docking mengambang.
    2. **Floating Save Dock (`#floatingSaveDock`)**:
       - Bilah melayang (*docking bar*) yang menempel di bagian bawah viewport ketika formulir mengalami perubahan nilai (*dirty state*).
       - Menampilkan ringkasan teks peringatan, tombol *Batal* (`#btnCancelFloatingSave`) untuk rollback nilai input ke snapshot awal, dan tombol *Simpan Perubahan* (`#btnFloatingSave`).
- **2. Mekanisme Dirty Checking & Pencegahan Fatal Error**:
  - Menggunakan fungsi terpadu `checkDirty()` yang membandingkan nilai input aktif terhadap `initialValues`.
  - Event listener `input`, `change`, dan klik tombol segmented (`[data-mode]`) secara reaktif memicu evaluasi status dirty.
  - Memastikan seluruh panggilan inisialisasi pada `DOMContentLoaded` merujuk ke nama fungsi yang konsisten (`checkDirty()`) untuk mencegah `ReferenceError` yang dapat menghentikan eksekusi script.








## 74. Standarisasi Unique Beacons Monitor: In-Place Text-Only Updates & Skema 13 Protokol BLE (2026)
- **1. Latar Belakang & Masalah Kerusakan Accordion Sebelumnya**:
  - Pada implementasi sebelumnya, setiap kali pesan WebSocket atau paket beacon baru masuk ke gateway (bisa puluhan hingga ratusan per detik), fungsi `renderTable()` dipanggil dan melakukan `uniqueCardsList.innerHTML = html;`.
  - Hal ini memusnahkan (*destroy*) seluruh pohon DOM kartu di tab *Unique Beacons*, mereset kelas status ekspansi (`.card.open`, `.frame.open`, `.hex.show`), dan menyebabkan pengguna tidak dapat berinteraksi, membuka kartu, atau memeriksa payload heksadesimal saat live stream berjalan.
- **2. Prinsip Pembaruan Teks In-Place (*In-Place Text-Only DOM Updates*)**:
  - **Pencegahan Re-render**: Pemanggilan `renderTable()` pada listener WebSocket (`ws.onmessage` dan testing hooks) dibatasi secara ketat hanya jika `activeTableTab === "fifo"`. Pada tab `unique`, aliran paket live ditangani langsung oleh `insertBeacon(beacon)`.
  - **Penambahan Entri Baru**: Jika alamat MAC perangkat belum ada di `devicesMap`, elemen kartu baru (`.card`) dibuat dan disisipkan dengan `uniqueCardsList.prepend(newCard)` tanpa menyentuh elemen-elemen kartu lain yang sedang terbuka.
  - **Pembaruan Entri yang Sudah Ada**: Jika kartu sudah ada di DOM (`.card[data-mac="..."]`), fungsi `insertBeacon()` HANYA memperbarui node teks target:
    - `.head .rssi`: Memperbarui bars sinyal dan teks `${r} dBm`.
    - `.meta span:first-child b`: Memperbarui jumlah tipe sub-frame `d.frames.length`.
    - `.meta [data-seen]`: Memperbarui label waktu relatif perangkat (`0s ago`).
    - `.sum b`: Memperbarui penghitung paket live `${d.pkts.toLocaleString('en-US')}`.
    - `.frame .fs [data-fseen]`: Memperbarui waktu relatif frame spesifik (`0s ago`).
    - `.frame .dl dd`: Memperbarui nilai RSSI dan field telemetri yang berubah.
    - `.frame .rawt em`: Memperbarui jumlah biner mentah (`· N bytes`).
    - `.frame .hex`: Memperbarui string heksadesimal mentah yang dipisah spasi.
    - `.frame .icobtn[data-act="copy"]`: Memperbarui atribut `data-raw`.
  - **Stabilitas State Accordion**: Status ekspansi kartu (`open`), sub-frame (`frOpen`), dan drawer raw payload (`rawOpen`) disimpan dalam `Set()` dan tidak pernah ter-reset saat ribuan paket streaming masuk.
- **3. Skema 13 Protokol Frame BLE & Token Warna Standar**:
  - `--moko: #f472b6`: `moko_alarm` (Mode, Status, Count, Device ID, Firmware, Password, Name), `moko_sensor` (Battery, Temp, Accel, Scale, Threshold, Ranging, MAC, Tx Power), `moko_device` (Battery, Firmware, Light sensor, Light, Connectable, Interval, Ranging, MAC, Tx Power, Name), `moko_acc` (Accel, Scale, Rate, Threshold, Battery, Interval, Ranging, MAC, Tx Power), `moko_th` (Temp, Humidity, Battery, Interval, Ranging, MAC, Tx Power), `moko_ibeacon` (Interval, Major, Minor, UUID, Ranging, Tx Power).
  - `--ibc: #a78bfa`: `ibeacon` (UUID, Major, Minor, Ranging, Tx Power).
  - `--eddy: #22d3ee`: `eddy_uid` (Type, Namespace, Instance, Ranging), `eddy_url` (Type, URL, Ranging, Tx Power), `eddy_tlm` (Type, Battery, Temp, Adv count, Uptime).
  - `--minew: #38bdf8`: `minew_ext` (UUID, Major, Minor, Battery, Ranging), `minew_info` (Name).
  - `--raw: #fbbf24`: `raw` (AdvType, Length, AD Type).
- **4. Teks, Ikon, & Toast Feedback 100% Sesuai Desain Acuan**:
  - Tombol Salin: Teks notifikasi toast seragam `Raw disalin` saat tombol copy diklik, disertai perubahan sementara ikon menjadi tanda centang hijau (`.icobtn.ok`).
  - Fallback Clipboard: Menyediakan mekanisme fallback textarea untuk lingkungan gateway HTTP non-SSL (`window.isSecureContext === false`).
  - Tombol Hapus: Menggunakan tombol `.sum .del` dengan ikon tempat sampah dan teks `Hapus`, menghapus node kartu dari DOM seketika (`card.remove()`) tanpa re-render penuh.


## 75. Arsitektur Zero-OOM Web & REST API: Direct Stream-Chunked JSON & Heap Guard (2026)
- **1. Latar Belakang Kegagalan Memori (Heap Starvation) Multi-Client**:
  - Pada pengujian bersamaan menggunakan browser laptop dan smartphone di halaman `/log.html`, serial monitor mencatat `Cannot allocate memory (... free heap: 16 bytes)` diikuti `wifi:fail to alloc timer, type=8` dan pemutusan koneksi Wi-Fi `ASSOC_FAIL (203)`.
  - **Akar Masalah**: Panggilan paralel `Promise.all([fetchNetLogs(), fetchDropLogs()])` setiap 5 detik memicu alokasi pohon memori monolitik cJSON (masing-masing 50 objek dengan 7 hingga 11 field, total 350 - 550 node cJSON) dan serialisasi buffer string kontigu ~25 KB via `cJSON_PrintUnformatted()`. Saat 2 perangkat mengakses secara bersamaan, lonjakan kebutuhan heap mencapai 260 KB, melampaui sisa dynamic DRAM ESP32 (~80-110 KB).
- **2. Standar Direct Stream-Chunked JSON Serializer (Zero-Heap DRAM)**:
  - **Penghapusan cJSON pada Hot Routes**: Seluruh endpoint berat (`/api/v1/system/net-logs`, `/api/v1/system/drop-logs`, `/api/v1/wifi/scan`, dan `/api/v1/system/version`) tidak lagi membuat pohon AST cJSON maupun string kontigu besar di heap.
  - **Stack-Chunked Streaming**: Data dibaca langsung dari buffer statis BSS (`s_tempLogs`, `s_tempDropLogs`) dan diformat baris demi baris ke dalam buffer stack lokal `char chunk[512]` menggunakan `snprintf()` dan `httpd_resp_send_chunk()`.
  - **Efisiensi Alokasi DRAM**: Alokasi memori dinamis di heap untuk seluruh endpoint ini menjadi **0 byte**, mengeliminasi fragmentasi memori dan lonjakan heap seketika.
- **3. Arsitektur Heap Guard Circuit Breaker**:
  - Menyisipkan pemeriksaan batas aman `check_heap_safety(req, 30000)` di awal setiap handler HTTP dan telemetri broadcast.
  - Jika free heap berada di bawah 30-35 KB (ambang batas kritis untuk stabilitas lwIP, Wi-Fi driver, dan mbedTLS MQTT), server web langsung mengembalikan respon `HTTP 503 Service Unavailable` dengan header `Retry-After: 3` tanpa memproses payload berat.
  - **Jaminan Prioritas Pipeline BLE**: Pipeline pembacaan UART di Core 0 (hingga 1.200–1.500 data/detik) dan antrean MQTT di Core 1 dipastikan **100% aman dan tidak pernah kelaparan memori (*memory starved*)** oleh aktivitas web browser.
- **4. Optimasi Frontend Polling (Tab-Aware & Visibility-Aware)**:
  - **Tab-Aware On-Demand Polling**: Skrip `data/assets/js/log.js` hanya memanggil `fetchNetLogs()` saat tab *Event Log Jaringan* aktif, dan hanya memanggil `fetchDropLogs()` saat tab *Data Log Drop* aktif. Penghapusan `Promise.all()` memangkas lalu lintas request API dan beban memori hingga 50%.
  - **Page Visibility Sensor**: Interval auto-refresh ditangguhkan secara otomatis ketika tab browser diminimalkan atau tidak aktif (`document.hidden`), dan langsung disinkronkan kembali saat tab dibuka (`visibilitychange`).
  - **Interval Aman**: Interval auto-refresh dinaikkan dari 5 detik menjadi 8 detik untuk mencegah perebutan bandwidth radio Wi-Fi.


## 76. Standarisasi Keselarasan UI/UX Multi-Halaman Gateway: Standar Acuan Overview (2026 Cybernetic Dark)
- **1. Latar Belakang & Filosofi Desain (Visual Harmony)**:
  - Sebelum standarisasi ini, beberapa halaman internal gateway (`settings.html`, `test.html`, `log.html`, `auth_admin.html`, `firmware.html`, `docs.html`) memiliki inkonsistensi visual dibandingkan halaman utama **Overview** (`index.html`): perbedaan warna latar kartu (`#1e293b` vs `#0d121f`), garis divider tebal pada header kartu yang memecah fokus, active state sidebar nav yang tidak seragam (cyan vs royal blue), ketiadaan *Slim Top Header Card* di beberapa halaman, serta teks tombol logout yang bercampur (`Keluar` vs `Logout`).
  - Untuk menciptakan pengalaman antarmuka kelas industri (*Tier-1 Industrial Web Dashboard*), seluruh halaman diselaraskan ke acuan desain tunggal: **Overview 2026 Cybernetic Dark Bento Grid**.
- **2. Palet Token Warna Terpusat (`data/assets/css/app.css`)**:
  - `--bg-main: #070c14`: Deep Space Dark Canvas.
  - `--card-bg: #0d121f`: Cyber Slate Primary Card Background.
  - `--card-border: #1a2333`: Subtle Blue-Slate Outer Border.
  - `--input-bg: #090d16`: Pitch Dark Nested Subcard & Form Control Background.
  - `--input-border: #1a2333`: Clean Nested Divider Border.
  - `--primary: #2563eb`: Royal Blue Solid Brand Accent (Hover: `#1d4ed8`, Glow: `rgba(37, 99, 235, 0.35)`).
- **3. Arsitektur Komponen Desain Standar (`.sg-*`)**:
  - **Slim Top Header Card (`.sg-header-card`)**:
    - Kontainer banner ramping atas: latar `#0d121f`, border `1px solid #1a2333`, sudut membulat 14px, padding 16px 22px, bayangan halus `0 4px 20px rgba(0, 0, 0, 0.35)`.
    - Ikon Chip 42x42px (`.sg-chip-icon`): latar transparan bernuansa biru/cyan/emerald/amber/purple dengan sudut membulat 10px dan border aksen.
    - Tipografi Judul (`.sg-title-group`): Judul halaman bold putih 1.08rem (`.sg-title`), dot pemisah `#475569` (`.sg-title-dot`), dan deskripsi abu-abu 0.88rem (`.sg-subtitle`).
    - Status Badge Pill (`.sg-badge-pill`): Kapsul status kanan dengan pulsing dot (misal: `● HARDWARE SYNC`, `● ROLLING FIFO (2x50)`, `● OTA RECOVERY READY`, `● ADMIN ACCESS`).
  - **Kontainer Bento Utama (`.sg-card` / `.bento-card`)**:
    - Latar `#0d121f`, border `1px solid #1a2333`, border-radius 16px, padding 24px 26px, bayangan elevasi `0 6px 24px rgba(0, 0, 0, 0.35)`.
  - **Header Kartu Tanpa Pembatas Bawah (`.sg-card-header`)**:
    - Mengeliminasi garis divider (`border-bottom: none`), menggunakan `gap: 12px` / `gap: 14px`.
    - Dilengkapi ikon kotak membulat 42x42px (`.sg-card-icon`) berwarna navy/cyan/emerald/amber/purple, judul h3 putih 1.05rem, dan subjudul p 0.84rem `#8191a6`.
  - **Subcard & Nested Pods (`.sg-subcard`)**:
    - Latar pitch dark `#090d16`, border `1px solid #1a2333`, sudut membulat 12px, padding 16px 18px. Diterapkan pada baris input pengaturan, dropzone file firmware, session stats monitor, dan drawer hex.
  - **Standarisasi Kontrol Form & Tombol**:
    - Input teks, angka, select, password, dan unit box diformat latar `#090d16` dan border `#1a2333` dengan fokus aksen sky blue `#38bdf8`.
    - Tombol simpan dan aksi primer diformat solid Royal Blue (`#2563eb`) dengan bayangan glow lembut dan ikon inline SVG.
- **4. Standarisasi Navigasi & Status Interaksi**:
  - **Sidebar Nav Active State**: Seluruh item menu aktif (`Overview`, `Live Test Monitor`, `Integration Docs`, `System Logs`, `Auth Settings`, `Device Settings`, `Update Firmware`) secara seragam menggunakan latar solid **Royal Blue (`#2563eb`)** dengan ikon dan teks putih tebal serta bayangan glow. Kelas khusus `nav-test-highlight` saat aktif juga diselaraskan ke Royal Blue.
  - **Navbar Brand & Tombol Logout**: Seluruh navbar di semua halaman menggunakan teks seragam **`Logout`** dengan ikon pintu keluar dan outline merah transparan (`rgba(244, 63, 94, 0.25)`), menggantikan varian teks lokal `Keluar`.
- **5. Hasil Verifikasi Multi-Halaman dengan Playwright Browser**:
  - Pengujian visual end-to-end dilakukan menggunakan browser headless Playwright dengan tangkapan layar penuh (*full page screenshots*):
    - `verify_settings_harmonized.png`: Slim Header Card dengan gear icon cyan, 4 bento card `#0d121f` tanpa border divider, subcard input `#090d16`, dan floating button "Simpan Pengaturan" Royal Blue yang sangat jelas terlihat.
    - `verify_log_harmonized.png`: Slim Header Card terminal icon, 4 kartu KPI jaringan `#0d121f`, tabel 8 kolom bersih, dan navigasi sidebar aktif Royal Blue.
    - `verify_log_drop_harmonized.png`: Tab Data Log Drop dengan 4 kartu KPI drop `#0d121f`, filter pill interaktif, dan tabel alasan drop.
    - `verify_test_harmonized.png`: Test Control Bar atas `#0d121f` dengan session stats `#090d16`, 8 kartu HUD Throughput `#0d121f`, dan container chart `#0d121f`.
    - `verify_firmware_harmonized.png`: Slim Header Card OTA, dua bento dropzone kartu firmware & filesystem `#0d121f` dengan tombol aksi Royal Blue.
    - `verify_auth_admin_harmonized.png`: Slim Header Card Admin purple, form kredensial `#0d121f` & `#090d16`, serta tombol floating simpan pembaruan.
    - `verify_docs_harmonized.png`: Hero header banner, code terminal tab, dan REST API list dalam kontainer Cybernetic Dark yang selaras 100%.

- **6. Penyatuan Struktur Sidebar Desktop 100% Identik (Zero-Discrepancy)**:
  - **Identifikasi Ketidakselarasan**: Ditemukan bahwa beberapa file (`test.html`, `log.html`, `firmware.html`) tidak memiliki atribut dimensi dan styling inline `style="min-width:22px;min-height:22px;width:22px;height:22px;flex-shrink:0;"` pada ikon SVG, item `/test` pada `log.html` kehilangan kelas `nav-test-highlight` (garis batas cyan saat inaktif), serta tautan `Integration Docs` (`/docs`) hilang pada `auth_teknisi.html`.
  - **Penyelarasan Total**: Seluruh 8 file HTML (`index.html`, `test.html`, `docs.html`, `log.html`, `auth_admin.html`, `auth_teknisi.html`, `settings.html`, `firmware.html`) kini menggunakan blok `<aside class="sidebar">` yang **100% seragam byte-for-byte**:
    1. Logo & User Avatar presisi seragam di seluruh halaman.
    2. Tepat 10 item navigasi berurutan identik: `Overview`, `Live Test Monitor`, `Integration Docs`, `System Logs`, `Auth Settings`, `Device Settings`, `Update Firmware`, `Reset Config`, `Reboot`, dan `Logout`.
    3. Seluruh SVG ikon menu memiliki dimensi `22x22px` dengan inline `min-width/height: 22px; flex-shrink: 0;` dan aturan penguat `app.css` (`width: 22px !important; height: 22px !important;`).
    4. Tombol `Live Test Monitor` pada seluruh halaman inaktif memiliki aksen kapsul cyan `.nav-test-highlight`, dan saat halaman `/test` aktif bertransisi menjadi Royal Blue `#2563eb` yang solid dan kontras.
    5. Menu `Integration Docs` (`/docs`) telah dipulihkan pada `auth_teknisi.html`.

---

## 77. Enterprise English UI Localization & Log Table Refinement (Standard Industri 2026)

### 77.1 Penghapusan Kolom 'Detail Informasi' & Perapihan Tabel Log
- **Masalah**: Kolom `Detail Informasi` pada tabel *Network & System Logs* redundan, memakan horizontal space, dan menyebabkan ketidakseimbangan visual pada layar dashboard. Terdapat pula duplikasi tag header `<th style="width: 150px;">Status / Event</th>` lama.
- **Penyelarasan & Eliminasi**:
  1. **Tabel Header (`data/log.html`)**: Kolom `Detail Informasi` dan header event duplikat telah dihapus total. Struktur tabel kini memiliki tepat **4 kolom presisi**:
     - `#` (Lebar 60px, ID urut)
     - `TIMESTAMP (LOCAL / UPTIME)` (Lebar 220px, monospace)
     - `SUBSYSTEM` (Lebar 160px, badge pill kategori: `SYSTEM`, `WIFI`, `MQTT`, `ETH`, `SOFTAP`)
     - `EVENT / STATUS` (Lebar fleksibel, status pill high-contrast)
  2. **Render DOM (`data/assets/js/log.js`)**:
     - Dihilangkan elemen `<td class="col-detail"><span class="detail-text">${escapeHtml(item.detail)}</span></td>` pada `renderNetTable()`.
     - `colspan` pada *empty state* dan loading indikator disesuaikan dari `5` ke `4`.
  3. **Tabel Dropped Packets (`dropLogTable`)**:
     - 8 kolom standar industri: `#`, `TIMESTAMP (LOCAL / UPTIME)`, `MAC ADDRESS`, `FRAME TYPE`, `IDENTIFIER / PAYLOAD`, `RSSI`, `DROP REASON`, `RAW 42B`.

### 77.2 Standarisasi Bahasa Inggris Engineer Enterprise (Padat, Lugas, Presisi)
Seluruh 9 halaman web (`index.html`, `test.html`, `settings.html`, `log.html`, `auth_admin.html`, `auth_teknisi.html`, `firmware.html`, `docs.html`, `login.html`) serta berkas JavaScript pendukung (`app.js`, `test.js`, `log.js`, `settings.js`, `firmware.js`) telah dimigrasikan dari campuran bahasa Indonesia ke **Bahasa Inggris Standar Engineer Enterprise**:

1. **Atribut Root HTML**:
   - Seluruh tag `<html>` diperbarui menjadi `<html lang="en">`.
2. **Kamus Terminologi Standar Enterprise**:
   - `Ringkasan Gateway` -> `Gateway Overview`
   - `Pengaturan Perangkat` -> `Device Settings` / `Device Configuration`
   - `Pengujian Langsung` -> `Live Test Monitor` / `BLE Gateway Live Diagnostics`
   - `Log Jaringan & Sistem` -> `Network & System Events`
   - `Riwayat Paket Drop` -> `Packet Drop Telemetry`
   - `Kredensial Admin` -> `Administrator Credentials (Full Access)`
   - `Pengaturan Teknisi` -> `Field Technician Setup`
   - `Pembaruan Firmware & LittleFS` -> `Firmware & LittleFS OTA`
   - `Dokumentasi API` -> `Integration Docs`
   - `Simpan Pengaturan` / `Simpan Pembaruan` -> `Save Configuration`
   - `Reboot Sekarang` -> `Reboot Now`
   - `Batalkan` -> `Discard Changes`
   - `Mode Pemindaian Radio` -> `Radio Scan Mode (nRF52840 Co-Processor)`
   - `Batas Minimum RSSI` -> `Minimum RSSI Threshold`
   - `Maksimal Beacon per Batch` -> `Max Beacons per Batch`
   - `Batas Waktu Buffer Flush` -> `Buffer Flush Interval (ms)`
   - `Masa Kadaluarsa Antrean (TTL)` -> `Queue Retention Timeout (TTL)`
   - `Server Waktu (NTP)` -> `Network Time Protocol (NTP)`
   - `Jadwal Reboot Otomatis Harian` -> `Daily Scheduled Reboot`
   - `Reboot saat Terputus (Watchdog)` -> `Connection Loss Watchdog`
   - `Unduh CSV` -> `Export CSV`
   - `Hapus Log` -> `Clear Logs`
   - `Raw Hex Berhasil Disalin!` -> `Raw hex payload copied to clipboard!`
3. **Modal Dialog, Toasts, & SweetAlert Alerts**:
   - Seluruh konfirmasi (`Purge Network & System Logs?`, `Confirm Gateway Reboot`, `Apply Configuration Changes`, `Confirm Sign Out`, `Confirm Firmware Flash`) kini menggunakan terminologi teknis berbahasa Inggris yang padat, formal, dan tidak membingungkan pengguna internasional/enterprise.

### 77.3 Verifikasi Kompilasi & Snapshot Browser Playwright
- **LittleFS**: Dikompilasi via `pio run -t buildfs` (58.02s) mengemas seluruh HTML terjemahan dan aset `.gz` (level 9).
- **Firmware**: Dikompilasi via `pio run` (103.29s, RAM: 35.3%, Flash: 90.5%).
- **Factory Binaries**: Disalin ke `factory_bins/flash_v1.3.0/board_v1.0.0/firmware.bin` & `littlefs.bin`.
- **Browser Playwright Screenshots**:
  - `verify_log_table.png`: Menampilkan 4 kolom tabel *Network & System Event History* tanpa kolom 'Detail Informasi', pill status bersih, dan tombol `Clear Logs`/`Export CSV`.
  - `verify_log_drop_tab.png`: Menampilkan tab *Packet Drop Telemetry* 8 kolom dengan kartu KPI dan filter segmented.
  - `verify_settings_bottom_polished.png`: Menampilkan kartu *MQTT Batching & Ingress Queue*, batas waktu `ms`, dan kontrol *Daily Scheduled Reboot*.
  - `verify_test_polished.png`: Menampilkan *BLE Gateway Live Diagnostics*, 8 kartu HUD throughput, dan tabel *FIFO Stream*.


## 78. Pemulihan Kolom Event Details, Penghapusan Total Kustomisasi Telemetri, & Deep Localization Bahasa Inggris (2026)

### 78.1 Pemulihan Kolom 'Event Details' pada Tabel Network & System Logs
- Kolom `Event Details` pada tabel Network & System Logs (`data/log.html` & `data/assets/js/log.js`) dipulihkan secara penuh dengan struktur 5 kolom terstandarisasi:
  1. `#` (No)
  2. `Timestamp (Local / Uptime)`
  3. `Subsystem`
  4. `Event / Status`
  5. `Event Details`
- Format detail event menyajikan informasi diagnostik jaringan mendalam secara lugas dan padat (misal: DNS error codes, IP address assignment, handshake state changes, link down notifications).

### 78.2 Penghapusan Fitur Kustomisasi Field Telemetri MQTT
- Fitur "Kustomisasi Field Telemetri MQTT" (bitmask switches/checkboxes) di `data/auth_admin.html` telah dihapus secara total (CSS rules, HTML bento card block, dan JavaScript synchronization function).
- Payload telemetri MQTT kini beroperasi secara konsisten menggunakan skema standar yang terprediksi tanpa overhead konfigurasi bitmask dinamis di sisi UI.

### 78.3 Deep Localization & Audit Eliminasi Bahasa Indonesia
- Pemindaian menyeluruh dilakukan di seluruh file HTML, JavaScript, dan backend C++:
  - `data/auth_admin.html`: Kredensial administrator & teknisi, penugasan IP, SoftAP darurat, broker telemetri MQTT, MQTTS encryption, button labels, dan SweetAlert submission prompt (`'Credentials and network configuration updated successfully.'`).
  - `data/auth_teknisi.html`: Form labels, IP assignment mode, Ethernet settings, scanner modal labels (`Rescan`).
  - `data/settings.html`: SweetAlert submission message (`'Gateway settings updated successfully.'`), Save Changes button.
  - `data/login.html`: Alert banner, SoftAP shutdown confirmation, security timeout warnings.
  - `data/test.html` & `test.js`: Tombol trash/delete (`Delete`), toast messages (`Device {mac} removed`, `Failed to copy to clipboard`), unload warnings (`Live beacon monitor is active...`).
  - `data/assets/js/app.js`: Factory reset modal prompts, AP shutdown dialogs, network discard prompts & status explanations.
  - `data/assets/js/firmware.js`: Flash write progress strings, LittleFS validation errors, incompatible board warnings, upload failure alerts.
  - `data/assets/js/settings.js`: NTP diagnostic test feedback badges and status descriptions.
  - `data/docs.html`: Seluruh contoh kode Python, JavaScript, Rust, dan cURL diterjemahkan ke standar Enterprise English.
- Backend C++ (`src/web/web_routes.cpp`, `src/network/mqtt_service.cpp`, `src/core/drop_log_service.cpp`, `src/network/net_manager.cpp`, `src/network/drivers/EthernetW5500.cpp`, `src/web/web_template.cpp`): Seluruh pesan respon JSON REST, drop reasons, dan string error MQTT/WiFi/Ethernet telah diterjemahkan ke Bahasa Inggris berstandar Enterprise.


## 79. Audit Menyeluruh & Verifikasi Playwright Halaman Firmware (2026)

### 79.1 Eliminasi Teks Bahasa Indonesia pada Halaman Firmware
Pemeriksaan mendalam pada `data/firmware.html` dan `data/assets/js/firmware.js` berhasil menemukan dan menstandardisasi seluruh teks yang sebelumnya masih menggunakan Bahasa Indonesia:
1. **Kartu Bento & Dropzone (`firmware.html`)**:
   - `Update Firmware Program (.bin)` $\rightarrow$ `Firmware Application Binary (.bin)`
   - `Update Filesystem Web Dashboard (LittleFS .bin)` $\rightarrow$ `Web Filesystem Image (LittleFS .bin)`
   - `Klik atau seret file firmware.bin ke sini` $\rightarrow$ `Click or drag firmware.bin here`
   - `Klik atau seret file littlefs.bin ke sini` $\rightarrow$ `Click or drag littlefs.bin here`
   - `Perbarui file antarmuka web dan skrip tanpa mengubah firmware program` $\rightarrow$ `Update web dashboard assets and scripts independently of firmware`
   - Tombol ganti berkas: `title="Ganti File"` $\rightarrow$ `title="Change File"`
2. **Komponen Banner Hotspot SoftAP (Lintas Seluruh 6 Halaman & `app.js`)**:
   - Tombol matikan AP darurat: `<span>Nonaktifkan AP</span>` $\rightarrow$ `<span>Disable AP</span>` pada `firmware.html`, `auth_admin.html`, `auth_teknisi.html`, `login.html`, `settings.html`, `test.html`, dan generator dinamis `app.js`.
3. **Modal Pop-Up OTA Progress & Pre-Flight Validation (`firmware.js`)**:
   - Judul modal: `Proses Update Firmware` $\rightarrow$ `Firmware Flashing Progress`
   - Tombol aksi: `Tutup & Coba Lagi` $\rightarrow$ `Close & Retry`, `Tutup` $\rightarrow$ `Close`
   - Validasi berkas pra-unggah:
     - `Bukan Berkas LittleFS!` $\rightarrow$ `Invalid LittleFS Binary!`
     - Pesan peringatan pemilihan berkas: `Warning: Please select a Firmware (.bin) file first!`
     - Dialog konfirmasi flash firmware: `Ensure gateway power supply is stable during OTA flashing. Proceed with upload?` $\rightarrow$ `Proceed with Update`
     - Dialog konfirmasi flash LittleFS: `Filesystem update will refresh web dashboard assets. Proceed with upload?` $\rightarrow$ `Proceed with Update`
     - Status transmisi: `Connecting to gateway and initiating upload stream...`, `Uploading binary image ({percent}%)...`

### 79.2 Verifikasi Interaktif Browser Playwright
Verifikasi visual dan pengujian fungsional interaktif dilakukan secara langsung:
- `verify_firmware_top_en.png`: Menampilkan header card, status OTA Recovery, dan area dropzone firmware dalam Bahasa Inggris enterprise.
- `verify_firmware_bottom_en.png`: Menampilkan area dropzone LittleFS dan tombol upload yang padat dan jelas.
- `verify_firmware_modal_english.png`: Menampilkan pengujian interaktif klik tombol upload tanpa file, memicu modal SweetAlert `Warning: Please select a Firmware (.bin) file first!` secara sempurna dalam Bahasa Inggris.


## 80. Rebranding Corporate Cubeacon, Generalisasi Filesystem, dan Eliminasi Kata Kunci Hardware (ESP32, NRF, W5500) (2026)

### 80.1 Perubahan Entitas Perusahaan ke Cubeacon
- Seluruh referensi entitas perusahaan lama (`IOTARU` / `Eyro`) digantikan menjadi **Cubeacon**:
  - Judul Tab Halaman: `<title>Cubeacon %model% - ...</title>` pada seluruh 9 berkas HTML (`index.html`, `test.html`, `log.html`, `settings.html`, `auth_admin.html`, `auth_teknisi.html`, `firmware.html`, `login.html`, `docs.html`).
  - Gambar Logo Sidebar & Navbar: Atribut `alt="Cubeacon"`.
  - Footer Hak Cipta: `&copy; Cubeacon Gateway (%model%).`
  - Informasi Manufacturer pada Overview: `Cubeacon`.
  - Integrasi Docs & mDNS Service Discovery: `Cubeacon Web Console`, `Cubeacon %s Gateway`, dan `Cubeacon Telemetry`.

### 80.2 Generalisasi Nomenklatur Filesystem (Menggantikan LittleFS)
- Seluruh penyebutan nama teknologi spesifik `LittleFS` pada antarmuka web, kartu bento, dropzone, peringatan SweetAlert, dan log viewer digantikan dengan istilah fungsional industri **Filesystem**:
  - `Update Firmware & LittleFS` $\rightarrow$ `Update Firmware & Filesystem`
  - `Over-The-Air (OTA) Dual-Partition Flasher: APP & LittleFS` $\rightarrow$ `Over-The-Air (OTA) Dual-Partition Flasher: Firmware & Filesystem`
  - `Web Filesystem Image (LittleFS .bin)` $\rightarrow$ `Web Filesystem Image (.bin)`
  - Dropzone: `Select or Drag Filesystem Binary (.bin)`, `Click or drag filesystem.bin here`
  - Storage Audit: `Dual-Engine Auditor: Filesystem Network Trail & BLE Drop Ring Buffer`
  - Modal SweetAlert: `Invalid Filesystem Binary!`, `All network and system event records will be permanently erased from Filesystem.`

### 80.3 Eliminasi Total Kata Kunci Hardware (ESP32 & NRF)
- Nomenklatur internal mikrokontroler dihapus sepenuhnya dari teks yang tampil pada antarmuka pengguna:
  - `ESP32 Gateway` $\rightarrow$ `Gateway` / `Cubeacon Gateway`
  - `ESP32 Firmware` $\rightarrow$ `Gateway Firmware` / `System Firmware`
  - `ESP32 Boot Counter` $\rightarrow$ `System Boot Counter`
  - `ESP32 application binary` $\rightarrow$ `Application binary`
  - `ESP32 Partition Table (partitions.bin)` $\rightarrow$ `Partition Table (partitions.bin)`
  - `Restart ESP32 microcontroller system` $\rightarrow$ `Restart gateway system`
  - `nRF52840 radio scan mode...` $\rightarrow$ `BLE receiver radio scan mode...`
  - `Radio Scan Mode (nRF52840 Co-Processor)` $\rightarrow$ `Radio Scan Mode (BLE Receiver)`

### 80.4 Abstraksi Pengontrol Ethernet (Menggantikan W5500)
- Nomenklatur hardware controller Ethernet SPI `W5500` diubah menjadi terminologi antarmuka jaringan standar:
  - `Ethernet W5500` $\rightarrow$ `Ethernet LAN`
  - Tooltip & Link Text: `Ethernet LAN {spd}M {dpx}`
  - Status Hardware: `Hardware Disconnected` (menggantikan `Modul SPI Disconnect`)

### 80.5 Verifikasi Visual Playwright
- Tangkapan layar bukti browser tersimpan:
  - `verify_cubeacon_firmware.png`: Menampilkan judul *Update Firmware & Filesystem*, kartu *Web Filesystem Image (.bin)*, dan *Gateway Firmware*.
  - `verify_cubeacon_log.png`: Menampilkan kartu *Ethernet LAN*, *System Boot Counter*, dan *Filesystem Network Trail*.
  - `verify_cubeacon_settings.png`: Menampilkan *Radio Scan Mode (BLE Receiver)*.
  - `verify_cubeacon_overview.png`: Menampilkan *Cubeacon AR-27 - Gateway Overview* dan *Manufacturer: Cubeacon*.


---

## 81. Arsitektur Zero-FOUC & Critical Rendering Path (November 2026)

### 81.1 Latar Belakang & Identifikasi Kerentanan FOUC
Pada arsitektur sistem tertanam (embedded gateway) yang menyajikan berkas antarmuka web melalui filesystem lokal dengan konektivitas Wi-Fi SoftAP atau Ethernet pada laju pengiriman paket telemetri tinggi (1.200 data/detik), latensi transmisi berkas statis dapat menimbulkan fenomena **Flash of Unstyled Content (FOUC)**:
1. **White Canvas Flash (Kanvas Putih Pra-Render):** Lembar gaya utama `app.css` dimuat secara eksternal. Sebelum berkas CSS diterima browser, mesin render merender latar belakang default agen pengguna (`#ffffff`). Pada koneksi terhambat, transisi dari putih ke palet cybernetic gelap (`#070c14`) memicu kedipan putih yang menyilaukan.
2. **Unsuppressed Banner Flash:** Elemen spanduk `<div id="pendingRebootBanner" class="pending-reboot-banner">` sebelumnya tidak memiliki atribut inline `style="display:none;"`, sehingga sempat diparsing dan muncul sekejap sebelum runtime JavaScript menyembunyikannya (memicu *Cumulative Layout Shift*).
3. **Parser-Blocking Script di `<head>`:** File `<script src="assets/js/swal-lite.js"></script>` dimuat tanpa atribut `defer`, menghentikan parser HTML selama proses unduh dan eksekusi skrip.
4. **Duplicate Stylesheet Link:** Berkas `log.html` memuat lembar gaya `app.css` dua kali berturut-turut.
5. **Inkonsistensi Cache Buster:** Versi stylesheet beragam (`?v=1.0.7`, `?v=%ui_version%`, dan beberapa tanpa query parameter).

### 81.2 Spesifikasi Critical Pre-paint Inlined CSS
Untuk memastikan browser merender kanvas gelap sejak Frame 0 (sebelum ada satu pun request HTTP untuk `app.css` yang selesai), blok styling inline minimal disuntikkan ke dalam `<head>` pada seluruh 9 berkas antarmuka HTML (`index.html`, `settings.html`, `auth_admin.html`, `auth_teknisi.html`, `log.html`, `firmware.html`, `test.html`, `docs.html`, `login.html`):

```html
<style>
  /* Critical Pre-paint CSS: Eliminates White Canvas Flash */
  html, body {
    background-color: #070c14 !important;
    color: #f8fafc;
    margin: 0;
    padding: 0;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Inter", sans-serif;
  }
  /* Instant Banner Suppression before external CSS loads */
  .pending-reboot-banner,
  .ap-banner-card,
  .wifi-modal-backdrop,
  .session-expired-banner {
    display: none !important;
  }
</style>
```

### 81.3 Standarisasi Atribut Banners & Parser-Non-Blocking
1. **Atribut Inline Guard:** Semua elemen banner kondisional (`pendingRebootBanner`, `apModeBanner`, `wifiScanModal`, `otaModal`, `sessionExpiredBanner`) kini dideklarasikan dengan `style="display:none;"` langsung di markup HTML.
2. **Non-Blocking Scripting:** Pemuatan `swal-lite.js` dioptimasi menggunakan atribut `defer` dan token versi dinamis:
   ```html
   <script src="assets/js/swal-lite.js?v=%firmware_version%" defer></script>
   ```
3. **Harmonisasi Cache Buster Global:** Seluruh tautan stylesheet `app.css` diseragamkan ke token firmware:
   ```html
   <link rel="stylesheet" href="assets/css/app.css?v=%firmware_version%">
   ```

---

## 82. Single-Row Streamlined MQTT Telemetry Layout & First-Page View Optimization (November 2026)

### 82.1 Optimisasi Tampilan Layar Penuh (*First-Page View Full Visibility*)
Untuk menjamin seluruh komponen dashboard Overview (Banner status, Panel Identitas Sistem, Panel Status Jaringan, antarmuka fisik Wi-Fi & Ethernet, serta Footer sistem) terlihat utuh di layar monitor/laptop (resolusi standar 1366x768 hingga 1440x900+) tanpa perlu *scrolling* (*zero-scroll viewport*), kartu MQTT Telemetry direkayasa menjadi **satu baris ringkas (*single-row streamlined layout*)**:
- **Tinggi Kartu:** Berkurang drastis dari 211 px menjadi **122 px** (penghematan ~89 px vertikal).
- **Keseimbangan Bento Grid:** Tinggi panel kiri (`.sg-panel-left`) dan panel kanan (`.sg-panel-right`) kini identik presisi pada **474 px** di desktop 1440x900.

### 82.2 Grid 5-Kolom Horizontal & Dynamic Text Ellipsis
Kelima parameter uplink MQTT ditata berjajar horizontal dalam 1 baris menggunakan CSS Grid yang responsif:
- **Broker Host:** Menggunakan `overflow: hidden; text-overflow: ellipsis; white-space: nowrap;` sehingga jika domain/IP broker sangat panjang (misal cluster broker cloud), teks otomatis dipotong rapi dengan tanda titik elipsis `...` di ujungnya tanpa merusak tata letak tile lain.
- **Tooltip & Salin Utuh:** Atribut `title` otomatis membawa hostname lengkap, dan tombol salin interaktif menyalin hostname asli tanpa terpotong.
- **Kolom Metrik Simetris:**
  1. `Broker Host`: Minmax(130px, 1.9fr) + Tombol Salin
  2. `Port`: Minmax(55px, 0.75fr)
  3. `Security (TLS)`: Minmax(95px, 1.15fr) dengan capsule badge
  4. `Delivery QoS`: Minmax(75px, 0.9fr) dengan capsule badge
  5. `Keep Alive`: Minmax(65px, 0.75fr)

```css
/* 5-Column Single-Row Grid on Desktop, Responsive 2x2 on Mobile */
.sg-mqtt-data-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 8px;
  width: 100%;
  box-sizing: border-box;
}
.sg-mqtt-col-host {
  grid-column: 1 / -1;
}
@media (min-width: 640px) {
  .sg-mqtt-data-grid {
    grid-template-columns: minmax(130px, 1.9fr) minmax(55px, 0.75fr) minmax(95px, 1.15fr) minmax(75px, 0.9fr) minmax(65px, 0.75fr);
    gap: 8px;
    align-items: stretch;
  }
  .sg-mqtt-col-host {
    grid-column: auto;
  }
}
.sg-mqtt-col {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  justify-content: center;
  gap: 3px;
  min-width: 0;
  background: #080c14;
  border: 1px solid #1a2333;
  border-radius: 8px;
  padding: 7px 10px;
  box-sizing: border-box;
}
.sg-mqtt-col-label {
  font-size: 0.68rem;
  font-weight: 600;
  color: #8191a6;
  letter-spacing: 0.03em;
  text-transform: uppercase;
  white-space: nowrap;
  line-height: 1.2;
}
.sg-mqtt-col-val {
  display: flex;
  align-items: center;
  gap: 6px;
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
  font-size: 0.88rem;
  font-weight: 700;
  color: #ffffff;
  min-width: 0;
  width: 100%;
  line-height: 1.2;
}
.sg-mqtt-col-host .sg-mqtt-col-val {
  justify-content: space-between;
}
.sg-mqtt-host-val {
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  min-width: 0;
  flex: 1 1 auto;
}
```

### 82.3 Responsivitas Mobile (< 640px)
Pada perangkat mobile, kartu tetap tertata rapi tanpa scrolling horizontal:
- Baris 1: Broker Host membentang 100% penuh dengan elipsis otomatis jika panjang.
- Baris 2 & 3: Port, TLS, QoS, dan Keep Alive tersusun dalam grid 2x2 yang simetris.

### 82.4 Bukti Verifikasi Playwright
- `verify_first_page_view_1440x900.png`: Menampilkan 100% halaman overview (termasuk footer) terlihat di layar tanpa *scrolling* pada monitor 1440x900.
- `verify_first_page_view_1366x768.png`: Menampilkan kelengkapan layout pada laptop standar 1366x768 dengan elipsis otomatis `socket.cubeacon...` yang aktif.
- `verify_mqtt_single_row_card.png`: Tampilan fokus kartu MQTT Telemetry dalam satu baris datar yang ramping dan profesional.
- `verify_network_panel_single_row.png`: Tampilan panel kanan menyeluruh memperlihatkan integrasi dengan Wi-Fi dan Ethernet.
- `verify_mqtt_mobile_responsive.png`: Tampilan mobile (375x812) dengan responsivitas 2x2 di bawah Broker Host.
---

## 83. Modal Pop-up Visibility & Stream Template Buffer Fix (November 2026)

### 83.1 Resolusi Error Browser `<path>` Attribute dan Pemotongan Metrik MQTT
- **Akar Masalah:** Pada fungsi rendering template ESP32 (`stream_template_file` di `src/web/web_template.cpp`), alokasi buffer sementara `val` dibatasi hanya `256` byte. Nilai string pengganti `%mqtt_tls_badge%` yang memuat elemen SVG kompleks berukuran ~304 byte terpotong tepat pada teks `<path d="`. Akibatnya, browser menganggap sisa markup HTML sesudahnya (termasuk tag penutup dan kolom *Delivery QoS*) sebagai isi dari atribut path SVG, menghasilkan galat konsol: `Error: <path> attribute d: Expected moveto path command ('M' or 'm')` dan menghilangkan kolom *Delivery QoS* pada layar pengguna.
- **Solusi Rekayasa:** Ukuran `val` dalam `StreamBuffer` ditingkatkan dari 256 byte menjadi **1024 byte**, serta `max_len` pada `get_placeholder_value` disinkronkan ke `sizeof(sbuf->val)`. Hal ini menjamin seluruh badge SVG dan teks pengganti berukuran panjang tertransmisi 100% utuh tanpa terpotong.

### 83.2 Resolusi Modal Progress Bar Update Firmware & Filesystem
- **Akar Masalah:** Aturan CSS pre-paint Zero-FOUC di `<head>` mendefinisikan `.wifi-modal-backdrop { display: none !important; }`. Karena memiliki deklarasi `!important`, instruksi JavaScript `modal.style.display = "flex"` kalah dalam hierarki spesifisitas CSS, sehingga jendela pop-up progress bar (`#otaModal`) tetap tersembunyi selama proses upload berlangsung.
- **Solusi Rekayasa:** 
  1. Deklarasi `!important` dihapus dari aturan pre-paint di seluruh 9 file HTML (`display: none;`). Karena elemen modal secara bawaan telah memiliki inline style `style="display:none;"`, penekanan FOUC tetap 100% efektif tanpa mengunci properti CSS.
  2. Fungsi `startUploadExecution` di `firmware.js` diperkuat menggunakan `modal.style.setProperty("display", "flex", "important")` saat memulai flashing, dan `setProperty("display", "none", "important")` saat ditutup, menjamin modal progress bar muncul secara andal pada setiap update firmware dan filesystem.


---

## 84. Dynamic Gateway MQTT Publish Topic & Heartbeat Interval Bento Card (November 2026)

### 84.1 Resolusi Tampilan Gateway MQTT Publish Topic Pill
- **Akar Masalah:** Pada halaman `settings.html`, prefix topik MQTT di dalam pill *Gateway MQTT Publish Topic* (`.subtopic-display-pill`) sebelumnya di-*hardcode* sebagai `<span class="subtopic-prefix">beacon/</span>`. Akibatnya, display topik menampilkan `beacon/<device_id>` padahal runtime gateway ESP32 mempublikasikan data telemetri BLE ke topik terstandarisasi `{dev_prefix}/data/{device_id}` (contoh: `ar/data/da353220` untuk model `AR-27`).
- **Solusi Rekayasa:** 
  1. Di `data/settings.html`, `<span class="subtopic-prefix">beacon/</span>` diganti dengan `<span class="subtopic-prefix">%mqtt_topic_prefix%</span>`.
  2. Di `src/web/web_template.cpp`, ditambahkan resolver `%mqtt_topic_prefix%` dan `%mqtt_topic_data%`. Kode mengekstrak model prefix secara dinamis dari `g_config.device_model` (mengambil karakter alfabet sebelum tanda `-`, diubah ke huruf kecil, e.g. `AR-27` $\rightarrow$ `ar`), sehingga prefix di-*render* secara dinamis sebagai `ar/data/` dan pill menampilkan `ar/data/da353220` secara sempurna tanpa *hardcode*.

### 84.2 Integrasi Konfigurasi Heartbeat Broadcast Interval (`hb_interval`)
- **Struktur Desain Bento Grid Simetris:**
  - Sebelumnya, Bento Card 2 (*MQTT Batching & Ingress Queue*) memiliki susunan:
    - Baris 1: *Gateway MQTT Publish Topic* (`.item.full`)
    - Baris 2: *Max Beacons per Batch* (`.item`) & *Buffer Flush Interval* (`.item`)
    - Baris 3: *Queue Retention Timeout (TTL)* (`.item.full` berdiri sendiri)
  - Desain diperbarui dengan menyeimbangkan Baris 3 menjadi layout dua kolom yang simetris:
    - Kolom Kiri: *Queue Retention Timeout (TTL)* (`.item`, unit: `ms`, min: 1000, max: 30000)
    - Kolom Kanan: *Heartbeat Broadcast Interval* (`.item`, unit: `s`, min: 1, max: 300, default: 5s)
- **Arsitektur & Manajemen Memori RAM:**
  - NVS Flash: Kunci `hb_interval` (`u32`, default `5`) ditambahkan pada namespace `gateway` di `nvs_config.csv`. Binary `nvs.bin` berukuran 24 KB dihasilkan ulang via `generate_nvs.py`.
  - Zero Dynamic Heap Overhead: Variabel `int hb_interval` dialokasikan secara statis pada `struct AppConfig` (hanya 4 byte di static BSS RAM). Tidak ada alokasi heap dinamis (`malloc`/`new`), menjaga Free Heap ESP32 tetap stabil pada **~46 KB**.
  - Runtime Timer: `mqtt_service.cpp` menggunakan timer non-blocking `(g_config.hb_interval * 1000)` ms dan mempublikasikan payload `deviceId` ke topik dinamis `{dev_prefix}/hb` (misalnya `ar/hb`).
  - Web API Clamping: `api_config_settings_handler` di `src/web/web_routes.cpp` memvalidasi dan membatasi input form pada rentang aman 1 hingga 300 detik sebelum disimpan ke Flash NVS.


---

## 85. Harmonized 2-Column Layout & Inline Toggles for Settings Page (November 2026)

### 85.1 Arsitektur UX 2-Kolom & Inline Toggles
Halaman `settings.html` diperbarui dengan tata letak modern 2-kolom yang ergonomis, mempertahankan tema gelap (*dark navy / industrial slate*) bawaan gateway:
1. **Penyatuan Switch Toggle (Inline Switch Controls):**
   - **Target UUID Filter:** Switch toggle iOS (`#filterUuidToggle`) ditempatkan berdampingan langsung pada satu baris yang sama dengan text input monospace (`#filter_uuid`) dan tombol paste clipboard (`#btnPasteUuid`).
   - **Target Major Filter:** Switch toggle iOS (`#filterMajorToggle`) ditempatkan pada satu baris yang sama dengan numeric input (`#filter_major`). Saat switch posisi OFF, baris otomatis berubah redup (`.row.off` dengan opacity 0.35) dan input dinonaktifkan.
   - **Daily Scheduled Reboot:** Switch toggle iOS (`#restartScheduleToggle`) diletakkan berdampingan dengan native time input (`#restart_schedule`), mengeliminasi baris terpisah yang tidak perlu.
2. **Compact 2x2 Field Grid (`.fgrid` / `.cell`):**
   - Pada kartu *MQTT Batching & Ingress Queue*, 4 parameter dikelompokkan ke dalam grid kuadran 2x2:
     - Kuadran 1: *Max beacons per batch* (10 – 200 per payload, kontrol stepper `[-] [50] [+]`)
     - Kuadran 2: *Buffer flush interval* (50 – 2000 ms, unit box addon `ms`)
     - Kuadran 3: *Queue retention timeout (TTL)* (1000 – 30000 ms, unit box addon `ms`)
     - Kuadran 4: *Heartbeat interval* (1 – 300 seconds, unit box addon `sec`)
   - Pada kartu *Reboot Schedule & System Watchdog*, 2 parameter disusun dalam grid 2-kolom:
     - Kolom Kiri: *Daily scheduled reboot* dengan inline switch
     - Kolom Kanan: *Connection-loss watchdog* dengan unit box addon `min`
3. **Topic Display Banner:**
   - Topik publikasi MQTT ditampilkan sebagai read-only dashed pill `.topic` di baris atas kartu MQTT dengan template resolver dinamis: `ar/data/da353220`.

### 85.2 Preservasi Tema Dark Navy & Token CSS
- Seluruh komponen memanfaatkan token desain:
  - Background Utama: `#070c14`
  - Permukaan Kartu (`.card`): `#0d121f` dengan border `1px solid #1a2333` dan box-shadow `0 6px 24px rgba(0,0,0,0.35)`
  - Permukaan Input: `#090d16` dengan border `#1a2333` dan fokus glow `#38bdf8`
  - Aksen Warna Ikon: Cyan `#38bdf8` (BLE), Emerald `#34d399` (MQTT), Amber `#fbbf24` (NTP), Slate `#94a3b8` (System)
- Berkas CSS `app.css` dan versi terkompresi `app.css.gz` disinkronkan untuk konsumsi LittleFS ESP32.

### 85.3 Bukti Verifikasi Playwright Browser
- `verify_settings_harmonized_desktop_top.png`: Tampilan utuh viewport desktop (1440x900) dengan sidebar, header, dan kartu yang harmonis.
- `verify_settings_ble_crop.png`: Tampilan fokus kartu BLE Radio Scanner memperlihatkan inline switch UUID & Major, paste button, badge status, dan merged minus RSSI slider.
- `verify_settings_mqtt_crop.png`: Tampilan kartu MQTT Batching dengan publish topic read-only dan grid kuadran 2x2 simetris.
- `verify_settings_ntp_crop.png`: Tampilan kartu NTP Time Protocol dengan tombol *Test probe* dan badge *Ready*.
- `verify_settings_sys_crop.png`: Tampilan kartu System Watchdog dengan inline switch reboot harian dan input menit watchdog.
- `verify_settings_harmonized_mobile.png`: Tampilan mobile (375x812) dengan responsivitas 1-kolom dan mobile bottom navigation yang rapi.

### 85.4 Resolusi Tampilan Header & Tipografi Monospace pada Mobile (375x812)
- **Akar Masalah:**
  1. Pada header halaman, sisa kartu lama `.sg-header-card` memuat elemen badge `.sg-badge-pill` (*HARDWARE SYNC*) yang bertumpuk (*overlapping*) tepat di atas judul teks `DEVICE SETTINGS (%model%)` serta memotong subtitle menjadi `Gate...` pada layar sempit (< 640px).
  2. Input UUID 32-karakter monospace (`.pw input`) mengalami pemotongan teks 2 karakter terakhir karena padding dan lebar toggle switch inline pada viewport 375px.
- **Solusi Rekayasa:**
  1. Kartu lama `.sg-header-card` diganti dengan komponen `.head` terstandarisasi yang fleksibel (`display: flex; flex-wrap: wrap;`), memuat judul `Device Settings`, deskripsi bersih, dan metadata tags (`Model %model%`, `FW %firmware_version%`).
  2. Pada viewport mobile (< 640px), `.pw input` disesuaikan ke `font-size: 11px !important; letter-spacing: -0.3px !important;` dengan padding-right 38px, menjamin **seluruh 32 karakter hexadecimal (`FDA50693A4E24FB1AFCFC6EB07647825`) terlihat 100% utuh tanpa terpotong**.
  3. Komponen `.cell` pada grid field bento dikonfigurasi menjadi susunan vertikal penuh (`display: flex; flex-direction: column; align-items: stretch;`) dengan lebar kontrol 100%, menghasilkan sentuhan tombol stepper (`[-] 50 [+]`) dan addon unit box yang sangat nyaman diakses pada perangkat layar sentuh.
- **Bukti Verifikasi:** `verify_settings_harmonized_mobile_perfect.png` mengonfirmasi resolusi visual bebas overlap dan keterbacaan data 100% sempurna.

### 85.5 Resolusi Penataan Header Mobile & Restorasi Ukuran Standar Font UUID (375x812)
- **1. Analisis Masalah Tampilan Header Mobile:**
  - Sebelumnya pada viewport mobile (< 768px), elemen logo Cubeacon (`.mobile-logo-link`) dan teks judul navigasi (`.navbar-brand` 'Device Settings') sama-sama ditampilkan berdampingan di dalam `.navbar-left`. Hal ini menimbulkan dua anomali kritis:
    1. Teks judul navigasi menempel tidak wajar di sebelah kanan logo (`cubeacon Device Settings`), menciptakan tata letak yang tidak terstandarisasi dan menduplikasi judul utama halaman `Device Settings` (`.head h1`) yang berada tepat di bawahnya.
    2. Akumulasi lebar logo (153px), gap, teks judul (100px), dan tombol logout (90px) dengan padding 24px melampaui lebar layar mobile (360px - 375px), mendorong tombol Logout (`.mobile-logout-btn`) keluar dari batas bingkai layar (*out of frame / clipped*) dan memicu horizontal scrolling pada dokumen (`bodyScrollWidth = 382px`).
  - Selain itu, ukuran font input target UUID (`.pw input`) sempat diperkecil secara artifisial ke `11px` demi memuat seluruh 32 karakter ke dalam satu baris sempit, padahal pemotongan teks (*clipping*) di dalam kotak input adalah perilaku yang disengaja (*intentional design*).

- **2. Solusi Rekayasa CSS Responsif (`app.css` & `app.css.gz`):**
  - **Standardisasi Mobile Navbar (< 768px):**
    - `.navbar`: Tinggi disesuaikan ke standar mobile `58px` dengan padding seimbang `padding: 0 16px !important; box-sizing: border-box !important; width: 100% !important; max-width: 100vw !important;`.
    - `.mobile-logo-img`: Tinggi logo disesuaikan ke `26px` (lebar ~125px) agar proporsional pada header mobile.
    - `.navbar-brand`: Diberi aturan `display: none !important;` pada mobile (< 768px). Judul halaman kini ditampilkan secara tunggal, bersih, dan elegan melalui header halaman `.head` (`<h1>Device Settings</h1>`), mengeliminasi teks ganda dan tabrakan visual.
    - `.mobile-logout-btn`: Diberi aturan `flex-shrink: 0 !important; margin-left: auto !important; padding: 6px 12px !important;`, menjamin posisi tombol logout 100% berada di dalam bingkai layar dengan margin kanan nyaman 16px (`right: 344px` pada kontainer 360px).
    - Menghilangkan *horizontal scrolling* seluruhnya (`bodyScrollWidth = 360px === docClientWidth = 360px`).
  - **Restorasi Tipografi Monospace UUID (`13px`):**
    - Menghapus aturan perkecilan font 11px pada media query mobile.
    - Mengembalikan ukuran font input UUID (`.pw input` dan `input.mono`) ke standar `13px !important; letter-spacing: 0.3px !important; font-family: ui-monospace, "IBM Plex Mono", Consolas, monospace !important;`. Karakter heksadesimal ditampilkan dengan ukuran jelas, tajam, dan dapat di-scroll secara natural di dalam kotak input.

- **3. Verifikasi UI/UX Menggunakan Playwright Browser:**
  - `navbar_final_verified.png`: Memverifikasi navbar atas mobile (375x812) dengan logo Cubeacon di kiri, tombol logout utuh di kanan dengan margin 16px, dan judul `Device Settings` di bawahnya tanpa tumpang tindih.
  - `ble_card_uuid_13px.png`: Memverifikasi ukuran font input UUID pada ukuran standar 13px yang tegas dan presisi.
  - `verify_settings_mobile_header_fixed.png` & `verify_settings_mobile_full_fixed.png`: Tangkapan layar utuh viewport mobile membuktikan 0px horizontal overflow dan tata letak yang sempurna.


## 86. Harmonisasi UI/UX Halaman Pengaturan Autentikasi & Uplink Jaringan (`auth_admin.html` & `auth_teknisi.html`)

### 86.1 Penyelarasan Layout & Sistem Kartu Bento Berbasis Enterprise
- **1. Struktur Kartu Terstandarisasi (`.card` / `.ch` / `.ci` / `.row`):**
  - Menggantikan tata letak form lama dengan pola kartu enterprise yang konsisten dengan `settings.html`:
    - **Header Kartu (`.ch`):** Memuat ikon tematik 20x20px dalam wadah rounded-rectangle `.ci` (padding 8px, border `rgba(255,255,255,0.06)`), judul tegas `<h3>`, dan deskripsi fungsi `<p>`.
    - **Semantik Palet Ikon:**
      - Administrator Credentials: `.ci.purple` (`rgba(168, 85, 247, 0.12)`, warna teks `#c084fc`)
      - Field Technician Credentials: `.ci.cyan` (`rgba(56, 189, 248, 0.12)`, warna teks `#38bdf8`)
      - Gateway Network Uplink Interface: `.ci.indigo` (`rgba(99, 102, 241, 0.12)`, warna teks `#818cf8`)
      - Wi-Fi Configuration: `.ci.amber` (`rgba(251, 191, 36, 0.12)`, warna teks `#fbbf24`)
      - Ethernet (LAN Cable) Configuration: `.ci.cyan` (`rgba(56, 189, 248, 0.12)`, warna teks `#38bdf8`)
      - Emergency Hotspot (SoftAP): `.ci.blue` (`rgba(59, 130, 246, 0.12)`, warna teks `#60a5fa`)
      - Gateway Telemetry MQTT Broker: `.ci.emerald` (`rgba(16, 185, 129, 0.12)`, warna teks `#34d399`)
    - **Susunan Baris Input (`.row`):**
      - Kolom kiri: Label judul (`.lbl > span`) dan petunjuk teknis di bawahnya (`.sub`).
      - Kolom kanan: Kontrol interaktif (input password dengan inline toggle eye, segmented control uplink, dropdown select, dan toggle switch iOS).

- **2. Kuadran Field Grid 2x2 (`.fgrid` / `.cell`):**
  - Parameter static IP (IP Address, Gateway IP, Subnet Mask, Primary DNS Server) dan parameter MQTT (Host / Broker IP, Port, Quality of Service, Keep Alive) dikelompokkan ke dalam grid kuadran 2x2 responsif yang otomatis beralih menjadi 1 kolom vertikal pada layar mobile.

- **3. Segmented Control Switcher Uplink:**
  - Komponen `.segmented-control` memungkinkan operator memilih antara antarmuka `Wi-Fi` dan `Ethernet (LAN)`.
  - Transisi panel mulus: saat memilih Wi-Fi, panel kartu Wi-Fi ditampilkan dan Ethernet disembunyikan; sebaliknya saat memilih Ethernet, panel Wi-Fi disembunyikan dan panel Ethernet ditampilkan.

### 86.2 Fitur Interaktivitas & Standarisasi UI Enterprise
- **1. Floating Sticky Save Dock (`#floatingSaveDock`):**
  - Terintegrasi dengan sistem dirty checking real-time pada setiap perubahan form.
  - Menampilkan indikator pulsasi kuning-amber, teks status bahasa Inggris `Unsaved Changes`, tombol `Discard Changes` (mengembalikan semua form field ke nilai awal seketika), dan tombol `Save Changes` yang mengirimkan payload JSON ke endpoint C++ `POST /api/v1/config/auth`.
- **2. Inline Password Peek Toggle:**
  - Input password menggunakan komponen `.password-wrapper` dengan tombol `.btn-toggle-pwd`. Mendukung pointer events (hold to reveal pada mouse/touch mobile) serta navigasi keyboard aksesibel (tekan Space/Enter untuk mengintip).
- **3. Wi-Fi Radar Scanner Modal:**
  - Tombol radar `#btnOpenWifiModal` membuka modal pemindaian nirkabel dengan sweep visual, counter persentase spektrum 2.4 GHz, serta menampilkan daftar AP dengan badge RSSI dBm dan indikator gembok proteksi (Open vs Password Protected).
  - Memilih SSID secara instan mengisi kolom `#w_ssid` dan menembakkan bubbling events (`change` & `input`) dengan capture phase untuk mengaktifkan save dock.
- **4. Standardisasi Bahasa Inggris 100% (Full English UI Consistency):**
  - Seluruh teks modal pemindai Wi-Fi (`Scanning Wi-Fi channels...`, `Scanning Nearby Networks`, `Searching 2.4 GHz spectrum (Channels 1 – 13)...`, `Found X networks`, `Password Protected`, `Open Network (Unsecured)`) dan banner pending reboot telah distandarisasi ke bahasa Inggris formal, mematuhi Section 46.

### 86.3 Verifikasi Visual & Audit UI/UX
- **Desktop (1440x900):**
  - `verify_auth_admin_desktop.png`: Tampilan atas memperlihatkan sidebar navigasi, header halaman terstandarisasi, kartu Admin Credentials, Technician Credentials, dan Gateway Network Uplink.
  - `verify_auth_admin_desktop_mid.png`: Tampilan kartu Wi-Fi Configuration dengan tombol scan radar dan Emergency Hotspot (SoftAP).
  - `verify_auth_admin_desktop_bottom.png`: Tampilan kartu MQTT Broker dengan switch MQTTS TLS, port dinamis, QoS selector, dan Keep Alive addon.
  - `verify_auth_admin_desktop_dirty_dock.png`: Memverifikasi floating dock muncul mulus dengan status `Unsaved Changes` dan aksi Simpan/Batal saat field diubah.
- **Mobile (375x812):**
  - `verify_auth_admin_mobile_top.png`: Header mobile 58px dengan logo kiri, logout kanan, zero clipping, dan metadata tags membungkus rapi.
  - `verify_auth_admin_mobile_mid1.png`, `mid2.png`, `mid3.png`, `bottom.png`: Penyelarasan kolom 100% vertikal dengan padding seimbang dan zero horizontal scroll (`bodyScrollWidth = 360px === docClientWidth = 360px`).
  - `verify_auth_admin_mobile_save_footer.png`: Tombol Save Configuration statis memiliki clearance aman dari bottom navigation bar mobile (64px).
  - `verify_auth_teknisi_mobile_top.png`: Halaman teknisi mandiri dengan hak akses terisolasi tanpa kartu admin.
  - `verify_auth_ethernet_panel.png`: Transisi mulus ke kartu konfigurasi Ethernet kabel LAN saat segmented control dipindahkan.
  - `verify_auth_wifi_scan_results.png`: Modal radar Wi-Fi menampilkan daftar access point dengan badge sinyal dan lock status secara elegan.


## 87. Eliminasi Komponen Header Lawas (`.sg-header-card`) & Standardisasi Enterprise Header (`.head`) pada Seluruh Modul Web Gateway

### 87.1 Identifikasi Masalah & Akar Penyebab
- **Anomali Komponen Lawas:**
  - Sebelumnya, halaman **Overview** (`data/index.html`), **Firmware Update** (`data/firmware.html`), dan **System Logs** (`data/log.html`) masih menggunakan komponen header lama `.sg-header-card`.
  - Pada layar mobile (< 640px / 375x812), `.sg-header-card` memuat kotak berbingkai kaku dengan `white-space: nowrap` pada judul dan subtitle.
  - Elemen ini membengkak hingga lebar 459px – 527px, memicu *horizontal scroll* yang tidak diinginkan (`bodyScrollWidth` mencapai 496px vs `clientWidth` 360px) serta mendorong badge status dan tombol logout keluar dari batas bingkai layar (*out of frame*).
  - Selain itu, grid utama `.sg-bento-grid` pada halaman Overview mengalami pemuaian otomatis (*auto-expansion*) karena kolom `1fr` belum dilengkapi `minmax(0, 1fr)`.

### 87.2 Rekayasa Komponen Enterprise `.head`
- **Penyelarasan Desain:**
  - Seluruh modul web gateway kini mengadopsi standar tunggal `.head` (`display: flex; flex-wrap: wrap; justify-content: space-between; align-items: flex-end;` pada desktop dan `align-items: flex-start; gap: 10px;` pada mobile):
    - **`index.html` (Overview):**
      - Judul: `Gateway Overview` (Title Case).
      - Subtitle: `Industrial smart BLE beacon gateway controller and real-time network telemetry.` (multi-line wrapping alami tanpa truncation).
      - Metadata Tags: `.tag` untuk `Model %model%`, `FW %firmware_version%`, `UI %ui_version%`.
    - **`firmware.html` (Update Firmware):**
      - Judul: `Update Firmware` (Title Case, selaras dengan item navigasi sidebar).
      - Subtitle: `Over-The-Air (OTA) dual-partition flasher for application firmware and filesystem.`
      - Metadata Tags: `.tag` untuk `● OTA Recovery Ready` (aksen hijau), `Model %model%`, `FW %firmware_version%`.
    - **`log.html` (System & Data Logs):**
      - Judul: `System & Data Logs` (Title Case).
      - Subtitle: `Dual-engine auditor: filesystem network trail and BLE drop ring buffer.`
      - Metadata Tags: `.tag` untuk `● Rolling FIFO (2x50)` (aksen cyan), `Model %model%`, `FW %firmware_version%`.

### 87.3 CSS Grid Defensif untuk Responsivitas Mobile (< 640px)
- Pada `data/index.html`:
  - `.sg-bento-grid`: Ditambahkan aturan `grid-template-columns: minmax(0, 1fr) !important; width: 100% !important;`.
  - `.sg-panel-left, .sg-panel-right`: Ditambahkan aturan `min-width: 0 !important; max-width: 100% !important; width: 100% !important; box-sizing: border-box !important;`.
  - `.sg-version-grid`: Ditambahkan aturan `grid-template-columns: repeat(3, minmax(0, 1fr)) !important; gap: 6px !important; margin-top: 16px !important;`.
  - `.sg-version-tile`: Ditambahkan `min-width: 0 !important; overflow: hidden !important; padding: 8px 3px !important;`.
  - `.sg-version-tile-val`: Ditambahkan `font-size: 0.85rem !important; overflow: hidden !important; text-overflow: ellipsis !important; white-space: nowrap !important;`.
- **Hasil Pengukuran:**
  - `document.body.scrollWidth === 360px === document.documentElement.clientWidth` (zero horizontal overflow, 100% bebas dari pemotongan teks atau scrollbar horizontal).

### 87.4 Bukti Audit Visual Playwright Browser
- **Overview:**
  - `verify_overview_mobile_fixed.png`: Tampilan mobile (375x812) membuktikan navbar atas, judul `Gateway Overview`, metadata tags, kartu identitas, dan tile versi sistem pas di dalam bingkai tanpa clipping.
  - `verify_overview_desktop_fixed.png`: Tampilan desktop (1440x900) dengan bento grid 2-kolom yang seimbang dan header yang elegan.
- **Firmware:**
  - `verify_firmware_mobile_fixed.png`: Tampilan mobile (375x812) membuktikan eliminasi total overflow dan judul `Update Firmware` yang membungkus rapi.
  - `verify_firmware_desktop_fixed.png`: Tampilan desktop (1440x900) yang harmonis dengan halaman pengaturan lainnya.
- **Logs:**
  - `verify_log_mobile_fixed.png`: Tampilan mobile (375x812) dengan header `System & Data Logs` terstandarisasi.

---

## 88. Standardisasi Lebar Card Pengaturan (1000px Max-Width Standard) & Perbaikan Engine Interaksi Saklar Toggle (.sw)

### 88.1 Diagnosa & Akar Masalah
- **Penyebab Card Terlalu Lebar (1109px – 1440px):**
  - Pada pembaharuan bento grid Overview (`index.html`), kelas `.content` secara global didefinisikan dengan aturan `max-width: 1440px !important; width: 100% !important;`.
  - Aturan ini terbawa dan diterapkan ke seluruh halaman form/pengaturan (`data/settings.html`, `data/auth_admin.html`, dan `data/auth_teknisi.html`).
  - Akibatnya, pada resolusi desktop (1440x900 ke atas), card form meregang hingga 1109px atau lebih, menciptakan jarak horizontal kosong yang terlalu lebar dan tidak ergonomis antara label pengaturan di sisi kiri dan kontrol/input/toggle di sisi kanan.
  - Sesuai standar desain enterprise (`settings-enterprise.html`), container halaman form berorientasi data harus dibatasi ke lebar baca maksimal (`max-width: 1000px` hingga `1040px`) dan dipusatkan secara horizontal (`margin: 0 auto; align-self: center;`).

- **Penyebab Saklar Toggle Tidak Berfungsi (Pointer-Events Blocking):**
  - Pada `app.css` aturan lama:
    ```css
    #uuidContainer.dimmed,
    #majorContainer.dimmed,
    .dimmed {
      opacity: 0.35;
      filter: grayscale(0.7);
      pointer-events: none;
    }
    #restartScheduleContainer.dimmed {
      opacity: 0.35;
      filter: grayscale(0.7);
      pointer-events: none;
    }
    ```
  - Properti `pointer-events: none;` diterapkan secara membabi-buta ke seluruh kontainer baris (`.row.dimmed` atau `.cell.dimmed`).
  - Karena saklar toggle (`.sw` / `#filterMajorToggle`, `#filterUuidToggle`, `#restartScheduleToggle`) berada di dalam baris tersebut:
    1. Pada saat halaman pertama kali dimuat dengan status filter tidak aktif (seperti `filter_major_en == 0`), baris `#majorRow` langsung memiliki kelas `dimmed`. Akibatnya, elemen `<input type="checkbox" id="filterMajorToggle">` sepenuhnya kehilangan kemampuan menerima klik atau sentuhan pengguna (*pointer events pass through to background*).
    2. Jika pengguna mematikan filter UUID (`filterUuidToggle`), JavaScript menambahkan kelas `dimmed` ke `#uuidRow`, yang seketika membekukan toggle tersebut sehingga tidak bisa diaktifkan kembali.
    3. Hal yang sama terjadi pada toggle jadwal reboot harian (`restartScheduleToggle`).

### 88.2 Rekayasa Desain & Solusi Komprehensif
1. **Penerapan Standar Lebar Form Enterprise (1000px Max-Width):**
   - Menambahkan scoped class `.content.settings-container` pada `data/settings.html` dan `.content.auth-container` pada `data/auth_admin.html` serta `data/auth_teknisi.html`.
   - Mengatur aturan CSS spesifik:
     ```css
     .content.settings-container,
     .content.auth-container,
     .content.content-form {
       max-width: 1000px !important;
       width: 100% !important;
       margin-left: auto !important;
       margin-right: auto !important;
       align-self: center !important;
       box-sizing: border-box !important;
     }

     @media (max-width: 1040px) {
       .content.settings-container,
       .content.auth-container,
       .content.content-form {
         max-width: 100% !important;
         padding-left: 20px !important;
         padding-right: 20px !important;
       }
     }

     @media (max-width: 640px) {
       .content.settings-container,
       .content.auth-container,
       .content.content-form {
         padding: 16px 14px calc(var(--bottom-nav-h, 60px) + 85px) 14px !important;
       }
     }
     ```
   - Card kini memiliki lebar proporsional 944px pada layar desktop 1440x900 (tepat di tengah panel utama) dan otomatis responsif 100% hingga 332px pada layar mobile 375x812 tanpa horizontal scroll.

2. **Restorasi Interaktivitas Saklar Toggle (.sw Engine):**
   - Menghapus `pointer-events: none;` dari kontainer `.dimmed`, `#uuidContainer`, `#majorContainer`, dan `#restartScheduleContainer`.
   - Mengisolasi efek peredupan (*dimmed/grayscale/pointer-events: none*) hanya pada elemen input turunan (`.ibox`, `.fld`, `input:not([type="checkbox"])`), bukan pada baris atau kontrol toggle.
   - Memberikan prioritas mutlak pada saklar `.sw`:
     ```css
     .sw {
       position: relative;
       display: inline-block;
       pointer-events: auto !important;
       cursor: pointer !important;
     }

     .sw input,
     .sw input[type="checkbox"] {
       position: absolute;
       inset: 0;
       width: 100% !important;
       height: 100% !important;
       opacity: 0 !important;
       margin: 0 !important;
       cursor: pointer !important;
       pointer-events: auto !important;
       z-index: 5 !important;
     }

     .sw i {
       pointer-events: none;
     }

     .row.dimmed .sw,
     .row.off .sw,
     .cell.dimmed .sw,
     .cell.off .sw,
     .dimmed .sw,
     .off .sw {
       pointer-events: auto !important;
       opacity: 1 !important;
       filter: none !important;
     }

     .row.dimmed .sw input,
     .row.off .sw input,
     .cell.dimmed .sw input,
     .cell.off .sw input,
     .dimmed .sw input,
     .off .sw input {
       pointer-events: auto !important;
       cursor: pointer !important;
     }
     ```

### 88.3 Verifikasi Pengujian & Hasil Audit Visual
- **Pengujian Interaksi Fungsional:**
  - `filterMajorToggle`: Dapat diklik langsung dari keadaan awal OFF -> status berubah menjadi `checked = true`, input teks `filter_major` terbuka (`disabled = false`), nilai `filter_major_en` menjadi `"1"`, dock simpan mengambang aktif. Saat diklik kembali -> status berubah menjadi `checked = false`, input terkunci kembali.
  - `filterUuidToggle`: Dapat dimatikan dan dinyalakan kembali tanpa kendala pembekuan.
  - `restartScheduleToggle`: Menghidupkan dan mematikan jadwal reboot 03:00 dengan pembaruan form instan.
  - `mqttSslToggle`: Mengubah enkripsi TLS MQTT aktif/non-aktif, secara cerdas memindahkan port 1884 <-> 8883 dan memicu dirty-state dock.
- **Audit Bukti Screenshot:**
  - `verify_settings_desktop_fixed.png`: Tampilan desktop Device Settings dengan card terpusat (lebar 944px di dalam kontainer 1000px), layout proporsional, dan tombol Save dock muncul sempurna.
  - `verify_auth_admin_desktop_fixed.png`: Tampilan desktop Auth Settings dengan card terpusat (lebar 944px), MQTTS switch aktif warna hijau toska, port 8883, dan floating save dock sinkron.
  - `verify_settings_mobile_fixed.png`: Tampilan mobile Device Settings (375x812) dengan responsivitas 100% (`scrollWidth === clientWidth === 360px`), bebas clipping.
  - `verify_auth_admin_mobile_fixed.png`: Tampilan mobile Auth Settings (375x812) dengan lebar penuh presisi dan floating dock terposisikan di atas navbar bawah.

---

## 89. Rekayasa Presisi Floating Sticky Save Dock ("You have unsaved changes") & Standardisasi Posisi Enterprise

### 89.1 Diagnosa & Masalah UI/UX Sebelumnya
1. **Bahasa Tidak Selaras (Indonesian vs English):**
   - Pada `data/settings.html`, teks notifikasi floating dock tertulis `"Belum disimpan"`, sementara seluruh ekosistem antarmuka pengguna Cubeacon Gateway berbasis bahasa Inggris enterprise.
   - Pada `data/auth_admin.html` dan `data/auth_teknisi.html`, teks menggunakan varian `"Unsaved Changes"`.
2. **Penyimpangan Posisi Horizontal Desktop (Skewing 130px):**
   - Aturan lama menggunakan `position: fixed; left: 50%; transform: translateX(-50%);`.
   - Karena sidebar desktop memiliki lebar tetap `260px`, titik tengah `left: 50%` dari viewport (720px pada layar 1440px) menyebabkan dock bergeser 130px ke kiri relatif terhadap panel kerja utama (`.main-panel`, titik tengah 850px).
   - Akibatnya, dock tampak melenceng dari sumbu simetri kartu-kartu formulir pengaturan.
3. **Penyimpangan Ketinggian Vertikal Desktop (Awkward 74px Float):**
   - Nilai `bottom: 74px;` merupakan sisa offset navigasi bawah mobile yang diterapkan tanpa media query terpisah.
   - Pada layar desktop yang tidak memiliki bottom navigation bar, jarak 74px membuat dock melayang terlalu tinggi di tengah layar dan menutupi elemen form terbawah.

### 89.2 Solusi Desain & Standarisasi Enterprise
1. **Standardisasi Bahasa & Copywriting:**
   - Seluruh modul (`settings.html`, `auth_admin.html`, `auth_teknisi.html`) kini diseragamkan ke standar bahasa Inggris enterprise:
     `<span class="floating-save-text">You have unsaved changes</span>`.
2. **Kalkulasi Posisi Simetris Desktop:**
   - Menerapkan formula centering presisi berbasis lebar sidebar:
     ```css
     @media (min-width: 769px) {
       .floating-save-dock {
         left: calc(var(--sidebar-w, 260px) + (100vw - var(--sidebar-w, 260px)) / 2) !important;
         bottom: 24px !important;
         transform: translateX(-50%) !important;
         width: calc(100% - 300px) !important;
         max-width: 520px !important;
       }
     }
     ```
   - Dock kini berada tepat di garis tengah visual panel utama (`skewDiff: 0px`) dan mengambang anggun 24px dari tepi bawah layar desktop.
3. **Responsivitas Mobile (< 768px):**
   - Mengambang tepat di atas bottom navigation bar dengan jarak nyaman 12px:
     ```css
     @media (max-width: 768px) {
       .floating-save-dock {
         left: 50% !important;
         bottom: calc(var(--bottom-nav-h, 60px) + 12px) !important;
         transform: translateX(-50%) !important;
         width: calc(100% - 24px) !important;
         max-width: 420px !important;
       }
     }
     ```
4. **Desain Visual Glassmorphism Cybernetic:**
   - **Box:** Background `rgba(13, 18, 31, 0.96)`, `backdrop-filter: blur(20px) saturate(180%)`, border cyan subtle `1px solid rgba(56, 189, 248, 0.32)`, radius `14px`, elevasi bayangan `0 20px 45px -10px rgba(0, 0, 0, 0.8)`.
   - **Indikator:** Pulsing amber dot (`#f59e0b`, `animation: pulseDot 1.5s infinite`).
   - **Tombol Discard:** Ghost pill subtle `background: rgba(255, 255, 255, 0.06)`, border `rgba(255, 255, 255, 0.14)`, hover transisi merah lembut (`#f87171`).
   - **Tombol Save:** Gradient primer `linear-gradient(135deg, #0284c7, #0ea5e9)` dengan icon SVG disket simpan.

### 89.3 Bukti Verifikasi Audit Visual Playwright
- **Desktop:** `verify_settings_dock_desktop_final.png` dan `verify_auth_admin_dock_desktop_final.png` membuktikan floating dock terpusat sempurna di tengah area konten kerja, teks `"You have unsaved changes"` rapi, dan tombol aksi sejajar proporsional.
- **Mobile:** `test_dock_mobile.png` membuktikan floating dock berada tepat 12px di atas bottom navbar tanpa clipping atau horizontal overflow (`scrollWidth: 360px === clientWidth: 360px`).


---

## 90. Standardisasi Event Bubbling Custom Controls & Arsitektur Clean Header Antarmuka Gateway

### 90.1 Latar Belakang & Akar Masalah
1. **Kegagalan Deteksi Perubahan Segmented Switch (Uplink Mode):**
   - Komponen kustom `.segmented-control` (pemilih Wi-Fi vs Ethernet) memperbarui nilai elemen input form (`<input type="hidden" id="connection_mode">`) secara programatik lewat JavaScript (`targetInput.value = this.dataset.value`).
   - Sesuai standar W3C DOM Level 2/3, penugasan nilai properti `.value` via skrip tidak memancarkan event `input` maupun `change` native.
   - Akibatnya, dirty checking listener pada formulir tidak terpanggil dan pop-up dock *"You have unsaved changes"* tidak pernah muncul saat berpindah mode jaringan.
2. **Visual Clutter & Isu Wrapping Metadata Tags di Header:**
   - Elemen tag di pojok kanan atas (`.tags`, `.meta`, `.docs-badge-tag`) menampilkan pengulangan informasi firmware, model, dan role yang sudah tersedia pada sidebar.
   - Pada layar mobile (< 768px), tag metadata ini kerap turun baris (*wrap*), menciptakan ketidakteraturan vertikal dan mengacaukan keselarasan header.

### 90.2 Solusi Desain & Standardisasi Enterprise
1. **Standardisasi Event Bubbling pada Custom Controls (`app.js`):**
   - Setiap komponen kontrol kustom (termasuk `.segmented-control`, switch toggle kustom, dan selector dinamis) diwajibkan memancarkan synthetic events dengan flag `bubbles: true`:
     ```javascript
     if (targetInput) {
       targetInput.value = this.dataset.value;
       try {
         targetInput.dispatchEvent(new Event("input", { bubbles: true }));
         targetInput.dispatchEvent(new Event("change", { bubbles: true }));
       } catch (err) {}
     }
     ```
   - Skrip dirty-state checker tingkat halaman juga dilengkapi *failsafe listener* langsung pada elemen tombol segmen:
     ```javascript
     document.querySelectorAll(".segmented-control .segmented-btn").forEach(function(btn) {
       btn.addEventListener("click", function() {
         setTimeout(checkDirty, 10);
       });
     });
     ```
2. **Penanganan Rollback Simetris pada Tombol "Discard":**
   - Tombol **Discard** menyinkronkan status visual tombol segmented kembali ke nilai awal (`initialValues["connection_mode"]`), mengembalikan kartu panel yang sesuai via `window.updateConnectionMode(modeVal)`, dan menyembunyikan dock pop-up secara bersih.
3. **Pembersihan Metadata Header (Clean Enterprise Header Hierarchy):**
   - Menghapus seluruh elemen tags pojok kanan atas pada 7 berkas HTML:
     - `index.html`: Dihapus `<div class="tags">...</div>`
     - `firmware.html`: Dihapus `<div class="tags">...</div>`
     - `log.html`: Dihapus `<div class="tags">...</div>`
     - `settings.html`: Dihapus `<div class="meta">...</div>`
     - `auth_admin.html`: Dihapus `<div class="meta">...</div>`
     - `auth_teknisi.html`: Dihapus `<div class="meta">...</div>`
     - `docs.html`: Dihapus `<div class="docs-badge-tag">...</div>`
   - Struktur header kini terstandarisasi penuh:
     ```html
     <div class="head">
       <div>
         <h1>[Page Title]</h1>
         <p>[Page Functional Subtitle]</p>
       </div>
     </div>
     ```
   - Memberikan tampilan yang tenang, proporsional, dan 100% bebas dari isu wrapping di smartphone sempit.

### 90.3 Bukti Verifikasi Audit Visual Playwright
- **Uplink Switch Trigger & Discard Rollback (Desktop 1440x900):**
  - `verify_auth_admin_clean_header_desktop.png` (Header bersih tanpa tag)
  - `verify_auth_admin_dock_triggered_desktop.png` (Pop-up dock *"You have unsaved changes"* muncul seketika saat beralih ke Ethernet)
- **Uplink Switch Trigger & Discard Rollback (Mobile 375x812):**
  - `verify_auth_admin_clean_header_mobile.png` (Header mobile bersih dan bebas wrapping)
  - `verify_auth_admin_dock_triggered_mobile.png` (Pop-up dock melayang proporsional di mobile tanpa tumpang tindih)
- **Audit Clean Header Antarhalaman Gateway:**
  - `verify_index_clean_header_desktop.png` & `verify_index_clean_header_mobile.png`
  - `verify_firmware_clean_header_desktop.png` & `verify_firmware_clean_header_mobile.png`
  - `verify_log_clean_header_desktop.png` & `verify_log_clean_header_mobile.png`
  - `verify_settings_clean_header_desktop.png` & `verify_settings_clean_header_mobile.png`
  - `verify_docs_clean_header_desktop.png` & `verify_docs_clean_header_mobile.png`

## 91. Standarisasi Enterprise UI/UX: Segmented Control Adaptif Mobile, Input Search Icon Guard, dan Engine Paginasi Slice 10-Kartu High-Performance

### 91.1 Latar Belakang & Analisis Permasalahan Enterprise
Pada antarmuka Live Test Monitor (`test.html`) di layar mobile (<640px, e.g. iPhone 375x812):
1. **Segmented Button Collision:** Segmented control container memiliki lebar terbatas (~278px), sementara ketiga tombol menampung label panjang (*"FIFO Stream"*, *"Unique Beacons"*, *"Dropped Data"*) dengan total lebar alami 339px. Karena `.segment-btn` diatur `flex: 1 1 0; min-width: 0; white-space: nowrap;` tanpa perlindungan overflow, teks label melompat keluar kontur tombol dan bertumpukan (*overlapping collision*).
2. **Search Icon Placeholder Collision:** Aturan spesifisitas global pada `input[type="text"]` menimpa padding kiri dari `.filter-search-box input`, menyebabkan teks placeholder *"Search MAC, name, UUID, ma..."* tertimpa tepat di bawah ikon pencarian SVG loupe.
3. **Kebutuhan Skalabilitas DOM Paginasi:** Saat scanning beacon di lapangan menangkap ratusan perangkat unik, merender seluruh elemen kartu ke DOM akan menyebabkan memory footprint membengkak, re-flow DOM berat, dan browser mobile mengalami freeze/lag.

---

### 91.2 Solusi Desain Sistem Enterprise & Implementasi

#### 1. Adaptive Micro-Copy Pattern (Segmented Controls)
- Diimplementasikan struktur dua lapis label responsif:
  ```html
  <button class="segment-btn active" id="tabFifo" onclick="switchTab('fifo')">
    <span class="status-dot live"></span>
    <span class="segment-label-full">FIFO Stream</span>
    <span class="segment-label-short">FIFO</span>
  </button>
  <button class="segment-btn" id="tabUnique" onclick="switchTab('unique')">
    <span class="segment-label-full">Unique Beacons</span>
    <span class="segment-label-short">Unique</span>
    <span class="counter-pill" id="tabUniqueCount">0</span>
  </button>
  <button class="segment-btn" id="tabDropped" onclick="switchTab('dropped')">
    <svg class="segment-icon" ...></svg>
    <span class="segment-label-full">Dropped Data</span>
    <span class="segment-label-short">Dropped</span>
    <span class="counter-pill counter-pill-danger" id="tabDroppedCount">0</span>
  </button>
  ```
- **Token CSS Responsif:**
  - Desktop (>=640px): `.segment-label-full { display: inline; }`, `.segment-label-short { display: none; }`
  - Mobile (<640px): `.segment-label-full { display: none; }`, `.segment-label-short { display: inline; }`
  - Tombol dilengkapi `overflow: hidden; text-overflow: ellipsis; justify-content: center; min-width: 0;` sehingga 100% fit dalam segmented bar tanpa tabrakan teks.

#### 2. Specificity Guard & Full-Width Search Input
- Penambahan token spesifisitas tinggi pada `.filter-search-box input`:
  ```css
  .filter-search-box {
    position: relative;
    width: 100%;
    margin-bottom: 0.75rem;
  }
  .filter-search-box input {
    width: 100% !important;
    padding-left: 38px !important; /* Clearance sempurna untuk ikon search 16px */
    height: 38px;
    font-size: 0.8125rem;
  }
  ```
- Di mobile, search bar menempati 100% lebar kontainer dengan jarak visual ideal dan clearance ikon tanpa tabrakan teks placeholder.

#### 3. Windowed 10-Item Pagination Engine (Zero-Lag Skalabilitas)
- **Slice Windowing DOM:**
  - Tetapkan konstanta `const UNIQUE_PER_PAGE = 10;`.
  - DOM kontainer `#uniqueCardsList` hanya menampung maksimal 10 kartu sesuai halaman aktif (`(uniqueCurrentPage - 1) * UNIQUE_PER_PAGE` hingga `sliceEnd`).
  - Kartu di luar halaman aktif tidak dibuat di DOM sama sekali. DOM tetap super ringan bahkan dengan 500+ beacon aktif.
- **In-Place Live Updates:**
  - Saat beacon yang sedang ditampilkan pada halaman aktif menerima paket baru, update nilai RSSI, packet count, dan frame dilakukan secara in-place via `querySelector('.card[data-mac="..."]')` tanpa render ulang list secara masif.
- **Pre-computed Sub-millisecond Search Index:**
  - Setiap perangkat dikompilasi indeks pencarian teks datar saat dibuat (`buildDeviceSearchIndex(d)`):
    `d._searchStr = [mac, name, tag, uuid, major, minor, namespace, instance, ...].join(" ").toLowerCase();`
  - Pencarian menggunakan perbandingan cepat `d._searchStr.indexOf(q) !== -1` dengan 50ms debouncing, memberikan respons pencarian seketika (*instant filter*) tanpa jeda frame.
- **Cybernetic Glassmorphic Pagination Controls:**
  - Komponen navigasi `#uniquePaginationBar` menyajikan tombol First, Prev, Pill Indikator `Page X of Y`, Next, dan Last.
  - Tombol non-aktif diberi status visual transparan `opacity: 0.35` dan `pointer-events: none` dengan atribut `disabled`.

---

### 91.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit |
|---|---|---|
| `verify_test_mobile_fixed.png` | 375x812 | Label adaptif (*FIFO*, *Unique [0]*, *Dropped*) proporsional dan tidak overlap. Search input memiliki padding kiri 38px dengan clearance penuh dari ikon. |
| `verify_unique_pagination_mobile_p1.png` | 375x812 | 25 beacon aktif berhasil dipaginasi. Halaman 1 merender tepat 10 kartu (*Showing 1-10 of 25 beacons*). Tombol Prev & First disabled. |
| `verify_unique_pagination_mobile_p2.png` | 375x812 | Halaman 2 merender kartu 11-20 (*Showing 11-20 of 25 beacons*). Seluruh kontrol navigasi aktif. |
| `verify_unique_pagination_mobile_p3.png` | 375x812 | Halaman 3 merender sisa 5 kartu (*Showing 21-25 of 25 beacons*). Tombol Next & Last otomatis disabled. DOM hanya berisi 5 kartu. |
| `verify_unique_search_mobile.png` | 375x812 | Pencarian instan mendeteksi unit spesifik seketika, DOM terfilter menjadi 1 kartu dengan paginasi reset ke *Page 1 of 1*. |

## 92. Standarisasi Enterprise UI/UX: Kerapatan Vertikal List Kartu (Gap 6-8px), Segmented Control Mobile Bebas Tabrakan, dan Penggantian Metadata Manufacturer dengan Scan Mode Dinamis

### 92.1 Latar Belakang & Analisis Permasalahan Enterprise
1. **Jarak Antar Kartu Terlalu Jauh (26px-30px):**
   - Aturan global `.card { margin-bottom: 20px; }` bocor ke dalam kontainer `.list` dan `.unique-cards-list` yang sudah memiliki `grid-gap: 10px`, menciptakan jarak vertikal 30px di antara setiap kartu berukuran 83px. Hal ini memboroskan ruang layar hingga 300px per halaman dan memaksa scrolling berlebih.
2. **Tabrakan Elemen Internal pada Segmented Control Mobile:**
   - Tombol segmented memuat icon SVG 14px, teks label, dan count badge secara bersamaan. Dalam lebar mobile yang terbatas (~85px per tombol), Tombol 2 (*Unique*) membutuhkan lebar konten >100px sehingga icon, teks, dan badge saling berdempetan dan menabrak tombol di sebelahnya (*Dropped*).
3. **Redundansi Informasi Manufacturer:**
   - Menampilkan *Manufacturer: Cubeacon* di kartu *Gateway Identity & System* bersifat statis dan redundan karena identitas merk sudah tertera di navbar. Operator lapangan membutuhkan parameter operasional radio BLE yang aktif saat runtime, yaitu **Scan Mode** (*Active* atau *Passive*).

---

### 92.2 Solusi Desain Sistem Enterprise & Implementasi

#### 1. High-Density List Architecture (Kerapatan Kartu Pemantauan)
- **Token CSS Normalisasi Spacing:**
  ```css
  /* Normalisasi kerapatan kartu pemantauan */
  .list, .unique-cards-list {
    display: flex !important;
    flex-direction: column !important;
    gap: 6px !important; /* Mobile gap rapat 6px */
    margin-top: 10px !important;
  }
  @media (min-width: 641px) {
    .list, .unique-cards-list {
      gap: 8px !important; /* Desktop gap 8px */
    }
  }
  .list .card, .unique-cards-list .card {
    margin: 0 !important;
    margin-bottom: 0 !important;
    border-radius: 9px !important;
  }
  .list .head, .unique-cards-list .head {
    padding: 9px 12px !important;
  }
  @media (min-width: 641px) {
    .list .head, .unique-cards-list .head {
      padding: 10px 14px !important;
    }
  }
  ```
- **Hasil Pengukuran Matematis:** Jarak vertikal antar kartu terpangkas dari **26px-30px** menjadi tepat **6.0px** (penurunan 77% ruang kosong), menghasilkan tampilan feed telemetri yang padat, rapi, dan mudah dibaca dalam satu layar.

#### 2. Segmented Control Mobile Homogeneity & Icon Omission
- **Eliminasi Icon Dekoratif pada Mobile (<640px):**
  - Sesuai panduan Apple HIG & Google Material 3, icon SVG dekoratif disembunyikan pada layar sempit:
    ```css
    @media (max-width: 640px) {
      .table-card-header-segmented .segment-btn .segment-icon {
        display: none !important;
      }
      .table-card-header-segmented .segment-btn {
        padding: 6px 4px !important;
        gap: 4px !important;
        font-size: 0.8125rem !important;
      }
      .segment-count-badge {
        padding: 1px 5px !important;
        font-size: 0.6875rem !important;
        margin-left: 3px !important;
        flex-shrink: 0 !important;
        border-radius: 10px !important;
      }
    }
    ```
- **Hasil Audit Ruang:** Pada layar 360px, clearance antar-elemen mencapai **>11px**. Pada layar terkecil di dunia (320px), clearance tetap positif **+4.71px** (100% bebas overlap atau clipping).

#### 3. Overview Gateway Telemetry: Scan Mode Active/Passive
- Di `data/index.html`, baris *Manufacturer Cubeacon* digantikan dengan baris operasional *Scan Mode*:
  ```html
  <!-- Scan Mode -->
  <div class="sg-kv-row">
    <span class="sg-kv-label">Scan Mode</span>
    <div class="sg-kv-value" style="display:flex;align-items:center;gap:6px;">
      <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="%scan_mode_color%" stroke-width="2">
        <path d="M4.9 19.1C1 15.2 1 8.8 4.9 4.9"/>
        <path d="M7.8 16.2c-2.3-2.3-2.3-6.1 0-8.5"/>
        <circle cx="12" cy="12" r="2"/>
        <path d="M16.2 7.8c2.3 2.3 2.3 6.1 0 8.5"/>
        <path d="M19.1 4.9C23 8.8 23 15.2 19.1 19.1"/>
      </svg>
      <span style="color:%scan_mode_color%;font-weight:700;">%scan_mode_text%</span>
    </div>
  </div>
  ```
- Di `src/web/web_template.cpp`, ditambahkan penyuntik nilai runtime:
  - `g_config.scan_mode == 2`: Label `"Active"` dengan warna Cyan (`#38bdf8`).
  - `g_config.scan_mode == 1`: Label `"Passive"` dengan warna Emerald (`#34d399`).

---

### 92.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_enterprise_card_gap_fixed.png` | 360x800 | Jarak vertikal antar kartu terukur tepat 6.0px (sebelumnya 26px-30px). 100% margin leak dieliminasi. |
| `verify_enterprise_segmented_mobile_360.png` | 360x800 | Ikon dekoratif disembunyikan di mobile. Teks dan badge memiliki clearance >11px tanpa sentuhan atau overlap. |
| `verify_overview_scan_mode_desktop.png` | 1440x900 | Baris Manufacturer bersih terhapus. Baris Scan Mode tampil elegan dengan icon radar dan warna Emerald untuk mode Passive. |
| `verify_overview_scan_mode_mobile.png` | 375x812 | Kartu Gateway Identity mobile tertata rapi, proporsional, dan padat informasi operasional. |

## 93. Standarisasi Enterprise UI/UX: Pembersihan Header Diagnostik, Eliminasi Badge Redundan, Dinamisasi Batas KPI Card, dan Segmented Mobile Text-Only

### 93.1 Latar Belakang & Analisis Permasalahan Enterprise
1. **Redundansi Status di Header:**
   - Keberadaan badge `testStatusBadge` (STANDBY / STREAMING LIVE) di header Live Diagnostics bersifat redundan terhadap tombol aksi *Start/Stop Test* dan streaming telemetri tabel yang sudah memberikan indikasi visual real-time secara jelas.
2. **Beban Kognitif dari Badge Sekunder:**
   - Badge `0% Buffer` pada kartu *Batch Buffer Queue* dan badge `Avg payload` pada kartu *Pkt Per Batch* menambah kebisingan visual (*visual noise*) tanpa nilai tambah informasi karena angka rasio dan progress bar visual sudah mengomunikasikan status tersebut secara presisi.
3. **Hardcoded Limits pada Metrik Telemetri:**
   - FreeRTOS Ingress Queue menampilkan batas hardcoded `300` (padahal alokasi memori firmware `BEACON_QUEUE_MAX` adalah `150`).
   - Batch Buffer dan Pkt Per Batch menampilkan batas hardcoded `100` (padahal setting pengguna `batch_max` diatur fleksibel antara 10 hingga 200 di menu Settings).
4. **Segmented Control Mobile yang Terlalu Padat:**
   - Kehadiran icon SVG dan count badge pada mobile menghabiskan ruang horizontal berharga pada tombol-tombol segmented.

---

### 93.2 Solusi Desain Sistem Enterprise & Implementasi

#### 1. Header & KPI Card Visual De-cluttering
- **Pembersihan Elemen:**
  - Menghapus `<div class="status-pill ready" id="testStatusBadge">` dari `data/test.html`.
  - Menghapus badge `#stat_batch_badge` (`0% Buffer`) dan `#stat_batch_pkt_badge` (`Avg payload`).
  - Menambahkan null guard `if (statusBadge)` di `data/assets/js/test.js` untuk mencegah error referensi saat pengujian dimulai/dihentikan.
- **Hasil Visual:** Header Live Diagnostics tampil lapang, tenang, dan fokus pada tombol aksi utama. KPI cards tampil bersih dengan fokus pada angka metrik dan progress bar.

#### 2. Configuration-Driven Dynamic KPI Architecture (Anti-Hardcode)
- **Backend Resolvers (`web_template.cpp` & `web_server.cpp`):**
  - Placeholder `%queue_max%` menyuntikkan nilai `BEACON_QUEUE_MAX` (`150`).
  - Placeholder `%batch_max%` menyuntikkan nilai setting NVS runtime `g_config.batch_max`.
  - Field `"queue_max": 150` dan `"batch_max"` dikirim secara dinamis dalam payload JSON WebSocket telemetri.
- **Frontend Logic (`test.js`):**
  - Batas FreeRTOS Queue dibaca dinamis: `const qMax = msg.queue_max || parseInt(document.getElementById("stat_queue_max")?.textContent, 10) || 150;`
  - Batas Batch Buffer dibaca dinamis dari setting `batch_max`.
  - Persentase progress bar `barBatchPkt` dihitung akurat terhadap `curBatchMax`: `Math.round((curBatchPktSize / curBatchMax) * 100)`.

#### 3. Segmented Control Mobile Text-Only (100% Minimalist)
- **Aturan CSS Responsif (`app.css`):**
  ```css
  @media (max-width: 640px) {
    /* Sembunyikan semua SVG icon dekoratif pada tombol segmented mobile */
    .table-card-header-segmented .segment-btn .segment-icon,
    .table-card-header-segmented .segment-btn svg {
      display: none !important;
    }
    /* Sembunyikan count badge unique pada segmented mobile */
    .table-card-header-segmented .segment-count-badge,
    .table-card-header-segmented .segment-badge,
    .table-card-header-segmented .tab-count-badge {
      display: none !important;
    }
  }
  ```
- **Tampilan Mobile:** Tombol tampil teks murni ultra-bersih:
  - `FIFO` (dengan dot live)
  - `Unique`
  - `Dropped`
- **Tampilan Desktop (>=640px):** Tetap menampilkan label penuh (*FIFO Stream*, *Unique Beacons*, *Dropped Data*) lengkap dengan icon SVG dan count badge.

---

### 93.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_test_mobile_clean_header_kpi.png` | 360x800 | Header bersih bebas `testStatusBadge`. Kartu FreeRTOS Queue menampilkan batas dinamis `0 / 150`. Kartu Batch Buffer menampilkan `0 / 50` tanpa badge `0% buffer`. Kartu Pkt Per Batch menampilkan unit `/ 50 pkt` tanpa badge `Avg payload`. |
| `verify_test_mobile_text_only_segmented.png` | 360x800 | Segmented Control mobile tampil teks murni (*FIFO*, *Unique*, *Dropped*). 0 icon SVG, 0 count badge, ruang bernapas lapang dan bebas benturan. |
| `verify_test_desktop_segmented_full.png` | 1440x900 | Tampilan desktop mempertahankan estetika cybernetic penuh: label panjang, icon SVG, dan count badge tampil sempurna. |


## 94. Standardisasi Enterprise Anatomi KPI HUD Card & Penguncian Dimensi Mobile Fixed Height

### 94.1 Latar Belakang & Masalah
1. **Penyimpangan Anatomi Antar-Kartu:**
   - Card 7 (*Pkt Per Batch*) memisahkan nilai numerik di `.hud-val` dan batas kapasitas di `.hud-unit` (`0` dan `/ 50 pkt`).
   - Sebaliknya, Card 4 (*FreeRTOS Ingress*) dan Card 5 (*Batch Buffer*) menggabungkan keduanya ke dalam `.hud-val` (`0 / 150`), menyebabkan inkonsistensi bobot visual dan pemborosan ruang horizontal.
2. **Badge Status Sekunder Redundan:**
   - Badge `#stat_queue_badge` (`0% Load Safe`) dan badge `Optimum` pada Card 6 (*Net Bandwidth*) menyita ruang horizontal, memicu *cognitive load*, serta berisiko tabrakan teks saat throughput meningkat.
3. **Pelebaran Metrik Throughput Ribuan:**
   - Ketika laju paket mencapai $\ge 1.000\text{ pkt/s}$ (misal `1250.4 pkt/s`), label sub-stat `Avg:` dan `Max:` bertambah panjang hingga 20+ karakter, berisiko meluap (*overflow*) atau mematahkan baris (*wrapping*).
4. **Jitter & Ketidaksamaan Tinggi Card Mobile:**
   - Penggunaan `min-height: 94px` tanpa pembatasan `height` tetap mengakibatkan beberapa kartu yang memiliki sub-stats memanjang (108px - 116px), sedangkan kartu dengan progress line tetap 94px. Hal ini membuat grid 2-kolom mobile bergelombang dan tidak presisi standar enterprise.

### 94.2 Implementasi Standar Desain

1. **Penyeragaman Anatomi Kartu & Eliminasi Badge:**
   - Card 4, 5, dan 7 memiliki format identik:
     ```html
     <div class="hud-val-row">
       <div style="display:flex;align-items:baseline;gap:6px;">
         <span class="hud-val white" id="stat_queue_load">0</span>
         <span class="hud-unit">/ <span id="stat_queue_max">%queue_max%</span> pkt</span>
       </div>
     </div>
     ```
   - Badge status sekunder statis/redundant (`0% Load Safe`, `Optimum`) dihapus secara menyeluruh dari template HTML dan dijaga dengan *null-guard* pada JavaScript.

2. **Formatting Angka Ribuan Ringkas (`formatMetricK`):**
   - Nilai metrik throughput diproses menggunakan helper fungsional:
     ```javascript
     function formatMetricK(val) {
       if (val === null || val === undefined || isNaN(val)) return "0";
       const n = Number(val);
       if (n >= 1000000) return (n / 1000000).toFixed(1).replace(/\.0$/, "") + "M";
       if (n >= 1000) return (n / 1000).toFixed(1).replace(/\.0$/, "") + "K";
       return Number.isInteger(n) ? String(n) : n.toFixed(1);
     }
     ```
   - Contoh output:
     - `950` $\rightarrow$ `950`
     - `1250.4` $\rightarrow$ `1.3K`
     - `3820` $\rightarrow$ `3.8K`

3. **Penguncian Dimensi Card KPI Mobile (Fixed Height 104px):**
   ```css
   @media (max-width: 640px) {
     .throughput-hud {
       grid-template-columns: repeat(2, 1fr) !important;
       gap: 8px !important;
     }
     .hud-card-v2 {
       padding: 10px 10px !important;
       height: 104px !important;
       min-height: 104px !important;
       max-height: 104px !important;
       box-sizing: border-box !important;
       overflow: hidden !important;
       display: flex !important;
       flex-direction: column !important;
       justify-content: space-between !important;
       border-radius: 12px;
     }
     .hud-header {
       margin-bottom: 0px !important;
       display: flex !important;
       align-items: center !important;
       justify-content: space-between !important;
       flex-shrink: 0 !important;
     }
     .hud-body {
       display: flex !important;
       flex-direction: column !important;
       justify-content: flex-end !important;
       flex: 1 !important;
       min-width: 0 !important;
     }
     .hud-val-row {
       display: flex !important;
       align-items: baseline !important;
       gap: 4px !important;
       line-height: 1.1 !important;
     }
     .hud-sub-stats {
       font-size: 0.64rem !important;
       gap: 2px 4px !important;
       margin-top: 3px !important;
       display: grid !important;
       grid-template-columns: 1fr 1fr !important;
       white-space: nowrap !important;
       overflow: hidden !important;
     }
     .hud-sub-stats span {
       white-space: nowrap !important;
       overflow: hidden !important;
       text-overflow: ellipsis !important;
     }
     .hud-sub-stats.link-info {
       display: flex !important;
       align-items: center !important;
       gap: 4px !important;
     }
     .hud-progress-line {
       height: 3px !important;
       margin-top: 5px !important;
       flex-shrink: 0 !important;
     }
   }
   ```

### 94.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_kpi_cards_mobile_burst_fixed2.png` | 360x800 (Mobile) | Seluruh 8 kartu KPI berukuran fix 104px presisi (`height: 104px !important`). Baris baseline rata sempurna. Angka throughput ribuan diformat ringkas (`1.3K`, `2.5K`, `3.1K`, `4.9K`) tanpa elipsis atau teks terpotong. Badge `0% Load Safe` dan `Optimum` tereliminasi bersih. Format antrean Card 4 (`120 / 150 pkt`) dan Card 5 (`35 / 50 pkt`) identik seragam dengan Card 7 (`45 / 50 pkt`). |
| `verify_kpi_cards_desktop_burst.png` | 1440x900 (Desktop) | Grid 4-kolom desktop tampil presisi dan bersih. Metrik `Avg` dan `Max` tampil ringkas tanpa redundansi unit `pkt/s`. Seluruh progress line render mulus dengan kontras warna cybernetic standar enterprise. |

---

## 95. Isolasi Tampilan Tab View (FIFO, Unique, Dropped) & Paginasi Mobile Kompak Bebas Clipping

### 95.1 Latar Belakang & Akar Masalah
1. **Konflik Spesifisitas CSS pada `#uniqueCardsList`:**
   - Deklarasi `.list, .unique-cards-list { display: flex !important; }` pada CSS mengesampingkan perintah JavaScript `uniqueCardsList.style.display = "none"`.
   - Akibatnya, daftar kartu beacon unik tetap dipaksa muncul di bawah tabel saat pengguna sedang membuka tab **Dropped Data** maupun **FIFO Stream**.
2. **Overflow & Clipping Horizontal Kontrol Paginasi Mobile:**
   - Pada viewport mobile (360px), bilah paginasi hanya memiliki ruang selebar ~263px.
   - Kontrol desktop dengan 5 elemen (`«`, `Prev`, `Page 1 of 3`, `Next`, `»`) membutuhkan 322px, menyebabkan tombol tepi `«` dan `»` terpotong keluar batas kartu.
3. **Karakter Corrupt pada Label Info Rentang:**
   - Karakter `` (`\ufffd`) tersimpan di berkas HTML sebagai pemisah angka (`Showing 1  10 of 25`).
4. **Bug Logika Realtime Ingress pada Halaman Terakhir:**
   - Pengecekan render baru sebelumnya hanya mengecek `uniqueCurrentPage === 1`. Saat pengguna berada di halaman terakhir ($p > 1$), beacon baru tidak dimunculkan ke DOM.

### 95.2 Implementasi Standar Enterprise

1. **Normalisasi Isolasi Tab:**
   ```css
   .list, .unique-cards-list {
     display: flex;
     flex-direction: column !important;
     gap: 6px !important;
     margin-top: 10px !important;
   }

   .unique-cards-list.is-hidden,
   .list.is-hidden,
   .unique-cards-list[style*="display: none"],
   .unique-cards-list[style*="display:none"],
   .list[style*="display: none"],
   .list[style*="display:none"] {
     display: none !important;
   }
   ```
   - Di JavaScript, perpindahan tab FIFO dan Dropped mengeksekusi `uniqueCardsList.style.setProperty("display", "none", "important")` dan `classList.add("is-hidden")`.
   - Di tab Unique, mengeksekusi `uniqueCardsList.style.setProperty("display", "flex", "important")` dan `classList.remove("is-hidden")`.
   - Fungsi `insertBeacon()` hanya memicu render DOM jika `activeTableTab === "unique"`.

2. **Paginasi Mobile Kompak Adaptif:**
   ```css
   @media (max-width: 640px) {
     .unique-pagination-bar {
       flex-direction: column !important;
       align-items: center !important;
       gap: 8px !important;
       padding: 10px 8px !important;
       box-sizing: border-box !important;
       width: 100% !important;
       overflow: hidden !important;
     }

     .pagination-info {
       font-size: 0.76rem !important;
       text-align: center !important;
       white-space: nowrap !important;
     }

     .pagination-controls {
       width: 100% !important;
       display: flex !important;
       justify-content: center !important;
       align-items: center !important;
       gap: 4px !important;
     }

     /* Sembunyikan label teks Prev & Next pada mobile, sisakan icon SVG */
     .pag-btn-text {
       display: none !important;
     }

     .btn-pag {
       width: 32px !important;
       height: 32px !important;
       min-width: 32px !important;
       padding: 0 !important;
       display: inline-flex !important;
       align-items: center !important;
       justify-content: center !important;
       border-radius: 8px !important;
       font-size: 0.82rem !important;
       flex-shrink: 0 !important;
     }

     .pag-pill {
       font-size: 0.72rem !important;
       padding: 4px 8px !important;
       white-space: nowrap !important;
       flex-shrink: 0 !important;
     }
   }
   ```

3. **Perbaikan Teks Rentang:**
   - Menggunakan HTML entity standar `&ndash;` (`Showing <span id="pagStart">0</span>&ndash;<span id="pagEnd">0</span> of <strong id="pagTotal">0</strong> beacons`).

4. **Realtime Ingress Halaman Terakhir:**
   ```javascript
   if (isNew && activeTableTab === "unique") {
     const total = getFilteredUniqueDevices().length;
     const totalPages = Math.max(1, Math.ceil(total / UNIQUE_PER_PAGE));
     const currentDomCount = uniqueCardsList.querySelectorAll(".card").length;

     if ((uniqueCurrentPage === 1 && currentDomCount < UNIQUE_PER_PAGE) ||
         (uniqueCurrentPage === totalPages && currentDomCount < UNIQUE_PER_PAGE)) {
       renderUniquePage();
     } else {
       updatePaginationControls(total, totalPages);
     }
   }
   ```

### 95.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_mobile_pagination_fixed.png` | 360x800 (Mobile) | Lebar kontrol 209px (sangat lapang di dalam kontainer 263px). Tombol `«` dan `»` utuh 100% dengan ruang bernapas lapang di kedua tepi. Teks rentang `Showing 1 – 10 of 25 beacons` bebas karakter corrupt. |
| `verify_dropped_tab_isolated_fixed.png` | 360x800 (Mobile) | Tab Dropped Data terisolasi sempurna. `#uniqueCardsList` berstatus `display: none !important;` (tinggi 0px). Nol kartu beacon unik yang bocor. |
| `verify_desktop_pagination_full.png` | 1440x900 (Desktop) | Tampilan desktop mempertahankan estetika tombol navigasi teks + icon penuh (`< Prev`, `Next >`) dengan penataan dua sisi sejajar sempurna. |

---

## 96. Eliminasi Redundansi Satuan RSSI (`dBm dBm`) pada Card Unique Beacons

### 96.1 Latar Belakang & Akar Masalah
- Pada baris 356 [`data/test.html`](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/test.html#L356), sub-stat RSSI didefinisikan dengan satuan statis:
  ```html
  <span>RSSI: <strong id="stat_beacon_rssi">-</strong> dBm</span>
  ```
- Di saat yang sama, pada baris 864 [`data/assets/js/test.js`](file:///c:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/assets/js/test.js#L864), JavaScript menyematkan string satuan tambahan saat memperbarui nilai:
  ```javascript
  elBeaconRssi.textContent = `${avgRssi} dBm`;
  ```
- Akibatnya, saat data telemetri beacon masuk, elemen `<strong>` memuat nilai `-50 dBm`, yang kemudian diikuti oleh teks `dBm` di luar tag, menghasilkan teks ganda yang cacat: `RSSI: -50 dBm dBm`.

### 96.2 Solusi Standar Enterprise
- Mengubah penetapan nilai di JavaScript menjadi nilai numerik murni:
  ```javascript
  if (elBeaconRssi && rssiSamples.length > 0) {
    const avgRssi = Math.round(rssiSamples.reduce((a, b) => a + b, 0) / rssiSamples.length);
    elBeaconRssi.textContent = avgRssi;
  }
  ```
- Nilai yang dirender kini tampil bersih dan tunggal: `RSSI: -50 dBm`.

### 96.3 Hasil Verifikasi Playwright

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_unique_beacons_rssi_fixed.png` | 360x800 (Mobile) | Card Unique Beacons menampilkan `RSSI: -50 dBm` (tepat 1 satuan `dBm`). 0 redundansi, tata letak kartu rapi dan presisi. |

---

## 97. Standarisasi Cybernetic Bento KPI Cards Halaman Log (4-Tier Decoupled Bento Standard)

### 97.1 Latar Belakang & Akar Masalah
1. **Truncation Judul & Nilai pada Desktop (265px Card)**:
   - Pada resolusi Desktop 1440px, kartu grid 4-kolom memiliki lebar 265px (konten bersih 229px).
   - Penempatan Ikon (32px), Judul panjang (110–165px), dan Badge status (70–100px) pada baris yang sama memicu pemotongan elipsis parah pada 6 dari 8 kartu (`WI-FI STA...`, `ETHERN...`, `SYSTEM BOOT ...`, `TOTAL DR...`, `MINIMUM R...`, `MQTT PUBLI...`).
   - Nilai panjang `Hardware Disconnected` (21 karakter) terpotong menjadi `Hardware Disconnect...`.
2. **Tabrakan Nilai Broker MQTT & Duplikasi Sub-Stat**:
   - Nilai default `-` (`#kpiMqttState`) berada di dalam baris nilai broker (`- socket.cubeacon.com`).
   - Sub-stat ganda pada Drop Card 1 (`TTL / Overflow`) dan badge ganda pada Drop Card 3 (`Mismatch`).

### 97.2 Solusi Standar Enterprise: 4-Tier Decoupled Bento (Datadog & Cloudflare Radar Standard)
1. **Pemisahan Anatomi 4-Tingkat (4-Tier Decoupled Structure)**:
   - **Tier 1 (Top Meta Row)**: `.kpi-card-header` berisi Ikon (32px) di kiri dan Badge Status semantik di kanan.
   - **Tier 2 (Component Title)**: `.kpi-title` diposisikan pada baris tersendiri dengan **lebar penuh 100% (~229px di desktop, ~284px di mobile)**. Seluruh judul memiliki ruang bernapas lapang dengan **0% pemotongan teks**.
   - **Tier 3 (Primary Value Hero)**: Nilai hero monospace (`font-size: 1.18rem; font-weight: 700;`) menampung teks panjang seperti `Hardware Disconnected` dan `socket.cubeacon.com` secara utuh.
   - **Tier 4 (Auxiliary Footer Sub-Stat)**: Informasi sekunder (`IP • Disconnects`, `TTL • Overflow`, `UUID • Major`) dengan garis pemisah halus `rgba(255, 255, 255, 0.05)`.
2. **Spesifikasi CSS Desktop & Mobile**:
   ```css
   /* Desktop 4-Tier Bento */
   .net-kpi-grid {
     display: grid;
     grid-template-columns: repeat(4, 1fr);
     gap: 16px;
     margin-bottom: 22px;
   }

   .kpi-card {
     background: #0d121f;
     border: 1px solid #1a2333;
     border-radius: 14px;
     padding: 16px 18px;
     min-height: 148px;
     display: flex;
     flex-direction: column;
     justify-content: space-between;
     box-shadow: 0 4px 20px rgba(0, 0, 0, 0.35);
     position: relative;
     overflow: hidden;
     transition: all 0.2s ease;
   }

   .kpi-card-header {
     display: flex;
     justify-content: space-between;
     align-items: center;
     margin-bottom: 8px;
   }

   .kpi-title {
     font-size: 0.74rem;
     font-weight: 700;
     color: #8191a6;
     text-transform: uppercase;
     letter-spacing: 0.05em;
     white-space: nowrap;
     overflow: hidden;
     text-overflow: ellipsis;
     margin-bottom: 4px;
     display: block;
     width: 100%;
   }

   .kpi-value-row {
     display: flex;
     align-items: baseline;
     gap: 6px;
     margin: 2px 0 6px 0;
   }

   .kpi-value {
     font-size: 1.18rem;
     font-weight: 700;
     color: #ffffff;
     font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
     white-space: nowrap;
     overflow: hidden;
     text-overflow: ellipsis;
   }

   .kpi-sub {
     font-size: 0.74rem;
     color: #8191a6;
     border-top: 1px solid rgba(255, 255, 255, 0.05);
     padding-top: 8px;
     margin-top: auto;
   }

   /* Mobile Strict Telemetry Strip */
   @media (max-width: 640px) {
     .net-kpi-grid {
       grid-template-columns: 1fr !important;
       gap: 10px !important;
       margin-bottom: 16px !important;
     }

     .kpi-card {
       padding: 12px 14px !important;
       height: 112px !important;
       min-height: 112px !important;
       max-height: 112px !important;
     }

     .kpi-title {
       font-size: 0.72rem !important;
       margin-bottom: 2px !important;
     }

     .kpi-value {
       font-size: 1.08rem !important;
     }

     .kpi-sub {
       font-size: 0.68rem !important;
       padding-top: 4px !important;
     }
   }
   ```
3. **Build LittleFS & Factory Binary Sync**:
   - `pio run -t buildfs` sukses mengompilasi image filesystem bersih.
   - Sinkronisasi SHA256 identik ke `factory_bins/flash_v1.3.0/board_v1.0.0/littlefs.bin` dan `filesystem.bin`.

### 97.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_log_desktop_net_kpi_rec1.png` | 1440x900 (Desktop) | Seluruh 4 kartu Net KPI memiliki tinggi seragam 148px presisi. Judul (`WI-FI STATION`, `ETHERNET LAN`, `MQTT BROKER`, `SYSTEM BOOT COUNTER`) dan nilai panjang (`Hardware Disconnected`, `socket.cubeacon.com`) tampil utuh 100% tanpa elipsis. |
| `verify_log_desktop_drop_kpi_rec1.png` | 1440x900 (Desktop) | Seluruh 4 kartu Drop KPI memiliki tinggi seragam 148px. Judul (`TOTAL DROPPED`, `MINIMUM RSSI FILTER`, `FILTER TARGET`, `MQTT PUBLISH FAILURES`) tampil utuh bebas truncation. |
| `verify_log_mobile_net_kpi_rec1.png` | 360x800 (Mobile) | Tampilan mobile telemetry strip lapang (lebar ~313px, tinggi 112px). Seluruh judul, badge status, SSID, dan IP tampil utuh. |
| `verify_log_mobile_drop_kpi_rec1.png` | 360x800 (Mobile) | Telemetri drop mobile tampil simetris dan rapi dengan tinggi 112px presisi. |

---

## 98. Master Tab Selector Enterprise Standard (50/50 Dual Segmented Rail)

### 98.1 Latar Belakang & Akar Masalah
- Pada [`data/log.html`](file:///C:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/data/log.html), selektor tab master (`.master-tabs`) menggunakan label teks panjang:
  - Tab 1: `Network & System Events` (~23 karakter)
  - Tab 2: `Packet Drop Telemetry` (~21 karakter)
- Kedua tab membutuhkan total lebar ~420px. Pada layar mobile (lebar 360px, area kontainer ~313px), `flex-wrap: wrap` menyebabkan tab kedua terdorong ke bawah (vertical wrapping collapse).
- Akibatnya, tab aktif berada di baris atas dengan ruang kosong di kanannya, sementara tab tidak aktif menggantung sendirian di baris bawah dalam satu wadah outline yang sama, memberikan kesan antarmuka yang cacat / tidak rapi.

### 98.2 Solusi Standar Enterprise: Responsive Dual Segmented Rail
1. **Penerapan Dual Label (Desktop Full vs Mobile Short)**:
   - Menggunakan pattern `.segment-label-full` dan `.segment-label-short`:
     - Desktop: `Network & System Events` dan `Packet Drop Telemetry`
     - Mobile: `System Events` dan `Drop Telemetry`
2. **Kuncian CSS 50/50 Mobile Rail**:
   ```css
   @media (max-width: 640px) {
     .master-log-tabs-wrap {
       width: 100% !important;
       margin-bottom: 16px !important;
     }

     .master-tabs {
       width: 100% !important;
       display: flex !important;
       flex-wrap: nowrap !important;
       padding: 4px !important;
       box-sizing: border-box !important;
       gap: 4px !important;
     }

     .master-tabs .segment-btn {
       flex: 1 1 50% !important;
       justify-content: center !important;
       padding: 8px 6px !important;
       font-size: 0.78rem !important;
       gap: 6px !important;
       white-space: nowrap !important;
       box-sizing: border-box !important;
     }

     .segment-label-full {
       display: none !important;
     }

     .segment-label-short {
       display: inline !important;
     }

     .master-badge-count {
       font-size: 0.70rem !important;
       padding: 1px 6px !important;
     }
   }
   ```
3. **Hasil Visual**:
   - Kedua tab terbagi seimbang presisi 50% / 50% (`149.6px` dan `149.6px`) pada baris horizontal yang sama (`top: 178.4px`).
   - Kontrol taktil gaya Apple HIG / Linear tanpa pemotongan dan tanpa wrapping.

### 98.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_master_tabs_fixed_mobile.png` | 360x800 (Mobile) | Selektor tab master membentang 100% penuh, terbagi presisi 50/50 (`System Events 3` dan `Drop Telemetry 0`). 0 vertical wrapping, active glow simetris. |

---


## 99. Log Page Filter & Action Toolbar Enterprise Standard (Streamlined 2-Row Action HUD)

### 99.1 Latar Belakang & Masalah Toolbar Sebelumnya
Pada audit tampilan mobile log page (`log.html`), toolbar filter & aksi sebelumnya memiliki beberapa kelemahan UI/UX enterprise:
1. **Vertical Space Bloat (~283px)**: Kontrol kategori dan tombol aksi terpecah menjadi 6 baris terpisah di mobile, menghabiskan lebih dari sepertiga viewport mobile sehingga tabel log terdorong jauh ke bawah.
2. **Search Icon Collision Bug (`🔍arch timestamp...`)**: Spesifisitas selector `input[type="text"]` pada stylesheet global `app.css` (`0, 1, 1`) menimpa selector `.log-search-input` (`0, 1, 0`), memaksa padding kiri dari `36px` menjadi `12px`. Hal ini menyebabkan placeholder teks menimpa ikon pencarian secara langsung.
3. **Ragged Wrapping pada Filter Kategori**: `.segmented-control` menggunakan `flex-wrap: wrap`, menyebabkan chip kategori jaringan dan packet drop terpotong ke baris kedua secara tidak rata.
4. **Resiko Mis-tap Tombol Destruktif**: Tombol destruktif `Clear Logs` diletakkan berhimpitan di baris aksi tanpa visual grouping yang jelas.

### 99.2 Arsitektur Solusi Rekomendasi A (Streamlined 2-Row Action HUD)
Berdasarkan standar dashboard observability enterprise 2026 (Datadog & Linear):

1. **Horizontal Swipeable Chip Rail (`.log-toolbar-card .segmented-control`)**:
   - Menerapkan `flex-wrap: nowrap !important`, `overflow-x: auto !important`, `-webkit-overflow-scrolling: touch !important`, dan `scrollbar-width: none !important`.
   - Chip filter kategori (`All`, `Wi-Fi`, `Ethernet`, `MQTT`, `SoftAP`, `System` untuk Jaringan; `All`, `RSSI Min`, `UUID`, `Major`, `Overflow`, `TTL`, `MQTT Fail` untuk Drop) berada dalam 1 baris horizontal presisi dengan zero vertical wrap.
2. **Fix Spesifisitas Search Input**:
   - Menggunakan selector `.log-search-wrap input[type="text"].log-search-input, input.log-search-input` dengan padding `8px 12px 8px 38px !important; height: 38px;`.
   - Placeholder disederhanakan menjadi `Search logs...` yang ringkas dan tidak pernah terpotong di layar 320px–360px.
3. **Cybernetic 2x2 Action Button Grid pada Mobile**:
   - Di mobile (`@media (max-width: 768px)`), tombol aksi disusun dalam grid 2 kolom seimbang:
     - Baris 1: `[ 🟢 Auto-Refresh (5s) ]` | `[ 🔄 Refresh ]`
     - Baris 2: `[ 📥 Export CSV ]` | `[ 🗑️ Clear Logs ]` (dengan aksen bahaya merah `#ef4444`)
   - Masing-masing tombol mendapatkan area sentuh ~148px (50% card width), menjamin label teks 100% terbaca tanpa singkatan, dan tombol destruktif terisolasi secara ergonomis.
4. **Counter Monospace yang Rapi**:
   - Menambahkan margin horizontal `margin: 0 3px;` pada `.log-counter-info strong` dengan font monospace, mencegah angka menempel pada kata sekitarnya.
5. **Reduksi Tinggi Drastis**:
   - Footprint vertikal mobile berkurang dari **~283px** menjadi **~156px** (pengurangan >45%), memberikan ruang visual langsung untuk melihat data log.
   - Pada desktop (>= 768px), toolbar tetap berada dalam 2 baris proporsional (~96px).

### 99.3 Kode Implementasi Terstandarisasi

```css
/* 4. TOOLBAR CARD (CYBERNETIC BENTO CONTROLS) */
.log-toolbar-card {
  background: #0d121f;
  border: 1px solid #1a2333;
  border-radius: 14px;
  padding: 14px 18px;
  margin-bottom: 20px;
  display: flex;
  flex-direction: column;
  gap: 12px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.35);
}

.log-toolbar-top {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: nowrap;
  gap: 12px;
}

.log-toolbar-bottom {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: nowrap;
  gap: 12px;
  border-top: 1px solid rgba(255, 255, 255, 0.05);
  padding-top: 12px;
}

.log-search-wrap input[type="text"].log-search-input,
input.log-search-input {
  width: 100%;
  padding: 8px 12px 8px 38px !important;
  background: #090e18;
  border: 1px solid #1a2333;
  border-radius: 8px;
  color: #f8fafc;
  font-size: 0.85rem;
  outline: none;
  box-sizing: border-box;
  height: 38px;
}

.log-toolbar-card .segmented-control {
  background: #090e18;
  border: 1px solid #1a2333;
  border-radius: 9px;
  padding: 3px;
  display: inline-flex;
  gap: 4px;
  flex-wrap: nowrap !important;
  overflow-x: auto !important;
  -webkit-overflow-scrolling: touch !important;
  scrollbar-width: none !important;
  max-width: 100%;
}

@media (max-width: 768px) {
  .log-toolbar-card {
    padding: 12px;
    gap: 10px;
    margin-bottom: 16px;
  }

  .log-toolbar-top {
    flex-direction: column;
    align-items: stretch;
    gap: 10px;
  }

  .log-actions-cluster {
    width: 100%;
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;
  }

  .log-actions-cluster .btn-action-pill {
    justify-content: center;
    padding: 8px 10px;
    font-size: 0.80rem;
  }

  .log-toolbar-bottom {
    flex-direction: column;
    align-items: stretch;
    gap: 8px;
    padding-top: 10px;
  }
}
```

### 99.4 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_log_toolbar_mobile_rec_a_final.png` | 360x800 (Mobile) | Toolbar rapi & kompak. Rail kategori dalam 1 baris swipeable (0 wrapping). Tombol aksi terdistribusi presisi dalam grid 2x2. Search input memiliki padding `38px` dengan 0 icon collision. Counter angka monospace terpisah jelas. |
| `verify_log_drop_toolbar_mobile.png` | 360x800 (Mobile) | Tab Packet Drop filter rail tampil dalam 1 baris scrollable (All, RSSI Min, UUID, Major...). Tombol aksi 2x2 simetris. Zero clipping. |
| `verify_log_toolbar_desktop_rec_a.png` | 1440x900 (Desktop) | Toolbar presisi 2 baris HUD (~96px). Row 1: Filter Rail (kiri) + 4 Action Buttons (kanan). Row 2: Search input lebar (kiri) + Counter (kanan). |

---


## 100. Standardisasi Penamaan EEPROM & Eliminasi Redundansi Counter Toolbar

### 100.1 Latar Belakang & Kebutuhan
1. **Penyederhanaan Identifikasi Media Penyimpanan**:
   - Kata teknis `AT24C256` diganti menjadi `EEPROM` agar lebih generik, mudah dipahami operator, dan selaras dengan terminologi memori industri.
   - Perubahan berlaku pada badge KPI Card 4 (`System Boot Counter`) di `data/log.html` dan logika status di `data/assets/js/log.js` (`data.eeprom_detected ? "EEPROM" : "RAM"`), serta payload string backend di `src/network/net_log_service.cpp`.
2. **Eliminasi Counter Redundan pada Toolbar (`Showing 0 of 0 records`)**:
   - Teks counter `Showing 0 of 0 records` pada toolbar (`.log-toolbar-bottom`) dihilangkan karena informasi entri tabel log sudah disajikan secara lengkap, dinamis, dan terformat rapi pada baris paginasi di bawah tabel (`.log-pagination-wrap`): `Showing 1 - 3 of 3 entries`.
   - Menghilangkan duplikasi ini menghemat ruang vertikal tambahan, mencegah tabrakan angka monospace pada viewport sempit, dan menjadikan baris pencarian `Search logs...` tampil rapi membentang penuh (100% width).

### 100.2 Ringkasan Perubahan Kode
- **`data/log.html`**:
  - Badge KPI: `<span class="kpi-badge badge-cyan" id="kpiBootStorageBadge">EEPROM</span>`
  - Toolbar Bottom: Elemen `.log-counter-info` dihapus total dari HTML dan stylesheet CSS.
  - Search Input: `.log-search-wrap` kini mengambil lebar 100% dari `.log-toolbar-bottom`.
- **`data/assets/js/log.js`**:
  - Logika badge: `kpiBootStorageBadge.textContent = data.eeprom_detected ? "EEPROM" : "RAM";`
  - Re-kompresi ke `data/assets/js/log.js.gz`.
- **`src/network/net_log_service.cpp`**:
  - String deskripsi mode penyimpanan diperbarui: `"EEPROM (200-Slot Rolling Wear-Leveling, FIFO 50)"`.

### 100.3 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_eeprom_badge_mobile.png` | 360x800 (Mobile) | KPI Card 4 menampilkan badge `EEPROM` (cyan pill) menggantikan `AT24C256`. 0 visual clipping. |
| `verify_log_toolbar_no_showing_mobile.png` | 360x800 (Mobile) | Toolbar rapi dan bersih. Tidak ada lagi teks `Showing of records` yang menempel/tumpang tindih. Tinggi toolbar berkurang menjadi ~135px. Data tabel langsung tampak di atas lipatan layar. |
| `verify_log_desktop_eeprom_no_showing.png` | 1440x900 (Desktop) | Desktop view menampilkan KPI badge `EEPROM` dan bilah pencarian `Search logs...` full-width yang lapang dan simetris. Paginasi bawah tetap menyajikan `Showing 1 - 3 of 3 entries`. |

---


## 101. Standardisasi RSSI Range Slider & Kalibrasi Presisi Ticks Alignment

### 101.1 Analisis Akar Masalah (Root Cause Analysis)
Berdasarkan investigasi UI/UX dan audit DOM pada halaman Pengaturan (`settings.html`):
1. **Penyebab Ticks Tidak Sejajar dengan Slider**:
   - Struktur HTML sebelumnya menempatkan `.ticks` (`-30`, `-65`, `-100`) langsung di dalam container `.ctl` bersama dengan input pill `.rv` (`- 100 dBm`).
   - Karena `.ctl` memiliki lebar total gabungan (pill 110px + gap 16px + slider 294px = 420px), `.ticks` dengan `justify-content: space-between` meregang selebar 420px.
   - Hal ini menyebabkan angka `-30` berada jauh di ujung kiri di bawah input pill, angka `-65` berada di dekat awal slider, dan `-100` tidak berada pas di ujung slider.
2. **Penyebab Pointer (Thumb) Tidak Sampai Ujung & Muncul Track Sisa Putih**:
   - Aturan global `input, select, textarea` di stylesheet `app.css` menetapkan `padding: 0px 14px;`, `height: 42px;`, dan border.
   - Karena `padding-right: 14px`, lintasan native slider Chromium berhenti 14px sebelum tepi kanan kotak elemen range.
   - Elemen juga tidak memiliki `-webkit-appearance: none;`, sehingga Chromium merender track native abu-abu/putih di dalam kotak elemen.
   - Saat nilai diatur ke 100 (`-100 dBm`), thumb berhenti di batas lintasan track native, sementara sisa track native putih tetap tampak terbuka di sebelah kanan thumb.

### 101.2 Solusi & Standarisasi Desain Enterprise
1. **Isolasi Container `.rssi-slider-wrap`**:
   - Membungkus `<input type="range" id="rssi_slider">` dan `<div class="ticks">` ke dalam satu container flex-column khusus (`.rssi-slider-wrap`) dengan `flex: 1; min-width: 0;`.
   - Input pill `.rv` tetap berada di kiri secara independen, sedangkan `.ticks` berada tepat di bawah lintasan slider.
2. **Reset Penuh Native Range & Track Kustom**:
   - Menerapkan `-webkit-appearance: none !important; appearance: none !important; height: 8px !important; padding: 0 !important; border: none !important;` pada `#rssi_slider`.
   - Track 8px dengan sudut tumpul 4px (`border-radius: 4px;`) yang diwarnai langsung oleh background gradient `linear-gradient(to right, #0284c7 0%, #38bdf8 ${pct}%, #1e293b ${pct}%, #1e293b 100%)`.
   - Kustomisasi `-webkit-slider-thumb` berukuran 20px berbentuk lingkaran biru cyan (`#38bdf8`) dengan border putih 2px dan drop-shadow, serta offset `margin-top: -6px` agar thumb terpusat sempurna di atas track 8px.
3. **Hasil Kalibrasi Ticks**:
   - Pada `.ticks`, diberikan padding presisi `padding: 0 4px;`.
   - `-30` berada tepat di bawah awal thumb (nilai 30).
   - `-65` berada tepat di titik tengah (nilai 65, yaitu 50% rentang).
   - `-100` berada tepat di bawah thumb saat nilai maksimal 100.
   - Pada nilai 100, track terisi 100% biru cyan solid dan thumb mencapai ujung paling kanan tanpa ada celah atau sisa track putih.

### 101.3 Kode Implementasi

```html
<div class="ctl">
  <div class="rssi">
    <div class="rv" id="rssiPill">
      <span>&minus;</span>
      <input type="text" inputmode="numeric" id="rssi_input_abs" value="%rssi_val_abs%" aria-label="RSSI absolute value" placeholder="80">
      <small>dBm</small>
    </div>
    <div class="rssi-slider-wrap">
      <input type="range" id="rssi_slider" min="30" max="100" step="1" value="%rssi_slider_val%" aria-label="RSSI slider" style="background: %rssi_slider_bg%;">
      <div class="ticks">
        <span>&minus;30</span>
        <span>&minus;65</span>
        <span>&minus;100</span>
      </div>
    </div>
  </div>
  <input type="hidden" id="rssi_min" name="rssi_min" value="%rssi_min%">
</div>
```

```css
.rssi {
  display: flex;
  align-items: center;
  gap: 16px;
  width: 100%;
}

.rssi-slider-wrap {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: center;
  min-width: 0;
}

.rssi-slider-wrap input[type="range"],
#rssi_slider {
  -webkit-appearance: none !important;
  appearance: none !important;
  width: 100% !important;
  height: 8px !important;
  border-radius: 4px !important;
  border: none !important;
  outline: none !important;
  padding: 0 !important;
  margin: 8px 0 4px 0 !important;
  cursor: pointer;
  box-shadow: none !important;
}

.rssi-slider-wrap input[type="range"]::-webkit-slider-thumb,
#rssi_slider::-webkit-slider-thumb {
  -webkit-appearance: none !important;
  appearance: none !important;
  width: 20px !important;
  height: 20px !important;
  border-radius: 50% !important;
  background: #38bdf8 !important;
  border: 2px solid #ffffff !important;
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.4), 0 0 8px rgba(56, 189, 248, 0.5) !important;
  cursor: grab;
  margin-top: -6px;
  transition: transform 0.1s ease, box-shadow 0.1s ease;
}

.rssi-slider-wrap .ticks {
  display: flex;
  justify-content: space-between;
  font-family: ui-monospace, "IBM Plex Mono", Consolas, monospace;
  font-size: 11px;
  color: var(--text-dim, #64748b);
  padding: 0 4px;
  margin-top: 4px;
}
```

### 101.4 Bukti Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_slider_desktop_fixed_100.png` | 1440x900 (Desktop) | Nilai 100: Thumb berada presisi di ujung paling kanan lintasan (0 celah/track putih). `-30`, `-65`, `-100` sejajar sempurna di bawah track slider. Input pill `- 100 dBm` terpisah rapi di kiri. |
| `verify_slider_desktop_fixed_80.png` | 1440x900 (Desktop) | Nilai 80 (default): Thumb berada di posisi proporsional antara -65 dan -100. Track terisi warna biru proporsional. |
| `verify_slider_mobile_fixed_100.png` | 360x800 (Mobile) | Tampilan mobile (360px): Slider dan ticks terisolasi dengan rapi, thumb mencapai ujung track, teks tick sejajar presisi di bawah lintasan. |

---


## 102. Filter Unknown BLE Format Control & Wire Protocol Standard

### 102.1 UI/UX Component Architecture (Card 1, Row 2)
- **Komponen Switch & Badge Dinamis**:
  - Diletakkan tepat di bawah "Radio scan mode" sebagai **Row 2** pada Card 1 (*BLE Radio Scanner & Signal Filtering*).
  - Menggunakan tactile switch inline (`label.sw`) dengan state default: **OFF / Disabled**.
  - Badge reaktif (`#unknownBadge`):
    - **Disabled**: Kelas `badge neutral` (Slate grey, background semi-transparan `rgba(148, 163, 184, 0.12)`).
    - **Active**: Kelas `badge valid` (Emerald green, background semi-transparan `rgba(34, 197, 94, 0.15)`).
  - Form integration: Terikat secara bidirectional ke input hidden `#filter_unknown_en` (`0` atau `1`), terdaftar pada `recordInitialValues()`, `isFormDirty()`, dan fungsi rollback pembatalan form.

### 102.2 Wire Protocol Standards & Binary Recognition Suite
Ketika filter aktif (`filter_unknown_en == 1`), paket raw/non-beacon yang tidak dikenali akan di-drop sebelum masuk ke ingress queue MQTT, menghemat kuota transmisi dan memori DRAM. Protokol yang dikenali dan diparsing:
1. **Apple iBeacon**: Manufacturer ID `0x004C`, Type `0x02`, Sub-type `0x15` (UUID 16 byte, Major 2 byte, Minor 2 byte, TX Power).
2. **Eddystone Protocol**: Service Data UUID `0xFEAA` (Frame UID, URL, TLM).
3. **MokoSmart Official Binary Suite**:
   - `0xEA01` (BXP-Sensor: Service Data 16-bit, Frame Type `0x80`, Battery mV, Sensor Data).
   - `0xFEE0` (BXP-Alarm: Button single/double/triple/hold trigger, SOS status).
   - `0xEA00` (BXP-ScanRsp: Official wire scan response data).
   - `0xFEAB` (Moko Beacon Multi-frame: telemetry `0x40/0x60/0x70`).
   - Manufacturer Data Company ID `0x0008` / `0x24 0x01` / `0x15 0x02` (Legacy Moko sensors).
   *(Catatan: Pengenalan tidak mengandalkan string nama dummy emulator seperti `"MOKO"`, melainkan validasi biner standar industri sehingga aman digunakan pada ribuan beacon produksi).*

### 102.3 Zero-Heap / RAM Safety Architecture
- **Eksekusi Stack-Only**: Seluruh proses parsing frame coprocessor nRF52840 (42 byte binary) beroperasi langsung di stack tanpa dynamic heap memory (`malloc`/`new`).
- **Alokasi DRAM**: Penambahan variabel counter atomik `s_dropUnknownFormat` dan field konfigurasi `filter_unknown_en` hanya mengonsumsi $< 20$ byte DRAM secara global.
- **Drop Log Integration**: Alasan drop dicatat sebagai `DropReason::UNKNOWN_FORMAT` (kode `6`) dengan badge warna **ORANGE** (`badge orange`, `#f97316`) pada Drop Log Table.

### 102.4 Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_settings_unknown_filter_desktop.png` | 1280x800 (Desktop) | Posisi Row 2 tepat di bawah Radio Scan Mode. Switch OFF, badge "Disabled" (neutral). Spasi vertikal konsisten 16px. |
| `verify_settings_unknown_filter_active.png` | 1280x800 (Desktop) | Switch diklik menjadi ON, badge seketika berubah menjadi "Active" (valid emerald). Floating Save Dock muncul presisi di bagian bawah tanpa pergeseran layout. |
| `verify_settings_unknown_filter_mobile.png` | 375x812 (Mobile) | Tampilan mobile iPhone responsive: Teks label dan badge tetap inline, hint text membungkus rapi, tombol toggle switch sejajar vertikal di sisi kanan, floating dock tidak menutupi bottom navigation bar. |

---


## 103. Persistent Header Telemetry Uptime & 4-Pillar MECE Drop KPI Architecture

### 103.1 Persistent Header Telemetry Uptime Widget
- **Penempatan Global (Di Luar Tab)**:
  - Ditempatkan di dalam `<header class="head">` di samping judul halaman `System & Data Logs`.
  - Bersifat persisten: tetap terlihat saat teknisi berpindah-pindah antara tab *Network & System Events* dan *Packet Drop Telemetry*.
- **Desain Typographic Bersih (Industrial Enterprise Minimalist)**:
  - **Bebas Icon**: Murni menggunakan tipografi monospaced untuk efisiensi visual tinggi tanpa elemen dekoratif yang mengalihkan perhatian (`Lifetime: 45d 13h │ Session: 3d 03h`).
  - **Bebas Jargon Hardware ("EEPROM")**: Tidak mengekspos kata EEPROM atau storage status badge di UI pengguna; fokus murni pada durasi uptime operasional gateway.
  - **Monospace Emphasis**: Angka menggunakan font `ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace` dengan aksen warna Putih terang (`#f8fafc`) untuk Lifetime dan Cyan (`#38bdf8`) untuk Session.
- **Ergonomi Responsif Mobile (Strip Bar 34px Single-Row)**:
  - Pada layar mobile ($\le 640\text{px}$), widget beralih secara dinamis menjadi strip horizontal setinggi 34px dengan layout `space-between`.
  - Teks Lifetime berada di sisi kiri, garis pemisah vertikal tipis di tengah, dan teks Session di sisi kanan dalam 1 baris tanpa wrapping/overflow teks.
- **Silent Hardware Failover (Graceful Degradation)**:
  - Bila modul external EEPROM I2C (AT24C256) belum terpasang, bus error, atau data korup (CRC checksum mismatch), firmware secara otomatis dan senyap (*silent failover*) mengalihkan telemetry `lifetime_uptime` menjadi sama dengan `session_uptime` di RAM.
  - Tidak memunculkan alert, error modal, atau visual crashing bagi pengguna.
- **EEPROM Endurance & Write Protection**:
  - Durasi detik bertambah langsung secara real-time di RAM.
  - Sinkronisasi ke EEPROM hanya dieksekusi secara periodik setiap 30 menit dan saat sistem menerima sinyal software reboot aman, menjamin ketahanan chip $> 57$ tahun pemakaian non-stop.

### 103.2 4-Pillar MECE Drop KPI Architecture (Policy vs Fault Separation)
Menggantikan metrik lama dengan arsitektur 4 pilar MECE (*Mutually Exclusive, Collectively Exhaustive*) berstandar Datadog / Cloudflare:
1. **Card 1: RF Signal Filter**
   - **Metrik**: Drop paket akibat sinyal BLE di bawah ambang batas RSSI (`rssi_min`).
   - **Badge**: `Ingress RF` (Cyan pill).
   - **Sub-info**: *"Dropped: RSSI below threshold"*.
2. **Card 2: Target & Format Filter**
   - **Metrik**: Total paket yang sengaja di-drop oleh kebijakan penyaringan perangkat (`uuid_mismatch` + `major_mismatch` + `unknown_format`).
   - **Badge**: `Policy Match` (Orange pill).
   - **Sub-info breakdown**: `UUID: X | Maj: Y | Unk: Z`.
3. **Card 3: Buffer & Queue Loss**
   - **Metrik**: Total kehilangan paket akibat tekanan memori dan keusangan buffer (`queue_overflow` + `ttl_expired`).
   - **Badge Dinamis**:
     - Nilai 0: `● Healthy (0 Loss)` (Emerald pill dengan glowing green pulse dot).
     - Nilai > 0: `● Backpressure` (Rose pill dengan pulsing red dot).
   - **Sub-info breakdown**: `Overflow: X | TTL Stale: Y`.
4. **Card 4: Uplink Dispatch Loss**
   - **Metrik**: Total kegagalan kirim atau antrian terbuang akibat terputusnya koneksi broker MQTT (`mqtt_failed`).
   - **Live Alarm Badge**:
     - Broker Online: `● Broker Connected` (Emerald pill dengan glowing green pulse dot).
     - Broker Offline: `● Broker Disconnected` (Rose pill dengan pulsing red dot).
   - **Sub-info**: `Uptime Drops: X frames`.

### 103.3 Hasil Audit UI/UX Enterprise (Playwright)

| File Screenshot | Viewport | Hasil Audit Enterprise |
|---|---|---|
| `verify_log_desktop_tab1.png` | 1280x800 (Desktop) | Tab 1 (System Events): Header Uptime Widget tampil di kanan atas (`Lifetime: 45d 13h │ Session: 3d 03h`). Typographic bersih, 0 icon, padding proporsional. |
| `verify_log_desktop_tab2_emerald.png` | 1280x800 (Desktop) | Tab 2 (Drop Telemetry): Header Uptime Widget tetap persisten. Grid 4 card tersusun simetris; Card 1 Ingress RF (7,820 pkt), Card 2 Policy Match (4,630 pkt dengan rincian UUID/Maj/Unk), Card 3 Healthy 0 Loss, Card 4 Live Alarm Broker Connected. |
| `verify_log_mobile_tab1.png` | 375x812 (Mobile) | Tampilan mobile iPhone Tab 1: Strip bar 34px memuat Lifetime & Session sejajar rapi di antara deskripsi header dan master tab switch. 0 overflow atau teks terpotong. |
| `verify_log_mobile_tab2_emerald.png` | 375x812 (Mobile) | Tampilan mobile iPhone Tab 2: Keempat card MECE tersusun rapi secara vertikal, badge status kontras dan jelas, live green pulse dot tampil presisi, bottom bar tidak terhalang. |

---

### 103.4 Compact Metric Notation Standard (K/M/B Maximum 3 Digits with 1 Decimal)
- **Problem Statement**: Laju drop paket BLE pada gateway padat dapat menembus puluhan ribu hingga jutaan paket per hari (`14,250,890 pkt`). Menampilkan angka integer penuh secara horizontal dapat memicu text overflow, wrapping tak diinginkan, atau memadati layout kartu mobile (375px).
- **Rule Spesifikasi Notasi Ringkas (`formatCompact`)**:
  - **$< 1.000$**: Tampil sebagai integer murni (`0` s.d. `999`).
  - **$1.000$ s.d. $99.949$**: Format satuan ribuan `K` dengan maksimal 1 angka desimal (`1K`, `1.2K`, `7.8K`, `12.4K`, `99.9K`).
  - **$99.950$ s.d. $999.499$**: Format ratusan ribu `K` tanpa desimal untuk membatasi panjang digit maksimal 3 angka (`100K`, `124K`, `999K`).
  - **$999.500$ s.d. $99.949.999$**: Format jutaan `M` dengan maksimal 1 angka desimal (`1M`, `1.4M`, `12.5M`, `99.9M`).
  - **$100.000.000+$**: Format ratusan juta `M` tanpa desimal (`100M`, `124M`) atau milyaran `B` (`1.2B`, `4.3B`).
  - **Zero-Stripping**: Jika nilai desimal tepat `.0`, angka di belakang koma dihilangkan secara elegan (`1K`, bukan `1.0K`).
- **Precision Tooltip (`title`)**:
  - Untuk menjaga keakuratan data industri, setiap elemen angka yang diringkas dilengkapi atribut HTML `title` yang memuat angka presisi eksak lengkap dengan pemisah ribuan (contoh: hover pada `1.4M pkt` menampilkan tooltip `1,421,220 pkt`).

| File Screenshot | Viewport | Hasil Audit Enterprise Compact Notation |
|---|---|---|
| `verify_compact_kpi_desktop.png` | 1280x800 (Desktop) | Card 1: `7.8K pkt`, Card 2: `1.4M pkt` (sub breakdown: `UUID: 1.4M \| Maj: 820 \| Unk: 400`). Teks rapi, proporsional, dan ringkas. |
| `verify_compact_kpi_mobile.png` | 375x812 (Mobile) | Tampilan 375px iPhone: Card 2 sub breakdown `UUID: 1.4M \| Maj: 820 \| Unk: 400` muat dalam 1 baris tanpa wrapping horizontal. |

---

### 103.5 Option 2 Refinement: Fixed-Width Compact Capsule (Max-Capacity Safe)
- **Eliminasi Masalah Stretched Box**:
  - Menghilangkan `width: 100%` yang sebelumnya menyebabkan teks Lifetime dan Session terdorong ke ujung layar berlawanan dengan area kosong melompong di tengah.
  - Mengubah bentuk container menjadi **kapsul ramping penuh (*stadium pill*)** dengan `border-radius: 9999px` dan lebar tetap terkalibrasi: `width: 304px !important;`.
- **Ketahanan Terhadap Angka Maksimum Teoritis (Max Data Safe)**:
  - Lebar 304px dirancang presisi untuk menampung batas matematis tertinggi firmware:
    - **Lifetime Maksimum**: `49710d 00h` (Batas maksimum 32-bit `uint32_t` $\approx 136\text{ tahun}$).
    - **Session Maksimum**: `9999d 23h` ($\approx 27\text{ tahun}$ operasi sesi tanpa restart).
  - Teks tetap 1 baris utuh (`white-space: nowrap`), tanpa pergeseran posisi, dan tanpa teks keluar frame pada semua layar smartphone ($\ge 360\text{px}$).
- **Hasil Audit UI/UX Enterprise**:

| File Screenshot | Viewport / Kondisi Data | Hasil Audit Visual |
|---|---|---|
| `verify_option2_normal_mobile.png` | 375x812 (Mobile) — Data Normal | Lifetime `45d 13h` & Session `3d 03h` tersusun rapat, proporsional, border kapsul semi-transparan elegan di bawah subtitle. |
| `verify_option2_max_uptime_mobile.png` | 375x812 (Mobile) — Data Maksimum | Lifetime `49710d 00h` & Session `9999d 23h` (136 tahun) tetap muat presisi dalam kapsul 304px tanpa wrapping atau pemotongan teks. |
| `verify_option2_max_uptime_mobile_tab2.png` | 375x812 (Mobile) — Tab 2 Telemetry | Kapsul Uptime persisten di posisi yang sama saat beralih ke Tab Drop Telemetry dengan nilai drop KPI `7.8K pkt` dan `1.4M pkt`. |

---

### 103.6 Terminology Alignment: "Session" -> "Uptime"
- **Standardisasi Terminologi Lintas Halaman**:
  - Untuk menciptakan kohesi informasi yang selaras dengan halaman **Overview** (di mana durasi sesi berjalan gateway dinamai **System Uptime**), label `Session:` diubah menjadi **`Uptime:`**.
  - Format teks final: `Lifetime: 45d 13h │ Uptime: 3d 03h`.
  - Panjang karakter "Uptime:" (7 huruf) 1 karakter lebih ringkas daripada "Session:" (8 huruf), memberikan ruang napas visual yang semakin optimal pada kapsul 304px.

| File Screenshot | Viewport | Hasil Audit Visual Keselarasan Terminologi |
|---|---|---|
| `verify_uptime_text_mobile.png` | 375x812 (Mobile) | Tampilan mobile iPhone: `Lifetime: 45d 13h │ Uptime: 3d 03h` tersusun presisi dalam kapsul stadium pill 304px. |
| `verify_uptime_text_desktop.png` | 1280x800 (Desktop) | Tampilan desktop: `Lifetime: 45d 13h │ Uptime: 3d 03h` di kanan atas header halaman selaras dengan label kartu System Uptime di halaman Overview. |

---


## 104. Standarisasi Eliminasi Nilai Dummy Fallback pada Parser Unique Beacons & Spesifikasi Biner Service Data 0x16 (`ibeacon_ext`) (2026)

### 104.1. Latar Belakang & Prinsip Integritas Data Enterprise
- **Zero-Mock Policy di Lingkungan Operasional**: Antarmuka diagnostik industrial dilarang keras memunculkan nilai mock/dummy hardcoded (seperti UUID tiruan `CB10023F-A318-3394-4199-A8730C7C1AEC`, Major `800`, Minor `1277`, URL dummy `nasipadang.com`, suhu `27.4 °C`, dsb.) ketika decoding frame gagal atau data tidak dipancarkan oleh beacon.
- **Standarisasi Representasi Field Kosong**: Setiap field yang tidak tersedia di payload mentah wajib dirender menggunakan simbol tanda strip (`-`) dengan warna redup (`var(--text-dim)` / `#64748b`), mencegah kesalahpahaman operator bahwa beberapa perangkat berbeda memancarkan data telemetri yang sama.

### 104.2. Spesifikasi Biner C++ Minew Format B (`ibeacon_ext`)
- **Struktur AD Type `0x16` (Service Data 16-bit UUID + Baterai $\ge 22$ Byte)**:
  - Byte `0..15`: 16-byte Proximity UUID asli.
  - Byte `16..17`: Major ID 16-bit (Big-Endian).
  - Byte `18..19`: Minor ID 16-bit (Big-Endian).
  - Byte `20`: Measured Tx Power (dBm, signed 8-bit).
  - Byte `21`: Persentase Baterai (0..100%).
- **Klasifikasi Pipeline**:
  - Ditetapkan sebagai `BeaconType::IBEACON_EXT` dengan kode `"ibeacon_ext"`.
  - Terdaftar sebagai format resmi (`isKnown == true`) sehingga otomatis lolos dari filter *Reject Unknown BLE Format*.

### 104.3. Parser Dinamis Frontend (`test.js`)
- **Ekstraksi Multi-Varian**:
  1. *Standar Apple iBeacon (`4C000215`)*: Ekstrak UUID, Major, Minor, dan TxPower dari offset payload 0x4C000215.
  2. *Minew Format B (Service Data `0x16`)*: Ekstrak UUID (16B), Major (2B), Minor (2B), TxPower (1B) dari sub-struktur AD Type `0x16`.
  3. *Fallback Tag ESP32*: Jika raw hex terpotong namun ESP32 mengirimkan `tag` format `Major-Minor` (`XXXXX-YYYYY`), pisahkan nilai Major dan Minor dari tag.
  4. *Nilai Default*: Semua parameter yang tidak ada diisi `"-"` tanpa dummy palsu.

### 104.4. Bukti Verifikasi Browser & Audit UI/UX Standar Enterprise

| File Screenshot | Viewport / Resolusi | Hasil Audit Visual & Verifikasi Data |
|---|---|---|
| `verify_unique_beacons_desktop.png` | 1280x800 (Desktop) | Kartu `AA:BB:CC:11:22:33` (iBeacon: Major 10001, Minor 205), kartu `DD:EE:FF:44:55:66` (Minew ibeacon_ext: Major 20002, Minor 408, Baterai 85%), dan kartu `11:22:33:77:88:99` (Raw BLE: field tak dikenal tampil bersih sebagai `-`). Tidak ada nilai dummy/duplikat antarkartu. |
| `verify_unique_beacons_mobile.png` | 375x812 (Mobile) | Tampilan kartu Unique Beacons terlipat & terbuka responsif pada layar mobile. Monospace MAC dan hex payload wrapping rapi tanpa clipping, padding 12px seimbang, kontras WCAG AAA. |

---

## 105. Implementasi Dynamic Multi-Protocol Decoders untuk 13 Protokol SCHEMA & Isolasi Apple Continuity (2026)

### 105.1. Pengetatan Klasifikasi Firmware C++ (`src/core/beacon_pipeline.cpp`)
- **Isolasi Apple Continuity**: Mengubah syarat evaluasi Manufacturer Data Apple `0x004C` subtipe `0x01` dari `dataLen >= 4` menjadi `dataLen >= 23`.
- **Hasil**: Paket kontinuitas background dari perangkat Apple (iPhone, iPad, Mac) yang berukuran 19-21 byte kini tetap diklasifikasikan sebagai `BeaconType::RAW_BLE` (kode `"BLE"`) dan dirender di kartu *Raw BLE Data*, menghentikan misklasifikasi sebagai *Minew iBeacon_ext*.

### 105.2. Ekstraksi Biner Dinamis Seluruh 13 Protokol pada `SCHEMA` (`test.js`)
- **Eddystone-UID (`AAFE00`)**: Ekstrak Ranging (1B signed), Namespace ID (10B hex), dan Instance ID (6B hex) langsung dari raw byte.
- **Eddystone-URL (`AAFE10`)**: Decode URL Scheme Prefix (`http://www.`, `https://www.`, `http://`, `https://`) beserta ekspansi kode domain (`.com/`, `.org/`, dsb.).
- **Eddystone-TLM (`AAFE20`)**: Ekstrak Tegangan Baterai (mV), Temperatur sensor (fixed-point 8.8 °C), Advertising Count (32-bit), dan Uptime perangkat.
- **MOKO iBeacon Info (`ABFE50`)**: Ekstrak Ranging (1B), Interval pancaran (1B * 100 ms), 16-Byte UUID, Major (2B Big-Endian), Minor (2B Big-Endian), dan TxPower dari AD Type `0x0A`.
- **MOKO BXP Alarm (`E0FE20`)**: Ekstrak Mode, Status trigger (Normal/Triggered), Trigger Count (2B Big-Endian), Device ID, dan nama perangkat dari struktur AD Type `0x09`.
- **MOKO Sensor / Device / Acc / TH (`0x40`, `0x60`, `0x70`)**: Ekstrak akselerometer 3 sumbu X/Y/Z, temperatur, kelembapan, dan persentase baterai.
- **Local Name Parser**: Iterasi AD Type `0x08` dan `0x09` untuk mengisi nama perangkat otomatis pada kartu yang memiliki field `name`.

### 105.3. Bukti Verifikasi Browser & Audit Standar Enterprise UI/UX

| File Screenshot | Viewport / Resolusi | Hasil Audit Visual & Verifikasi Data Multi-Protokol |
|---|---|---|
| `verify_all_protocols_desktop.png` | 1280x800 (Desktop) | Kartu `48:27:E2:1A:CA:E5` berhasil membongkar 3 sub-frame secara akurat: MOKO Alarm (`Count: 11`, `Name: BXP-B-D_E5`), Eddystone-UID (`Namespace: EDD1E001020304050607`, `Instance: 010203040503`), dan MOKO iBeacon (`Major: 10002`, `Minor: 1002`, `UUID: CB10023F...`). Kartu `50:40:90:0A:94:05` (Apple) tampil bersih sebagai *Raw BLE Data*. |
| `verify_all_protocols_mobile.png` | 375x812 (Mobile) | Tampilan mobile iPhone: Monospace MAC dan Namespace 20 karakter membungkus rapi (*responsive wrapping*), padding 12px seimbang, indikator status hijau tegas, dan kontras WCAG AAA. |




---

## 106. Penyelarasan Penuh Parser test.js Sesuai Acuan esp32_nrf_parsing (2026)

### 106.1. Harmonisasi Protokol MOKO BXP Alarm (`0xFEE0`)
- **Mode**: Diambil dari frame type byte `ad_data[2]` (hex chars `idx+4..idx+6`):
  - `0x20` $\rightarrow$ **`"Single Press"`**
  - `0x21` $\rightarrow$ **`"Double Press"`**
  - `0x22` $\rightarrow$ **`"Long Press"`**
  - `0x23` $\rightarrow$ **`"Inactivity"`**
  - Fallback: `Mode 0x%02X`
- **Status & Indikator Visual**: Diambil dari bitmask status flag `ad_data[3]` (hex chars `idx+6..idx+8`):
  - Bit 1 (`& 0x02`): **`"TRIGGERED (ALARM!)"`** (menyalakan dot merah berkedip & tag alert pada kartu).
  - Bit 2 (`& 0x04`): **`"SUB_TRIGGERED"`**.
  - Kondisi default: **`"Standby"`** (dot hijau tenang).
- **Password**: Bit 0 (`& 0x01`) dari status flag: **`"ON"`** jika aktif, **`"OFF"`** jika nonaktif.
- **Count**: 2 byte Big-Endian `ad_data[4..5]` $\rightarrow$ nilai integer bersih (contoh: `13`).
- **Device ID**: Tepat 3 byte hex `ad_data[6..8]` $\rightarrow$ `"000101"` (sebelumnya keliru terbawa 5 byte).
- **Firmware**: 1 byte hex `ad_data[9]` $\rightarrow$ `"0x00"`.
- **Name**: AD Type `0x09` (Complete Local Name) $\rightarrow$ `"BXP-B-D_E5"`.

### 106.2. Harmonisasi Seluruh 13 Protokol SCHEMA
- **Eddystone-UID**: Type `"UID"`, Namespace 10-byte hex (`EDD1E001020304050607`), Instance 6-byte hex (`010203040503`), Ranging signed dBm (`-20 dBm`).
- **MOKO iBeacon Info**: Interval waktu (`1000 ms`), Major (`10002`), Minor (`1002`), UUID 16-byte bertanda strip (`CB10023F-A318-3394-4199-A8730C7C1AEC`), Ranging (`-59 dBm`), Tx Power (`0 dBm` dari AD `0x0A`).
- **Raw BLE Data (Apple Continuity)**: Isolasi penuh paket Apple `0x004C01` dengan AD Type `0xFF`, PDU AdvType `0x04`, Length `21 bytes`. Tidak lagi mengalami false-positive sebagai Minew.

### 106.3. Bukti Verifikasi Browser & Audit Standar Enterprise UI/UX

| File Screenshot | Viewport / Resolusi | Hasil Audit Visual & Verifikasi Data Harmonized |
|---|---|---|
| `verify_harmonized_desktop.png` | 1280x800 (Desktop) | Kartu `48:27:E2:1A:CA:E5` menampilkan ketiga sub-frame dengan presisi sempurna: MOKO Alarm menampilkan *Mode: Single Press*, *Status: TRIGGERED (ALARM!)* (merah menyala), *Count: 13*, *Device ID: 000101*, *Firmware: 0x00*, *Password: OFF*, *Name: BXP-B-D_E5*. Eddystone-UID dan MOKO iBeacon Info terurai lengkap. Kartu `50:40:90:0A:94:05` tetap berada di Raw BLE Data. |
| `verify_harmonized_mobile.png` | 375x812 (Mobile) | Tampilan mobile iPhone: Seluruh field terformat rapi tanpa text clipping atau horizontal scrolling. Pill status alarm berkedip dengan kontras teks tajam, hex payload ter-wrap aman di dalam code container, tata letak grid dan tombol collapse/expand berfungsi normal. Memenuhi standar WCAG AAA. |


---

## 107. Penghapusan Tx Power & Penegasan Ranging Calibrated Payload (2026)

### 107.1. Koreksi SCHEMA & Isolasi Variabel Asli Acuan (`main.c`)
- **Penghapusan Baris Tx Power pada `ibeacon` & `eddy_url`**:
  - `SCHEMA.ibeacon`: Disederhanakan menjadi `[['uuid','UUID'],['major','Major'],['minor','Minor'],['ranging','Ranging']]`. Baris `Tx Power` resmi dihapus karena tidak ada variabel daya pancar pada paket iBeacon standar Apple.
  - `SCHEMA.eddy_url`: Disederhanakan menjadi `[['type','Type'],['url','URL'],['ranging','Ranging']]`. Baris `Tx Power` resmi dihapus sesuai acuan `esp32_nrf_parsing/src/main.c` (line 1324).
- **Alokasi Nilai Presisi**:
  - **iBeacon**: Byte ke-24 `d[24]` (`0xC5` signed $\rightarrow$ `-59 dBm`) murni dialokasikan ke field **`Ranging`** (Measured Power @ 1m), bukan lagi ke RSSI atau Tx Power.
  - **Eddystone-URL**: Byte ke-3 Service Data `ad_data[3]` (`0xE8` signed $\rightarrow$ `-24 dBm`) dialokasikan ke field **`Ranging`** (Calibrated Tx Power @ 0m).
  - **Minew ibeacon_ext**: Byte ke-20 dialokasikan ke **`Ranging`**, dan byte ke-21 ke **`Battery`** (`%u%`).

### 107.2. Bukti Audit Visual Browser Enterprise UI/UX

| File Screenshot | Viewport / Resolusi | Hasil Audit Visual & Verifikasi Data |
|---|---|---|
| `verify_ibeacon_cleaned_desktop.png` | 1280x800 (Desktop) | Kartu `C3:00:00:32:4B:DD` menampilkan frame `iBeacon` bersih dengan 4 field (*UUID*, *Major 800*, *Minor 1277*, *Ranging -59 dBm*) tanpa baris Tx Power. Frame `Eddystone-URL` menampilkan *Type: URL*, *URL: http://nasipadang.com*, dan *Ranging: -24 dBm* tanpa baris Tx Power. |
| `verify_ibeacon_cleaned_mobile.png` | 375x812 (Mobile) | Resolusi mobile 375px: Tata letak bersih, jarak baris seimbang, wrapping teks UUID dan URL tanpa horizontal scrollbar atau clipping. Memenuhi standar enterprise WCAG AAA. |


---

## 108. Penyelarasan Presisi Parsing MOKO Sesuai Dokumen Resmi Pabrikan & Penerbitan Dokumen Acuan Tunggal `parsing.md` (2026)

### 108.1. Koreksi Offset 1 Byte pada `moko_sensor` (`0x00EA`)
- **Akar Masalah**: Pada Service Data `0x00EA` (Scan Response General Device Info), Byte 02 adalah **Frame Type (`0x00`)**. Kode sebelumnya membaca mulai dari index UUID + 4 bytes sebagai `scaleCode` sehingga seluruh field setelahnya bergeser 1 byte (2 karakter hex) ke kiri.
- **Koreksi Presisi**:
  - `Scale`: Membaca byte ke-3 (`idxEa + 6..8`) $\rightarrow$ `0`: ±2g, `1`: ±4g, `2`: ±8g, `3`: ±16g.
  - `Motion Threshold`: Membaca byte ke-4..5 (`idxEa + 8..12`) $\rightarrow$ unit `mg`.
  - `Accel X, Y, Z`: Membaca berturut-turut byte ke-6..7, 8..9, 10..11 (`idxEa + 12..24`) sebagai signed 16-bit `mg`.
  - `Temperature`: Membaca byte ke-12..13 (`idxEa + 24..28`) $\rightarrow$ signed 16-bit `* 0.1` °C.
  - `Ranging`: Membaca byte ke-14 (`idxEa + 28..30`) $\rightarrow$ signed 8-bit dBm.
  - `Battery`: Membaca byte ke-15..16 (`idxEa + 30..34`). Sesuai spesifikasi resmi: jika nilai `> 100` adalah tegangan mV (dihitung persentase kurva discharge), jika `<= 100` adalah persentase langsung (`%`).

### 108.2. Formula Resmi 12-Bit Signed Accelerometer `moko_acc` (`0xFEAB 0x60`)
- **Implementasi Fungsi `calcMokoAcc(raw, s)`**:
  Sesuai *MOKO BeaconX Pro APP User Manual FAQ Section 5* dan implementasi acuan `esp32_nrf_parsing/src/main.c` (baris 201-207):
  $$\text{acc} = \begin{cases} \lfloor \text{raw} \gg 4 \rfloor \times s, & \text{jika } \text{raw} < \text{0x8000} \\ (\lfloor \text{raw} \gg 4 \rfloor - \text{0x1000}) \times s, & \text{jika } \text{raw} \ge \text{0x8000} \end{cases}$$
  Di mana faktor skala $s$ adalah:
  - Skala 0 (±2g): $s = 1$
  - Skala 1 (±4g): $s = 2$
  - Skala 2 (±8g): $s = 4$
  - Skala 3 (±16g): $s = 12$
- **Akurasi**: Nilai percepatan sumbu X, Y, dan Z kini presisi hingga satuan mg tanpa distorsi skala 16x.

### 108.3. Penegasan Kebijakan Nol-Mock & Alamat MAC
- Atribut `data.mac` selalu dipertahankan menggunakan alamat MAC pengirim paket BLE radio (`mac`) dari header frame. MAC tersemat di dalam payload hanya bersifat informatif dan tidak menimpa identitas perangkat pada antarmuka pengguna maupun primary key gateway.

### 108.4. Dokumen Acuan Tunggal `parsing.md`
- Dokumen acuan resmi [parsing.md](file:///C:/Users/syafr/OneDrive/Documents/PlatformIO/Projects/esp32_ibeacon_espidf/parsing.md) telah diterbitkan di root workspace mencakup standar lengkap 13 protokol BLE, byte map, formula matematis, schema UI, dan vektor uji regresi untuk menjamin konsistensi lintas platform.

### 108.5. Bukti Audit Visual Browser Enterprise UI/UX

| File Screenshot | Viewport / Resolusi | Hasil Audit Visual & Verifikasi Data |
|---|---|---|
| `verify_moko_aligned_desktop_frames.png` | 1280x800 (Desktop) | Kartu `C8:1E:02:01:06:1A` (iBeacon) bersih tanpa Tx Power (*UUID, Major 800, Minor 1277, Ranging -59 dBm*). Kartu `CD:19:02:01:06:03` (Eddystone-URL) bersih tanpa Tx Power (*Type: URL, URL: http://nasipadang.com, Ranging -24 dBm*). |
| `verify_moko_aligned_moko_card.png` | 1280x800 (Desktop) | Kartu `E5:00:C8:1C:02:01` menampilkan sub-frame MOKO dengan presisi sempurna: MOKO Alarm (*Status: TRIGGERED (ALARM!)* warna merah menyala dengan count 13), MOKO Sensor Info (*Battery: 3276 mV (100%), Temp: 10.8 °C, Accel: X:40 Y:-124 Z:984 mg, Scale: ±2g, Threshold: 16 mg, Ranging: -59 dBm, MAC: E5:00:C8:1C:02:01*). |
| `verify_moko_aligned_mobile.png` | 375x812 (Mobile) | Resolusi mobile 375px iPhone: Spasi elemen proporsional, font tajam dengan kontras enterprise WCAG AAA, tidak ada teks atau tombol yang terpotong/overflow. |

## 109. Rekonstruksi Enterprise Responsive Matrix Tab Dropped Data, Capping Durabilitas Throughput Tinggi 10 Data, dan Click-to-Filter Interaktif (2026)

### 1. Latar Belakang & Kebutuhan User
Pada tab **Dropped Data** di halaman `test.html`, KPI ringkasan sebelumnya belum sepenuhnya merepresentasikan ke-7 alasan drop filter firmware secara seimbang dan tidak responsif. User menginstruksikan pembaruan berstandar enterprise:
1. **Zero Horizontal Scroll**: Seluruh data dan kartu harus tampil langsung dalam *page view* tanpa scroll horizontal baik di desktop maupun mobile.
2. **Geometri 8-Cell Genap Mobile**: Memasukkan `TOTAL REJECTED` sebagai salah satu sel kartu sehingga pada layar mobile membentuk grid 2 kolom x 4 baris = 8 sel genap tanpa tombol semu/dummy filler.
3. **Desktop Mode Spanning Header**: Pada mode desktop (>1024px), kartu `TOTAL REJECTED` tetap berada di posisi atas membentang penuh (*full-width span 1 / -1*), dengan 7 kartu alasan drop berbaris rapi tepat di bawahnya dalam 1 baris 7 kolom.
4. **Capping 10 Data & Throttling Durabilitas Tinggi**: Capping buffer dan rendering tabel ditahan ketat pada maksimal 10 data dropped serta didecouple menggunakan `requestAnimationFrame` agar sistem UI browser tetap kokoh, ringan, dan tidak mengalami *layout thrashing* meskipun dihujani ribuan paket per detik.
5. **Verifikasi Kolom Tabel**: Memastikan ke-8 kolom tabel dropped data (`#`, `Timestamp`, `MAC Address`, `Frame Type`, `Identifier / Payload`, `RSSI`, `Drop Reason`, `Raw 42B`) relevan dan mencakup seluruh alasan drop biner firmware.

---

### 2. Arsitektur Responsive Grid 8-Cell

```
Desktop (> 1024px):
+---------------------------------------------------------------------------------------------------+
|  [● TOTAL REJECTED]                                         [ 14,320 pkt ]   [ 100% Ingress Rate ] |
+---------------------------------------------------------------------------------------------------+
| RSSI < MIN | UUID MISMATCH | MAJOR MISMATCH | UNKNOWN FORMAT | QUEUE FULL | MQTT FAIL | TTL EXPIRED   |
+---------------------------------------------------------------------------------------------------+

Tablet (641px - 1024px) [4 x 2 = 8 Sel]:
+---------------------+---------------------+---------------------+---------------------+
| TOTAL REJECTED      | RSSI < MIN          | UUID MISMATCH       | MAJOR MISMATCH      |
+---------------------+---------------------+---------------------+---------------------+
| UNKNOWN FORMAT      | QUEUE FULL          | MQTT FAIL           | TTL EXPIRED         |
+---------------------+---------------------+---------------------+---------------------+

Mobile (<= 640px) [2 x 4 = 8 Sel]:
+---------------------------+---------------------------+
| TOTAL REJECTED            | RSSI < MIN                |
+---------------------------+---------------------------+
| UUID MISMATCH             | MAJOR MISMATCH            |
+---------------------------+---------------------------+
| UNKNOWN FORMAT            | QUEUE FULL                |
+---------------------------+---------------------------+
| MQTT FAIL                 | TTL EXPIRED               |
+---------------------------+---------------------------+
```

---

### 3. Pemetaan Drop Reason & Skema Warna Enterprise

| Reason ID | Reason Code (Firmware) | CSS Card Class | Dot / Val Color | Header Badge Class | Deskripsi Alasan Drop |
| :---: | :--- | :--- | :--- | :--- | :--- |
| **All** | Aggregate Anchor | `card-total` | Amber (`#f59e0b`) | `.drop-active-filter-badge` | Total seluruh paket beacon yang ditolak gateway |
| **1** | `RSSI_OUT_OF_RANGE` | `card-rssi` | Cyan (`#06b6d4`) | `.badge-drop.drop-rssi` | Nilai RSSI beacon lebih lemah dari ambang batas minimum |
| **2** | `UUID_MISMATCH` | `card-uuid` | Indigo (`#6366f1`) | `.badge-drop.drop-uuid` | Proximity UUID tidak cocok dengan target filter gateway |
| **3** | `MAJOR_MISMATCH` | `card-major` | Purple (`#a855f7`) | `.badge-drop.drop-major` | Nilai Major beacon tidak cocok dengan konfigurasi gateway |
| **6** | `UNKNOWN_FORMAT` | `card-unknown` | Sky (`#0284c7`) | `.badge-drop.drop-unknown` | PDU format tidak dikenali / parsing payload gagal |
| **4** | `QUEUE_OVERFLOW` | `card-queue` | Blue (`#3b82f6`) | `.badge-drop.drop-queue` | Buffer antrean FreeRTOS penuh saat traffic ingress ekstrem |
| **5** | `MQTT_FAIL` | `card-mqtt` | Red (`#ef4444`) | `.badge-drop.drop-mqtt` | Kegagalan transmisi uplink client MQTT (offline/broken pipe) |
| **0** | `TTL_EXPIRED` | `card-ttl` | Rose (`#f43f5e`) | `.badge-drop.drop-ttl` | Waktu simpan paket melebihi batas deduplikasi TTL (stale) |

---

### 4. Implementasi High-Throughput Durability (10-Item Cap & RAF Throttle)

```javascript
// test.js
const DROPPED_MAX_ITEMS = 10;
let droppedBeaconsList = []; // strictly capped to 10
let dropTableRenderScheduled = false;

function scheduleDropTableRender() {
  if (dropTableRenderScheduled) return;
  dropTableRenderScheduled = true;
  requestAnimationFrame(() => {
    dropTableRenderScheduled = false;
    if (activeTableTab === "dropped") {
      renderTable();
    }
  });
}
```

Ketika paket dropped datang melalui WebSocket (`msg.dropped_beacons`):
1. Elemen baru dimasukkan di awal (`unshift`).
2. Array langsung dipotong (`if (droppedBeaconsList.length > DROPPED_MAX_ITEMS) droppedBeaconsList.length = DROPPED_MAX_ITEMS;`).
3. Render dijadwalkan via `scheduleDropTableRender()`, mengeliminasi overhead render beruntun pada frekuensi ratusan hingga ribuan paket per detik.


---

## 121. Standardisasi Format Deskripsi L-T-V (Length-Type-Value) Protokol BLE & Zero-Overflow LittleFS

### 1. Anatomi L-T-V BLE pada Kolom Deskripsi Kompak
Mengikuti standar Bluetooth Core Specification, setiap frame Advertising Data (maksimal 31 byte) tersusun atas satu atau beberapa struktur **AD Element** dengan susunan baku **L-T-V (Length - Type - Value)**:
1. **Length (1 Byte)**: Ukuran total payload berikutnya (`1 Byte Type + N Bytes Value`).
2. **AD Type (1 Byte)**: Identifier resmi Bluetooth SIG (contoh: `0x01` Flags, `0x0A` Tx Power Level, `0x16` Service Data 16-bit UUID, `0xFF` Manufacturer Specific Data).
3. **Value / Data (N Bytes)**: Muatan nilai aktual.

### 2. Standardisasi Deskripsi Granular untuk Engineer
Untuk menjembatani kebutuhan engineer firmware/integrasi tanpa memecah baris tabel (mempertahankan 1 row per blok semantik demi mematuhi plafon memori flash LittleFS <= 85.864 bytes):
- **Flags AD (3B)**: `Len (0x02), Type (0x01 Flags), Val (0x06): LE Discoverable.`
- **Tx Power AD (3B)**: `Len (0x02), Type (0x0A Tx Power), Val (0x00): Tx Power 0 dBm.`
  - *Catatan Engineer*: Penjelasan ini memperjelas secara definitif mengapa nilai Tx Power berada di **Byte 05** pada frame standar (Flags `00..02` + Tx Power Header `03..04` + Tx Value `05`), atau berada di **Byte 02** pada protokol seperti MOKO iBeacon Info yang menempatkan blok Tx Power langsung di awal frame (`Byte 00` Length `0x02`, `Byte 01` Type `0x0A`, `Byte 02` Value `0x00`).
- **Service Data Header (2B)**: `Len (0x12: 18B data), Type (0x16: Service Data 16-bit UUID).`
- **Manufacturer Specific Header (2B)**: `Len (0x1A: 26B data), Type (0xFF: Manufacturer Data).`

### 3. Arsitektur Kompresi Flash & Rekayasa Payload
- **Snippet Comment Stripping & 2-Space Indent**: Eliminasi komentar blok/baris pada snippet Node.js, Python, Go, Rust menghemat 2.676 bytes.
- **Type Array Mapping (`TYPES_LIST`)**: Pemetaan string tipe data tabel ke integer index menghemat 930 bytes.
- **Rendering Dinamis**: Komponen JavaScript memetakan kembali integer type code ke label badge `TYPES[f[3]]` pada saat runtime, menjamin integritas tampilan 100%.
- **Ukuran File Terjaga**: `docs.html` terkompilasi pada **85.220 bytes** dengan line ending murni Unix LF (`\n`, 0 `\r`), diverifikasi lolos kompilasi LittleFS partition (`pio run -t buildfs`) dengan exit code 0 (`[SUCCESS]`).


---

## 122. Arsitektur Pemisahan Folder Sumber (`web_src/`) vs Distribusi (`data/`) & Kebijakan Pure Gzip Assets

### 1. Latar Belakang & Eliminasi Redundansi Memori Flash
Sebelumnya, folder `data/` menyimpan file mentah uncompressed dan file `.gz` secara bersamaan (`app.css` 134 KB + `app.css.gz` 22 KB, `test.js` 120 KB + `test.js.gz` 27 KB, dsb.), yang memboroskan > 400 KB kapasitas partisi LittleFS (1 MB total) dan menyebabkan partisi hampir penuh (terisi 952 KB).

### 2. Standar Dua Folder: Source vs Distribution
Mengadopsi standar arsitektur enterprise perangkat embedded (Cisco, Ubiquiti, Mikrotik):
1. **Folder Sumber Tunggal (`web_src/`)**:
   - Berisi seluruh kode sumber asli (HTML, CSS, JavaScript, gambar, dan metadata JSON).
   - Menjadi satu-satunya folder yang diedit oleh tim pengembang/engineer.
   - Bersih dari file kompresi `.gz` (100% human-readable source code).
2. **Folder Distribusi Ramping (`data/`)**:
   - Berfungsi sebagai folder output kompilasi (*distribution build*) yang ditargetkan oleh PlatformIO `mklittlefs`.
   - **Kebijakan Pure Gzip Assets**: Pada direktori `data/assets/css/` dan `data/assets/js/`, **HANYA** menyimpan file `.gz`. Seluruh file mentah `.css` dan `.js` dieliminasi 100%.
   - File `.html` disinkronkan dengan normalisasi murni Unix LF (`\n`, zero `\r`) agar template engine C++ firmware dapat memproses placeholder konfigurasi secara lancar.

### 3. Pipeline Build Otomatis (`scripts/build_web_assets.py`)
Setiap kali perubahan antarmuka dilakukan di `web_src/`, pipeline otomatis dijalankan:
`python scripts/build_web_assets.py`
Tugas pipeline:
- Menormalisasi line ending seluruh file teks ke Unix LF.
- Menyinkronkan file HTML dan aset statis ke `data/`.
- Mengompresi seluruh file CSS dan JS menggunakan gzip level 9 (`compresslevel=9`).
- Menghapus residu file mentah `.css` dan `.js` dari folder `data/assets/`.

### 4. Hasil Efisiensi Memori Flash LittleFS
- **Ukuran Direktori data/**: Anjlok dari 952.378 bytes menjadi **540.724 bytes (~528 KB)**.
- **Penghematan Bersih**: **315 KB - 403 KB memori flash dibebaskan**.
- **Kapasitas Bebas (Headroom)**: Tersisa **507.852 bytes (~508 KB / ~49% partisi bebas)**.
- **Integritas Firmware**: Diverifikasi lolos kompilasi LittleFS partition (`pio run -t buildfs`) dengan exit code 0 (`[SUCCESS]`).
