# LAPORAN KEMAJUAN PEKANAN 1
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2

---

### Informasi Umum Proyek
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Ke-:** 1 (Satu)
- **Periode:** 26 Agustus 2026 – 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
- **Ketua Kelompok:** Daffa Hardhan

---

### 1. Ringkasan Kemajuan Pekanan
Pada pekan ke-1, tim melakukan kick-off realisasi proposal dan re-planning proyek. Kegiatan difokuskan pada penelaahan kembali permasalahan, solusi, arsitektur sistem, target kinerja, realistic constraints, pembagian tugas, Work Breakdown Structure (WBS), milestone, serta penyesuaian jadwal semester. Satuan Acara Perkuliahan (SAP) menetapkan kegiatan tersebut sebagai fokus utama Pekan 1 guna memastikan kesiapan implementasi pada tahapan berikutnya.

---

### 2. Target Pekan Ini
| No | Target Pekerjaan | Status Capaian | Keterangan Verifikasi |
|:---:|:---|:---:|:---|
| 1 | Review proposal dan target kinerja kuantitatif | [x] Tercapai [ ] Belum | Seluruh parameter teknis proposal Bab 1–5 telah ditelaah ulang |
| 2 | Menyusun WBS implementasi operasional semester | [x] Tercapai [ ] Belum | Dokumen WBS 14 pekan tersedia pada `03_MASTER_SEMESTER_WBS.md` |
| 3 | Menetapkan pembagian tugas final (Fixed RACI) | [x] Tercapai [ ] Belum | Matriks tanggung jawab tetap terkunci pada `01_MASTER_TASK_ALLOCATION.md` |
| 4 | Menyusun jadwal realisasi semester dan jalur kritis | [x] Tercapai [ ] Belum | Lini masa 14 pekan dan milestone terintegrasi di WBS |
| 5 | Menetapkan daftar kebutuhan komponen dan perangkat lunak | [x] Tercapai [ ] Belum | 15 item BoM terkunci pada `02_BOM_PROCUREMENT.md` |

---

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil dan Bukti Nyata (Tangible Output / Evidence) |
|:---:|:---|:---|:---|
| 1 | Review proposal menyeluruh | Daffa Hardhan + seluruh anggota | Dokumen catatan evaluasi kesesuaian arsitektur sistem terhadap target proposal Bab 4.4 dan Bab 5 (Bukti: Notula Rapat Kick-off W1). |
| 2 | Review target dan realistic constraints | Raka Arrayan M. + domain PIC | Daftar identifikasi batasan lingkungan operasi blank-spot, standar keselamatan sanitasi, dan matriks RTM awal pada `04_PHASE_GATE_AND_TRACEABILITY.md`. |
| 3 | Penyusunan WBS operasional semester | Daffa Hardhan | Dokumen WBS 14 pekan untuk 5 domain kerja dan tautan live Google Sheets pada `03_MASTER_SEMESTER_WBS.md`. |
| 4 | Finalisasi alokasi tugas anggota | Daffa Hardhan | Matriks RACI fixed baseline 205 jam kerja mengikat (Daffa 40h, Raka 35h, Siti 45h, Ilman 45h, Darrel 40h) pada `01_MASTER_TASK_ALLOCATION.md`. |
| 5 | Review BoM dan kebutuhan software | Raka Arrayan M. + M. Ilman Z. + Siti Amalia N. | Daftar 15 komponen BoM terverifikasi, identifikasi variansi harga sensor gas MQ-137/136, dan keputusan tanpa custom PCB pada `02_BOM_PROCUREMENT.md`. |
| 6 | Review kebutuhan mekanik casing 3D dan pengujian | Darrel Alfath | Pemetaan dimensi enclosure casing IoT, standar proteksi IP54, dan Master Test Plan awal (TC-01 s.d. TC-06) pada `04_PHASE_GATE_AND_TRACEABILITY.md`. |

---

### 4. Kemajuan Tiap Anggota
| No | Nama Anggota | Tugas Pekan Ini | Realisasi Jam Kerja | Persentase Penyelesaian |
|:---:|:---|:---|:---:|:---:|
| 1 | Daffa Hardhan | WBS, master schedule, project coordination, architecture review | 20 Jam | 100% (Target W1 tercapai) |
| 2 | Raka Arrayan Muttaqien | Requirement, BoM, procurement tracker, feasibility review | 18 Jam | 100% (Target W1 tercapai) |
| 3 | Siti Amalia Nurfaidah | Embedded/software architecture review, interface mapping | 22 Jam | 100% (Target W1 tercapai) |
| 4 | Muhammad Ilman Zuhriy | Electrical/power review, solar sizing, sensor conditioning | 26 Jam | 100% (Target W1 tercapai) |
| 5 | Darrel Alfath | Mechanical constraints, 3D IoT casing review, test planning | 20 Jam | 100% (Target W1 tercapai) |

