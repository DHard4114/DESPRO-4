# LAPORAN KEMAJUAN PEKANAN 2
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2

---

### Informasi Umum Proyek
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Ke-:** 2 (Dua)
- **Periode:** 02 September 2026 – 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
- **Ketua Kelompok:** Daffa Hardhan

---

### 1. Ringkasan Kemajuan Pekanan
Pada pekan ke-2, tim menyelesaikan fase kesiapan implementasi (*implementation readiness*) mencakup penerimaan dan verifikasi fisik 15 komponen BoM di laboratorium, penyiapan lingkungan pengembangan firmware ESP32 dan pengujian periferal awal, perancangan model 3D CAD casing IoT dan persiapan cetak 3D PETG oleh Darrel Alfath, perancangan arsitektur catu daya solar 10Wp dan topologi baterai 1S4P oleh Muhammad Ilman Zuhriy (tanpa custom PCB), serta aktivasi repositori Git dan task tracker oleh Daffa Hardhan.

---

### 2. Target Pekan Ini
| No | Target Pekerjaan | Status Capaian | Keterangan Verifikasi |
|:---:|:---|:---:|:---|
| 1 | Eksekusi pengadaan dan verifikasi fisik $\ge 80\%$ komponen BoM | [x] Tercapai [ ] Belum | 15/15 item ($100\%$) komponen tiba dan terverifikasi di lab |
| 2 | Penyiapan IDE/toolchain, baseline firmware ESP32, dan uji GPIO/ADC/PWM | [x] Tercapai [ ] Belum | Program baseline terkompilasi dan sukses diunggah via serial |
| 3 | Finalisasi skematik daya surya 10Wp, topologi 1S4P, dan rencana wiring | [x] Tercapai [ ] Belum | Skematik final dan rencana wiring harness terisolasi tuntas |
| 4 | Finalisasi berkas CAD 3D casing IoT (STEP/STL) & gambar teknik 2D | [x] Tercapai [ ] Belum | Berkas 3D CAD tersimpan pada `assets/cad/` siap dicetak 3D |
| 5 | Aktivasi repositori Git, task tracker issue, dan persiapan server lokal | [x] Tercapai [ ] Belum | Repositori Git aktif dan backend server lokal terkonfigurasi |
| 6 | Pengelolaan bukti pengadaan dan pemenuhan kriteria Phase Gate 2 | [x] Tercapai [ ] Belum | 8/8 Kriteria Gate 2 dinyatakan LULUS (PASS) |

---

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil dan Bukti Nyata (Tangible Output / Evidence) |
|:---:|:---|:---|:---|
| 1 | Eksekusi BoM & penerimaan fisik barang | Raka Arrayan M. | 15 komponen diterima, di-unboxing, dan dicatat kuitansinya pada `assets/procurement/` (`EV-W2-PROC-001`). |
| 2 | Toolchain & baseline firmware ESP32 | Siti Amalia N. | Repositori firmware aktif, pengujian GPIO/ADC/PWM sukses tercatat di log terminal (`EV-W2-SW-002`, `EV-W2-SW-003`). |
| 3 | Verifikasi daya elektrikal & topologi baterai | M. Ilman Zuhriy | Skematik sirkuit kelistrikan final (`EV-W2-HW-001`) dan verifikasi topologi 1S4P baterai 18650 (`EV-W2-HW-003`). |
| 4 | Pemodelan 3D CAD casing IoT & weatherproofing | Darrel Alfath | Berkas 3D CAD format STEP/STL (`EV-W2-MECH-002`) dan gambar teknik 2D dimensi (`EV-W2-MECH-001`). |
| 5 | Repositori Git & backend server lokal | Daffa Hardhan | Repositori Git terstandarisasi, issue board aktif, dan server data sink lokal (`EV-W2-PM-001`). |

---

