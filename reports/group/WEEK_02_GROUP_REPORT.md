# LAPORAN KEMAJUAN PEKANAN
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2

**Judul Proyek** : Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
**Kelompok** : 4 (Empat)
**Pekan Ke-** : 2 (Dua)
**Periode** : 31 Agustus 2026 – 05 September 2026
**Dosen Pembimbing** : Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
**Ketua Kelompok** : Daffa Hardhan
**Anggota Kelompok** : 
1. Daffa Hardhan (2306161763)
2. Raka Arrayan Muttaqien (2306161800)
3. Siti Amalia Nurfaidah (2306161851)
4. Muhammad Ilman Zuhriy (2306266786)
5. Darrel Alfath (2306266810)

---

### 1. Ringkasan Kemajuan Pekanan
Pada pekan ke-2 ini, tim berfokus pada penyiapan seluruh sumber daya proyek sesuai target CPMK. Kegiatan meliputi verifikasi spesifikasi 15 komponen kelistrikan dan IoT, eksekusi pemesanan (*checkout*), serta penyusunan register aset dan Bundel Kuitansi Final dengan status hemat. Selain itu, tim juga telah merampungkan pembuatan repositori Git, penyiapan server lokal Go untuk backend, setup IDE PlatformIO untuk ESP32, dan penyusunan draf desain 3D CAD awal untuk *enclosure* tahan cuaca. Secara fisik, komponen sedang dalam proses pengiriman dan beberapa telah tiba untuk dicek.

### 2. Target Pekan Ini
| No | Target | Status |
|:---|:---|:---|
| 1 | Pengadaan komponen kelistrikan & BoM final (15 item) | [x] Tercapai &nbsp; [ ] Belum |
| 2 | Persiapan lingkungan pengembangan firmware (PlatformIO) | [x] Tercapai &nbsp; [ ] Belum |
| 3 | Persiapan repositori proyek & setup Server Go Backend | [x] Tercapai &nbsp; [ ] Belum |
| 4 | Finalisasi rancangan gambar teknik 2D dan Draf 3D CAD | [x] Tercapai &nbsp; [ ] Belum |
| 5 | Setup format laporan, master WBS, dan sistem logbook | [x] Tercapai &nbsp; [ ] Belum |

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil |
|:---|:---|:---|:---|
| 1 | Verifikasi & checkout BoM 15 komponen hardware | Raka Arrayan & Tim | Komponen dipesan, anggaran Rp1.828.500 (hemat). |
| 2 | Setup repositori GitHub, Inisiasi Server Go & PostgreSQL/SQLite | Daffa Hardhan | Git repo siap, server Go lokal dapat dijalankan. |
| 3 | Instalasi IDE PlatformIO & board package ESP32 | Siti Amalia N. | PlatformIO siap digunakan untuk kompilasi C++. |
| 4 | Penghitungan daya panel surya 10Wp & topologi 1S4P | M. Ilman Zuhriy | Skema arsitektur perakitan kabel tanpa PCB custom. |
| 5 | Perancangan gambar teknik 2D mekanikal & 3D Casing | Darrel Alfath | Sketsa CAD 3D enclosure IP54 siap direview. |

### 4. Kemajuan Tiap Anggota
| Nama | Tugas Pekan Ini | Persentase Penyelesaian |
|:---|:---|:---:|
| Daffa Hardhan | Setup Git, inisiasi Go server, setup pipeline DB PostgreSQL/SQLite | 100% |
| Raka Arrayan M. | Eksekusi BoM, kuitansi invoice, register aset | 100% |
| Siti Amalia N. | Toolchain IDE, setup framework ESP32 PlatformIO | 100% |
| M. Ilman Zuhriy | Verifikasi hardware list, rencana topologi & kabel | 100% |
| Darrel Alfath | Gambar teknik mekanik 2D, rencana desain 3D casing | 100% |

### 5. Hasil Implementasi
**Hardware**
*   **Komponen yang telah direalisasikan:** 15 item BoM (ESP32, TP4056, Panel Surya 10Wp, LoRa SX1278, Baterai 18650, dll) telah terverifikasi spesifikasinya dan dieksekusi pesanannya.
*   **Status integrasi:** Belum diintegrasikan secara fisik; status masih dalam penerimaan barang di lab dan inspeksi kelengkapan visual.

**Software**
*   **Fitur yang telah selesai:** Repositori GitHub, infrastruktur lokal *native binary* Server Go (`esos-server.exe`), konfigurasi database PostgreSQL 16 & SQLite Edge, kerangka REST API awal.
*   **Fitur yang masih dikembangkan:** Firmware sensor ESP32, skrip sampling ADC gas, filter debouncing, dan telemetri pengiriman paket via komunikasi serial LoRa.

