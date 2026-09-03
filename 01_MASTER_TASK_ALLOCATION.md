# 01 MASTER ALOKASI TUGAS & MATRIKS TANGGUNG JAWAB
## Fixed Responsibility Matrix — Smart-Sanitation eSOS

Status Dokumen: MATRIKS TANGGUNG JAWAB TETAP (CONTROLLED BASELINE)  
Acuan Proyek: Bab 4.4 Pemetaan Keahlian & Bab 5 Proposal Desain Proyek 1 DTE FTUI  
Total Alokasi Waktu: 205 Jam Kerja Semester  

---

## 1. Landasan Penetapan Peran & Keputusan Desain Khusus

Struktur pembagian tanggung jawab ini mengikat seluruh anggota tim tanpa tumpang tindih (*clear separation of concerns*) berdasarkan Bab 4.4 Proposal DP1:
- **Daffa Hardhan:** Manajer Proyek dan Pengembang (40 Jam)
- **Raka Arrayan Muttaqien:** Analis Solusi dan Integrasi Layanan (35 Jam)
- **Siti Amalia Nurfaidah:** Pengembang Perangkat Lunak (45 Jam)
- **Muhammad Ilman Zuhriy:** Perancang Perangkat Keras (45 Jam)
- **Darrel Alfath:** Desainer Mekanis dan Penguji Kualitas (40 Jam)

### Keputusan Desain Khusus (Design & Scope Decision):
1. **Peran Khusus Mekanikal (Darrel Alfath):**  
   Darrel bertanggung jawab penuh atas perancangan **Model 3D CAD Parametrik dan Fabrikasi 3D Print Enclosure / Casing untuk seluruh perangkat IoT Smart-Sanitation eSOS** (kompartemen ESP32, dudukan probe ultrasonik JSN-SR04T, kisi ventilasi sensor gas MQ-137/136, housing baterai 18650, alur gasket silikon RTV IP54, dan klem mounting eksternal).
2. **Keputusan Perakitan Elektrikal (No Custom Group PCB):**  
   Untuk meminimalkan risiko kegagalan fabrikasi, menekan lead-time manufaktur, dan mempercepat integrasi mekanik, sistem **TIDAK MENGGUNAKAN PCB CUSTOM KELOMPOK (No custom fabricated/etched PCB)**. Perakitan elektrikal menggunakan modul breakout board standar industri (ESP32 DevKit, modul TP4056 BMS, breakout MQ, breakout LoRa) yang diintegrasikan secara kokoh menggunakan *high-reliability perfboard / terminal shield wiring harness* oleh Muhammad Ilman Zuhriy.
3. **Model Pengadaan Kolektif Terdistribusi (Collaborative Joint Procurement by All 5 Members):**  
   Pengadaan 15 komponen BoM **dilaksanakan secara bersama oleh seluruh 5 anggota tim**, di mana setiap anggota mengeksekusi pembelian komponen sesuai domain subsistemnya (Daffa: Router CPE220 & Network, Siti: ESP32 & LoRa, Ilman: Daya Solar & Sensor Gas, Darrel: Casing 3D, Servo & Gasket, Raka: Konsolidasi Finansial & Audit Invoice).

---

## 2. Rincian Batas Tanggung Jawab Domain (Domain Boundaries)

### 2.1 Daffa Hardhan — *Project Management, System Architecture, Web Dashboard & Data Pipeline*
* **Tanggung Jawab Utama (Primary PIC):**
  - **Pembangunan Web Dashboard Pemantauan Real-Time:** Frontend UI interaktif (HTML5, Tailwind CSS, Chart.js) untuk visualisasi level air, konsentrasi gas amonia/H2S, tegangan baterai, alarm SOS, dan kendali servo.
  - **Pembangunan Pipeline Data Sensor ke Basis Data:** Backend server Go berkinerja tinggi (`src/server/`), Streaming Ingestion, Batch ETL Engine, binary mandiri (`src/bin/esos-server.exe`), basis data SQLite WAL mode, dan skema PostgreSQL ACID UUIDv7 (`src/server/database/postgres_schema.sql`).
  - **Konfigurasi & Integrasi Jaringan Outdoor TP-Link CPE220:** Jaringan wireless jarak jauh 2.4GHz (`src/config/network_cpe220.conf`) menjembatani gateway sensor bilik sanitasi ke server posko.
  - **Pengadaan Komponen Jaringan:** Eksekusi pembelian Access Point Outdoor TP-Link CPE220 dan kabel jaringan outdoor STP Cat6.
  - Master WBS 14 Pekan, linimasa semester, jalur kritis, dan tautan live Google Sheets.
  - Tata kelola repositori Git dan sistem issue tracker proyek.
  - Koordinasi integrasi lintas domain dan audit Phase Gate 1 s.d. 4.
  - Konsolidasi Laporan Kemajuan Kelompok Pekanan (Pekan 1 s.d. 14).
