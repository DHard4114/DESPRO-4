# LAPORAN PEKANAN INDIVIDU — MUHAMMAD ILMAN ZUHRIY (PEKAN 1)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Muhammad Ilman Zuhriy  
NPM: 2306266786  
Peran Proposal: Perancang Perangkat Keras (Alokasi: 45 Jam)  
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
- Meninjau arsitektur kelistrikan dan konsumsi arus beban sistem eSOS dari proposal DP1.
- Menghitung anggaran daya (*power budget*) dan memverifikasi kapasitas solar panel 10Wp serta baterai 18650.
- Merancang skematik sirkuit kelistrikan manajemen daya dan pengkondisi sinyal sensor gas analog.
- Menyiapkan instrumen pemanasan awal (*pre-heating*) sensor gas amonia ($NH_3$) dan $H_2S$.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 1: 26.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 26 Agt 2026 | Analisis konsumsi arus beban aktif ESP32, LoRa TX, dan heater MQ | Tabel analisis profil arus beban | EV-W2-HW-002 | 5.0 |
| Kamis, 27 Agt 2026 | Sizing kapasitas panel surya 10 Wp dan baterai 18650 operasi 24 jam | Lembar kalkulasi power budget | EV-W2-HW-002 | 5.0 |
| Jumat, 28 Agt 2026 | Perancangan skematik sirkuit utama manajemen daya dan regulator | File skematik KiCad v1.0 | EV-W2-HW-001 | 6.0 |
| Sabtu, 29 Agt 2026 | Desain sirkuit pengkondisi sinyal analog sensor MQ-137 dan MQ-136 | Skematik modul buffer/filter analog | EV-W2-HW-001 | 5.0 |
| Senin, 31 Agt 2026 | Uji pemanasan awal (*pre-heating*) sensor gas dan evaluasi Gate 1 | Log suhu dan resistansi pemanas | EV-W2-HW-005 | 5.0 |
| **TOTAL PEKAN 1** | | | | **26.0 Jam** |

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W1-HW-01** | Arsitektur kelistrikan | Diagram blok pengkabelan dan distribusi daya tuntas | `DONE` | EV-W2-HW-001 |
| **W1-HW-02** | Kalkulasi power budget | Perhitungan membuktikan solar 10Wp surplus $42.5\%$ beban kontinu | `DONE` | EV-W2-HW-002 |
| **W1-HW-04** | Kebutuhan arus sensor | Arus pemanas sensor MQ ($\approx 150\text{ mA}$) terisolasi aman | `DONE` | EV-W2-HW-001 |
| **W1-HW-05** | Skematik sirkuit sistem | Skematik manajemen daya 3.3V dan 5V tervalidasi DRC | `DONE` | EV-W2-HW-001 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Skematik kelistrikan sistem (`assets/schematics/schematic_v1.pdf`) dan lembar kalkulasi power budget.
- **Rincian Teknis:** Membuktikan secara matematis bahwa panel surya 10 Wp dan susunan baterai Li-ion 18650 1S4P (12000 mAh) mampu menopang sistem beroperasi mandiri 24 jam tanpa interupsi daya eksternal. Menetapkan keputusan perakitan menggunakan *perfboard wiring harness* modular (tanpa custom PCB etching) untuk mempercepat integrasi.
- **Penerimaan Teknis:** Skematik kelistrikan disetujui pada evaluasi Phase Gate 1 (PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Konsumsi arus pemanas sensor gas MQ yang relatif besar berpotensi menyebabkan drop tegangan sesaat.
- **Tindakan Korektif:** Memisahkan rel daya pemanas sensor gas dan menambahkan kapasitor buffer low-ESR 470uF.
- **Hasil:** Rel tegangan 3.3V mikrokontroler terlindung penuh dari drop tegangan.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-HW-001`, `EV-W2-HW-002`, `EV-W2-HW-005`.
- **Kontribusi:** Merancang fondasi catu daya mandiri dan pengkondisi sinyal analog yang stabil untuk sistem eSOS.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 26h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 2)
- Verifikasi komponen fisik tiba di lab, penguncian topologi TP4056 + baterai 18650 1S4P, dan perakitan wiring harness.

---

### L. PERSETUJUAN
- **Mahasiswa:** Muhammad Ilman Zuhriy *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
