# TEMPLATE LAPORAN PEKANAN INDIVIDU
**Capstone Project Desain Proyek 2**

### A. Identitas
| Item | Keterangan |
|:---|:---|
| Nama | Daffa Hardhan |
| NPM | 2306161763 |
| Program Studi | Teknik Komputer |
| Kelompok | 4 (Empat) |
| Judul Proyek | Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana |
| Pekan ke- | 4 (Empat) |
| Periode | 13 September 2026 – 19 September 2026 |
| Tanggal | 19 September 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

### B. Target Mingguan
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Finalisasi Web Dashboard (WebSocket) & Intersep Timestamp | UI Tervalidasi & Sinkronisasi Waktu (`time_sync`) bekerja | [x] Selesai &nbsp; [ ] Belum |
| 2 | Pengujian Jaringan Outdoor TP-Link CPE220 | Hasil Ping test stabil di area *blank spot* simulasi | [x] Selesai &nbsp; [ ] Belum |
| 3 | Evaluasi Logbook PM & Dokumentasi Uji Fungsional | Pembaruan Google Sheets W4 & *Evidence Register* | [x] Selesai &nbsp; [ ] Belum |

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Selasa, 15 Sep 2026 | Membantu tim Firmware (Siti) memvalidasi injeksi *True Timestamping* via MQTT `time_sync`. | 3 | Log terminal Go Server |
| Rabu, 16 Sep 2026 | Uji coba integrasi Web Dashboard dengan *native WebSocket* untuk visualisasi *real-time*. | 2 | Tampilan UI Dashboard |
| Jumat, 18 Sep 2026 | Pendampingan pengujian jaringan *outdoor* TP-Link CPE220 (simulasi area *blank spot*). | 3 | Hasil *Ping Test* |
| Sabtu, 19 Sep 2026 | Mengisi catatan performa sistem & melengkapi konsolidasi logbook W4 di Google Sheets. | 2 | Bukti Tracker Sheets W4 |

**Total Jam Kerja Minggu Ini : 10 Jam**

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan keempat ini, aktivitas teknis saya bergeser dari pengembangan kode murni menuju fase **Pengujian Integrasi**. Saya mendampingi *Firmware Engineer* (Siti) untuk memastikan Gateway ESP32 berhasil menangkap injeksi Unix Epoch (`time_sync`) dari Go Server setiap kali status berubah menjadi `ONLINE`. Selain itu, kami telah mengeksekusi uji coba pemancaran jaringan lokal (*intranet*) menggunakan TP-Link CPE220 di area *outdoor* guna memvalidasi resiliensi topologi jaringan posko pengungsian. Sebagai PM, saya juga memastikan setiap tahap uji fungsional ini dicatat metrik performanya sesuai instruksi Dosen Pembimbing pada evaluasi minggu lalu.

**2. Luaran yang Dihasilkan**
- Validasi fungsional fitur *True Timestamping* MQTT.
- Data hasil *Ping Test* jaringan TP-Link CPE220.
- Pembaruan *Daily Logbook Tracker* W4.

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Pendampingan uji jaringan CPE220 | Area cakupan Wi-Fi terpetakan dengan baik | 100% |
| Software | Validasi injeksi `time_sync` ke Gateway | Gateway sukses melakukan sinkronisasi waktu | 100% |
| Mekanik | - | - | 0% |
| Pengujian | *Ping test* & Uji latensi WebSocket | Dashboard merespon perubahan data tanpa *delay* berarti | 100% |
| Dokumentasi | *Update* kinerja fungsional di buku log | Log performa tercatat di tracker | 100% |

**Ringkasan Kemajuan**
Oleh karena tulang punggung arsitektur lokal (DB, MQTT Worker) sudah stabil sejak pekan lalu (W3), pekan ini saya dapat mengalokasikan waktu penuh untuk mendampingi rekan-rekan tim lain (*Firmware* & *Hardware*) yang mulai memasuki fase perakitan dan integrasi fisik.

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | Fluktuasi sinyal (*packet loss*) pada uji CPE220 di area banyak halangan (*Non-Line-of-Sight*). | Keandalan komunikasi MQTT dari Gateway bisa terputus sesaat. | Sedang |