---

### 5. Hasil Implementasi

#### Perangkat Keras (Hardware)
Belum memasuki tahap implementasi fisik penuh. Tahap pada pekan ini berupa verifikasi kebutuhan elektrikal, kalkulasi power budget (solar panel 10 Wp + baterai Li-ion 18650), pengkondisian beban pemanas sensor gas MQ-137/MQ-136, serta penetapan metode perakitan menggunakan breakout board + perfboard wiring harness (tanpa custom PCB) untuk memastikan kesiapan implementasi Pekan 2.

#### Perangkat Lunak (Software)
Struktur kebutuhan software dan antarmuka embedded telah disiapkan sebagai dasar Pekan 2. Pemetaan pin ESP32, rancangan driver modular (JSN-SR04T, LoRa RA-02 SPI, PWM servo MG996R, ADC sensor gas), dan kerangka firmware skeleton v0.1 telah terdefinisi.

#### Mekanikal (Mechanical)
Kebutuhan casing 3D IoT modular untuk perangkat eSOS, konsep model CAD, titik mounting komponen internal, jalur perutean kabel, dan standar weatherproofing IP54 dengan sealing gasket RTV telah dipetakan sebagai dasar finalisasi desain manufaktur pada Pekan 2 oleh Darrel Alfath.

---

### 6. Dokumentasi Hasil
1. **Foto / Screenshot Diskusi dan Review Proposal:** Catatan dan dokumentasi rapat koordinasi tim dalam menetapkan pembagian domain teknis dan baseline jadwal.
2. **Screenshot WBS dan Timeline Pelaksanaan:** Tangkapan layar struktur WBS dan lini masa semester pada berkas `03_MASTER_SEMESTER_WBS.md`.
3. **Screenshot Task Allocation dan BoM Tracker:** Tangkapan layar tabel matriks tanggung jawab RACI (`01_MASTER_TASK_ALLOCATION.md`) dan daftar 15 komponen BoM (`02_BOM_PROCUREMENT.md`).

---

### 7. Pengujian yang Dilakukan
| Parameter Pengujian | Target Spesifikasi | Hasil Pengujian Pekan 1 | Status Kelulusan |
|:---|:---|:---|:---:|
| Review Target Sistem | Target kinerja kuantitatif teridentifikasi penuh | 6 parameter kinerja utama (akurasi level, deteksi gas, jangkauan LoRa, respon servo, solar uptime 24 jam, IP54) terdefinisi | [x] Lulus [ ] Tidak |
| Review Kebutuhan Komponen | Seluruh 15 komponen BoM terpetakan spesifikasinya | 15 item BoM terdaftar lengkap beserta estimasi harga vendor dan PIC penerimaan | [x] Lulus [ ] Tidak |
| Review Pembagian Tugas | PIC tunggal dan Acceptance Owner terdefinisi per domain | 100% tugas memiliki penanggung jawab tunggal tanpa tumpang tindih | [x] Lulus [ ] Tidak |

#### Analisis Singkat
Pekan 1 belum merupakan tahap pengujian performa sistem fisik secara riil di lapangan. Pengujian difokuskan pada verifikasi kelayakan desain (*design verification review*), validasi konsistensi parameter antarmuka antar-subsistem, dan simulasi kecukupan daya. Validasi teknis fungsional utama (TC-01 sampai dengan TC-06) dijadwalkan pada tahap perakitan dan integrasi berikutnya.

---

### 8. Kendala dan Solusi
| No | Kendala yang Dihadapi | Dampak Terhadap Proyek | Solusi dan Tindakan Korektif yang Dilakukan |
|:---:|:---|:---|:---|
| 1 | Perbedaan harga pasar riil komponen sensor gas dengan RAB proposal | Potensi pembengkakan anggaran (*budget overrun*) | Melakukan pemeriksaan pasar mendalam, menandai komponen kritis (`GATE-PROC-01` & `GATE-PROC-02`), dan menetapkan batas pagu ketat Rp2.000.000. |
| 2 | Spesifikasi pinout dan arus pemanas beberapa komponen perlu diverifikasi | Risiko kesalahan pemilihan barang saat pengadaan | Menetapkan kewajiban verifikasi datasheet resmi pabrikan sebelum persetujuan checkout dilakukan oleh Acceptance Owner. |
| 3 | Kebutuhan integrasi lintas domain yang kompleks (casing 3D, elektrikal, firmware) | Risiko kebingungan kepemilikan dan overlap tugas | Mengunci batas kepemilikan domain (Darrel untuk 3D Casing IoT, Ilman untuk wiring daya tanpa custom PCB, Siti untuk firmware). |