**Mekanik/Biomedik (jika ada)**
*   Draft dimensi fisik ruang *casing* telah disesuaikan dengan dimensi komponen breakout ESP32 dan modul kelistrikan untuk kesiapan cetak filamen 3D PETG.

### 6. Dokumentasi Hasil
**Foto Kegiatan**
*(Lampirkan foto implementasi, perakitan, pengujian, atau diskusi tim)*
Gambar 1. Bundel faktur pembelian komponen elektronik (*e-commerce*) - tercatat di `assets/procurement/`.
Gambar 2. Bukti layout draf gambar teknik 3D (*screenshot* cad).

**Screenshot Sistem**
*(Jika proyek berbasis software atau IoT)*
- *Screenshot* VS Code Workspace dengan inisiasi file server Go dan setup PlatformIO.

### 7. Pengujian yang Dilakukan
| Parameter | Target | Hasil | Status |
|:---|:---|:---|:---|
| Status Pengadaan Komponen | Minimal 80% komponen tersedia | 15/15 (100%) komponen dipesan | [x] Lulus &nbsp; [ ] Tidak |
| Lingkungan Pengembangan | IDE dan Server dapat *running* lokal | `esos-server.exe` jalan 0 error | [x] Lulus &nbsp; [ ] Tidak |
| Sistem Dokumentasi Proyek | Git repo aktif & terhubung | WBS & GitHub diakses publik | [x] Lulus &nbsp; [ ] Tidak |

**Analisis Singkat:**
Hasil pengecekan kesiapan menunjukkan tim telah memenuhi seluruh syarat untuk memulai perakitan perangkat keras dan pengembangan *firmware*. Ketersediaan 100% BoM memastikan tidak ada *bottleneck* pengadaan untuk pekan selanjutnya.

### 8. Kendala dan Solusi
| Kendala | Dampak | Solusi yang Dilakukan |
|:---|:---|:---|
| Fluktuasi stok dan harga sensor gas MQ-137 di beberapa *supplier*. | Risiko keterlambatan penerimaan barang uji. | Langsung membandingkan 3 toko di marketplace pada hari yang sama dan langsung mengeksekusi pembelian kolektif sebelum stok habis. |

### 9. Deviasi Terhadap Jadwal
| Aktivitas | Jadwal Awal | Realisasi | Keterangan |
|:---|:---|:---|:---|
| Pengadaan BoM | W2 | W2 | Selesai tepat waktu. |
| Setup IDE & Repo | W2 | W2 | Selesai tepat waktu. |

**Analisis Deviasi:**
Tidak ada keterlambatan pekerjaan (*zero deviation*). Seluruh capaian persiapan sumber daya tercapai 100% tepat pada Pekan ke-2 sesuai proposal master WBS.

### 10. Rencana Kerja Pekan Berikutnya
| No | Aktivitas | PIC | Target Selesai |
|:---|:---|:---|:---|
| 1 | Pencetakan casing IoT dengan printer 3D (filamen PETG) | Darrel Alfath | Pekan 3 |
| 2 | Solder dan perakitan *wiring harness* tanpa custom PCB | M. Ilman Zuhriy | Pekan 3 |
| 3 | Penulisan kode integrasi sensor Analog & LoRa | Siti Amalia N. | Pekan 3 |
| 4 | Sinkronisasi Endpoint API Server dan Ingestion Logic | Daffa Hardhan | Pekan 3 |
| 5 | Cek kondisi fisik (*Quality Control*) komponen yang tiba | Raka Arrayan M. | Pekan 3 |

### 11. Persentase Progress Proyek
| Komponen | Bobot (%) | Progress (%) |
|:---|:---:|:---:|
| Perancangan | 15% | 15% |
| Implementasi Hardware | 25% | 2% |
| Implementasi Software | 25% | 5% |
| Integrasi Sistem | 15% | 0% |
| Pengujian | 10% | 0% |
| Dokumentasi | 10% | 2% |
| **Progress Total Proyek** | **100%** | **24%** |

### 12. Kesimpulan
Pada pekan ini tim berfokus penuh dan berhasil menyelesaikan **tahap persiapan proyek dan pengadaan barang (BoM)**. Repositori perangkat lunak dan lingkungan pengembangan sudah aktif, serta seluruh barang dalam perjalanan/telah tiba. Tim sangat siap memasuki tahap Implementasi dan Perakitan Hardware pada Pekan ke-3.

### Persetujuan
| Jabatan | Nama | Tanda Tangan |
|:---|:---|:---:|
| Ketua Kelompok | Daffa Hardhan | *(Digital)* |
| Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ..................... |