**Analisis Penyebab**
Pohon dan tembok gedung menghalangi propagasi sinyal 2.4GHz dari CPE220, sehingga *Gateway* kadang mengalami *disconnect*. Namun, ini justru memvalidasi perlunya fitur *Store-and-Forward* via LittleFS di ESP32 yang sedang dikerjakan Siti.

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Fluktuasi sinyal | Mengaktifkan pengaturan QoS 1 pada *publisher* dan merevisi *timeout* MQTT. Fitur *Store-and-Forward* akan menutupi celah ini saat data di-*flush* ke server setelah *reconnect*. | Daffa & Siti | Berjalan |

### H. Dokumentasi Kemajuan
> **[PLACEHOLDER BUKTI 1: SCREENSHOT PING TEST CPE220]**
> *Cara mengambil bukti: Screenshot Command Prompt Windows saat melakukan `ping` ke IP Gateway di area outdoor.*
> `![Bukti Ping Test](../media/w4_daffa_ping_test.png)`

> **[PLACEHOLDER BUKTI 2: SCREENSHOT LOG TIME_SYNC]**
> *Cara mengambil bukti: Screenshot terminal server saat mengirim "esos/gateway_01/time_sync" ke broker.*
> `![Bukti Time Sync](../media/w4_daffa_time_sync.png)`

> **[PLACEHOLDER BUKTI 3: SCREENSHOT TRACKER SHEETS W4]**
> *Cara mengambil bukti: Screenshot Tab 'Daily_Logbook_Tracking' di Google Sheets untuk pekan keempat.*
> `![Bukti Live Tracking Sheets](../media/w4_daffa_sheets_tracking.png)`

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Diskusi Lapangan:** Berkolaborasi dengan Siti untuk menguji integrasi *time_sync* secara *end-to-end*.
- **Koordinasi Pengujian:** Menjalankan skenario *stress-test* jaringan dengan Ilman/Raka.

**Kontribusi Pribadi**
Fokus pada integrasi *Software* dengan realita fisik di lapangan (Hardware/Jaringan) serta memastikan bahwa pesan revisi arsitektur dari Dosen Pembimbing (terkait metrik performa fungsional) tereksekusi dengan baik.

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Proses intersep waktu (Unix Epoch) via MQTT berjalan sangat mulus dan sinkron dengan arsitektur Go.
- Pengujian lapangan memberikan gambaran *real* tentang *blind spot* sinyal 2.4GHz.

**Yang Perlu Diperbaiki**
- UI/UX dari Web Dashboard masih berupa purwarupa dasar; perlu dipoles lebih elegan dengan Tailwind CSS agar layak didemonstrasikan saat *Mid-Term Review*.

**Pelajaran yang Didapat Minggu Ini**
- Pengujian di dunia nyata (*outdoor*) jauh lebih menantang daripada simulasi di dalam *localhost*, terutama terkait keandalan sinyal (*resilience*).

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Membantu integrasi *Store-and-Forward* (LittleFS) dengan sistem ETL backend. | Log data *offline* sukses ter-ingest ke PostgreSQL tanpa kerusakan *timestamp*. | 5 Jam |
| 2 | Optimasi *Frontend* Web Dashboard & Persiapan Presentasi UTS (Phase Gate 3). | Antarmuka *Dashboard* Final & Materi Presentasi. | 5 Jam |

### L. Persetujuan
**Mahasiswa**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 19 September 2026

**Ketua Kelompok**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 19 September 2026

**Catatan Pembimbing**
- "Oke. Jangan lupa dokumentasikan setiap tahap. Uji fungsional sangat penting, maka harus dicatat secara mendetail dalam *logbook* bagaimana metrik performanya (*performance*)."
