# 03 MASTER WBS SEMESTER & SISTEM MONITORING
## Operational Work Breakdown Structure — Smart-Sanitation eSOS

Status Dokumen: MASTER WBS SEMESTER TERKENDALI (CONTROLLED BASELINE)  
Proyek: Smart-Sanitation eSOS  
Mata Kuliah: Desain Proyek 2 (Gasal 2026/2027)  
Acuan Resmi: Bab 5.5 Jadwal dan Perancangan Proposal Desain Proyek 1 DTE FTUI  
Total Alokasi Waktu: 205 Jam Kerja Semester  

---

## 1. Tautan Lembar Kerja Live Spreadsheet & Sistem Monitoring Harian

Untuk memfasilitasi kolaborasi dinamis 5 anggota tim secara *real-time* tanpa risiko bentrok versi (*Git merge conflict*), seluruh pencatatan logbook harian, status tugas per pekan, dan pelacakan anggaran dihubungkan secara langsung ke **Google Sheets Live Master WBS Tracker**:

> **Tautan Master Tracking Spreadsheet:**  
> [Live Master WBS & BoM Tracker — Smart-Sanitation eSOS (Google Sheets)](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing)

### Arsitektur Struktur Tab Google Sheets:
1. **Tab 1: `Master_WBS_Gantt`**  
   Memuat daftar lengkap paket kerja Pekan 1 s.d. Pekan 14, dependensi jalur kritis, durasi hari, dan diagram Gantt interaktif otomatis.
2. **Tab 2: `Daily_Logbook_Tracking`**  
   Digunakan setiap hari oleh seluruh 5 anggota untuk mencatat jam kerja aktual, uraian aktivitas, luaran, dan ID bukti dengan rumus kalkulasi otomatis:
   $$\text{Total Jam Individu} = \text{SUMIFS}(\text{Jam}, \text{Nama}, \text{"Nama Anggota"}, \text{Pekan}, \text{Nomor Pekan})$$
3. **Tab 3: `BoM_Procurement_Tracker`**  
   Tabel pelacak 15 komponen, vendor, nomor resi kurir, status kedatangan, dan formula kontrol anggaran:
   $$\text{Budget Sisa} = \text{Rp2.000.000} - \text{SUM}(\text{Total\_Harga\_Beli})$$
4. **Tab 4: `Phase_Gate_KPI_Dashboard`**  
   Dashboard visual persentase progres berbobot per subsistem dan indikator kelulusan Gerbang 1 sampai 4.

---

## 2. Alur Struktur Hierarki WBS Semester Penuh

