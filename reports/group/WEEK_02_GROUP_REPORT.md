# LAPORAN KEMAJUAN PEKANAN 2
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2 (DTE FTUI)

---

### Informasi Umum Proyek
- **Mata Kuliah:** Desain Proyek 2 (Gasal 2026/2027)
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Ke-:** 2 (Dua) — Fase Pengadaan dan Persiapan Implementasi Sistem
- **Periode:** 02 September 2026 – 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
- **Ketua Kelompok:** Daffa Hardhan

---

### 1. Ringkasan Kemajuan Pekanan 2
Sesuai dengan CPMK Pekan 2 (*"Mahasiswa mampu menyiapkan seluruh sumber daya yang diperlukan untuk mewujudkan sistem yang dirancang"*), fokus utama seluruh anggota Kelompok 4 pada pekan ini adalah **menyiapkan seluruh sumber daya proyek**:
1. **Pengadaan Komponen & Manajemen Anggaran:** Memverifikasi spesifikasi teknis 15 komponen BoM, memeriksa ketersediaan stok vendor marketplace, mengeksekusi pemesanan secara terdistribusi oleh 5 anggota tim, serta menyusun daftar register aset proyek di bawah plafon Rp2.000.000.
2. **Persiapan Lingkungan Pengembangan Firmware:** Instalasi IDE PlatformIO pada VS Code, board framework ESP32, dan penyiapan pustaka dependensi (`arduino-LoRa`, `ArduinoJson`, `ESP32Servo`).
3. **Inisiatif Pengembangan Server Go & Repositori Git:** Daffa Hardhan menginisiasi pembuatan backend server Go berkinerja tinggi (`src/server/main.go`), binary mandiri (`src/bin/esos-server.exe`), basis data lokal SQLite WAL mode dengan kunci UUIDv7 (`src/data/esos_telemetry.db`), serta antarmuka web dashboard pemantauan.
4. **Persiapan Desain Mekanikal:** Finalisasi gambar teknik 2D dan persiapan berkas desain 3D CAD casing enclosure IoT kustom berstandar IP54 untuk persiapan cetak 3D di Pekan 3.
5. **Sistem Dokumentasi Proyek:** Repositori GitHub resmi aktif, template master WBS 14 pekan, template logbook harian 205 jam, dan matriks pengujian (TC-RTM) aktif di Google Sheets.

---

### 2. Target & Realisasi Aktivitas Mahasiswa Pekan 2

| Bidang | Aktivitas Sesuai Silabus DP2 | Realisasi Capaian Pekan 2 | Penanggung Jawab | Status |
|:---|:---|:---|:---:|:---:|
| **Hardware** | 1. Verifikasi spesifikasi seluruh komponen <br> 2. Pengadaan komponen <br> 3. Pemeriksaan ketersediaan stok <br> 4. Pembuatan daftar aset proyek | • Tabel 15 komponen BoM terverifikasi spesifikasinya <br> • Pemesanan dilakukan kolektif oleh 5 anggota <br> • Seluruh tautan toko marketplace dipastikan aktif <br> • Daftar register aset & bundel faktur dibuat | **M. Ilman Zuhriy & Raka Arrayan M.** | **`TERCAPAI`** |
| **Software** | 1. Instalasi IDE dan tool pengembangan <br> 2. Konfigurasi server/database <br> 3. Persiapan repository Git | • VS Code & PlatformIO terpasang untuk ESP32 <br> • Inisiatif Daffa: Go server, DB SQLite WAL, REST API & Web UI <br> • Repositori Git & remote GitHub aktif | **Daffa Hardhan & Siti Amalia N.** | **`TERCAPAI`** |
| **Mekanik** | 1. Finalisasi gambar teknik <br> 2. Persiapan desain casing atau fixture | • Gambar teknik kerja 2D dimensi toleransi selesai <br> • Berkas 3D CAD casing IoT modular disiapkan | **Darrel Alfath** | **`TERCAPAI`** |
| **Dokumentasi** | 1. Menyiapkan format logbook mingguan <br> 2. Menyiapkan template dokumentasi uji | • Tab `Daily_Logbook_Tracking` 14 pekan aktif <br> • Template matriks uji TC-01..06 (RTM) aktif | **Daffa Hardhan & Raka Arrayan M.** | **`TERCAPAI`** |

---

### 3. Kemajuan & Alokasi Jam Kerja Anggota Pekan Ini

| No | Nama Anggota | Domain Peran | Fokus Aktivitas Pekan 2 | Realisasi Jam Pekan 2 | Akumulasi W1+W2 |
|:---:|:---|:---|:---|:---:|:---:|
| 1 | **Daffa Hardhan** | PM & Backend Lead | Setup Git GitHub, inisiasi Go server, pipeline database SQLite WAL UUIDv7, Web Dashboard UI, & koordinasi WBS | 8 Jam | 15 Jam |
| 2 | **Raka Arrayan M.** | Solution & Finance Lead | Verifikasi BoM 15 komponen, audit anggaran belanja (Hemat Rp72.500), kompilasi faktur kuitansi, cek stok vendor | 6 Jam | 11 Jam |
| 3 | **Siti Amalia N.** | Firmware Lead | Setup IDE VS Code + PlatformIO, board package ESP32, konfigurasi pinout macros, & uji kompilasi baseline | 8 Jam | 15 Jam |
| 4 | **Muhammad Ilman Z.** | Hardware & Power Lead | Perhitungan power budget solar 10Wp, topologi baterai 1S4P, skematik kelistrikan, & persiapan wiring harness | 8 Jam | 16 Jam |
| 5 | **Darrel Alfath** | Mechanical & QA Lead | Pembuatan gambar teknik 2D, finalisasi model 3D CAD casing enclosure IP54, & persiapan slicer cetak 3D | 7 Jam | 14 Jam |
| | **TOTAL TIM** | | | **37 Jam** | **71 Jam** |

