# PANDUAN PENGUJIAN STANDALONE LORA SX1278 (TX & RX) [v2.0]
## Subkolektif: Node WC (Bilik Sanitasi) — Proyek Smart-Sanitation eSOS (Kelompok 4 FTUI)

---

## 1. Ringkasan Eksekutif & Batas Pengujian

Dokumen ini adalah panduan teknis resmi pengujian mandiri (*standalone test*) untuk subsistem komunikasi telemetri nirkabel LoRa pada **Node WC**:
- **Modul Radio**: Ai-Thinker Ra-02 (Semtech SX1278).
- **Mikrokontroler Target**: DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa LX6 32-bit).
- **Library Driver**: `RadioLib` (v6.6.0) by Jan Gromes.
- **Framework**: Arduino-ESP32 v2.0.17 (ESP-IDF v4.4) dan native Espressif FreeRTOS.
- **Struktur Folder Terstandarisasi**:
  - `standalone_test/test_lora_node_tx/`: Uji mandiri pemancar (*transmitter* uplink telemetri).
  - `standalone_test/test_lora_node_rx/`: Uji mandiri penerima (*receiver* downlink actuator & uplink monitor).

### Batas Lingkup Pengujian Standalone (v2.0):
1. **Fokus Subsistem**: Memvalidasi integritas komunikasi bus SPI perangkat keras, modulasi RF Semtech SX1278, interupsi hardware DIO0, antrean (queue) FreeRTOS, dan pemrosesan kontrak payload biner.
2. **Isolasi Subsistem**: Aktuator servo valve, sensor ultrasonik JSN-SR04T, dan sensor gas MQ tidak diaktifkan pada pengujian ini.
3. **Kepatuhan Regulasi Spektrum**: Mengikuti alokasi pita frekuensi Indonesia berdasarkan **Permenkomdigi No. 2 Tahun 2025** (Pita LPWAN/SRD 433 MHz).

---

## 2. Parameter Modulasi & Regulasi Spektrum Radio

| Parameter | Nilai Konfigurasi | Dasar Regulasi / Pertimbangan Teknis |
| :--- | :--- | :--- |
| **Frekuensi Tengah** | **433.175 MHz** | Permenkomdigi No. 2 Tahun 2025: Alokasi pita 433,050 – 434,790 MHz. |
| **Bandwidth (BW)** | **125.0 kHz** | Batas bandwidth kanal maksimum yang diizinkan regulasi Indonesia. |
| **Spreading Factor (SF)** | **SF9** | Titik kompromi optimal antara sensitivitas link budget ($SNR \approx -12.5\text{ dB}$) dan Time-on-Air (ToA). |
| **Coding Rate (CR)** | **4/7 (CR = 3)** | Perlindungan FEC terhadap degradasi akibat derau interferensi lingkungan bilik sanitasi. |
| **Sync Word** | **0x12** | Isolasi jaringan privat untuk mencegah crosstalk dengan jaringan LoRaWAN publik (0x34). |
| **Conducted Power** | **+10 dBm (10 mW)** | Kepatuhan terhadap batas EIRP maksimum 12,15 dBm (setara ERP 10 dBm) dengan antena default +2 dBi. |

---

## 3. Tabel Pengkabelan Jumper Fisik (Wiring Harness)

Modul Ai-Thinker Ra-02 dihubungkan ke DOIT ESP32 DevKit V1 menggunakan konfigurasi pin berikut:

| Pin Ra-02 (SX1278) | Pin ESP32 DevKit V1 | Warna Kabel Fisik | Deskripsi Perangkat Keras |
| :--- | :---: | :---: | :--- |
| **3.3V** (Kiri Pin 3) | **3V3** (Kanan Pin 1) | Putih | Catu Daya 3.3V Stabil (**DILARANG 5V/VIN!**) |
| **GND** (Kiri Pin 2) | **GND** (Kanan Pin 2) | Hitam | Ground Bersama (*Common Ground*) |
| **RST** (Kiri Pin 4) | **D15 (GPIO 15)** | Biru | Reset Hardware SX1278 (Aktif LOW) |
| **DIO0** (Kiri Pin 5) | **D2 (GPIO 2)** | Ungu | Interupsi Hardware `RX_DONE` / `TX_DONE` |
| **NSS** (Kanan Pin 2) | **D5 (GPIO 5)** | Kuning | SPI Slave Select / Chip Select (CS) |
| **MOSI** (Kanan Pin 3) | **D18 (GPIO 18)** | Oranye | SPI Master Out Slave In |
| **MISO** (Kanan Pin 4) | **D19 (GPIO 19)** | Merah | SPI Master In Slave Out |
| **SCK** (Kanan Pin 5) | **D21 (GPIO 21)** | Cokelat | SPI Clock Bus (Dikonfigurasi 1 MHz untuk kabel jumper) |