```text
Smart-Sanitation eSOS (Total Scope Semester: 205 Jam)
│
├── 1.0 INITIATION & RE-PLANNING (Pekan 1 — 20h Daffa, 18h Raka, 22h Siti, 26h Ilman, 20h Darrel)
│     ├── 1.1 Evaluasi Proposal, Target Kinerja, dan Batasan Sistem (W1-PM, W1-SA)
│     └── 1.2 Master Schedule 14 Pekan, RACI, dan Repositori Governance (W1-PM, W1-SW, W1-HW, W1-MQ)
│
├── 2.0 PROCUREMENT & IMPLEMENTATION READINESS (Pekan 2 — Jendela Pengadaan 31 Agt – 05 Sep)
│     ├── 2.1 Eksekusi BoM 15 Komponen & Verifikasi Fisik Kedatangan (W2-PROC: Raka)
│     ├── 2.2 Penyiapan Toolchain Firmware & Baseline Code ESP32 (W2-SW: Siti)
│     ├── 2.3 Validasi Kelistrikan Daya Surya 10Wp & Topologi 1S4P (W2-HW: Ilman)
│     ├── 2.4 Finalisasi Model 3D CAD Casing IoT & Gambar Teknik 2D (W2-MECH: Darrel)
│     └── 2.5 Setup Repositori Git Resmi, Task Tracker & Server Lokal (W2-PM: Daffa)
│
├── 3.0 SUBSYSTEM FABRICATION & HARDWARE PREPARATION (Pekan 3 – Pekan 4)
│     ├── 3.1 Fabrikasi 3D Printing Casing PETG & Sealing Gasket RTV IP54 (Darrel)
│     ├── 3.2 Perakitan Modul Daya & Perfboard Wiring Harness Terisolasi (Ilman — No Custom PCB)
│     ├── 3.3 Kalibrasi Sensor Gas Analog MQ-137/136 & Sensor Jarak JSN-SR04T (Ilman, Siti)
│     └── 3.4 Pengembangan Driver Modular Firmware ESP32 & JSON Serialization (Siti)
│
├── 4.0 WIRELESS TELEMETRY & SYSTEM INTEGRATION (Pekan 5 – Pekan 6)
│     ├── 4.1 Implementasi Stack Nirkabel LoRa RA-02 433 MHz & Mekanisme Servo (Siti, Darrel)
│     ├── 4.2 Integrasi Casing 3D IoT + Hardware + Firmware ESP32 (Darrel, Ilman, Siti)
│     └── 4.3 Setup Local Web Server Dashboard & Integrasi TP-Link CPE220 (Daffa)
│
├── 5.0 MID-TERM EVALUATION & PHASE GATE 3 (Pekan 7 — Evaluasi Tengah Semester / UTS)
│     └── 5.1 Demonstrasi Prototipe Terintegrasi & Evaluasi Kesiapan Midterm (Seluruh Tim)
│
├── 6.0 FIELD TESTING & DURABILITY VALIDATION (Pekan 8 – Pekan 10)
│     ├── 6.1 Uji Ketahanan Cuaca IP54 & Siklus Daya Surya 24 Jam Pasca-Bencana (Darrel, Ilman)
│     ├── 6.2 Uji Transmisi Nirkabel LoRa Jarak Jauh pada Wilayah Blank Spot (Siti, Daffa)
│     └── 6.3 Eksekusi Penuh Skenario Pengujian Mutu TC-01 s.d. TC-06 (Darrel + Seluruh PIC)
│
└── 7.0 FINAL OPTIMIZATION, REPORTING & DEMO (Pekan 11 – Pekan 14 / UAS)
      ├── 7.1 Freeze Versi Rilis Casing 3D, Firmware, dan Server (Seluruh Tim)
      ├── 7.2 Penyusunan Laporan Akhir Capstone Komprehensif (Seluruh Tim — Lead: Daffa)
      ├── 7.3 Pembuatan Media Presentasi, Banner & Video Demonstrasi Sistem (All)
      └── 7.4 Sidang Akhir Capstone Desain Proyek 2 & Demonstrasi Live (All)
```

---

## 3. Rincian Paket Kerja Semester (Bab 5.5 Proposal DP1: Pekan 1 s.d. Pekan 14)

### Fase 1: Perencanaan Ulang dan Penetapan Baseline (Pekan 1)
- **Target Periode:** 26 Agustus 2026 – 01 September 2026
- **Aktivitas Utama:**
  - `W1-PM-01` s.d. `W1-PM-10` (Daffa): Master WBS, jadwal semester, alokasi RACI fixed 205 jam, struktur repositori.
  - `W1-SA-01` s.d. `W1-SA-08` (Raka): Analisis kebutuhan, verifikasi silang BoM, pelacak pengadaan.
  - `W1-SW-01` s.d. `W1-SW-08` (Siti): Pemetaan modul firmware, pinout ESP32, rancangan driver sensor.
  - `W1-HW-01` s.d. `W1-HW-09` (Ilman): Power budget daya surya 10Wp, skematik daya, pre-heating sensor gas.
  - `W1-MQ-01` s.d. `W1-MQ-08` (Darrel): Batasan mekanik fasilitas sanitasi, konsep 3D enclosure, Master Test Plan.
- **Milestone:** **Phase Gate 1 Review — PASS**.

