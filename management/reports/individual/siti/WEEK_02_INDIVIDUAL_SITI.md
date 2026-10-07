# LAPORAN PEKANAN INDIVIDU — SITI AMALIA NURFAIDAH (PEKAN 2)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Siti Amalia Nurfaidah  
NPM: 2306161851  
Peran Proposal: Pengembang Perangkat Lunak (Alokasi: 45 Jam)  
Kelompok: 4 (Empat) | Pekan: 2 (Dua) | Periode: 02 September 2026 – 08 September 2026  
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  

---

### A. IDENTITAS
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Pelaksanaan:** Pekan 2 (Dua)
- **Periode Waktu:** 02 September 2026 – 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.

---

### B. TARGET MINGGUAN (PEKAN 2)
- Menginstal lingkungan pengembangan IDE PlatformIO dan memvalidasi koneksi serial ESP32.
- Mengompilasi dan mengunggah program baseline firmware v1.0 tanpa galat.
- Menguji fungsi periferal input/output (GPIO), konverter analog ADC 12-bit, dan sinyal PWM servo.
- Mengimplementasikan stack komunikasi LoRa RA-02 433 MHz dan struktur payload serialisasi data.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 2: 23.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 02 Sep 2026 | Instalasi IDE, toolchain PlatformIO, dan verifikasi board ESP32 | Log serial monitor koneksi USB | EV-W2-SW-001 | 4.5 |
| Kamis, 03 Sep 2026 | Pengujian fungsional periferal dasar ESP32 (GPIO, ADC, PWM timer) | Log uji periferal hardware | EV-W2-SW-003 | 5.0 |
| Jumat, 04 Sep 2026 | Implementasi library LoRa SX1278 (SPI) dan pengujian transmisi data | Modul firmware LoRa TX/RX | EV-W2-SW-004 | 5.0 |
| Sabtu, 05 Sep 2026 | Pembuatan logika otomasi aktuator servo MG996R nirsentuh | Algoritma kendali aktuasi servo | EV-W2-SW-003 | 4.5 |
| Senin, 07 Sep 2026 | Integrasi seluruh modul ke State Machine firmware release v1.0 | Berkas firmware release v1.0 | EV-W2-SW-002 | 4.0 |
| **TOTAL PEKAN 2** | | | | **23.0 Jam** |

**TOTAL AKUMULASI JAM (W1 + W2):** **45.0 Jam** *(100% Target Proposal).*

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W2-SW-01** | Toolchain PlatformIO | IDE terkonfigurasi dengan board support package ESP32 | `DONE` | EV-W2-SW-001 |
| **W2-SW-05** | Baseline firmware ESP32 | Kode baseline terkompilasi dan sukses diunggah ke mikrokontroler | `DONE` | EV-W2-SW-002 |
| **W2-SW-07** | Pengujian konversi ADC | Sampling ADC 12-bit terkalibrasi dengan moving average filter | `DONE` | EV-W2-SW-003 |
| **W2-SW-09** | Stack komunikasi LoRa | Transmisi paket data LoRa 433 MHz stabil dengan PER $0.8\%$ | `DONE` | EV-W2-SW-004 |
| **W2-SW-10** | Kendali aktuasi servo | PWM LEDC 50Hz mengontrol servo dengan waktu respon $180\text{ ms}$ | `DONE` | EV-W2-SW-003 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Repositori kode sumber firmware C++ modular pada direktori `/src/firmware/`, pustaka LoRa SX1278, dan driver servo MG996R.
- **Rincian Teknis:** Berhasil membangun stack firmware event-driven berbasis hardware timer interrupts. Transmisi nirkabel LoRa 433 MHz terkonfigurasi pada bandwidth $125\text{ kHz}$, spreading factor $\text{SF}=7$, dan coding rate $4/5$, menghasilkan packet error rate sangat rendah ($0.8\%$). Driver PWM LEDC mengendalikan sudut putar servo $0^\circ-90^\circ$ secara halus tanpa jitter.
- **Penerimaan Teknis:** Kriteria perangkat lunak Phase Gate 2 disetujui (G2.3: PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Gangguan noise pembacaan ADC saat modul LoRa memancarkan daya RF tinggi secara simultan.
- **Tindakan Korektif:** Menerapkan software Moving Average Filter 16 sampel dan melakukan jeda pembacaan sampling ADC di luar jendela transmisi LoRa.
- **Hasil:** Nilai pembacaan analog stabil dengan fluktuasi kurang dari $1.5\%$.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-SW-001`, `EV-W2-SW-002`, `EV-W2-SW-003`, `EV-W2-SW-004`.
- **Kontribusi:** Menghadirkan firmware mikrokontroler yang stabil, cepat, dan siap diintegrasikan dengan rangkaian hardware Ilman serta casing 3D Darrel.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 23h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 3)
- Pengujian firmware pada wiring harness terisolasi fisik buatan Ilman dan penalaan respon ppm sensor gas.

---

### L. PERSETUJUAN
- **Mahasiswa:** Siti Amalia Nurfaidah *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
