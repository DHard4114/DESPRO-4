# REPOSITORI UTAMA PROYEK — SMART-SANITATION eSOS
## Sistem Pemantauan Sanitasi Cerdas Berbasis IoT & Transmisi Nirkabel Daerah Bencana

Status Dokumen: REPOSITORI MASTER TERKENDALI (CONTROLLED BASELINE) — v5.0 Ultimate Architecture  
Institusi: Departemen Teknik Elektro, Fakultas Teknik Universitas Indonesia (DTE FTUI)  
Mata Kuliah: Desain Proyek 2 (Semester Gasal 2026/2027)  
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  
Kelompok: 4 (Empat)  

---

## 1. Glosarium Singkatan & Istilah Resmi Repositori (Master Glossary)

Untuk memastikan kemudahan pemahaman bagi seluruh penguji dan anggota tim, berikut adalah daftar kepanjangan resmi dan definisi setiap istilah teknis yang digunakan di seluruh repositori:

| Singkatan | Kepanjangan Lengkap (Full Term) | Penjelasan Sederhana & Fungsi dalam Sistem eSOS |
|:---|:---|:---|
| **eSOS** | *Emergency Sanitation Operating System* | Nama sistem perangkat lunak dan arsitektur pemantauan fasilitas sanitasi darurat terpadu kelompok 4 Despro. |
| **WBS** | *Work Breakdown Structure* (Struktur Rincian Kerja) | Dekomposisi hierarkis 14 pekan dari seluruh ruang lingkup pekerjaan proyek sesuai Bab 5.5 Proposal DP1. |
| **BoM** | *Bill of Materials* (Daftar Kebutuhan Komponen) | Daftar lengkap 15 komponen perangkat keras, spesifikasi teknis, batas anggaran (RAB), dan status verifikasi vendor. |
| **RACI** | *Responsible, Accountable, Consulted, Informed* | Matriks pembagian peran dan tanggung jawab kerja 205 jam total yang mengikat seluruh anggota tim. |
| **ETL** | *Extract, Transform, Load* | Alur pemrosesan data tiga tahap: mengekstrak paket sensor, mentransformasikan kalibrasi gas/air, dan memuatnya ke basis data. |
| **ACID** | *Atomicity, Consistency, Isolation, Durability* | Empat standar keandalan transaksi basis data agar data dijamin utuh dan tidak korup saat genset/daya padam mendadak. |
| **UUIDv7** | *Universally Unique Identifier Version 7 (RFC 9562)* | Standar kunci unik global 128-bit terurut waktu (*time-ordered*) yang mencegah tabrakan data (*zero ID collision*) saat multi-posko digabungkan. |
| **ERD** | *Entity Relationship Diagram* | Diagram visual relasi keterhubungan antar-tabel data di dalam basis data sistem. |
| **DDL** | *Data Definition Language* | Sekumpulan perintah SQL untuk membangun kerangka skema tabel, tipe data, dan indeks basis data. |
| **API** | *Application Programming Interface* | Antarmuka perantara komunikasi program antara mikrokontroler sensor dan server backend Go. |
| **REST** | *Representational State Transfer* | Standar arsitektur layanan web berbasis HTTP dengan format pertukaran data JSON. |
| **WebSocket** | *WebSocket Protocol (RFC 6455)* | Saluran komunikasi dua arah berlatensi sangat rendah ($< 1\text{ ms}$) untuk menyiarkan pembaruan data sensor langsung ke dashboard peramban web. |
| **WAL** | *Write-Ahead Logging* | Mekanisme pencatatan log transaksi permanen pada disk sebelum penulisan basis data SQLite/PostgreSQL selesai. |
| **MVCC** | *Multi-Version Concurrency Control* | Manajemen konkurensi database agar operasi pembacaan dashboard tidak pernah saling mengunci dengan operasi penulisan paket data sensor. |
| **CPE** | *Customer Premises Equipment* | Perangkat pemancar nirkabel luar ruangan (*outdoor access point*) TP-Link CPE220 berdaya pancar tinggi 23 dBm. |
| **AP** | *Access Point* (Titik Akses Nirkabel) | Pemancar sinyal Wi-Fi lokal tempat mikrokontroler ESP32 di bilik sanitasi terhubung. |
| **LoRa** | *Long Range Radio* | Modulasi frekuensi radio 433 MHz berdaya rendah untuk transmisi data cadangan jarak jauh menembus reruntuhan ($2-3\text{ km}$). |
| **PPM** | *Parts Per Million* (Bagian per Sejuta) | Satuan konsentrasi gas amonia ($NH_3$) dan hidrogen sulfida ($H_2S$) di udara. |
| **AQI** | *Air Quality Index* (Indeks Mutu Udara) | Klasifikasi mutu udara lingkungan bilik sanitasi (*Baik*, *Sedang*, atau *Berbahaya*). |
| **RTM** | *Requirements Traceability Matrix* | Matriks penelusuran dari kebutuhan proposal, arsitektur sistem, hingga skenario uji verifikasi. |
| **IP54** | *Ingress Protection 54* | Standar ketahanan casing terhadap debu partikel padat (level 5) dan percikan air dari segala arah (level 4). |
| **Light-Sleep** | *ESP32 Power Mode* | Mode hemat daya ESP32 yang mematikan CPU namun mempertahankan RAM dan status *FreeRTOS Queues*. |
| **LittleFS** | *Little Flash File System* | Sistem file pada memori *Flash* untuk menyimpan *Circular Buffer* saat jaringan mati. |