### Fase 2: Kesiapan Implementasi dan Pengadaan (Pekan 2)
- **Target Periode:** 02 September 2026 – 08 September 2026 (Jendela Pengadaan: 31 Agt – 05 Sep)
- **Aktivitas Utama:**
  - `W2-PROC-01` s.d. `W2-PROC-11` (Raka): Eksekusi pembelian 15 item BoM, verifikasi fisik barang tiba, buku aset.
  - `W2-SW-01` s.d. `W2-SW-10` (Siti): Setup PlatformIO, kompilasi baseline firmware, uji GPIO/ADC/PWM.
  - `W2-HW-01` s.d. `W2-HW-10` (Ilman): Verifikasi spesifikasi fisik, penguncian topologi baterai 1S4P, rencana wiring harness.
  - `W2-MECH-01` s.d. `W2-MECH-08` (Darrel): Gambar teknik 2D, pemodelan 3D CAD casing IoT, persiapan cetak PETG.
  - `W2-PM-01` s.d. `W2-PM-09` (Daffa): Pembangunan Web Dashboard pemantauan real-time (`index.html`), pipeline penerimaan data sensor ke SQLite (`app.py`), konfigurasi jaringan outdoor TP-Link CPE220 (`network_cpe220.conf`), repositori modular, dan live Google Sheets tracker.
- **Milestone:** **Phase Gate 2 Review (Kesiapan $\ge 80\%$ Komponen) — PASS**.

### Fase 3: Fabrikasi Casing 3D dan Perakitan Elektrikal (Pekan 3 – Pekan 4)
- **Target Periode:** 09 September 2026 – 22 September 2026
- **Aktivitas Utama:**
  - **Darrel:** Pencetakan 3D casing IoT berbahan PETG, aplikasi sealing gasket silikon RTV, dan pembuatan bracket tiang.
  - **Ilman:** Perakitan rangkaian catu daya solar 10Wp, modul TP4056 BMS, baterai 18650, dan wiring harness modular (No Custom PCB).
  - **Siti:** Pembuatan driver modular sensor ultrasonik JSN-SR04T non-blocking dan filtering digital sinyal gas analog.
  - **Daffa:** Pengembangan backend database server lokal dan endpoint API penerimaan data telemetri.
  - **Raka:** Audit anggaran pengeluaran riil dan evaluasi kepatuhan komponen terhadap standar sanitasi.

### Fase 4: Integrasi Nirkabel LoRa dan Mekanisme Aktuator (Pekan 5 – Pekan 6)
- **Target Periode:** 23 September 2026 – 06 Oktober 2026
- **Aktivitas Utama:**
  - **Siti & Ilman:** Integrasi modulasi nirkabel LoRa RA-02 433 MHz SPI, pengujian Packet Error Rate, dan optimasi daya pancar.
  - **Darrel & Siti:** Perakitan mekanisme linkage transmisi putar motor servo MG996R untuk pembukaan katup nirsentuh.
  - **Darrel & Ilman:** Pemasangan terpadu seluruh modul elektrikal ke dalam casing 3D IoT (fit check 100% pas).
  - **Daffa:** Pengujian koneksi gateway LoRa ke access point outdoor TP-Link CPE220 dan visualisasi dashboard live.
  - **Raka:** Penyusunan dokumen keterlacakan verifikasi komponen terhadap matriks RTM.

### Fase 5: Evaluasi Tengah Semester / UTS (Pekan 7)
- **Target Periode:** 07 Oktober 2026 – 13 Oktober 2026
- **Aktivitas Utama:**
  - **Seluruh Anggota:** Demonstrasi fungsional prototipe berjalan di hadapan Dosen Pembimbing.
  - **Daffa:** Kompilasi Laporan Tengah Semester dan evaluasi kelulusan **Phase Gate 3 (Mid-term Prototype Review)**.

