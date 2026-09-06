# 05 REGISTER BUKTI CAPAIAN TEKNIS (EVIDENCE REGISTER)
## Sistem Penjaminan Mutu dan Ketertelusuran Bukti — Smart-Sanitation eSOS

Status Dokumen: REGISTER BUKTI TERKENDALI (CONTROLLED REGISTER) — **v2.0 Selaras dengan ADR**
Mengacu pada: `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md`
Proyek: Smart-Sanitation eSOS
Penanggung Jawab Tata Kelola: Daffa Hardhan (Manajer Proyek) dan Darrel Alfath (Penjaminan Mutu)

---

## 1. Standar Penamaan Bukti (Evidence Naming Convention)

$$\text{Format Baku: } \mathbf{EV-W[PEKAN]-[DOMAIN]-[NOMOR\_URUT]}$$

### Contoh Penamaan Sesuai Domain:
- `EV-W1-PM-001` : Bukti domain Project Management & System Architecture pada Pekan 1.
- `EV-W1-PM-003` : **[BARU]** Bukti Architecture Decision Record (ADR) — pemersatu keputusan arsitektur final.
- `EV-W2-PM-001` : Bukti Web Dashboard Monitoring Real-Time buatan Daffa pada Pekan 2.
- `EV-W2-PM-002` : Bukti Backend Go — MQTT Subscriber Worker Pool & ETL Engine pada Pekan 2.
- `EV-W2-PM-003` : Bukti Konfigurasi Jaringan Access Point TP-Link CPE220 pada Pekan 2.
- `EV-W2-PM-004` : **[BARU]** Bukti Konfigurasi Mosquitto MQTT Broker & ACL pada Pekan 2.
- `EV-W2-PROC-001` : Bukti domain Pengadaan dan Logistik pada Pekan 2.
- `EV-W2-SW-001` s.d. `004` : Bukti domain Perangkat Lunak dan Firmware FreeRTOS pada Pekan 2.
- `EV-W2-HW-001` : Bukti domain Perangkat Keras dan Kelistrikan pada Pekan 2.
- `EV-W2-MECH-001` : Bukti domain Mekanikal dan Casing 3D pada Pekan 2.
- `EV-W2-QA-001` : Bukti domain Penjaminan Mutu dan Pengujian pada Pekan 2.

---

## 2. Matriks Bukti Wajib Proyek (Required Evidence Matrix)