---

## 2. Ringkasan Eksekutif Sistem Smart-Sanitation eSOS

**Smart-Sanitation eSOS** adalah sistem rekayasa sanitasi cerdas modular yang dirancang khusus untuk fasilitas tanggap darurat pasca-bencana pada wilayah tanpa jangkauan seluler (*blank spot*). Sistem ini mengintegrasikan:
1. **Penginderaan Lingkungan Otomatis:** Sensor gas amonia ($NH_3$) MQ-137, hidrogen sulfida ($H_2S$) MQ-136, dan sensor jarak ultrasonik kedap air JSN-SR04T.
2. **Kendali Aktuasi Higienis:** Mikrokontroler ESP32 DevKitC V4 dan motor servo logam MG996R untuk mekanisme buka-tutup katup nirsentuh (*touchless*).
3. **Komunikasi Nirkabel Jarak Jauh Jalur Ganda (*Dual-Path*):** Jalur Utama Wi-Fi 2.4GHz via Access Point TP-Link CPE220 dan Jalur Cadangan LoRa RA-02 433 MHz SX1278.
4. **Kemandirian Daya Surya Mandiri:** Panel surya 20 Wp, modul manajemen pengisian baterai TP4056 BMS, sel Li-ion 18650 1S4P terproteksi, Master DC Switch, dan Inline Fuse 2A (operasi 24 jam kontinu).
5. **Casing 3D IoT Modular (Weatherproof):** Enclosure kustom hasil cetak 3D PETG dengan proteksi standar setara IP54 (dirancang khusus oleh Darrel Alfath).
6. **Firmware FreeRTOS Multi-Task & Ultra-Low Power:** Menggunakan *Light-Sleep*, *Tickless Idle*, dan *Power Management Locks* untuk menghemat baterai tanpa mengorbankan RAM, disertai mitigasi hardware (*Blind Zone* & *Debouncing*).
7. **Infrastruktur Backend Zero-Trust:** Backend Go dengan Worker Pool MQTT, PostgreSQL UUIDv7 (ACID compliant), Threshold Cache (LISTEN/NOTIFY), dan Web Dashboard WebSocket *offline-ready* dengan *Idempotency-Key*.

---

## 3. Struktur Direktori Repositori Modular (Enterprise Architecture)

