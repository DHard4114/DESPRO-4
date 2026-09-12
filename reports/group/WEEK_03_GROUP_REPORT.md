# LAPORAN KEMAJUAN PEKANAN
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2

**Judul Proyek** : RANCANG BANGUN SISTEM MONITORING SMART-SANITATION ESOS BERBASIS IOT UNTUK WILAYAH BLANK SPOT PASCA-BENCANA
**Kelompok** : 4
**Pekan Ke-** : 3
**Periode** : 09 September 2026 – 12 September 2026
**Dosen Pembimbing** : Prof. Dr. Muhammad Suryanegara, S.T., M.Sc
**Ketua Kelompok** : Daffa Hardhan (2306161763)
**Anggota Kelompok** : 
1. Raka Arrayan Muttaqien (2306161800)
2. Siti Amalia Nurfaidah (2306161851)
3. Darrell Alfath (2306266810)
4. Muhammad Ilman Zuhriy (2306266786)

---

### 1. Ringkasan Kemajuan Pekanan
Pada pekan ketiga ini, fokus utama tim beralih pada fondasi *Software Architecture* yang tangguh untuk sistem eSOS. Tim berhasil merampungkan struktur basis data **PostgreSQL** lokal yang mengimplementasikan standar **UUIDv7** untuk performa *time-series* yang optimal. Lebih lanjut, sistem **MQTT Subscriber Worker Pool** berbasis Goroutine sukses diimplementasikan untuk menangani *ingestion* data bervolume tinggi (*non-blocking*). Dari sisi administratif, evaluasi *Daily Logbook Tracker* di Google Sheets telah dilakukan untuk memantau pemenuhan jam kerja 205 jam. Bug *integer overflow* pada skrip SQL juga berhasil dimitigasi.

### 2. Target Pekan Ini
| No | Target | Status |
|:---|:---|:---|
| 1 | Migrasi skema database PostgreSQL lokal menggunakan `postgres_schema.sql` (UUIDv7 & Listen/Notify). | [x] Tercapai &nbsp; [ ] Belum |
| 2 | Pengembangan arsitektur *MQTT Subscriber Worker Pool* (Golang) untuk ingest data. | [x] Tercapai &nbsp; [ ] Belum |
| 3 | Evaluasi capaian *Daily Logbook* anggota di Google Sheets dan dokumentasi pelaporan. | [x] Tercapai &nbsp; [ ] Belum |

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil |
|:---|:---|:---|:---|
| 1 | Pembuatan dan perbaikan *seed data* pada DDL PostgreSQL. | Daffa Hardhan | Skema SQL terbebas dari bug *encoding* dan *integer overflow*. |
| 2 | Migrasi database ke `esos_db` via terminal `psql`. | Daffa Hardhan | Tabel time-series siap digunakan di localhost. |
| 3 | Pemrograman Pipeline ETL & Goroutine Workers. | Daffa Hardhan | `pipeline.go` dan `subscriber.go` selesai dan terkoneksi ke Mosquitto Broker. |
| 4 | Pengecekan konsistensi *Google Sheets Live Tracker*. | Daffa Hardhan | Rekapitulasi jam kerja pekan 3 terkonsolidasi dengan baik. |

### 4. Kemajuan Tiap Anggota
| Nama | Tugas Pekan Ini | Persentase Penyelesaian |
|:---|:---|:---:|
| Daffa Hardhan | Pengembangan DB PostgreSQL, Worker Pool MQTT Go, dan manajemen logbook/pelaporan. | 100% |
| Raka Arrayan M. | Persiapan penerimaan komponen perangkat keras dan update buku aset. | 100% |
| Siti Amalia N. | Riset task FreeRTOS dan struktur payload. | 100% |
| Darrell Alfath | Menyiapkan desain 3D awal untuk komponen yang telah dipesan. | 100% |
| M. Ilman Zuhriy | Persiapan perakitan kelistrikan solar panel dan sensor analog. | 100% |

### 5. Hasil Implementasi
**Hardware**
*   **Komponen yang telah direalisasikan:** *(Proses Pengiriman Vendor)*
*   **Status integrasi:** Menunggu ketibaan fisik untuk dirakit pada pekan 4.