> [!CAUTION]
> Chip Semtech SX1278 **hanya mentoleransi tegangan 3.3V**. Menghubungkan pin VCC Ra-02 ke rel 5V/VIN akan merusak modul secara permanen.

---

## 4. Kontrak Struktur Payload Biner

### 4.1 TelemetryPayload (34 Bytes — Uplink)
Struktur telemetri uplink yang dikirim oleh `test_lora_node_tx`:
```cpp
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte  : Skema biner (= 1)
    char node_code[8];      // Offset  1 | 8 bytes : Kode node ("WC_01\0\0\0")
    uint32_t sequence_no;   // Offset  9 | 4 bytes : Nomor urut paket
    uint32_t uptime_seconds;// Offset 13 | 4 bytes : Waktu operasional (s)
    float water_level_cm;   // Offset 17 | 4 bytes : Level air tangki
    float ammonia_ppm;      // Offset 21 | 4 bytes : Konsentrasi NH3
    float h2s_ppm;          // Offset 25 | 4 bytes : Konsentrasi H2S
    float battery_voltage;  // Offset 29 | 4 bytes : Tegangan sel Li-ion
    uint8_t sos_triggered;  // Offset 33 | 1 byte  : Flag status darurat
};
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: TelemetryPayload harus 34 bytes!");
```

### 4.2 ActuatorCommand (10 Bytes — Downlink)
Struktur perintah kendali aktuator yang diterima oleh `test_lora_node_rx`:
```cpp
struct __attribute__((packed)) ActuatorCommand {
    char node_code[8]; // Target node code ("WC_01\0\0\0")
    uint8_t command_id; // 1 = LOCK_DOOR, 2 = UNLOCK_DOOR, 3 = FLUSH, 4 = PING
    uint8_t parameter;  // Sudut servo (0-180) atau parameter kontrol
};
static_assert(sizeof(ActuatorCommand) == 10, "FATAL: ActuatorCommand harus 10 bytes!");
```

---

## 5. Arsitektur FreeRTOS & Kepatuhan Espressif

### 5.1 Arsitektur Transmitter (`test_lora_node_tx`)
- **Task Producer (`vTaskGenerateTelemetry`)**: Berjalan di **Core 0** (Prioritas 1) menghasilkan paket simulasi setiap 5.000 ms dan memasukkannya ke antrean `xLoRaQueue`.
- **Task Transmitter (`vTaskLoRaTransmitter`)**: Berjalan di **Core 1** (Prioritas 2) memblokir antrean secara pasif (*zero CPU spin*), melakukan transmisi sinkronus via chip SX1278, dan menghitung durasi transmisi nyata vs estimasi Time-on-Air (ToA).

### 5.2 Arsitektur Receiver (`test_lora_node_rx`)
- **ISR Hardware (`isr_lora_rx`)**: Terpanggil oleh sinyal RISING pada pin **GPIO2 (DIO0)** saat paket radio selesai diterima. Mengirimkan notifikasi task via `vTaskNotifyGiveFromISR()`.
- **Task Radio Receiver (`vTaskLoRaRx`)**: Berjalan di **Core 1** (Prioritas 3) menunggu notifikasi interupsi, membaca paket dari buffer internal SX1278, mencatat metrik RF (RSSI, SNR, Frequency Error), dan mengirim ke antrean paket `xQueueRxPackets`.
- **Task Processor & Logger (`vTaskProcessor`)**: Berjalan di **Core 0** (Prioritas 1) sebagai *sole-writer* Serial UART. Mendekode payload biner sesuai kontrak data, menyaring apakah paket ditujukan untuk `WC_01`, dan memonitor penggunaan stack watermark dalam satuan byte.

---

## 6. Prosedur Eksekusi & Uji Hardware

### Menggunakan PlatformIO Core CLI:
```powershell
# 1. Kompilasi & Upload Standalone Transmitter:
pio run -d src/firmware/node_wc -e test_lora_node_tx --target upload

# 2. Buka Serial Monitor Transmitter:
pio device monitor -d src/firmware/node_wc -b 115200

# 3. Kompilasi & Upload Standalone Receiver (pada ESP32 kedua):
pio run -d src/firmware/node_wc -e test_lora_node_rx --target upload

# 4. Buka Serial Monitor Receiver:
pio device monitor -d src/firmware/node_wc -b 115200
```

### Menggunakan Arduino IDE:
1. Buka file `.ino` dari direktori mirror:
   - `C:\Users\dapah\Documents\Arduino\test_lora_node_tx\test_lora_node_tx.ino`
   - `C:\Users\dapah\Documents\Arduino\test_lora_node_rx\test_lora_node_rx.ino`
2. Pilih Board: **DOIT ESP32 DEVKIT V1**.
3. Pastikan library **RadioLib** (v6.6.0) telah terinstal melalui Library Manager.
4. Upload dan amati output pada Serial Monitor pada baud rate **115200 bps**.