| ID Bukti Baku | Deskripsi Item Bukti Teknis | Penanggung Jawab (Owner) | Standar Bukti Minimum (Minimum Proof) | Format Berkas / Lokasi Repositori | Kategori Kualitas |
|:---|:---|:---:|:---|:---|:---:|
| **EV-W1-PM-001** | Master Work Breakdown Structure (WBS) 14 Pekan | Daffa Hardhan | Dokumen WBS 14 Pekan & link Google Sheets | `03_MASTER_SEMESTER_WBS.md` | Kuat (*Strong*) |
| **EV-W1-PM-002** | Matriks Alokasi Tanggung Jawab Tetap (Fixed RACI) | Daffa Hardhan | Matriks 205 jam kerja mengikat seluruh anggota | `01_MASTER_TASK_ALLOCATION.md` | Kuat (*Strong*) |
| **EV-W1-PM-003** | **Architecture Decision Record (ADR)** | **Daffa Hardhan** | **Dokumen pemersatu 8 keputusan arsitektur (MQTT, FreeRTOS, REST v1, UUIDv7) yang mengikat seluruh implementasi berikutnya** | `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md` | **Kuat (*Strong*)** |
| **EV-W2-PM-001** | **Web Dashboard Monitoring Real-Time** | **Daffa Hardhan** | **Antarmuka web interaktif HTML5/Tailwind/Chart.js & WebSocket, menampilkan event `TELEMETRY_STREAM`, `EMERGENCY_ALERT`, `GATEWAY_STATUS`, `ACTUATOR_STATUS`** | `src/server/static/index.html`, `docs/architecture/03_FRONTEND_DASHBOARD_ARCHITECTURE.md` | **Kuat (*Strong*)** |
| **EV-W2-PM-002** | **Backend Server Go — MQTT Subscriber & ETL Engine** | **Daffa Hardhan** | **Klien MQTT persisten (paho.mqtt.golang), Worker Pool ETL, Threshold Cache (LISTEN/NOTIFY), REST API v1** | `src/server/mqtt/subscriber.go`, `src/server/etl/pipeline.go`, `docs/architecture/02_BACKEND_ETL_AND_API_ARCHITECTURE.md` | **Kuat (*Strong*)** |
| **EV-W2-PM-003** | **Konfigurasi Jaringan Outdoor TP-Link CPE220** | **Daffa Hardhan** | **Konfigurasi IP statis, AP SSID & port mapping (radio WiFi eksklusif untuk Gateway)** | `src/config/network_cpe220.conf`, `docs/architecture/04_GATEWAY_ROUTER_NETWORK_PIPELINE.md` | **Kuat (*Strong*)** |
| **EV-W2-PM-004** | **Konfigurasi Mosquitto MQTT Broker & ACL** | **Daffa Hardhan** | **`mosquitto.conf` (auth wajib, persistence, keep-alive) & `mosquitto_acl` (isolasi topik per node)** | `src/config/mosquitto.conf`, `src/config/mosquitto_acl`, `docs/architecture/05_LORA_MQTT_TELEMETRY_PIPELINE.md` §7 | **Kuat (*Strong*)** |
| **EV-W1-SA-001** | Master Bill of Materials (BoM) & RAB | Raka Arrayan M. | Tabel 15 komponen, RAB proposal, dan vendor | `02_BOM_PROCUREMENT.md` | Kuat (*Strong*) |
| **EV-W1-SA-002** | Datasheet Resmi MQ-137 ($NH_3$) & MQ-136 ($H_2S$) | Raka / Ilman | Datasheet PDF resmi pabrikan Hanwei | `assets/docs/datasheet_mq137.pdf` | Kuat (*Strong*) |
| **EV-W2-PROC-001** | Faktur & Kuitansi Pengadaan 15 Komponen | Raka Arrayan M. | Bundel faktur pembelian resmi dan nomor resi | `assets/procurement/receipts_bundle.pdf` | Kuat (*Strong*) |
| **EV-W2-PROC-002** | Buku Register Aset & Foto Unboxing Komponen | Raka / Ilman | Foto fisik unboxing dan kode aset inventaris | `assets/procurement/asset_photos/` | Kuat (*Strong*) |
| **EV-W2-PROC-003** | Master BoM & Link Checkout 15 Komponen | Raka Arrayan M. | Dokumen BoM terpadu & Google Sheets Live | `02_BOM_PROCUREMENT.md` | Kuat (*Strong*) |
| **EV-W2-SW-001** | Konfigurasi Toolchain & IDE PlatformIO | Siti Amalia N. | Berkas konfigurasi PlatformIO & board ESP32 (Node + Gateway) | `src/firmware/platformio.ini`, `src/firmware_gateway/platformio.ini` | Kuat (*Strong*) |
| **EV-W2-SW-002** | **Source Code Firmware FreeRTOS Multi-Task** | Siti Amalia N. | **Kode C++ dengan `xTaskCreatePinnedToCore` sesuai tabel task di `01_SENSOR_AND_ACTUATOR_REFERENCES.md` (bukan lagi single-loop Arduino)** | `src/firmware/src/main.cpp`, `src/firmware_gateway/src/main.cpp` | Kuat (*Strong*) |
| **EV-W2-SW-003** | Konfigurasi Pinout & Parameter LoRa 433 MHz | Siti Amalia N. | Berkas header pinout dan parameter RF LoRa (migrasi ke RadioLib non-blocking) | `src/firmware/include/config.h`, `src/config/lora_config.json` | Kuat (*Strong*) |
| **EV-W2-SW-004** | **Arsitektur Firmware FreeRTOS (Task/Queue/Priority)** | Siti Amalia N. | **Tabel task lengkap: priority, core affinity, stack size, mekanisme antar-task (Queue/Notify)** | `docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md` §1–§2 | Kuat (*Strong*) |
| **EV-W2-HW-001** | Skematik Sirkuit Kelistrikan Sistem | M. Ilman Zuhriy | Berkas skematik sirkuit daya dan regulator | `assets/schematics/schematic_v1.pdf` | Kuat (*Strong*) |
| **EV-W2-HW-002** | Kalkulasi Power Budget Panel Surya 10 Wp | M. Ilman Zuhriy | Lembar perhitungan power budget 24 jam kontinu | `assets/docs/solar_power_budget.pdf` | Kuat (*Strong*) |
| **EV-W2-HW-003** | Diagram Topologi Baterai 18650 1S4P & TP4056 | M. Ilman Zuhriy | Skema konfigurasi 1S4P dan dual TP4056 BMS | `assets/schematics/battery_topology.png`| Kuat (*Strong*) |
| **EV-W2-HW-004** | Rangkaian Perfboard Wiring Harness (No Custom PCB) | M. Ilman Zuhriy | Foto rangkaian wiring harness terisolasi | `assets/schematics/wiring_harness.png` | Kuat (*Strong*) |
| **EV-W2-MECH-001**| Gambar Teknik Manufaktur 2D Casing IoT | Darrel Alfath | Lembar gambar teknik dimensi dan toleransi fit | `assets/cad/technical_drawing_2d.pdf` | Kuat (*Strong*) |
| **EV-W2-MECH-002**| Berkas Model 3D CAD Parametrik Casing IoT | Darrel Alfath | Berkas 3D CAD format STEP dan file STL print | `assets/cad/sanitation_modular_v1.step` | Kuat (*Strong*) |
| **EV-W2-QA-001** | Master Test Plan & Prosedur Uji (TC-01..06) | Darrel Alfath | Dokumen skenario uji TC-01 sampai TC-06 | `04_PHASE_GATE_AND_TRACEABILITY.md` | Kuat (*Strong*) |
| **EV-W2-QA-002** | Video Uji Ketahanan Air Casing 3D IP54 | Darrel Alfath | Rekaman video uji semprotan air 5 menit | `assets/media/water_resistance_test.mp4` | Kuat (*Strong*) |

