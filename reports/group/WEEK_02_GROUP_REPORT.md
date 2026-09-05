# LAPORAN KEMAJUAN PEKANAN
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2

**Judul Proyek** : RANCANG BANGUN SISTEM MONITORING SMART-SANITATION ESOS BERBASIS IOT UNTUK WILAYAH BLANK SPOT PASCA-BENCANA
**Kelompok** : 4
**Pekan Ke-** : 2
**Periode** : 31 Agustus 2026 – 05 September 2026
**Dosen Pembimbing** : Prof. Dr. Muhammad Suryanegara, S.T., M.Sc
**Ketua Kelompok** : Daffa Hardhan (2306161763)
**Anggota Kelompok** : 
1. Raka Arrayan Muttaqien (2306161800)
2. Siti Amalia Nurfaidah (2306161851)
3. Darrell Alfath (2306266810)
4. Muhammad Ilman Zuhriy (2306266786)

---

### 1. Ringkasan Kemajuan Pekanan
Pada pekan ini, tim telah menyelesaikan riset spesifikasi teknis dan menyusun dokumen Master Bill of Materials (BoM) lengkap dengan komparasi vendor serta estimasi anggaran proyek. Tim juga telah melaksanakan sesi konsultasi tatap muka bersama dosen pembimbing untuk validasi rencana pengadaan, menyelesaikan finalisasi daftar belanja (*listing*) komponen siap transaksi, menginisialisasi repositori GitHub untuk manajemen kode dan kolaborasi tim, serta mengaktifkan sistem *daily logbook tracker* guna memantau linimasa. **Selain itu, terdapat inisiatif pengembangan awal (*prototyping*) dari sisi perangkat lunak oleh Ketua Kelompok, berupa pembangunan *backend server* mandiri menggunakan bahasa Go dan penyusunan skema basis data PostgreSQL.**

### 2. Target Pekan Ini
| No | Target | Status |
|:---|:---|:---|
| 1 | Penyusunan dokumen Master Bill of Materials (BoM) lengkap dengan spesifikasi teknis dan komparasi harga toko online | [x] Tercapai &nbsp; [ ] Belum |
| 2 | Konsultasi dan validasi rencana desain sistem serta pengadaan komponen bersama dosen pembimbing | [x] Tercapai &nbsp; [ ] Belum |
| 3 | Finalisasi daftar belanja (*listing*) komponen siap beli serta inisialisasi repositori GitHub dan sistem logbook tracking tim | [x] Tercapai &nbsp; [ ] Belum |
| 4 | **(Inisiatif Software)** Inisiasi prototipe *backend server* (Go) dan arsitektur basis data (PostgreSQL/SQLite) sebagai langkah akselerasi | [x] Tercapai &nbsp; [ ] Belum |

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil |
|:---|:---|:---|:---|
| 1 | Riset spesifikasi teknis hardware, komparasi toko daring, dan penyusunan daftar belanja produk BoM | Seluruh Anggota Tim | Dokumen Master BoM tervalidasi dengan rincian teknis, tautan toko, dan siap transaksi. |
| 2 | Rapat koordinasi tim via Zoom Meeting dan inisialisasi Daily Logbook Tracker | Seluruh Anggota Tim | Kesepakatan alokasi tugas & lembar pelacakan jam kerja aktif. |
| 3 | Asistensi dan bimbingan tatap muka bersama dosen pembimbing di Gedung MRPQ Lt. 1 | Seluruh Anggota Tim | Persetujuan arah perancangan sistem dan arahan pengadaan komponen. |
| 4 | Inisialisasi repositori GitHub tim sebagai pusat manajemen berkas proyek dan pelaporan | Daffa Hardhan & Tim | Repositori GitHub (DESPRO-4) aktif lengkap dengan folder dan template. |
| 5 | **(Inisiatif)** Pemrograman awal Server Go, integrasi arsitektur PostgreSQL 16 & SQLite Edge, serta desain awal Web Dashboard | Daffa Hardhan | Kode sumber *backend* awal selesai ditulis dan dikompilasi menjadi *native binary*. |