> *Catatan:* Target 205 jam adalah pagu komitmen seluruh semester (14 pekan). Akumulasi 71 jam pada Pekan 2 menunjukkan alokasi jam kerja tim berjalan tepat waktu dan proporsional.

---

### 4. Evaluasi Penugasan Pekan 2

#### Penugasan 1: Bill of Materials (BoM) Final
* **Pagu Maksimum Proposal (*Budget Ceiling*):** Rp2.000.000
* **Target RAB Proposal (15 Komponen):** Rp1.901.000
* **Total Realisasi Belanja Aktual Toko:** **Rp1.828.500**
* **Status Finansial:** **`HEMAT Rp72.500`** (Sisa saldo kas surplus: **Rp171.500**).
* **Tabel Komponen:** 15 item lengkap dengan spesifikasi teknis minimum, vendor, dan tautan belanja aktif di [`02_BOM_PROCUREMENT.md`](https://github.com/DHard4114/DESPRO-4/blob/main/02_BOM_PROCUREMENT.md).

#### Penugasan 2: Laporan Status Pengadaan
* Pengadaan 15 komponen dieksekusi secara **kolektif terdistribusi oleh 5 anggota tim** sesuai domain masing-masing.
* **100% komponen (15 dari 15 item)** telah dipesan dan dialokasikan ke anggota penanggung jawab.
* Sistem **TIDAK MENGGUNAKAN PCB CUSTOM KELOMPOK (*No custom group PCB*)**, melainkan menggunakan modul breakout board standar industri yang dihubungkan dengan *perfboard wiring harness* terisolasi oleh Muhammad Ilman Zuhriy.

#### Penugasan 3: Dokumentasi Setup Lingkungan Pengembangan
* **Firmware ESP32:** VS Code + PlatformIO Core (`src/firmware/platformio.ini`), Arduino Framework Espressif 32.
* **Backend Server (Inisiatif Daffa):** Go v1.22+, Streaming Ingestion & Batch ETL Pipeline, basis data SQLite WAL Mode dengan primary key UUIDv7 (`src/data/esos_telemetry.db`), WebSocket Hub, dan binary executable mandiri (`src/bin/esos-server.exe`).
* **Panduan Operasional:** Tercatat lengkap pada `00_MASTER_README.md` Bab 4.

---

### 5. Evaluasi 3 Luaran Pekan 2 (*Exit Criteria*)

| No | Luaran Wajib Pekan 2 | Target Silabus | Realisasi Kelompok 4 | Status |
|:---:|:---|:---|:---|:---:|
| 1 | **Ketersediaan Komponen** | Minimal 80% komponen telah tersedia | 15 dari 15 item BoM (100%) telah dipesan & tersedia | **`TERCAPAI (100%)`** |
| 2 | **Lingkungan Pengembangan** | Lingkungan pengembangan siap digunakan | Toolchain PlatformIO ESP32 siap, dan server Go mandiri aktif | **`TERCAPAI (100%)`** |
| 3 | **Sistem Dokumentasi** | Sistem dokumentasi proyek aktif | Repositori GitHub aktif, 5 master docs terkendali, 5 tabs sheets | **`TERCAPAI (100%)`** |

---

### 6. Kendala dan Solusi Pekan 2
| No | Kendala yang Dihadapi | Dampak Terhadap Proyek | Solusi dan Tindakan Korektif yang Dilakukan |
|:---:|:---|:---|:---|
| 1 | Variasi harga sensor gas MQ-137 antar toko marketplace | Potensi pembengkakan anggaran | Raka membandingkan 3 vendor resmi dan mengunci harga terbaik (Rp155.000) sehingga tetap hemat. |
| 2 | Keterbatasan waktu kompilasi jika anggota lain belum menginstal Go SDK | Ketergantungan dependensi runtime | Daffa mengompilasi backend menjadi *single native binary* (`src/bin/esos-server.exe`) sehingga dapat langsung dijalankan tanpa instalasi Go. |

---

### 7. Rencana Kerja Pekan Berikutnya (Pekan 3: Awal Fabrikasi & Perakitan Hardware)
1. **Mekanikal (Darrel):** Memulai proses cetak 3D printing casing IoT menggunakan filamen PETG di lab/workshop.
2. **Hardware (Ilman):** Memulai perakitan rangkaian daya solar 10Wp, modul TP4056 BMS, dan wiring harness perfboard (tanpa custom PCB).
3. **Firmware (Siti):** Mengembangkan driver sampling pembacaan ADC sensor gas dan filter digital jarak ultrasonik.
4. **Backend (Daffa):** Menyiapkan skenario integrasi API ingest telemetri dan WebSocket stream dengan format paket data sensor.
5. **Finansial & Standar (Raka):** Mengarsipkan fisik faktur pembelian dan menyinkronkan status pengadaan di Google Sheets.

---

### Lembar Pengesahan Laporan Pekan 2

| Jabatan | Nama Mahasiswa / Dosen | Status Tanda Tangan | Tanggal Pengesahan |
|:---|:---|:---:|:---:|
| **Ketua Kelompok** | Daffa Hardhan (NPM 2306161763) | *(Disahkan secara digital)* | 08 September 2026 |
| **Dosen Pembimbing** | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ........................................ | ........................................ |
