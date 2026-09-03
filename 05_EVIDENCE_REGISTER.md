# 05 REGISTER BUKTI CAPAIAN TEKNIS (EVIDENCE REGISTER)
## Sistem Penjaminan Mutu dan Ketertelusuran Bukti — Smart-Sanitation eSOS

Status Dokumen: REGISTER BUKTI TERKENDALI (CONTROLLED REGISTER)  
Proyek: Smart-Sanitation eSOS  
Tujuan Sistem: Memastikan setiap klaim kemajuan (*progress claim*) memiliki bukti fisik atau digital yang sah, terukur, dan dapat ditelusuri (*traceable*) secara langsung.  
Penanggung Jawab Tata Kelola: Daffa Hardhan (Manajer Proyek) dan Darrel Alfath (Penjaminan Mutu)  

---

## 1. Standar Penamaan Bukti (Evidence Naming Convention)

$$\text{Format Baku: } \mathbf{EV-W[PEKAN]-[DOMAIN]-[NOMOR\_URUT]}$$

### Contoh Penamaan Sesuai Domain:
- `EV-W1-PM-001` : Bukti domain Project Management & System Architecture pada Pekan 1.
- `EV-W2-PM-001` : Bukti Web Dashboard Monitoring Real-Time buatan Daffa pada Pekan 2.
- `EV-W2-PM-002` : Bukti Pipeline Pengambilan Data Sensor ke Basis Data buatan Daffa pada Pekan 2.
- `EV-W2-PM-003` : Bukti Konfigurasi Jaringan Access Point TP-Link CPE220 pada Pekan 2.
- `EV-W2-PROC-001` : Bukti domain Pengadaan dan Logistik pada Pekan 2.
- `EV-W2-SW-001` : Bukti domain Perangkat Lunak dan Firmware pada Pekan 2.
- `EV-W2-HW-001` : Bukti domain Perangkat Keras dan Kelistrikan pada Pekan 2.
- `EV-W2-MECH-001` : Bukti domain Mekanikal dan Casing 3D pada Pekan 2.
- `EV-W2-QA-001` : Bukti domain Penjaminan Mutu dan Pengujian pada Pekan 2.

---

## 2. Matriks Bukti Wajib Proyek (Required Evidence Matrix)