### 4. Kemajuan Tiap Anggota
| Nama | Tugas Pekan Ini | Persentase Penyelesaian |
|:---|:---|:---:|
| Daffa Hardhan | Repositori GitHub, riset BoM, koordinasi tim, **serta inisiatif ekstra mengembangkan *backend server* Go dan database PostgreSQL/SQLite.** | 100% |
| Raka Arrayan M. | Riset spesifikasi teknis hardware, menyusun master daftar belanja (listing) BoM, serta pemetaan tautan toko | 100% |
| Siti Amalia N. | Riset kompatibilitas komponen elektronika/daya, membantu penyusunan BoM | 100% |
| Darrell Alfath | Verifikasi ketersediaan stok, komparasi harga toko daring, serta mempelajari dasar 3D design | 100% |
| M. Ilman Zuhriy | Riset modul antarmuka/mikrokontroler pada BoM, serta konfigurasi templat pelaporan tim | 100% |

### 5. Hasil Implementasi
**Hardware**
*   **Komponen yang telah direalisasikan:** Dokumen Master Bill of Materials (BoM) final dan daftar belanja komponen siap transaksi (*ready to checkout*).
*   **Status integrasi:** Tahap pra-integrasi / verifikasi kompatibilitas elektrik. Perakitan dan integrasi fisik dijadwalkan segera setelah proses transaksi pengadaan tiba di laboratorium.

**Software**
*   **Fitur yang telah selesai:** Inisialisasi struktur repositori GitHub tim (DESPRO-4), berkas .gitignore, README.md, templat pelaporan, dan *Daily Logbook*. **Telah selesai pula arsitektur awal *backend server native binary* Go (`esos-server.exe`), kerangka API awal, dan skema database PostgreSQL 16 (UUIDv7).**
*   **Fitur yang masih dikembangkan:** Penyiapan *environment* IDE/toolchain untuk mikrokontroler, penulisan *baseline firmware* pengujian I/O ESP32, dan sinkronisasi LoRa.

**Mekanik/Biomedik (jika ada)**
*   Belum dilakukan pengerjaan fisik pada subsistem mekanik pada pekan ini. Pemodelan dan perancangan fisik 3D akan dimulai setelah komponen tiba agar dimensi selaras.

### 6. Dokumentasi Hasil
**Lampiran & Penjelasan**
1. **(Gambar 1 - Zoom Meeting):** Pelaksanaan rapat koordinasi daring internal tim via Zoom Meeting untuk membahas penyusunan dan finalisasi daftar komponen BoM serta pembagian pos pengadaan hardware.
2. **(Gambar 2 - Asistensi MRPQ):** Sesi bimbingan bersama dosen pembimbing di Gedung MRPQ Lantai 1 untuk meninjau kelayakan arsitektur desain sistem dan konsultasi pengadaan.
3. **(Gambar 3 - Arsitektur Software):** Screenshot *source code* inisiasi Server Go dan *database* PostgreSQL di repositori GitHub.

### 7. Pengujian yang Dilakukan
| Parameter | Target | Hasil | Status |
|:---|:---|:---|:---|
| Uji Kompilasi *Backend Server* Go | 0 Error saat *build* binary mandiri | File `esos-server.exe` sukses terbentuk tanpa galat | [x] Lulus &nbsp; [ ] Tidak |
| *Hardware / Mekanik* | - | Belum ada pengujian fisik yang dilakukan pekan ini | [ ] Lulus &nbsp; [ ] Tidak |

**Analisis Singkat:**
Pada pekan ini belum dilaksanakan kegiatan pengujian perangkat keras (hardware) dikarenakan fokus tim masih pada perancangan BoM dan komparasi toko daring. Namun, dari sisi perangkat lunak (software), **telah dilakukan pengujian kompilasi pada *backend server* berbasis Go hasil inisiatif awal**, yang berjalan sukses tanpa galat (*error*). Pengujian terintegrasi baru akan dimulai setelah komponen fisik tiba.

### 8. Kendala dan Solusi
| Kendala | Dampak | Solusi yang Dilakukan |
|:---|:---|:---|
| Proses verifikasi ketersediaan stok, kesesuaian spesifikasi, dan komparasi harga antartoko daring memakan waktu telaah yang cukup lama. | Transaksi pembelian (*checkout*) komponen BoM belum dapat diselesaikan pada pekan ini. | Mengunci tautan produk tervalidasi pada Master BoM, menyiapkan opsi toko cadangan (*backup seller*), dan menjadwalkan *checkout* serentak awal pekan depan. |

