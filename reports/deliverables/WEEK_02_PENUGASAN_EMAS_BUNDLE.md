# BUNDEL PENUGASAN & LUARAN PEKAN 2 — EMAS 3 UI
## Desain Proyek Teknik Elektro, Komputer, Biomedik 2 (Gasal 2026/2027)

---

### Informasi Akademik & Proyek
* **Mata Kuliah:** Desain Proyek 2 (ECEN604002) — Departemen Teknik Elektro, Fakultas Teknik Universitas Indonesia
* **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
* **Kelompok:** 4 (Empat)
* **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
* **Tautan Repositori GitHub:** 👉 [https://github.com/DHard4114/DESPRO-4.git](https://github.com/DHard4114/DESPRO-4.git)
* **Tautan Master Live Google Sheets:** 👉 [Live Master BoM & WBS Tracker (5 Tabs)](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing)

---

### Susunan Anggota Tim & Alokasi Peran (Total 205 Jam):
1. **Daffa Hardhan (2306161763)** — Ketua Kelompok / Manajer Proyek & Penanggung Jawab Backend/Pipeline Data (40 Jam)
2. **Raka Arrayan Muttaqien (2306161800)** — Analis Solusi, Standar Sanitasi & Konsolidasi Finansial Pengadaan (35 Jam)
3. **Siti Amalia Nurfaidah (2306161851)** — Pengembang Firmware Mikrokontroler ESP32 & LoRa SX1278 (45 Jam)
4. **Muhammad Ilman Zuhriy (2306266786)** — Perancang Hardware Daya Solar 10Wp, Baterai 1S4P & Sensor Shield (45 Jam)
5. **Darrel Alfath (2306266810)** — Desainer Mekanikal 3D CAD Enclosure IoT IP54 & Penguji Kualitas/QA (40 Jam)

---

## PENUGASAN 1: Bill of Materials (BoM) Final

*Dokumen Sumber Repositori:* [`02_BOM_PROCUREMENT.md`](https://github.com/DHard4114/DESPRO-4/blob/main/02_BOM_PROCUREMENT.md)  
*Lembar Kerja Interaktif:* [Tab `Master_BoM_Procurement` di Google Sheets](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing)

### Rekapitulasi Finansial Anggaran Proyek:
- **Pagu Maksimum Proposal (*Budget Ceiling*):** **Rp2.000.000**
- **Target Anggaran Proposal (15 Komponen):** **Rp1.901.000**
- **Total Realisasi Belanja Aktual Marketplace:** **Rp1.828.500**
- **Efisiensi Penghematan Belanja:** **Rp72.500 (`HEMAT`)**
- **Sisa Saldo Kas Terhadap Plafon:** **Rp171.500 (`SURPLUS KAS POSITIF`)**

### Tabel 15 Komponen Perangkat Keras Final:
| No | Nama Komponen Perangkat Keras | Spesifikasi Teknis Minimum | Qty | Satuan | Harga Target RAB | Harga Aktual Toko | Total Aktual | Status Anggaran | Toko Vendor | PIC Pembelian & Verifikasi |
|:---:|:---|:---|:---:|:---:|---:|---:|---:|:---:|:---|:---:|
| 1 | Access Point Outdoor TP-Link CPE220 | 2.4GHz 300Mbps High-Power 23dBm 12dBi Directional | 1 | Unit | Rp600.000 | Rp585.000 | Rp585.000 | `HEMAT` | TP-Link Official | **Daffa Hardhan** |
| 2 | Panel Surya Polikristalin 10 Wp | 10 Wp 18V Output Vmp 17.5V Voc 21.5V Waterproof | 1 | Unit | Rp130.000 | Rp125.000 | Rp125.000 | `HEMAT` | Solar Solution ID | **M. Ilman Zuhriy** |
| 3 | Baterai Li-ion 18650 3.7V (Pack 4 pcs) | Sony VTC6 / Panasonic 3000mAh 3.7V Original Grade A | 4 | Pcs | Rp180.000 | Rp42.500 | Rp170.000 | `HEMAT` | Battery Center | **M. Ilman Zuhriy** |
| 4 | Modul Charger BMS Baterai TP4056 | 5V 1A Micro USB / Type-C with Overdischarge DW01 | 2 | Unit | Rp12.000 | Rp5.000 | Rp10.000 | `HEMAT` | E-Store Robotic | **M. Ilman Zuhriy** |
| 5 | Mikrokontroler ESP32 DevKitC V4 | 38-Pin CP2102 Dual Core Wi-Fi + BLE 4MB Flash | 1 | Unit | Rp75.000 | Rp72.000 | Rp72.000 | `HEMAT` | Digiware Store | **Siti Amalia N.** |
| 6 | Sensor Gas Amonia MQ-137 | Deteksi NH3 Range 5 - 500 ppm Analog + Digital | 1 | Unit | Rp160.000 | Rp155.000 | Rp155.000 | `HEMAT` | Sensor Shop ID | **M. Ilman Zuhriy** |
| 7 | Sensor Gas Hidrogen Sulfida MQ-136 | Deteksi H2S Range 1 - 200 ppm Analog + Digital | 1 | Unit | Rp175.000 | Rp168.000 | Rp168.000 | `HEMAT` | Sensor Shop ID | **M. Ilman Zuhriy** |
| 8 | Sensor Jarak Ultrasonik JSN-SR04T | Probe Kedap Air (Waterproof) Blind Zone 20cm | 1 | Unit | Rp85.000 | Rp82.000 | Rp82.000 | `HEMAT` | RoboTech Official | **Siti Amalia N.** |
| 9 | Motor Servo Metal Gear MG996R | Torsi 11 kg.cm Metal Gear 180 Degree 4.8V-7.2V | 1 | Unit | Rp55.000 | Rp52.000 | Rp52.000 | `HEMAT` | Hobby King Store | **Darrel Alfath** |
| 10 | Modul Transceiver RF LoRa RA-02 433MHz | Semtech SX1278 SPI Interface 433MHz + Antena Spring | 2 | Unit | Rp140.000 | Rp68.000 | Rp136.000 | `HEMAT` | Radio Wireless ID | **Siti Amalia N.** |
| 11 | Step-Down Buck Converter DC-DC LM2596 | Input 4.5-40V Output 1.25-35V 3A Adjustable | 2 | Unit | Rp24.000 | Rp10.500 | Rp21.000 | `HEMAT` | Komponen Elektronika | **M. Ilman Zuhriy** |
| 12 | Filamen 3D Printing PETG 1kg | 1.75mm Weatherproof Heat & UV Resistant (eSUN) | 1 | Roll | Rp160.000 | Rp155.000 | Rp155.000 | `HEMAT` | 3D Filament Store | **Darrel Alfath** |
| 13 | Gland Kabel Waterproof PG7 & Baut M3/M4 | Set PG7 Nylon Waterproof IP68 + Baut Mur Stainless | 1 | Set | Rp35.000 | Rp32.000 | Rp32.000 | `HEMAT` | Baut Mur Teknik | **Darrel Alfath** |
| 14 | Perfboard Dot-Matrix & Kabel AWG22 (Wiring Harness — No Custom PCB)| Single Side Perfboard FR4 + Kabel Silicone 22AWG 5m | 1 | Set | Rp45.000 | Rp42.000 | Rp42.000 | `HEMAT` | Elektronika Mandiri | **M. Ilman Zuhriy** |
| 15 | Tombol Darurat SOS Push Button 16mm | Stainless Steel Waterproof Momentary LED Ring 5V | 1 | Unit | Rp25.000 | Rp23.500 | Rp23.500 | `HEMAT` | Saklar Industri | **Darrel Alfath** |
| **TOTAL** | **TOTAL AKUMULASI (15 ITEM)** | | | | **Rp1.901.000** | | **Rp1.828.500** | **`HEMAT (Rp72.500)`** | | |

---

## PENUGASAN 2: Laporan Status Pengadaan

*Dokumen Sumber Repositori:* [`reports/group/WEEK_02_GROUP_REPORT.md`](https://github.com/DHard4114/DESPRO-4/blob/main/reports/group/WEEK_02_GROUP_REPORT.md)

1. **Model Eksekusi Pengadaan:**  
   Pengadaan dilaksanakan secara **Kolektif Terdistribusi oleh Seluruh 5 Anggota Tim**, di mana masing-masing anggota mengeksekusi pembelian komponen sesuai keahlian domain subsistemnya.
2. **Status Ketersediaan Fisik:**  
   **15 dari 15 Item ($100\%$)** telah dipesan, tervalidasi spesifikasi datasheet-nya, dan tiba di laboratorium.
3. **Penyimpanan Bukti Pembelian:**  
   Seluruh invoice, nomor resi kurir, dan bukti pembayaran dihimpun oleh Raka Arrayan Muttaqien ke dalam berkas bundel resmi `assets/procurement/receipts_bundle.pdf` (`EV-W2-PROC-001`).
4. **Keputusan Desain Perakitan Hardware:**  
   Sistem **TIDAK MENGGUNAKAN PCB CUSTOM KELOMPOK (*No custom group PCB*)**. Perakitan elektrikal menggunakan modul breakout standar industri dan *perfboard / wiring harness* terisolasi berkeandalan tinggi oleh Muhammad Ilman Zuhriy.

---

## PENUGASAN 3: Dokumentasi Setup Lingkungan Pengembangan

*Dokumen Sumber Repositori:* [`00_MASTER_README.md`](https://github.com/DHard4114/DESPRO-4/blob/main/00_MASTER_README.md) & [`reports/individual/daffa/WEEK_02_INDIVIDUAL_DAFFA.md`](https://github.com/DHard4114/DESPRO-4/blob/main/reports/individual/daffa/WEEK_02_INDIVIDUAL_DAFFA.md)

### A. Lingkungan Pengembangan Firmware Mikrokontroler (ESP32):
* **IDE & Platform:** Visual Studio Code + PlatformIO Core 6.1+ (`src/firmware/platformio.ini`).
* **Framework:** Arduino Framework for Espressif 32 (C++11/C++14).
* **Dependensi Pustaka Terpasang:**
  - `sandeepmistry/arduino-LoRa@^0.8.0` (Komunikasi Radio LoRa SX1278 SPI 433 MHz)
  - `bblanchon/ArduinoJson@^6.21.3` (Serialisasi Payload Sensor Telemetri JSON)
  - `madhephaestus/ESP32Servo@^1.1.1` (Kendali PWM Motor Servo Katup Air)
* **Kompilasi & Upload:**
  ```bash
  cd src/firmware
  pio run -e esp32dev --target upload
  ```

### B. Lingkungan Pengembangan Backend Server & Data Pipeline (Go):
* **Bahasa & Runtime:** Go (Golang) v1.22+
* **Arsitektur Pipeline:** Dual-Mode Ingestion (Streaming Ingestion & Micro-Batch ETL Engine).
* **Basis Data:** SQLite WAL Mode (`src/data/esos_telemetry.db`) & PostgreSQL 16 DDL (`src/server/database/postgres_schema.sql`) dengan format primary key **UUIDv7 (RFC 9562)**.
* **Protokol Real-Time:** WebSocket Hub (RFC 6455) dengan latensi siar $< 1\text{ ms}$.
* **Cara Menjalankan Server Native Binary:**
  ```powershell
  # Menjalankan binary mandiri tanpa perlu install runtime
  .\src\bin\esos-server.exe
  ```
  Dashboard pemantauan langsung aktif di: **`http://localhost:8000/`**.

---

## PEMBUKTIAN 3 LUARAN PEKAN 2 (EXIT CRITERIA)

| Luaran Wajib Pekan 2 | Status Capaian | Bukti Nyata & Keterangan |
|:---|:---:|:---|
| **1. Minimal 80% komponen telah tersedia** | **`100% TERCAPAI`** | 15 dari 15 item BoM ($100\%$) telah terverifikasi di lab dan dialokasikan ke 5 anggota tim. |
| **2. Lingkungan pengembangan siap digunakan** | **`100% TERCAPAI`** | Firmware C++ ESP32 sukses terkompilasi di PlatformIO, dan binary server Go `src/bin/esos-server.exe` beroperasi normal melayani REST API & WebSocket. |
| **3. Sistem dokumentasi proyek aktif** | **`100% TERCAPAI`** | Repositori GitHub resmi aktif, 5 Master Docs terkendali, 5 Tabs Google Sheets aktif, dan logbook individu terisi penuh. |