**Software**
*   **Fitur yang telah selesai:** Tabel basis data `esos_db` termigrasi 100%, sistem *MQTT Subscriber* berbasis antrean (Channel Go) aktif, dan *bug fixing* UUIDv7 selesai.
*   **Fitur yang masih dikembangkan:** Antarmuka Web Dashboard dengan Native WebSocket dan intersep *True Timestamping*.

**Mekanik/Biomedik (jika ada)**
*   Tahap preparasi desain CAD 3D enclosure IP54.

### 6. Dokumentasi Hasil
**Lampiran & Penjelasan**
1. **(Gambar 1 - Database):** Tangkapan layar migrasi tabel pada database `esos_db` menggunakan `psql`.
2. **(Gambar 2 - Worker Pool):** Tangkapan layar Terminal VS Code yang menunjukkan MQTT Subscriber Worker berhasil berjalan tanpa *error*.
3. **(Gambar 3 - Logbook Sheets):** Rekapitulasi `Daily_Logbook_Tracking` pada Google Sheets untuk Week 3.

### 7. Pengujian yang Dilakukan
| Parameter | Target | Hasil | Status |
|:---|:---|:---|:---|
| Uji Ingesti Data MQTT Worker Pool | Worker mampu menelan pesan MQTT tanpa *blocking* / *crash* | Worker pool mengeksekusi pesan dengan Goroutine secara asinkronus | [x] Lulus &nbsp; [ ] Tidak |
| *Database Migration* | `postgres_schema.sql` dieksekusi tanpa error | *Integer overflow* diperbaiki, migrasi sukses 100% | [x] Lulus &nbsp; [ ] Tidak |

### 8. Kendala dan Solusi
| Kendala | Dampak | Solusi yang Dilakukan |
|:---|:---|:---|
| Fungsi `uuid_generate_v7()` mengalami *integer overflow* karena manipulasi *bit-shift* *timestamp* 2026 melebihi batas 32-bit. | Eksekusi file SQL gagal saat membuat *seed data*. | Melakukan *refactor* fungsi pada SQL dengan mengeksekusi operator `& 255` (pemotongan byte) sebelum melakukan *casting* ke Integer. |

### 9. Deviasi Terhadap Jadwal
| Aktivitas | Jadwal Awal | Realisasi | Keterangan |
|:---|:---|:---|:---|
| **Pengembangan *Backend* DB & MQTT** | Pekan 3-4 | Pekan 3 | **Sesuai & Lebih Cepat** (Fitur backend ditarik maju untuk mendukung tes integrasi firmware). |

### 10. Rencana Kerja Pekan Berikutnya
| No | Aktivitas | PIC | Target Selesai |
|:---|:---|:---|:---|
| 1 | Perakitan fisik perangkat keras (Solar Panel, Sensor, ESP32) setelah barang tiba. | Ilman & Siti | Pertengahan Pekan 4 |
| 2 | Finalisasi Web Dashboard (WebSocket) dan uji transmisi LoRa-MQTT. | Daffa & Siti | Akhir Pekan 4 |

### 11. Persentase Progress Proyek
| Komponen | Bobot (%) | Progress (%) | Kontribusi (%) |
|:---|:---:|:---:|:---:|
| Perancangan | 25% | 90% | 22.5% |
| Implementasi Hardware | 25% | 10% | 2.5% |
| Implementasi Software | 20% | **45%** | 9.0% |
| Integrasi Sistem | 15% | 0% | 0.0% |
| Pengujian | 5% | 5% | 0.25% |
| Dokumentasi | 10% | 70% | 7.0% |
| **Progress Total Proyek** | **100%** | | **41.25%** |

*Catatan perhitungan:*
- Peningkatan drastis pada Implementasi Software karena tulang punggung arsitektur lokal (DB, MQTT Worker) sudah berhasil stabil di pekan ini.

### 12. Kesimpulan
Pada pekan ketiga, tim sukses mengakselerasi pengembangan perangkat lunak, khususnya *backend server*. Isu teknis terkait basis data berhasil diatasi dengan cepat. Fokus administratif melalui *Daily Logbook Tracker* berjalan tertib. Hal ini memberikan keleluasaan waktu yang signifikan untuk mengantisipasi kompleksitas perakitan *hardware* dan *firmware* FreeRTOS pada pekan depan.

### Persetujuan
| Jabatan | Nama | Tanda Tangan |
|:---|:---|:---:|
| Ketua Kelompok | Daffa Hardhan | *(Digital)* |
| Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ..................... |