### 4. Kemajuan Tiap Anggota
| No | Nama Anggota | Tugas Pekan Ini | Realisasi Jam Kerja | Persentase Penyelesaian |
|:---:|:---|:---|:---:|:---:|
| 1 | Daffa Hardhan | Setup Git, task tracker, local server, koordinasi Gate 2 | 20 Jam | 100% (Target W2 tercapai) |
| 2 | Raka Arrayan M. | Eksekusi BoM, kuitansi invoice, register aset, mitigasi MQ | 17 Jam | 100% (Target W2 tercapai) |
| 3 | Siti Amalia N. | Toolchain IDE, baseline firmware, uji GPIO/ADC/PWM | 23 Jam | 100% (Target W2 tercapai) |
| 4 | M. Ilman Zuhriy | Verifikasi hardware, topologi baterai, rencana wiring harness | 19 Jam | 100% (Target W2 tercapai) |
| 5 | Darrel Alfath | Gambar teknik 2D, 3D CAD casing IoT, rencana cetak PETG | 20 Jam | 100% (Target W2 tercapai) |

**Total Jam Kerja Kelompok Terakumulasi:** **205 Jam Kerja** (100% dari target proposal Bab 4.4).

---

### 5. Hasil Implementasi

#### Perangkat Keras (Hardware)
Seluruh 15 item komponen fisik telah tiba di laboratorium. Modul daya surya 10Wp dan baterai 18650 terverifikasi mampu menopang beban sistem dengan surplus daya $> 40\%$. Arsitektur perakitan dipastikan menggunakan modul breakout berkualitas tinggi dan *perfboard wiring harness* (tanpa custom PCB kelompok), sehingga siap untuk perakitan fisik pada Pekan 3.

#### Perangkat Lunak (Software)
Toolchain PlatformIO dan board package ESP32 berhasil dikonfigurasi. Kode program baseline v1.0 sukses dikompilasi dan diunggah ke mikrokontroler. Pengujian konversi ADC 12-bit terbukti stabil dengan filter digital *Moving Average*, sinyal PWM servo terverifikasi presisi, dan pustaka komunikasi LoRa SX1278 siap diintegrasikan.

#### Mekanikal (Mechanical)
Darrel Alfath telah memfinalisasi model 3D CAD casing enclosure IoT kustom berstandar IP54 dengan alur gasket silikon RTV. Dimensi ruang dalam telah dikunci untuk memuat ESP32, baterai 18650, dan sensor dengan toleransi fit check yang presisi, siap dieksekusi untuk proses 3D printing filamen PETG.

---

### 6. Dokumentasi Hasil
1. **Foto Fisik Komponen yang Tiba:** Dokumentasi unboxing dan foto 15 komponen BoM (`assets/procurement/asset_photos/`).
2. **Tangkapan Layar Toolchain dan Serial Monitor:** Log terminal kompilasi program dan serial monitor ESP32 (`assets/docs/ide_toolchain_setup.png`).
3. **Render dan Gambar Teknik 3D CAD Casing IoT:** Berkas model 3D CAD assembly casing IoT (`assets/cad/sanitation_modular_v1.step`).

---

### 7. Pengujian yang Dilakukan (Kesiapan Implementasi)
| Parameter Pengujian | Target Spesifikasi | Hasil Pengujian Pekan 2 | Status Kelulusan |
|:---|:---|:---|:---:|
| Ketersediaan Komponen BoM | $\ge 80\%$ komponen fisik tersedia | 15/15 item ($100\%$) komponen tersedia di lab | [x] Lulus [ ] Tidak |
| Kompilasi Baseline Firmware | 0 Galat kompilasi & upload serial sukses | Program berjalan normal membaca GPIO/ADC | [x] Lulus [ ] Tidak |
| Fit Check Dimensi CAD Casing | Clearance toleransi komponen $\ge 1.5\text{ mm}$ | Seluruh modul terpasang pas di ruang CAD | [x] Lulus [ ] Tidak |
| Regulasi Tegangan Rail Catu Daya | Tegangan 3.3V dan 5V stabil $\pm 2\%$ | Tegangan 3.30V & 5.02V dengan ripple $< 15\text{ mV}$ | [x] Lulus [ ] Tidak |

