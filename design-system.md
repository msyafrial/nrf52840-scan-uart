# Design System — ESP32 Real-Time BLE Beacon Dashboard (Vertical MAC-Centric)

Modern, dark-themed, glassmorphic design system tailored for real-time BLE beacon monitoring on ESP32 web server.
**Layout Pattern**: 1 Card per MAC Address, Vertical Stack (list ke bawah), Multi-Frame Rows per Card.

---

## 1. Color Palette

### 1.1 Base Theme (Dark Space Slate)
- **Background Root**: `#0a0e17` (Deep obsidian navy)
- **Background Secondary**: `#111827` (Charcoal slate)
- **Card Background**: `rgba(17, 24, 39, 0.78)` with `backdrop-filter: blur(16px)`
- **Card Background Hover**: `rgba(26, 35, 52, 0.90)`
- **Row Sub-card / Slot**: `rgba(15, 23, 42, 0.65)` with border `rgba(255, 255, 255, 0.05)`
- **Border Default**: `rgba(255, 255, 255, 0.08)`
- **Border Highlight**: `rgba(99, 102, 241, 0.45)` (Electric Indigo glow)

### 1.2 Accent & Protocol Badges
- **Primary / Brand**: `#6366f1` (Electric Indigo)
- **Primary Glow**: `rgba(99, 102, 241, 0.25)`
- **iBeacon (Apple Standard)**: Background `rgba(168, 85, 247, 0.15)`, Border `rgba(168, 85, 247, 0.4)`, Text `#d8b4fe`
- **ibeacon_ext (Minew Format B)**: Background `rgba(14, 165, 233, 0.15)`, Border `rgba(14, 165, 233, 0.4)`, Text `#38bdf8` (Sky Blue)
- **MOKO Tag Info (BXP Tag 0xEA01 / Sensors & Telemetry)**: Background `rgba(236, 72, 153, 0.15)`, Border `rgba(236, 72, 153, 0.4)`, Text `#f472b6` (Pink Rose)
- **Minew Device Info**: Background `rgba(59, 130, 246, 0.15)`, Border `rgba(59, 130, 246, 0.4)`, Text `#93c5fd`
- **Eddystone TLM/UID**: Background `rgba(6, 182, 212, 0.15)`, Border `rgba(6, 182, 212, 0.4)`, Text `#67e8f9`
- **Unknown / Custom**: Background `rgba(245, 158, 11, 0.15)`, Border `rgba(245, 158, 11, 0.4)`, Text `#fcd34d`

### 1.3 Signal Strength (RSSI) Tokens
- **Good (>= -65 dBm)**: Emerald `#10b981` (bg: `rgba(16, 185, 129, 0.12)`)
- **Medium (-66 to -80 dBm)**: Amber `#f59e0b` (bg: `rgba(245, 158, 11, 0.12)`)
- **Poor (< -80 dBm)**: Red `#ef4444` (bg: `rgba(239, 68, 68, 0.12)`)

---

## 2. Layout Structure (1 Card = 1 MAC)

```text
+---------------------------------------------------------------------------------------------+
| [MAC] FC:E8:C0:12:34:56  | Device: Moko Smart Tag | Total Packets: 1,240 | Last Seen: 1s ago |
+---------------------------------------------------------------------------------------------+
|  [> RAW] | [iBeacon]          | -59 dBm | UUID: FDA50693-... | Maj: 10001 | Min: 205 | Tx: -59 |
|  [> RAW] | [MOKO Tag Info]    | -39 dBm | TagID: 1 | Batt: 2850 mV (81%) | Hall: Absent | Motion: None | Accel: X:0 Y:0 Z:0 |
|  [> RAW] | [ibeacon_ext]       | -60 dBm | UUID: CB10023F-... | Maj: 10001 | Min: 1001 | Batt: 2850 mV |
+---------------------------------------------------------------------------------------------+
```

### 2.1 Card Container
- Direction: Vertical stack (`flex-direction: column`, `gap: 16px`).
- Width: Full width single column (`max-width: 1200px; margin: 0 auto;`).
- Elevation: Rounded `16px`, subtle border, backdrop blur `16px`.

### 2.2 Frame Row (Sub-item)
- Display: Flexbox / Grid row with columns:
  - Column 1: `[> RAW]` toggle button (Collapsible raw 42-byte hex drawer).
  - Column 2: Protocol Type Pill (`iBeacon`, `Moko`, `Minew`, `Device Info`, `Unknown`).
  - Column 3: Live RSSI Pill (`-59 dBm` with color indicator).
  - Column 4: Parsed Data Summary (Key-value tags, sensor data, UUID/Major/Minor).
  - Column 5: Status / Last Updated ticker.
- Animation: Row flash highlight (`@keyframes rowUpdatePulse`) when updated.