---

## 3. Kriteria Klasifikasi Kualitas Bukti (Evidence Quality)

1. **Kuat (*Strong*):** Bukti mendemonstrasikan luaran teknis secara langsung. Contoh: kode sumber, berkas CAD asli (.step), dataset CSV hasil pengujian fisik, video demonstrasi, kuitansi resmi bertanda tangan, atau dokumen arsitektur yang menjadi rujukan implementasi (ADR).
2. **Dapat Diterima (*Acceptable*):** Bukti mendemonstrasikan pelaksanaan aktivitas namun belum mencakup verifikasi kuantitatif penuh.
3. **Lemah (*Weak*):** Bukti hanya membuktikan kehadiran atau notula diskusi tanpa luaran teknis nyata.
4. **Tidak Sah (*Invalid*):** Tidak memiliki keterhubungan yang dapat diidentifikasi dengan tugas WBS yang diklaim → **Ditolak (0%)**.

---

## 4. Rantai Ketertelusuran Teknis Daffa Hardhan (*Data Pipeline & Web Dashboard*) — REVISI

> **Jalur Ketertelusuran Resmi:** Mengacu pada arsitektur final (ADR-01, ADR-06) yang diimplementasikan:

$$\text{Sensor Node (FreeRTOS)} \xrightarrow[\text{LoRa 433MHz}]{\text{JSON Envelope}} \text{ESP32 Gateway (Buffer)} \xrightarrow[\text{Wi-Fi via CPE220}]{\text{MQTT Publish QoS 1/2}} \text{Mosquitto Broker} \xrightarrow[\text{paho.mqtt.golang}]{\text{Subscribe}} \text{Go ETL Worker Pool} \xrightarrow[\text{Bulk Insert}]{\text{UUIDv7}} \text{PostgreSQL + TimescaleDB} \xrightarrow[\text{WebSocket}]{\text{Push}} \text{Live Web Dashboard UI}$$

- **Luaran Frontend:** `src/server/static/index.html` (Menampilkan gauge air, grafik amonia/H2S live, baterai, banner alarm SOS, dan tombol kontrol servo yang terhubung ke endpoint aktuator REST v1 asinkron).
- **Luaran Backend & Pipeline:** `src/server/mqtt/subscriber.go`, `src/server/etl/pipeline.go`, `src/server/database/postgres_schema.sql`.
- **Luaran Jaringan:** `src/config/network_cpe220.conf`, `src/config/mosquitto.conf`, `src/config/mosquitto_acl`.
- **Dokumen Arsitektur Rujukan:** `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md`, `05_LORA_MQTT_TELEMETRY_PIPELINE.md`, `02_BACKEND_ETL_AND_API_ARCHITECTURE.md`.