* **Batasan Kerja (Tidak Mengambil Alih):**
  - Wiring elektrikal daya dan kalibrasi sensor analog $\rightarrow$ Ilman.
  - Firmware mikrokontroler dan stack LoRa $\rightarrow$ Siti.
  - 3D CAD casing enclosure IoT dan weatherproofing $\rightarrow$ Darrel.

### 2.2 Raka Arrayan Muttaqien — *Solution Analysis, Sanitation Standards & Financial Consolidation*
* **Tanggung Jawab Utama (Primary PIC):**
  - Analisis kebutuhan fungsional dan Requirements Traceability Matrix (RTM).
  - Verifikasi kepatuhan terhadap standar sanitasi darurat dan keselamatan (Permenkes).
  - Komparasi spesifikasi teknis 15 komponen BoM terhadap target proposal.
  - Pengumpulan dan pengarsipan lembar data resmi (*datasheets*) pabrikan.
  - **Konsolidasi Finansial & Audit Pengadaan:** Menghimpun seluruh kuitansi/faktur pembelian dari 5 anggota ke dalam bundel resmi `assets/procurement/receipts_bundle.pdf` (`EV-W2-PROC-001`).
  - Pengendalian anggaran proyek (Committed Cost $\le$ Ceiling Rp2.000.000).
  - Penanganan variansi harga sensor gas MQ-137/136 (`GATE-PROC-01` & `GATE-PROC-02`).
  - Pembuatan buku register aset inventaris proyek.
* **Aturan Penerimaan Teknis (*Technical Acceptance Rule*):**
  - Seluruh 5 anggota membeli komponen sesuai subsistem masing-masing, dan penerimaan teknis wajib diuji oleh Domain Engineer:
    - *Hardware acceptance* $\rightarrow$ Ilman.
    - *Software/tool acceptance* $\rightarrow$ Siti / Daffa.
    - *Mechanical material acceptance* $\rightarrow$ Darrel.

### 2.3 Siti Amalia Nurfaidah — *Embedded Software, LoRa & Control*
* **Tanggung Jawab Utama (Primary PIC):**
  - Penyiapan lingkungan pengembangan (VS Code PlatformIO / Arduino IDE).
  - Pengembangan repositori kode sumber firmware modular ESP32 DevKitC V4.
  - Pengolahan sinyal digital dan analog (GPIO, ADC 12-bit sampling, LEDC PWM).
  - Pembuatan driver sensor ultrasonik waterproof JSN-SR04T (non-blocking).
  - Pengembangan stack komunikasi nirkabel LoRa RA-02 433 MHz (SX1278 SPI).
  - Algoritma kontrol aktuator servo MG996R dan proteksi batas gerak.
  - Perancangan struktur serialisasi paket data telemetri (biner / JSON).
  - Eksekusi unit test perangkat lunak dan penanganan error handling.

### 2.4 Muhammad Ilman Zuhriy — *Hardware, Power Management & Sensor Calibration*
* **Tanggung Jawab Utama (Primary PIC):**
  - Arsitektur kelistrikan sistem dan diagram pengkabelan terpusat.
  - Perhitungan anggaran daya (*power budget*) dan efisiensi solar 10Wp.
  - Perancangan topologi baterai Li-ion 18650 1S4P dan dual modul charger TP4056 BMS.
  - Perakitan modul daya, regulator tegangan 3.3V dan 5V, serta proteksi arus balik.
  - Desain dan perakitan *perfboard wiring harness* modular (tanpa custom PCB etching).
  - Pengkondisian sinyal analog (*signal conditioning*) untuk sensor gas MQ-137 dan MQ-136.
  - Setup instrumen pemanasan awal (*pre-heating*) dan kurva kalibrasi sensor analog.
  - Pengujian stabilitas tegangan rail dan ketahanan suplai daya 24 jam nonstop.

### 2.5 Darrel Alfath — *Mechanical 3D CAD, IoT Enclosure & QA/Testing*
* **Tanggung Jawab Utama (Primary PIC):**
  - **Perancangan Model 3D CAD Parametrik untuk Enclosure / Casing Perangkat IoT eSOS.**
  - Penataan tata letak ruang dalam casing untuk memuat modul ESP32, baterai, sensor, dan servo.
  - Desain alur lidah-alur (*tongue-and-groove*) dan gasket silikon RTV untuk standar IP54.
  - Ekspor berkas CAD 3D format STEP dan penyiapan file STL untuk proses cetak 3D PETG.
  - Fabrikasi dan perakitan fisik casing 3D print, bracket mounting tiang luar, dan fixture mekanik.
  - Desain mekanisme linkage transmisi putar motor servo untuk buka-tutup katup sanitasi.
  - Penyusunan Master Test Plan dan skenario pengujian menyeluruh (TC-01 s.d. TC-06).
  - Pelaksanaan penjaminan mutu fungsional (*functional QA*), uji semprotan air, dan manajemen register bukti.

