# Smart-Sanitation eSOS 🚀
### Emergency Sanitation Operating System — Modular & Offline-First

![Build Status](https://img.shields.io/badge/firmware-passing-brightgreen)
![Go Backend](https://img.shields.io/badge/backend-Go%201.21-blue)
![Database](https://img.shields.io/badge/database-PostgreSQL%2015%2B-indigo)
![LoRa Compliance](https://img.shields.io/badge/LoRa-Permenkomdigi%20No.2%2F2025-orange)
![RTOS](https://img.shields.io/badge/RTOS-ESP--IDF%20FreeRTOS-red)

---

## 📖 Deskripsi Singkat

**Smart-Sanitation eSOS** (*Emergency Sanitation Operating System*) adalah sistem rekayasa sanitasi cerdas modular dan *Offline-First* yang dirancang khusus untuk fasilitas tanggap darurat pasca-bencana di wilayah terisolir (*blank spot* tanpa internet). Sistem ini mengkombinasikan teknologi radio jarak jauh **Semtech SX1278 LoRa (433 MHz)** dengan arsitektur server berkinerja tinggi **Golang**, **PostgreSQL ACID**, dan antarmuka web pemantauan darurat real-time (*Emergency Management System / EMS*).

Sistem dikembangkan oleh **Kelompok 4 - Desain Proyek 2, Departemen Teknik Elektro, Fakultas Teknik Universitas Indonesia (2026)**.

---

## 🗂️ Struktur Repositori Terstruktur

Repositori ini distrukturisasi secara tegas memisahkan **Keperluan Manajemen & Administrasi** dengan **Keperluan Teknis Rekayasa**:

```
DESPRO2_SMART_SANITATION_MODULAR/
│
├── management/                        📋 [MANAJEMEN] Tata Kelola, WBS, BOM & Laporan
│   ├── README.md                      # Indeks Navigasi Manajemen
│   ├── 01_MASTER_TASK_ALLOCATION.md   # Matriks RACI & Pembagian Beban Kerja Tim
│   ├── 02_BOM_PROCUREMENT.md          # Bill of Materials & Tracking Pengadaan Suku Cadang
│   ├── 03_MASTER_SEMESTER_WBS.md      # Work Breakdown Structure (WBS) & Jadwal Semester
│   ├── 04_PHASE_GATE_AND_TRACEABILITY.md # Phase Gate Review & Traceability Matrix
│   ├── 05_EVIDENCE_REGISTER.md        # Register Bukti Pengujian, Bench Testing & Lab Photos
│   └── reports/                       # Arsip Laporan Kemajuan Berkala (Weekly Reports)
│       ├── group/                     # Laporan Kelompok Mingguan (Week 01 - 04)
│       ├── individual/                # Laporan Individu per Anggota (Daffa, Darrel, Ilman, Raka, Siti)
│       └── media/                     # Dokumentasi Foto & Visual Laboratorium
│
├── docs/                              📚 [TEKNIS] Arsitektur Sistem & Spesifikasi Rekayasa
│   ├── architecture/                  # Architecture Decision Records (ADR 00 - 06)
│   ├── hardware_references/           # Lembar Data Komponen, Wiring & Kalibrasi Sensor
│   └── AGENT_IMPLEMENTATION_BRIEF.md  # Ringkasan Spesifikasi Teknis Implementasi
│
└── src/                               ⚙️ [TEKNIS] Kode Sumber Sistem Terintegrasi
    ├── firmware/                      # Firmware ESP32 Berbasis FreeRTOS
    │   ├── docs/                      # Dokumentasi Master Pipeline LoRa & Wiring Matrix
    │   │   └── README.md
    │   ├── node_wc/                   # Firmware Node Bilik Sensor (Ultrasonik, MQ, SOS, Servo)
    │   │   ├── docs/README.md         # Dokumentasi FreeRTOS Task, Prioritas, & Class A RX
    │   │   ├── src/main.cpp           # Kode Sumber Produksi FreeRTOS
    │   │   ├── standalone_test/       # Sketch Uji Mandiri Hardware Node TX
    │   │   └── platformio.ini         # Konfigurasi Build PlatformIO Node
    │   └── gateway/                   # Firmware Gateway Router (LoRa RX, Store-Forward, WiFi)
    │       ├── docs/README.md         # Dokumentasi FreeRTOS Task, Ring Buffer, & DIO0 ISR
    │       ├── src/main.cpp           # Kode Sumber Produksi FreeRTOS
    │       ├── standalone_test/       # Sketch Uji Mandiri Hardware Gateway RX
    │       └── platformio.ini         # Konfigurasi Build PlatformIO Gateway
    ├── server/                        # Mesin Server Golang & Emergency Dashboard Web
    │   ├── docs/README.md             # Dokumentasi Arsitektur Backend, ETL Worker, & REST API
    │   ├── api/                       # REST API (Chi) & Native WebSocket Hub
    │   ├── etl/                       # In-Memory Cache (LISTEN/NOTIFY) & Dynamic Batch Inserter
    │   ├── database/                  # PostgreSQL Connection Pool (pgxpool)
    │   ├── mqtt/                      # MQTT Subscriber & Actuator Downlink Publisher
    │   ├── static/index.html          # Dashboard EMS Tunggal (Tailwind CSS + Chart.js)
    │   └── main.go                    # Entrypoint Server dengan Graceful Shutdown
    ├── data/                          # Basis Data & Skema Relasional
    │   ├── docs/README.md             # Dokumentasi Skema Relasional & ERD
    │   └── postgres_schema.sql        # DDL PostgreSQL dengan Indeks UUIDv7 & Trigger
    └── scripts/                       # Simulator & Skrip Pengujian Beban
        ├── docs/README.md             # Dokumentasi Utilitas Simulator
        └── dummy_gateway.go           # Generator Telemetri Sintetis via MQTT
```

---

## 📡 Regulasi & Parameter Radio LoRa

Sistem LoRa eSOS mematuhi sepenuhnya regulasi spektrum frekuensi radio Indonesia:
*   **Regulasi Rujukan:** **Permenkomdigi No. 2 Tahun 2025** (Pita LPWAN / Non-Exclusive SRD).
*   **Rentang Alokasi Pita:** 433,050 – 434,790 MHz (Bandwidth Maksimum 125 kHz).
*   **Frekuensi Tengah Operasional ($f_c$):** **`433.175 MHz`** (Kanal nominal 433,1125 – 433,2375 MHz).
*   **Modulasi LoRa:** Spreading Factor 9 (**SF9**) | Bandwidth **125.0 kHz** | Coding Rate **4/7** | SyncWord **`0x12`** (Private).
*   **Kontrak Payload Telemetri:** Biner terkompresi **34 Bytes** (validasi waktu kompilasi via `static_assert`).

---

## 🔌 Pinout Hardware Resmi (Wiring Bus SPI)

Untuk menghindari tabrakan fungsi pin ESP32 (*pin contention*), konfigurasi pinout fisik berikut telah diverifikasi dan dikunci:

| Pin SX1278 (Ra-02) | Pin ESP32 DevKit V1 | Warna Kabel Jumper Fisik | Keterangan Fungsi |
| :--- | :--- | :--- | :--- |
| **NSS / CS** | **GPIO 5** | Kuning | SPI Slave Select Manual via Driver Hardware |
| **MOSI** | **GPIO 18** | Oranye | SPI Master Output Slave Input (VSPI / HSPI) |
| **MISO** | **GPIO 19** | Merah | SPI Master Input Slave Output |
| **SCK** | **GPIO 21** | Cokelat | SPI Serial Clock |
| **RST** | **GPIO 15** | Biru | Hardware Reset Pulsa Aktif Rendah |
| **DIO0** | **GPIO 2** | Ungu | External Interrupt (Packet Rx / Tx Done) |
| **VCC (3.3V)** | **3V3 Pin** | Putih | Tegangan Operasi Semtech SX1278 |
| **GND** | **GND Pin** | Hitam | Ground Referensi Sistem Bersama |

> [!IMPORTANT]
> Pin `GPIO 21` dialokasikan untuk LoRa SCK. Oleh karena itu, jalur komunikasi **I2C Modul RTC DS3231** pada Gateway dialokasikan secara eksplisit ke:
> *   **SDA:** `GPIO 4`
> *   **SCL:** `GPIO 22`

---

## ⚡ Panduan Menjalankan Sistem (Quick Start)

### 1. Backend Server & Database
```powershell
# 1. Jalankan PostgreSQL dan terapkan schema DDL
psql -U postgres -d esos_db -f src/data/postgres_schema.sql

# 2. Jalankan MQTT Broker
mosquitto -v

# 3. Jalankan Server Go
cd src/server
go run main.go
```
*Akses dashboard pemantau darurat di peramban web pada alamat `http://localhost:8000`.*

### 2. Kompilasi & Flash Firmware (PlatformIO)
```powershell
# Kompilasi Firmware Node WC
pio run -d src/firmware/node_wc

# Kompilasi Firmware Gateway Router
pio run -d src/firmware/gateway

# Unggah ke ESP32 yang terhubung ke port COM
pio run -d src/firmware/node_wc -t upload
pio run -d src/firmware/gateway -t upload
```

### 3. Pengujian Mandiri Cepat (Standalone Test)
Tersedia skrip mandiri Arduino IDE di setiap subfolder:
*   **Node TX:** [`src/firmware/node_wc/standalone_test/test_lora_node_tx.ino`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/firmware/node_wc/standalone_test/test_lora_node_tx.ino)
*   **Gateway RX:** [`src/firmware/gateway/standalone_test/test_lora_gateway_rx.ino`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/firmware/gateway/standalone_test/test_lora_gateway_rx.ino)
*   **Simulasi Beban MQTT:** [`src/scripts/dummy_gateway.go`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/scripts/dummy_gateway.go)

---

## 👥 Tim Pengembang (Kelompok 4)
*   **Daffa** - Project Lead, Integrator Sensor & Aktuator, FreeRTOS Firmware Gateway, LoRa RF Pipeline Architecture
*   **Darrel** - Mechanical Structural Design, Sanitation Cubicle Enclosure, IP54 Weatherproofing
*   **Ilman** - Power Management (PV Solar 10-20Wp, BMS Li-Ion 18650), Hardware Schematic & Wiring Harness
*   **Raka** - Penanggung Jawab Hardware Aktuator (Servo/Solenoid Lock), Solution Analyst, Sanitation Standards & Procurement
*   **Siti** - FreeRTOS Sensor Node Firmware, Ultrasonic Level & Gas Detection Integration
