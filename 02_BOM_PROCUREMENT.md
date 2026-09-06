# 02 KONTROL PENGADAAN DAN BILL OF MATERIALS (BOM)
## Smart-Sanitation eSOS (Emergency Sanitation Operating System)

Status Dokumen: KONTROL PENGADAAN TERKENDALI (CONTROLLED BASELINE)  
Batas Pagu Anggaran (*Budget Ceiling*): Rp2.000.000  
Anggaran Proposal Baseline (RAB): Rp1.976.000  
Estimasi Belanja Aktual Marketplace: Rp1.313.500 (Penghematan: Rp686.500 | Pemanfaatan Aset CPE220)  
Model Pengadaan: **Pengadaan Kolektif Terdistribusi oleh Seluruh 5 Anggota Tim (Joint Procurement by All Members)**  
Koordinator Konsolidasi Finansial & Faktur: Raka Arrayan Muttaqien  
Verifikator Anggaran & Manajer Proyek: Daffa Hardhan  

---

## 1. Tautan Lembar Sebar Live Terpadu (Single Source of Truth)

Pengadaan 15 komponen BoM dieksekusi secara bersama oleh seluruh anggota tim sesuai domain subsistemnya. Seluruh pelacakan 15 item perangkat keras, tautan marketplace, status anggaran otomatis, serta bukti fisik invoice pembelian disatukan secara penuh pada tautan resmi Google Sheets berikut:

> **Tautan Master Live Google Sheets (BoM & WBS Tracker):**  
>  [Live Master BoM & Procurement Tracker — Smart-Sanitation eSOS](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing)

---

## 2. Tabel Master Bill of Materials (BoM) Terpadu

| No | Kategori | Nama Komponen & Spesifikasi Mutlak | Qty | PIC Pengadaan | Total Aktual (Rp) | Link / Invoice |
|:---|:---|:---|:---:|:---:|---:|:---|
| 1 | POWER | **Panel Surya Polikristalin** (20Wp 18V Output) | 1 Unit | Ilman | 0 | - |
| 2 | POWER | **Baterai Li-ion 18650 3.7V** (Grade A / Original - 1S4P) | 4 Pcs | Ilman | 0 | - |
| 3 | POWER | **Modul Charger BMS TP4056** (5V 1A Type-C with Protection) | 2 Unit | Ilman | 0 | - |
| 4 | POWER | **Step-Down Buck Converter LM2596** (Input 4.5-40V) | 2 Unit | Darrel | 0 | - |
| 5 | POWER | **Switch Rocker DC** (Master Power Switch On/Off) | 1 Unit | Darrel | 0 | - |
| 6 | POWER | **Inline Glass/Blade Fuse 2A + Holder** (Proteksi Positif Baterai) | 1 Set | Ilman | 0 | - |
| 7 | POWER | **Perfboard Dot-Matrix & Kabel** (FR4 + Silicone 22AWG) | 1 Set | Raka | 0 | - |
| 8 | POWER | **Baterai CR2032 Panasonic** (Baterai untuk Modul RTC) | 2 Pcs | Dapa | 19.544 | - |
| 9 | CONTROLLER | **Mikrokontroler ESP32 DevKitC V4** (38-Pin Dual Core) | 2 Unit | Siti | 0 | - |
| 10 | CONTROLLER | **LoRa RA-02 + Antena (NET-0048)** (WAJIB 433MHz SX1278) | 2 Unit | Dapa | 149.209 | Beli |
| 11 | CONTROLLER | **RTC DS3231SN (EP000559)** (Untuk Stempel Waktu Gateway) | 1 Unit | Dapa | 41.446 | Beli |
| 12 | I/O | **Sensor Gas Amonia MQ-137** (Range 5 - 500 ppm) | 1 Unit | Siti | 0 | - |
| 13 | I/O | **Sensor Gas Hidrogen Sulfida MQ-136** (Range 1 - 200 ppm) | 1 Unit | Siti | 0 | - |
| 14 | I/O | **Sensor Ultrasonik JSN-SR04T** (WAJIB Waterproof, Bukan HC-SR04) | 1 Unit | Raka | 0 | - |
| 15 | I/O | **Tombol Darurat SOS Push Button 16mm** (WAJIB Momentary) | 1 Unit | Darrel | 0 | - |
| 16 | I/O | **Motor Servo DS3218 / TD-8120MG** (WAJIB 20kg Metal Gear) | 1 Unit | Raka | 0 | - |
| 17 | MEKANIKAL | **Filamen 3D Printing PETG 1.75mm** (WAJIB Tahan Panas 1kg) | 1 Roll | Darrel | 0 | - |
| 18 | MEKANIKAL | **Gland Kabel Waterproof PG7 & Baut** (Set IP68 + M3/M4) | 1 Set | Darrel | 0 | - |
| **-** | **NETWORK** | **TP-Link CPE220 Outdoor AP** (Aset Pribadi Kelompok) | 1 Unit | Dapa | 0 | ASET |
| **-** | **TOTAL** | **AKUMULASI SEMENTARA** | | | **Rp 210.199** | |