### 9. Deviasi Terhadap Jadwal
| Aktivitas | Jadwal Awal | Realisasi | Keterangan |
|:---|:---|:---|:---|
| Penyusunan BoM & Listing Toko | Pekan 2 | Pekan 2 | Sesuai jadwal (*on track*). |
| Inisialisasi Repo GitHub & Logbook | Pekan 2 | Pekan 2 | Sesuai jadwal (*on track*). |
| **Transaksi Pengadaan (*Checkout*)** | Pekan 2 | Awal Pekan 3 | **Tertunda (*delayed*)**; fokus pada finalisasi toko. |
| **Pengembangan *Backend* Server** | Pekan 3 | Pekan 2 | **Lebih Cepat (*accelerated*)**; Inisiatif Daffa. |

**Analisis Deviasi:**
Keterlambatan pada eksekusi pengadaan disebabkan oleh perlunya proses verifikasi mendalam terhadap kesesuaian spesifikasi teknis dan ketersediaan stok guna memastikan efisiensi anggaran. Sebagai kompensasi *delay* ini, pengembangan *software backend* yang awalnya dijadwalkan pekan depan berhasil **ditarik lebih maju** pada pekan ini. Seluruh daftar komponen berstatus *ready to checkout* untuk awal pekan depan tanpa mengganggu *critical path* proyek.

### 10. Rencana Kerja Pekan Berikutnya
| No | Aktivitas | PIC | Target Selesai |
|:---|:---|:---|:---|
| 1 | Eksekusi transaksi pembelian (*checkout*) seluruh komponen BoM dan pelacakan pengiriman | Seluruh Tim | Awal Pekan Depan |
| 2 | Kompilasi bukti invoice resmi dan pengajuan administrasi *reimbursement* (batas maks 25 hari) | Seluruh Tim | Pertengahan Pekan |
| 3 | Penyiapan *environment* pemrograman (IDE), *baseline firmware*, & skematik antarmuka | Seluruh Tim | Akhir Pekan Depan |

### 11. Persentase Progress Proyek
| Komponen | Bobot (%) | Progress (%) | Kontribusi (%) |
|:---|:---:|:---:|:---:|
| Perancangan | 25% | 80% | 20.0% |
| Implementasi Hardware | 25% | 5% | 1.25% |
| Implementasi Software | 20% | **15%** | 3.0% |
| Integrasi Sistem | 15% | 0% | 0.0% |
| Pengujian | 5% | 0% | 0.0% |
| Dokumentasi | 10% | 60% | 6.0% |
| **Progress Total Proyek** | **100%** | | **30.25%** |

*Catatan perhitungan:*
- **Perancangan (80%):** Konsep sistem, skematik dasar, dan BoM sudah selesai.
- **Implementasi Hardware (5%):** Hanya mencakup listing dan kesiapan keranjang belanja (*checkout*).
- **Implementasi Software (15%):** *(Peningkatan)* Inisialisasi GitHub, arsitektur berkas, **ditambah pembuatan prototipe *backend* Go Server & PostgreSQL.**
- **Dokumentasi (60%):** Repositori, *daily tracker*, dan laporan pekanan.

### 12. Kesimpulan
Pada pekan ini, tim telah menyelesaikan fase inisialisasi awal yang berfokus pada penyusunan dokumen Master BoM, verifikasi spesifikasi komponen di toko daring, pembuatan repositori pada GitHub (DESPRO-4), dan pengaktifan Daily Logbook Tracker. Tim juga telah berkonsultasi langsung dengan dosen pembimbing di MRPQ Lantai 1. Meski transaksi *checkout* perangkat keras bergeser ke awal pekan depan, tim berhasil mengompensasinya dengan kemajuan signifikan di sektor *Software* melalui inisiatif pengembangan *backend server* Go dan skema PostgreSQL.

**Catatan Pembimbing:**
- Segera tuntaskan seluruh target tugas mingguan dan percepat proses eksekusi pengadaan (*checkout*) komponen BoM agar linimasa perancangan serta perakitan perangkat keras tetap berjalan sesuai jadwal (*on track*).
- Perhatikan secara ketat tenggat waktu administratif pengajuan dana; pastikan proses klaim *reimbursement* beserta kelengkapan bukti transaksi diselesaikan sebelum batas maksimal 25 hari.
- Untuk agenda asistensi dan bimbingan progres rutin tiap pekan, koordinasi dapat disepakati dan dilaksanakan secara daring (*online*).

### Persetujuan
| Jabatan | Nama | Tanda Tangan |
|:---|:---|:---:|
| Ketua Kelompok | Daffa Hardhan | *(Digital)* |
| Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ..................... |