#### Analisis Singkat
Seluruh pengujian kesiapan implementasi (*Implementation Readiness Tests*) menunjukkan bahwa seluruh subsistem perangkat keras, perangkat lunak, mekanik, dan repositori telah memenuhi kriteria keluar (*exit criteria*) dan siap masuk ke tahap fabrikasi dan integrasi fisik pada Pekan 3.

---

### 8. Kendala dan Solusi
| No | Kendala yang Dihadapi | Dampak Terhadap Proyek | Solusi dan Tindakan Korektif yang Dilakukan |
|:---:|:---|:---|:---|
| 1 | Fluktuasi nilai ADC saat transmisi LoRa aktif pada daya pancar tinggi | Spiking pembacaan analog gas | Penerapan software Moving Average Filter 16 sampel dan sinkronisasi jendela pembacaan ADC. |
| 2 | Deformasi sudut (*warping*) saat pencetakan 3D bidang datar lebar | Celah pada sambungan penutup casing | Pengaturan suhu bed 85°C, penambahan brim 8mm, dan pelapisan gasket silikon RTV. |

---

### 9. Deviasi Terhadap Jadwal
- **Aktivitas:** Semua aktivitas terlaksana sesuai jadwal awal WBS Pekan 2.
- **Analisis Deviasi:** Nol deviasi negatif (*zero critical delay*); seluruh prasyarat implementasi tuntas tepat waktu.

---

### 10. Rencana Kerja Pekan Berikutnya (Pekan 3: Fabrikasi Casing 3D & Perakitan Wiring)
| No | Rencana Aktivitas Operasional | Penanggung Jawab (PIC) | Target Waktu Penyelesaian |
|:---:|:---|:---:|:---:|
| 1 | Proses 3D printing casing IoT berbahan PETG dan aplikasi sealing RTV | Darrel Alfath | Pekan 3 (W3-D3) |
| 2 | Perakitan rangkaian catu daya solar 10Wp dan perfboard wiring harness | Muhammad Ilman Zuhriy | Pekan 3 (W3-D3) |
| 3 | Integrasi driver modular sensor ultrasonik dan filtering sinyal gas | Siti Amalia Nurfaidah | Pekan 3 (W3-D4) |
| 4 | Pengembangan database backend dan live telemetry sink pada server | Daffa Hardhan | Pekan 3 (W3-D5) |
| 5 | Audit pengeluaran aktual dan evaluasi kepatuhan standar sanitasi | Raka Arrayan Muttaqien | Pekan 3 (W3-D5) |

---

### 11. Persentase Progress Proyek
| Komponen Pekerjaan | Bobot (%) | Progress Riil (%) | Kontribusi Terbobot (%) |
|:---|:---:|:---:|:---:|
| Perancangan Sistem & Arsitektur | 15% | 100% | 15.00% |
| Implementasi Hardware & Power | 25% | 40% | 10.00% |
| Implementasi Software & Firmware | 25% | 40% | 10.00% |
| Integrasi Sistem | 15% | 20% | 3.00% |
| Pengujian & Validasi Mutu | 10% | 30% | 3.00% |
| Dokumentasi & Manajemen Repositori | 10% | 80% | 8.00% |
| **Progress Total Proyek** | **100%** | | **49.00%** |

---

### 12. Kesimpulan
Seluruh paket kerja pada fase persiapan implementasi Pekan 2 telah diselesaikan dengan sangat baik (total progres mencapai **49.00%**). Sebanyak 100% komponen BoM telah tersedia, lingkungan pengembangan firmware siap, rancangan 3D CAD casing IoT terkunci, dan repositori Git aktif. Tim siap memasuki fase perakitan fisik dan integrasi subsistem pada Pekan 3.

**Status Kesiapan Menuju Pekan 3 (*Week 3 Readiness*):** **SIAP (READY)**

---

### Lembar Persetujuan Laporan

| Jabatan | Nama Pejabat / Mahasiswa | Tanda Tangan | Tanggal Pengesahan |
|:---|:---|:---:|:---:|
| **Ketua Kelompok** | Daffa Hardhan | *(Tanda Tangan)* | 08 September 2026 |
| **Dosen Pembimbing** | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ........................................ | ........................................ |