### Fase 6: Pengujian Lapangan, Keandalan Kontinu & Validasi QA (Pekan 8 – Pekan 10)
- **Target Periode:** 14 Oktober 2026 – 03 November 2026
- **Aktivitas Utama:**
  - **Darrel & Ilman:** Eksekusi pengujian ketahanan semprotan air IP54 (TC-06) dan uji operasional kontinu daya surya 24 jam (TC-05).
  - **Siti & Daffa:** Pengujian jangkauan transmisi LoRa jarak jauh pada simulasi area blank spot kampus (TC-03).
  - **Siti & Ilman:** Pengujian akurasi level cairan tangki (TC-01) dan sensitivitas gas amonia/$H_2S$ (TC-02).
  - **Darrel:** Eksekusi uji siklus gerak katup mekanis 500 kali (TC-04) dan pengarsipan bukti pada `05_EVIDENCE_REGISTER.md`.

### Fase 7: Optimasi Akhir, Finalisasi Dokumen & Sidang Capstone (Pekan 11 – Pekan 14)
- **Target Periode:** 04 November 2026 – 01 Desember 2026
- **Aktivitas Utama:**
  - **Pekan 11 (Seluruh Tim):** Pembekuan (*freeze*) versi rilis casing 3D fisik, firmware v1.0, dan antarmuka server.
  - **Pekan 12 (Lead Daffa + All):** Penyusunan draf Laporan Akhir Capstone Desain Proyek 2 lengkap sesuai format FTUI.
  - **Pekan 13 (All):** Geladi bersih demonstrasi alat, pembuatan video demo profesional, dan materi presentasi sidang.
  - **Pekan 14 (All):** **Phase Gate 4 (Ujian Akhir Semester / Sidang Capstone DP2 & Live Exhibition)**.

---

## 4. Distribusi Alokasi Jam Kerja Semester (Total: 205 Jam)

| Nama Anggota | Peran Proposal | Alokasi Proposal | W1 – W2 (Persiapan) | W3 – W6 (Implementasi) | W7 – W10 (Uji & QA) | W11 – W14 (Final & Demo) | Total Akumulasi |
|:---|:---|:---:|:---:|:---:|:---:|:---:|:---:|
| **Daffa Hardhan** | PM dan Pengembang | **40 Jam** | 20 Jam | 8 Jam | 6 Jam | 6 Jam | **40 Jam (100%)** |
| **Raka Arrayan M.** | Analis Solusi & BoM | **35 Jam** | 18 Jam | 7 Jam | 5 Jam | 5 Jam | **35 Jam (100%)** |
| **Siti Amalia N.** | Software & LoRa | **45 Jam** | 22 Jam | 11 Jam | 6 Jam | 6 Jam | **45 Jam (100%)** |
| **Muhammad Ilman Z.**| Hardware & Power | **45 Jam** | 26 Jam | 9 Jam | 5 Jam | 5 Jam | **45 Jam (100%)** |
| **Darrel Alfath** | 3D Casing IoT & QA | **40 Jam** | 20 Jam | 8 Jam | 6 Jam | 6 Jam | **40 Jam (100%)** |
| **TOTAL TIM** | | **205 Jam** | **106 Jam** | **43 Jam** | **28 Jam** | **28 Jam** | **205 Jam (100%)** |

---

## 5. Prosedur Sinkronisasi Mingguan (*SOP Google Sheets to Git Markdown*)

1. **Setiap Hari Kerja (Senin–Jumat):** Anggota mengisi log aktivitas dan jam kerja aktual pada Google Sheets Live Tracker.
2. **Setiap Jumat Sore:** Formula Google Sheets menghitung persentase ketercapaian target pekanan dan status inventaris.
3. **Setiap Sabtu Pagi:** Manajer Proyek (Daffa) mengonsolidasikan ringkasan capaian ke dalam berkas Markdown laporan mingguan (`reports/group/` dan `reports/individual/`), memperbarui `05_EVIDENCE_REGISTER.md`, dan melakukan *Git Commit* sebagai arsip resmi.