---

## 3. Gerbang Teknis Kritis Sebelum Pembelian (Critical Technical Gates)

1. **`GATE-PROC-01` (Sensor Gas MQ-137):**  
   Verifikasi identitas modul fisik, tipe sensing element ($SnO_2$), arus pemanas ($\approx 150\text{ mA}$), dan ketersediaan datasheet resmi pabrikan Hanwei.
2. **`GATE-PROC-02` (Sensor Gas MQ-136):**  
   Verifikasi rentang deteksi $H_2S$, resistansi sensor $R_s$, dan metode pemanasan awal (*pre-heating* 24 jam).
3. **`GATE-PROC-03` (Kapasitas Daya Tenaga Surya 20 Wp):**  
   Rekonsiliasi konsumsi beban operasional membuktikan pembangkitan solar 20 Wp memberikan surplus charging yang jauh lebih besar saat mendung.
4. **`GATE-PROC-04` (Topologi Baterai 18650 & TP4056):**  
   Konfigurasi 1S4P (3.7V nominal, 12000 mAh) dengan dual charger TP4056 dan proteksi ganda DW01A tervalidasi aman.
5. **`GATE-PROC-05` (Verifikasi Kapasitas Baterai 18650):**  
   Uji kapasitas debit riil membuktikan setiap sel memiliki kapasitas $\ge 2950\text{ mAh}$ tanpa risiko penurunan tegangan berlebih.
6. **`GATE-PROC-06` (Keamanan Hubungan Singkat - Fuse):**  
   Verifikasi peletakan Fuse 2A pada kutub positif baterai sebelum masuk ke step-down/load.
7. **`GATE-PROC-07` (Keamanan Baterai RTC CR2032):**
   Karena tim menggunakan baterai *Non-Rechargeable* CR2032 pada modul RTC DS3231 (ZS-042), maka sebelum perakitan, PIC Hardware **WAJIB mencungkil/mencabut komponen Dioda (D2)** atau Resistor (201) pada papan RTC untuk memutus sirkuit pengecasan. Kegagalan melakukan ini akan menyebabkan baterai meledak/bocor saat ESP32 dinyalakan.

---

## 4. Alur Kerja Pengadaan dan Standar Bukti Minimum

$$\text{Pemeriksaan Spesifikasi} \rightarrow \text{Verifikasi Datasheet} \rightarrow \text{Komparasi Vendor} \rightarrow \text{Persetujuan Gate} \rightarrow \text{Pemesanan} \rightarrow \text{Faktur & Resi} \rightarrow \text{Fisik Tiba} \rightarrow \text{Registrasi Aset} \rightarrow \text{Uji Acceptance}$$

Arsip bukti pengadaan wajib tersimpan pada repositori:
- **Dokumen Faktur & Resi:** Terkumpul pada bundel PDF `assets/procurement/receipts_bundle.pdf` (`EV-W2-PROC-001`).
- **Foto Fisik Komponen:** Tersimpan pada direktori `assets/procurement/asset_photos/` (`EV-W2-PROC-002`).
- **Pelacakan Live:** Terkoneksi ke [Google Sheets BoM Tracker](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing).