---

### 9. Deviasi Terhadap Jadwal
| Aktivitas | Jadwal Awal (Rencana) | Realisasi Aktual | Keterangan dan Status |
|:---|:---|:---|:---|
| Review Proposal & Target Kinerja | Pekan 1 (26–28 Agt 2026) | 26–28 Agustus 2026 | Selesai 100% sesuai jadwal awal |
| Penyusunan Master WBS & Timeline | Pekan 1 (28–30 Agt 2026) | 28–30 Agustus 2026 | Selesai 100% sesuai jadwal awal |
| Finalisasi Task Allocation & RACI | Pekan 1 (29–31 Agt 2026) | 29–31 Agustus 2026 | Selesai 100% sesuai jadwal awal |
| Baseline BoM & Procurement Tracker | Pekan 1 (30 Agt–01 Sep 2026) | 30 Agustus – 01 September 2026 | Selesai 100% siap eksekusi pengadaan |

#### Analisis Deviasi
Tidak terjadi deviasi negatif (*zero schedule deviation*). Seluruh paket kerja perencanaan ulang pada Pekan 1 berhasil diselesaikan tepat waktu sesuai lini masa yang ditetapkan dalam SAP.

---

### 10. Rencana Kerja Pekan Berikutnya (Pekan 2: Persiapan Implementasi & Pengadaan)
| No | Rencana Aktivitas Operasional | Penanggung Jawab (PIC) | Target Waktu Penyelesaian |
|:---:|:---|:---:|:---:|
| 1 | Eksekusi pengadaan 15 komponen BoM dan verifikasi fisik saat kedatangan | Raka Arrayan M. + Domain PIC | Pekan 2 (05 Sep 2026) |
| 2 | Instalasi development environment (IDE/toolchain), Git repo, dan baseline firmware | Siti Amalia N. + Daffa Hardhan | Pekan 2 (04 Sep 2026) |
| 3 | Finalisasi skematik daya, topologi TP4056 + 18650, dan rencana wiring perfboard | Muhammad Ilman Zuhriy | Pekan 2 (04 Sep 2026) |
| 4 | Finalisasi berkas 3D CAD casing IoT (STEP/STL) dan persiapan cetak 3D PETG | Darrel Alfath | Pekan 2 (05 Sep 2026) |
| 5 | Pengelolaan bukti pengadaan, aktivasi task tracker, dan kompilasi laporan Pekan 2 | Daffa Hardhan + Seluruh Anggota | Pekan 2 (06 Sep 2026) |

---

### 11. Persentase Progress Proyek
| Komponen Pekerjaan | Bobot (%) | Progress Riil (%) | Kontribusi Terbobot (%) |
|:---|:---:|:---:|:---:|
| Perancangan Sistem & Arsitektur | 15% | 100% | 15.00% |
| Implementasi Hardware & Power | 25% | 15% | 3.75% |
| Implementasi Software & Firmware | 25% | 15% | 3.75% |
| Integrasi Sistem | 15% | 10% | 1.50% |
| Pengujian & Validasi Mutu | 10% | 20% | 2.00% |
| Dokumentasi & Manajemen Repositori | 10% | 50% | 5.00% |
| **Progress Total Proyek** | **100%** | | **31.00%** |

---

### 12. Kesimpulan
Pada Pekan 1, tim berhasil memfokuskan pekerjaan pada penerjemahan proposal akhir Desain Proyek 1 menjadi rencana implementasi operasional yang terukur. Seluruh target luaran yang diamanatkan oleh SAP berhasil diselesaikan, mencakup Master WBS semester, jadwal realisasi, pembagian tugas final (RACI), serta daftar kebutuhan 15 komponen BoM dan perangkat lunak.

**Status Kesiapan Menuju Pekan 2 (*Week 2 Readiness*):** **SIAP (READY)**

---

### Lembar Persetujuan Laporan

| Jabatan | Nama Pejabat / Mahasiswa | Tanda Tangan | Tanggal Pengesahan |
|:---|:---|:---:|:---:|
| **Ketua Kelompok** | Daffa Hardhan | *(Tanda Tangan)* | 01 September 2026 |
| **Dosen Pembimbing** | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ........................................ | ........................................ |