---

## 3. Matriks Kepemilikan 15 Komponen & Acceptance Owner

| No | ID BoM | Nama Komponen & Aset | Spesifikasi / Qty | Primary PIC | Supporting PIC | Acceptance Owner | Standar Verifikasi |
|:---:|:---:|:---|:---|:---:|:---:|:---:|:---|
| 1 | **HW-01** | ESP32 DevKitC V4 | WROOM-32D Dual Core (2 unit) | **Siti** | Ilman, Daffa | **Siti + Ilman** | Uji boot serial, flash firmware, pinout GPIO/ADC/SPI |
| 2 | **HW-02** | JSN-SR04T Sensor | Ultrasonic Waterproof (1 unit) | **Siti** | Ilman, Darrel | **Siti** | Uji jarak tangki air, deviasi pengukuran $\le 1.0\text{ cm}$ |
| 3 | **HW-03** | MQ-137 Gas Sensor | Ammonia ($NH_3$) Module (1 unit) | **Ilman** | Siti, Raka | **Ilman** | Uji resistansi pemanas ($R_H$), respon analog terhadap gas |
| 4 | **HW-04** | MQ-136 Gas Sensor | $H_2S$ Gas Module (1 unit) | **Ilman** | Siti, Raka | **Ilman** | Uji kurva kalibrasi tegangan analog terhadap ppm |
| 5 | **HW-05** | LoRa RA-02 433 MHz | SX1278 SPI Transceiver (2 unit) | **Siti** | Ilman | **Siti + Ilman** | Uji transmisi nirkabel jarak jauh, Packet Error Rate $< 1\%$ |
| 6 | **HW-06** | MG996R Servo Motor | Metal Gear High-Torque 180° (1 unit)| **Siti** | Ilman, Darrel | **Siti + Darrel** | Uji torsi mekanik, waktu respon $< 250\text{ ms}$, linkage fit |
| 7 | **HW-07** | TP-Link CPE220 AP | Outdoor Wireless 2.4GHz (1 unit) | **Daffa** | Raka, Ilman | **Daffa** | Uji throughput jaringan $> 54\text{ Mbps}$, jangkauan sinyal |
| 8 | **HW-08** | Solar Panel 10 Wp | Monocrystalline 18V (1 unit) | **Ilman** | Raka | **Ilman** | Uji tegangan terbuka ($V_{oc}$) dan arus hubung singkat ($I_{sc}$) |
| 9 | **HW-09** | TP4056 + Protection | Dual BMS Charger Board (2 unit) | **Ilman** | Raka | **Ilman** | Uji pemutusan otomatis overcharge 4.2V & overdischarge 2.5V |
| 10 | **HW-10** | Li-ion 18650 Cell | 3.7V 3000mAh (4 unit / 1S4P) | **Ilman** | Raka | **Ilman** | Uji kapasitas debit teruji $\ge 2950\text{ mAh}$ per sel |
| 11 | **ME-01** | PETG / PC Filament | Spool 500g 1.75mm (1 roll) | **Darrel** | Raka | **Darrel** | Kualitas cetak 3D casing, ketahanan impak dan suhu |
| 12 | **HW-11** | Perfboard & Wiring Kit| Industrial Proto Shield Kit (1 pack)| **Ilman** | Siti | **Ilman** | Kontinuitas jalur wiring harness terisolasi (No Custom PCB)|
| 13 | **HW-12** | Solder Wire 60/40 | High Grade Resin Core (1 roll) | **Ilman** | — | **Ilman** | Daya lekat solder dan integritas sambungan mekanik |
| 14 | **HW-13** | Cable / Heat-shrink | AWG22/24 Wire & Selongsong (1 pack)| **Ilman** | Darrel | **Ilman** | Kekuatan isolasi tegangan dan kerapian perutean kabel |
| 15 | **ME-02** | RTV Silicone Sealant| Waterproof Non-Corrosive (1 tube) | **Darrel** | Ilman | **Darrel** | Uji ketahanan semprotan air IP54 pada alur casing 3D |

---

## 4. Matriks RACI Semester Penuh (Pekan 1 s.d. Pekan 14)