| ID Bukti Baku | Deskripsi Item Bukti Teknis | Penanggung Jawab (Owner) | Standar Bukti Minimum (Minimum Proof) | Format Berkas / Lokasi Repositori | Kategori Kualitas |
|:---|:---|:---:|:---|:---|:---:|
| **EV-W1-PM-001** | Master Work Breakdown Structure (WBS) 14 Pekan | Daffa Hardhan | Dokumen WBS 14 Pekan & link Google Sheets | `03_MASTER_SEMESTER_WBS.md` | Kuat (*Strong*) |
| **EV-W1-PM-002** | Matriks Alokasi Tanggung Jawab Tetap (Fixed RACI) | Daffa Hardhan | Matriks 205 jam kerja mengikat seluruh anggota | `01_MASTER_TASK_ALLOCATION.md` | Kuat (*Strong*) |
| **EV-W2-PM-001** | **Web Dashboard Monitoring Real-Time** | **Daffa Hardhan** | **Antarmuka web interaktif HTML5/Tailwind/Chart.js & WebSocket** | `src/server/static/index.html` | **Kuat (*Strong*)** |
| **EV-W2-PM-002** | **Backend Server Go (Batch & Streaming ETL)** | **Daffa Hardhan** | **Server Go, Binary Mandiri `src/bin/esos-server.exe`, Mesin ETL & DB** | `src/bin/esos-server.exe`, `src/server/main.go`, `src/server/etl/pipeline.go` | **Kuat (*Strong*)** |
| **EV-W2-PM-003** | **Konfigurasi Jaringan Outdoor TP-Link CPE220** | **Daffa Hardhan** | **Konfigurasi IP statis, AP SSID & port mapping** | `src/config/network_cpe220.conf` | **Kuat (*Strong*)** |
| **EV-W1-SA-001** | Master Bill of Materials (BoM) & RAB | Raka Arrayan M. | Tabel 15 komponen, RAB proposal, dan vendor | `02_BOM_PROCUREMENT.md` | Kuat (*Strong*) |
| **EV-W1-SA-002** | Datasheet Resmi MQ-137 ($NH_3$) & MQ-136 ($H_2S$) | Raka / Ilman | Datasheet PDF resmi pabrikan Hanwei | `assets/docs/datasheet_mq137.pdf` | Kuat (*Strong*) |
| **EV-W2-PROC-001** | Faktur & Kuitansi Pengadaan 15 Komponen | Raka Arrayan M. | Bundel faktur pembelian resmi dan nomor resi | `assets/procurement/receipts_bundle.pdf` | Kuat (*Strong*) |
| **EV-W2-PROC-002** | Buku Register Aset & Foto Unboxing Komponen | Raka / Ilman | Foto fisik unboxing dan kode aset inventaris | `assets/procurement/asset_photos/` | Kuat (*Strong*) |
| **EV-W2-PROC-003** | Master BoM & Link Checkout 15 Komponen | Raka Arrayan M. | Dokumen BoM terpadu & Google Sheets Live | `02_BOM_PROCUREMENT.md` | Kuat (*Strong*) |
| **EV-W2-SW-001** | Konfigurasi Toolchain & IDE PlatformIO | Siti Amalia N. | Berkas konfigurasi PlatformIO & board ESP32 | `src/firmware/platformio.ini` | Kuat (*Strong*) |
| **EV-W2-SW-002** | Source Code Firmware Sensor & LoRa Transmitter | Siti Amalia N. | Kode C++ modular event-driven di `/src/firmware/` | `src/firmware/src/main.cpp` | Kuat (*Strong*) |
| **EV-W2-SW-003** | Konfigurasi Pinout & Parameter LoRa 433 MHz | Siti Amalia N. | Berkas header pinout dan parameter RF LoRa | `src/firmware/include/config.h`, `src/config/lora_config.json` | Kuat (*Strong*) |
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

1. **Kuat (*Strong*):**  
   Bukti mendemonstrasikan luaran teknis secara langsung (*directly demonstrates output*). Contoh: kode sumber dashboard web yang berjalan, berkas CAD asli (.step), source code firmware, dataset CSV hasil pengujian fisik, video demonstrasi alat beroperasi, atau kuitansi resmi bertanda tangan.
2. **Dapat Diterima (*Acceptable*):**  
   Bukti mendemonstrasikan pelaksanaan aktivitas namun belum mencakup verifikasi kuantitatif penuh.
3. **Lemah (*Weak*):**  
   Bukti hanya membuktikan kehadiran atau notula diskusi tanpa adanya luaran teknis nyata yang dihasilkan.
4. **Tidak Sah (*Invalid*):**  
   Tidak memiliki keterhubungan yang dapat diidentifikasi dengan tugas WBS yang diklaim $\rightarrow$ **Ditolak ($0\%$)**.

---

## 4. Rantai Ketertelusuran Teknis Daffa Hardhan (*Data Pipeline & Web Dashboard*)

$$\text{Sensor Node} \xrightarrow[\text{LoRa 433MHz}]{\text{Paket JSON}} \text{LoRa Gateway} \xrightarrow[\text{Ethernet / Wi-Fi}]{\text{TP-Link CPE220}} \text{FastAPI Ingest API} \xrightarrow[\text{SQL Insert}]{\text{SQLite DB}} \text{Live Web Dashboard UI}$$

- **Luaran Frontend:** [src/server/static/index.html](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/static/index.html) (Menampilkan gauge air, grafik amonia/H2S live, baterai, banner alarm SOS, dan tombol kontrol servo).
- **Luaran Backend & DB:** [src/server/app.py](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/app.py) dan [src/server/schema.sql](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/schema.sql).
- **Luaran Jaringan:** [config/network_cpe220.conf](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/config/network_cpe220.conf).
