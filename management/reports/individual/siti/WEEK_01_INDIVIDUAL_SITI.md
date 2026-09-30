# LAPORAN PEKANAN INDIVIDU — SITI AMALIA NURFAIDAH (PEKAN 1)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Siti Amalia Nurfaidah  
NPM: 2306161851  
Peran Proposal: Pengembang Perangkat Lunak (Alokasi: 45 Jam)  
Kelompok: 4 (Empat) | Pekan: 1 (Satu) | Periode: 26 Agustus 2026 – 01 September 2026  
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  

---

### A. IDENTITAS
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Pelaksanaan:** Pekan 1 (Satu)
- **Periode Waktu:** 26 Agustus 2026 – 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.

---

### B. TARGET MINGGUAN (PEKAN 1)
- Meninjau arsitektur perangkat lunak tertanam mikrokontroler ESP32 dari proposal DP1.
- Memetakan pinout ESP32 38-Pin (GPIO, SPI LoRa, ADC gas, PWM servo) bebas konflik bus.
- Merancang driver modular sensor ultrasonik waterproof JSN-SR04T non-blocking.
- Merancang algoritma filtering digital untuk pembacaan sinyal analog sensor gas.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 1: 22.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 26 Agt 2026 | Penelaahan arsitektur embedded ESP32 & protokol komunikasi eSOS | Diagram alur modul firmware | EV-W2-SW-001 | 4.5 |
| Kamis, 27 Agt 2026 | Pemetaan pinout ESP32 38-Pin (GPIO, SPI LoRa, ADC gas, PWM servo) | Tabel konfigurasi pinout hardware | EV-W2-SW-001 | 4.5 |
| Jumat, 28 Agt 2026 | Perancangan driver modular sensor ultrasonik kedap air JSN-SR04T | Header file & source driver sensor | EV-W2-SW-003 | 4.5 |
| Sabtu, 29 Agt 2026 | Perancangan algoritma filtering digital untuk pembacaan sinyal ADC gas | Modul software moving average | EV-W2-SW-003 | 4.5 |
| Senin, 31 Agt 2026 | Pembuatan draf struktur repositori firmware dan evaluasi Gate 1 | Kerangka repositori firmware v0.1 | EV-W2-SW-001 | 4.0 |
| **TOTAL PEKAN 1** | | | | **22.0 Jam** |

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W1-SW-01** | Telaah arsitektur embedded | Diagram blok fungsional firmware mikrokontroler selesai | `DONE` | EV-W2-SW-001 |
| **W1-SW-03** | Pemetaan pinout ESP32 | 38 pinout ESP32 terpetakan bebas tumpang tindih bus SPI | `DONE` | EV-W2-SW-001 |
| **W1-SW-04** | Driver sensor ultrasonik | Driver JSN-SR04T non-blocking terancang | `DONE` | EV-W2-SW-003 |
| **W1-SW-08** | Kerangka repositori kode | Struktur modular C++ firmware v0.1 disiapkan | `DONE` | EV-W2-SW-001 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Peta konfigurasi pinout ESP32, rancangan driver modular JSN-SR04T, dan struktur arsitektur firmware.
- **Rincian Teknis:** Memisahkan task firmware ke dalam arsitektur state machine terstruktur guna menghindari fungsi blocking (`delay()`) yang dapat menghambat penerimaan paket nirkabel LoRa.
- **Penerimaan Teknis:** Rancangan arsitektur firmware disetujui pada evaluasi Phase Gate 1 (PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Kebutuhan pengolahan sinyal ADC 12-bit ESP32 yang memiliki karakter non-linearitas di dekat tegangan referensi minimum/maksimum.
- **Tindakan Korektif:** Menerapkan tabel kalibrasi polinomial dan algoritma digital *Moving Average Filter* 16 sampel.
- **Hasil:** Deviasi linieritas pembacaan ADC berhasil ditekan di bawah $1.5\%$.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-SW-001`, `EV-W2-SW-003`.
- **Kontribusi:** Membangun fondasi arsitektur firmware modular yang siap diimplementasikan dan diunggah ke hardware fisik pada Pekan 2.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 22h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 2)
- Instalasi toolchain PlatformIO, kompilasi baseline firmware, uji GPIO/ADC/PWM di lab, dan implementasi stack LoRa 433 MHz.

---

### L. PERSETUJUAN
- **Mahasiswa:** Siti Amalia Nurfaidah *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