| No | Paket Aktivitas / Deliverable Semester | Accountable (A) | Responsible (R) | Consulted (C) | Informed (I) | Target Luaran Teknis |
|:---:|:---|:---:|:---:|:---:|:---:|:---|
| 1 | Master WBS 14 Pekan & Jadwal Semester | **Daffa** | **Daffa** | All | Dosen | `03_MASTER_SEMESTER_WBS.md` |
| 2 | Requirements Baseline & Matriks RTM | **Raka** | **Raka** | Daffa, All Technical PICs | All | `04_PHASE_GATE_AND_TRACEABILITY.md` |
| 3 | Baseline BoM & Konsolidasi Finansial | **Raka** | **Raka** | Daffa, Ilman, Siti, Darrel | All | `02_BOM_PROCUREMENT.md` |
| 4 | Eksekusi Pengadaan 15 Komponen (Kolektif) | **Daffa & Raka** | **Seluruh 5 Anggota sesuai Domain** | — | All | Bundel Faktur & Aset Fisik Tiba |
| 5 | Desain 3D CAD Casing IoT & Fixture | **Darrel** | **Darrel** | Ilman, Siti | Daffa, Raka | Berkas CAD `.step` & File `.stl` |
| 6 | Fabrikasi 3D Printing Casing PETG | **Darrel** | **Darrel** | Ilman | All | Casing 3D Fisik IP54 |
| 7 | Power Management & Wiring Harness | **Ilman** | **Ilman** | Siti, Darrel | Daffa, Raka | Sirkuit Daya Solar & Harness Fisik |
| 8 | Setup Kalibrasi Sensor Gas Analog MQ | **Ilman** | **Ilman** | Siti, Raka | All | Dataset Kurva Kalibrasi Gas |
| 9 | Firmware ESP32 & Driver Modular | **Siti** | **Siti** | Ilman, Daffa | Darrel, Raka | Repositori Kode `/src/firmware/` |
| 10 | Stack Komunikasi LoRa 433 MHz | **Siti** | **Siti** | Ilman | Daffa | Modul Transmisi Nirkabel |
| 11 | Local Server Dashboard & CPE220 AP | **Daffa** | **Daffa** | Ilman, Siti | All | Backend & Dashboard Telemetri |
| 12 | Integrasi Fisik & Fungsional Sistem | **Daffa** | **Daffa + Domain PICs** | All | All | Prototipe Terintegrasi eSOS |
| 13 | Master Test Plan & Eksekusi QA (TC-01..06)| **Darrel** | **Darrel + Domain PICs**| All | Dosen | Laporan Hasil Pengujian QA |
| 14 | Evaluasi Tengah Semester (UTS / Gate 3) | **All (Lead: Daffa)**| **All** | — | Dosen | Berkas Presentasi Midterm |
| 15 | Laporan Kemajuan Kelompok Mingguan | **Daffa** | **Daffa** | All | Dosen | `reports/group/WEEK_XX_...` |
| 16 | Laporan Logbook Mingguan Individu | **Setiap Anggota**| **Setiap Anggota** | Daffa | Dosen | `reports/individual/...` |
| 17 | Master Evidence Register & Audit | **Daffa** | **Masing-Masing PIC** | Darrel | All | `05_EVIDENCE_REGISTER.md` |
| 18 | Evaluasi Akhir Semester (UAS / Demo) | **All (Lead: Daffa)**| **All** | — | Dosen | Live Prototype Demo & Sidang Akhir |

---

## 5. Tata Kelola Operasional & Definition of Done (DoD)

### Aturan Kepemilikan Tunggal (Single Primary Ownership):
1. Setiap paket kerja pada WBS hanya memiliki **satu PIC utama** yang bertanggung jawab atas kualitas dan ketepatan waktu.
2. Keputusan teknis lintas domain diselesaikan secara musyawarah dan dicatat dalam *Project Decision Log*.
3. Tidak ada tugas dengan status *"milik bersama"* tanpa penanggung jawab eksplisit.

### Kriteria Selesai (Definition of Done — DoD):
Sebuah tugas operasional hanya dinyatakan **`DONE`** apabila memenuhi 7 kriteria mutlak:
1. **Artefak Teknis Tersedia:** Berkas CAD, kode program, atau rangkaian fisik selesai dibuat.
2. **Bukti Terkumpul (Tingkat E3/E4):** Terdapat foto fisik, log data, atau video pengujian nyata.
3. **Keterlacakan Target:** Terhubung langsung ke REQ-ID pada RTM dan kode WBS.
4. **Self-Check PIC:** PIC utama telah memverifikasi kesesuaian terhadap parameter desain.
5. **Acceptance Owner Sign-off:** Telah diuji dan disetujui oleh pemilik penerimaan teknis.
6. **Tercatat di Register Bukti:** Kode bukti terdaftar pada `05_EVIDENCE_REGISTER.md`.
7. **Terdokumentasi pada Laporan:** Dimasukkan ke dalam laporan individu dan laporan kelompok.