```text
DESPRO2_SMART_SANITATION_MODULAR/
│
├── 00_MASTER_README.md                   # Dokumen Induk & Peta Repositori
├── 01_MASTER_TASK_ALLOCATION.md          # Matriks Tanggung Jawab Tetap (RACI & Boundaries)
├── 02_BOM_PROCUREMENT.md                 # Master BoM 15 Komponen, RAB, & Technical Gates
├── 03_MASTER_SEMESTER_WBS.md             # Master WBS Semester (W1-W14) & Link Live Spreadsheet
├── 04_PHASE_GATE_AND_TRACEABILITY.md     # Governance Gerbang 1-4 & Matriks Keterlacakan (RTM)
├── 05_EVIDENCE_REGISTER.md               # Master Register Bukti Teknis & Rantai Ketertelusuran
├── MANIFEST.json                         # Metadata Repositori & Katalog Berkas JSON
├── README.md                             # Berkas Root Readme Utama Repositori
│
├── assets/                               # Direktori Artefak dan Berkas Rekayasa
│   ├── cad/                              # Berkas 3D CAD Parametrik (.step, .stl, gambar 2D)
│   ├── schematics/                       # Skematik Sirkuit Elektrikal & Wiring Diagram
│   ├── docs/                             # Dokumen Spesifikasi, Notula, & Panduan
│   ├── logs/                             # Dataset Pengujian Sensor & Log Transmisi
│   ├── media/                            # Foto Perangkat & Rekaman Video Pengujian
│   ├── procurement/                      # Dokumen Pengadaan & Bukti Pembelian
│   │   ├── receipts_bundle.pdf           # Bundel Faktur & Kuitansi Pembelian Resmi
│   │   └── asset_photos/                 # Foto Fisik Unboxing Komponen
│   └── sheets_templates/                 # Berkas Template CSV Siap Impor ke Google Sheets
│       ├── 01_DAILY_LOGBOOK_TRACKING.csv # Logbook 14 Pekan Lengkap (205 Jam Target)
│       ├── 02_MASTER_WBS_GANTT_CHART.csv # Jadwal WBS 14 Pekan & Bobot Progres
│       ├── 03_PHASE_GATE_GOVERNANCE.csv  # Kriteria Audit Gerbang 1 sampai 4
│       └── 04_TEST_MATRIX_TC_RTM.csv     # Matriks Pengujian Lab Skenario TC-01..06
│
├── reports/                              # Direktori Laporan Berkala (Tidak Mengotori Root)
│   ├── group/                            # Laporan Kemajuan Kelompok Mingguan (Pekan 1 s.d. 14)
│   │   ├── WEEK_01_GROUP_REPORT.md
│   │   ├── WEEK_02_GROUP_REPORT.md
│   │   └── ...
│   │
│   └── individual/                       # Laporan Logbook Individu per Anggota (Pekan 1 s.d. 14)
│       ├── daffa/ (WEEK_01 & WEEK_02)
│       ├── raka/ (WEEK_01 & WEEK_02)
│       ├── siti/ (WEEK_01 & WEEK_02)
│       ├── ilman/ (WEEK_01 & WEEK_02)
│       └── darrel/ (WEEK_01 & WEEK_02)
│
└── src/                                  # SELURUH SISTEM & KODE SUMBER LENGKAP
    ├── bin/                              # Binary Executable Siap Pakai (Standalone Executable)
    │   └── esos-server.exe               # Single Executable Native Binary Server Go (Port :8000)
    │
    ├── config/                           # Berkas Konfigurasi Sistem End-to-End
    │   ├── sensor_thresholds.json        # Kalibrasi & Ambang Batas Sensor (Gas, Level, Volt)
    │   ├── lora_config.json              # Parameter RF LoRa 433 MHz & Format Payload
    │   ├── network_cpe220.conf           # Konfigurasi Jaringan Access Point TP-Link CPE220
    │   └── .env.example                  # Template Variabel Lingkungan Server Lokal
    │
    ├── data/                             # Direktori Penyimpanan Basis Data Lokal
    │   └── esos_telemetry.db             # Basis Data SQLite WAL Mode Teroptimasi
    │
    ├── firmware/                         # Source Code Firmware ESP32 (C++ / PlatformIO)
    │   ├── platformio.ini                # Konfigurasi Build PlatformIO & Dependensi
    │   ├── include/config.h              # Pinout Hardware & Threshold Macros
    │   └── src/main.cpp                  # Implementasi Sensor Sampling & LoRa TX
    │
    └── server/                           # High-Performance Go Backend & Web Dashboard
        ├── main.go                       # Entry Point HTTP & WebSocket Server Go
        ├── go.mod & go.sum               # Modul Dependensi Go (UUIDv7, SQLite, WebSocket)
        ├── models/models.go              # Definisi Struktur Data & Model Telemetri UUIDv7
        ├── database/database.go          # SQLite WAL Mode Manager & Transactional Batch Insert
        ├── database/postgres_schema.sql  # Skema Produksi PostgreSQL 15/16 (ACID & UUIDv7)
        ├── etl/pipeline.go               # Streaming & Batch ETL Engine (Worker & Anomaly Detection)
        ├── api/handlers.go               # REST API Endpoints (Telemetry, Alerts, History)
        ├── api/websocket.go              # Real-Time WebSocket Streaming Hub
        ├── static/index.html             # Web Dashboard Frontend (HTML5, Tailwind, Chart.js)
        ├── schema.sql                    # Skema Basis Data SQLite Bawaan
        └── docs/                         # Rangkaian Dokumentasi Teknis Lengkap
            ├── 01_DATABASE_ARCHITECTURE_AND_ERD.md
            ├── 02_BACKEND_ETL_AND_API_ARCHITECTURE.md
            ├── 03_FRONTEND_DASHBOARD_ARCHITECTURE.md
            └── 04_GATEWAY_ROUTER_NETWORK_PIPELINE.md
```

---

## 4. Cara Menjalankan Server & Dashboard (Quick Start Guide)

Untuk menjalankan server backend Go dan membuka dashboard pemantauan di komputer posko bencana:

```powershell
# 1. Pindah ke direktori root repositori
cd c:\Users\dapah\Documents\DESPRO\DESPRO2_SMART_SANITATION_MODULAR

# 2. Jalankan binary server Go mandiri
.\src\bin\esos-server.exe
```

Setelah server aktif, buka peramban web pada alamat:
👉 **`http://localhost:8000/`** (Akses Lokal) atau **`http://192.168.0.100:8000/`** (Akses Jaringan Nirkabel CPE220).

---

## 5. Susunan Tim & Kepemilikan Domain (Alokasi 205 Jam Proposal)

| No | Nama Anggota | NPM | Peran Proposal | Domain Tanggung Jawab Utama | Alokasi Proposal |
|:---:|:---|:---:|:---|:---|:---:|
| 1 | **Daffa Hardhan** | 2306161763 | Manajer Proyek dan Pengembang | Manajemen proyek, master WBS, arsitektur sistem, Go backend server (Batch & Streaming ETL), Web Dashboard UI, integrasi CPE220 | **40 Jam** |
| 2 | **Raka Arrayan Muttaqien** | 2306161800 | Analis Solusi dan Integrasi Layanan | Requirement analysis, verifikasi standar sanitasi, evaluasi vendor BoM, koordinasi pengadaan, feasibility review | **35 Jam** |
| 3 | **Siti Amalia Nurfaidah** | 2306161851 | Pengembang Perangkat Lunak | Firmware mikrokontroler ESP32, pengolahan ADC/PWM, stack protokol LoRa RA-02, logika kendali servo | **45 Jam** |
| 4 | **Muhammad Ilman Zuhriy** | 2306266786 | Perancang Perangkat Keras | Arsitektur daya solar 20 Wp + baterai 18650, modul TP4056, perakitan Fuse 2A & Switch, wiring harness daya & sensor (No Custom PCB), kalibrasi gas analog | **45 Jam** |
| 5 | **Darrel Alfath** | 2306266810 | Desainer Mekanis dan Penguji Kualitas | Desain 3D CAD casing/enclosure IoT modular, weatherproofing IP54, perakitan fixture mekanik, Master Test Plan, QA | **40 Jam** |
| | **TOTAL AKUMULASI** | | | | **205 Jam** |

---

## 6. Indeks Navigasi Cepat Dokumen Master

- [01_MASTER_TASK_ALLOCATION.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/01_MASTER_TASK_ALLOCATION.md) : Matriks RACI semester penuh, aturan kepemilikan domain, dan batas tanggung jawab.
- [02_BOM_PROCUREMENT.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/02_BOM_PROCUREMENT.md) : Kontrol pengadaan 15 komponen BoM, gerbang verifikasi teknis, dan [Google Sheets Live BoM & WBS Tracker](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing).
- [03_MASTER_SEMESTER_WBS.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/03_MASTER_SEMESTER_WBS.md) : Master WBS 14 Pekan (Bab 5.5 Proposal), alur hirarki, dan [Tautan Live Google Sheets](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing).
- [04_PHASE_GATE_AND_TRACEABILITY.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/04_PHASE_GATE_AND_TRACEABILITY.md) : Governance Gerbang 1-4 dan Matriks Keterlacakan Kebutuhan (RTM).
- [05_EVIDENCE_REGISTER.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/05_EVIDENCE_REGISTER.md) : Register bukti teknis (`EV-W[PEKAN]-[DOMAIN]-[NO]`) dan rantai verifikasi.
- [src/bin/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/bin/) : Direktori binary server Go mandiri (`src/bin/esos-server.exe`).
- [src/config/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/config/) : Direktori konfigurasi ambang batas sensor, jaringan CPE220, dan protokol LoRa.
- [src/data/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/data/) : Direktori basis data lokal SQLite WAL mode (`src/data/esos_telemetry.db`).
- [src/firmware/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/firmware/) : Kode sumber firmware C++ PlatformIO mikrokontroler ESP32.
- [src/server/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/) : Kode sumber backend Go (Streaming & Batch ETL), WebSocket Hub, Web Dashboard, dan skema basis data.
- [src/server/docs/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/docs/) : Dokumentasi lengkap Arsitektur Data ERD DDL, Mesin ETL Go, Frontend Dashboard, dan Pipeline Router Gateway.
- [reports/group/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/reports/group/) : Direktori Laporan Kemajuan Kelompok Mingguan resmi FTUI.
- [reports/individual/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/reports/individual/) : Direktori Laporan Logbook Individu Pekanan seluruh anggota.
- [MANIFEST.json](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/MANIFEST.json) : Metadata katalog repositori JSON terstruktur.
