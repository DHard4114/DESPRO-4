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
| Periode | 14 September 2026 – 19 September 2026 |
| Tanggal | 19 September 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

### B. Target Mingguan
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pembangunan & Redesain Web Dashboard SCADA (EMS) Full-Width | Antarmuka web SCADA mandiri dengan WebSocket & Chart.js live | [x] Selesai &nbsp; [ ] Belum |
| 2 | Pembuatan *Base Code* Firmware & Pendampingan *Wiring* Hardware | Kerangka kode C++ I/O & rangkaian kelistrikan dasar terpasang | [x] Selesai &nbsp; [ ] Belum |
| 3 | Evaluasi Logbook PM & Dokumentasi Uji Fungsional | Pembaruan Google Sheets W4 & *Evidence Register* | [x] Selesai &nbsp; [ ] Belum |

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Selasa, 15 Sep 2026 | Menulis *base code* (C++) untuk inisialisasi sensor analog dan motor servo ESP32. | 4 | *Screenshot Base Code* |
| Rabu, 16 Sep 2026 | Bertemu luring untuk membantu Ilman melakukan perakitan kabel (*wiring*) perangkat keras. | 4 | Foto Rangkaian Fisik |
| Jumat, 18 Sep 2026 | Mengembangkan arsitektur *frontend* Web Dashboard SCADA (EMS) dengan standar UI institusional. | 3 | Kode Sumber `index.html` |
| Sabtu, 19 Sep 2026 | Mengintegrasikan WebSocket real-time broadcast pada server Go, menguji simulasi data, dan rekap Sheets W4. | 3 | *Screenshot Dashboard EMS* & Sheets |

**Total Jam Kerja Minggu Ini : 14 Jam**

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan keempat ini, saya berhasil merealisasikan dua pilar capaian penting:
1. **Front-End & SCADA Dashboard (EMS):** Mengembangkan dan merombak antarmuka Web Dashboard Pemantauan Lingkungan (*Environmental Monitoring System*) secara penuh (*100% Full-Width Fluid Canvas*). Dashboard ini mengadopsi standar visual SCADA profesional dengan palet warna Makara UI, metrik KPI real-time (level air, tegangan baterai, gas amonia, H2S), visualisasi grafik dinamis Chart.js, panel kendali katup pembilasan (idempotency UUIDv7), serta tabel audit paket telemetri live.
2. **Kolaborasi Cross-Functional (Hardware & Firmware):** Karena modul radio LoRa fisik sedang disempurnakan di tempat rekan tim (Siti), saya mengambil inisiatif menulis *base code* kerangka I/O aktuator/sensor serta membantu perakitan fisik (*wiring harness*) bersama Ilman.
3. **Manajemen Proyek:** Melakukan evaluasi logbook dan penelusuran deviasi target pada *Daily Logbook Tracker* Google Sheets.

**2. Luaran yang Dihasilkan**
- Antarmuka *Web Dashboard SCADA (EMS)* live di port `:8000` dengan dukungan WebSocket hub dan REST API.
- Generator simulasi data telemetri (`dummy_gateway.go`) untuk validasi *pipeline* tanpa ketergantungan perangkat keras.
- Kerangka kode sumber (*Base Code*) C++ ESP32.
- Pembaruan *Daily Logbook Tracker* W4.

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Bantuan *wiring* kelistrikan fisik modul | Komponen dasar tersambung jumper | 100% |
| Software (FW) | Pembuatan *Base Code* Aktuator & Sensor | Kode dasar selesai dan diserahkan ke tim | 100% |
| Software (Web) | Dashboard EMS SCADA Full-Width & WebSocket | Web UI terintegrasi dengan Go ETL live | 100% |
| Mekanik | - | - | 0% |
| Pengujian | Simulasi ingesti data telemetri real-time | Grafik bergerak dinamis tiap 2 detik | 100% |
| Dokumentasi | *Update* rekapitulasi pekan 4 | Logbook W4 terkonsolidasi | 100% |

**Ringkasan Kemajuan**
Meski pengujian transmisi lapangan nirkabel LoRa berjarak jauh ditunda menunggu perakitan akhir oleh rekan tim, penyelesaian subsistem Web Dashboard SCADA (EMS) secara mandiri membuktikan bahwa seluruh rantai pemrosesan data (Database $\rightarrow$ Server Go $\rightarrow$ WebSocket $\rightarrow$ Browser) telah berfungsi dengan sempurna.

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | Modul LoRa fisik masih berada di lokasi rekan tim (Siti) untuk penyetelan frekuensi radio, sehingga pengujian propagasi nirkabel outdoor jarak jauh CPE220 ditunda. | Pengujian komunikasi RF fisik jarak jauh tertunda satu pekan. | Rendah |
| 2 | Terjadi *packet dropping* pada backend Go saat menerima data telemetri berulang karena pemeriksaan urutan ketat (*monotonic sequence number check*). | Telemetri dengan nomor urut sama atau mundur diabaikan oleh ETL. | Sedang (Terselesaikan) |
| 3 | *Canvas rendering bug* pada Chart.js di mana ikon *legend marker* bertabrakan dengan karakter awal label teks pada layar lebar. | Tampilan metrik terpotong dan tidak memenuhi standar UI SCADA. | Rendah (Terselesaikan) |

**Analisis Penyebab & Troubleshooting**
1. **Deduplikasi QoS 1 MQTT:** Mesin ETL Go (`pipeline.go`) menerapkan algoritma deduplikasi ketat di mana paket dengan `SequenceNo <= lastSeq` otomatis di-*drop*. *Dummy gateway* awal tidak menginkrementasikan sequence number. Solusi: Generator disempurnakan dengan `atomic.AddUint32` sehingga setiap paket terkirim memiliki nomor urut berurutan.
2. **Legend Collision Chart.js:** Opsi `usePointStyle: true` pada Chart.js kanvas mengalami konflik kalkulasi lebar font pada resolusi tinggi. Solusi: Menonaktifkan *legend canvas* dan menggantinya dengan *custom flexbox badge* Tailwind CSS yang rapi dan terisolasi.
3. **Penyelarasan Modul Fisik:** Komponen modul radio terpisah untuk efisiensi pekerjaan paralel. Mitigasi: Validasi *pipeline* perangkat lunak diselesaikan 100% menggunakan simulator, sementara *base code* firmware telah disiapkan untuk penggabungan fisik langsung pada Pekan 5.

### G. Tindakan Korektif dan Solusi (Rencana Pekan 5 - 7)
| No | Masalah / Gap Performa | Rencana Tindakan Korektif (Corrective Action) | Alokasi Pekan | PIC |
|:---:|:---|:---|:---:|:---|
| 1 | Pengujian Komunikasi Nirkabel Fisik Belum Terlaksana | Melakukan uji komunikasi lapangan end-to-end (ESP32 LoRa TX $\rightarrow$ Gateway RX $\rightarrow$ Mosquitto Broker) di lingkungan Posko FTUI. | Pekan 5 | Siti & Daffa |
| 2 | Verifikasi Mekanisme Aktuator Nyata | Menguji respons fisik motor servo MG996R terhadap sinyal *downlink command* dari tombol REST API Web Dashboard dengan beban aliran air sesungguhnya. | Pekan 5 | Ilman & Daffa |
| 3 | Optimasi Konsumsi Daya & Manajemen Tidur | Mengimplementasikan mode *Light/Deep Sleep* FreeRTOS pada ESP32 serta mengukur arus (*current draw*) saat mode transmisi vs mode *idle*. | Pekan 6 | Siti & Ilman |
| 4 | Kalibrasi Sensor Gas & Nilai Ambang Batas | Mengkalibrasi kurva resistansi sensor MQ-137 dan MQ-136 terhadap gas referensi serta menguji respons *incident alert* ke database. | Pekan 6 | Raka & Daffa |
| 5 | Ketahanan Jaringan & *Store-and-Forward* | Menguji skenario saat gateway mati: verifikasi penyimpanan ring buffer LittleFS pada ESP32 dan pengiriman ulang (*backlog flush*) saat koneksi pulih. | Pekan 7 | Daffa & Siti |

### H. Dokumentasi Kemajuan
Berikut adalah bukti autentik realisasi pekerjaan pekan keempat yang memenuhi kriteria luaran PMK Pekan 4 (Integrasi Perangkat Keras, Perangkat Lunak, dan Interaksi Sensor-Aktuator):

**1. Bukti Integrasi Perangkat Lunak & Aliran Data End-to-End (Dashboard SCADA EMS Live)**
*Antarmuka Web Dashboard SCADA (EMS) mandiri yang berjalan penuh pada resolusi fluid width di port `:8000`. Menampilkan grafik tren kualitas udara & ketinggian air dinamis via WebSocket, 4 KPI cards, tombol kendali katup pembilasan, dan log audit paket telemetri.*
![Dashboard EMS SCADA](../media/Dashboard%20eSOS.png)

---

**2. Bukti Integrasi Perangkat Keras & Bantuan Wiring Fisik**
*Dokumentasi keterlibatan aktif dalam perakitan kabel kelistrikan (*wiring harness*), pemasangan modul daya baterai LiPo, dan koneksi pin I/O bersama rekan tim (Ilman) di meja perakitan lab/posko.*
![Bantuan Perakitan Wiring Hardware](../media/Bantu%20Wiring.png)

---

**3. Bukti Interaksi Sensor, Pemrosesan, dan Aktuator pada Basecode Firmware (C++)**
*Kerangka kode firmware C++ (`main.cpp`) untuk mikrokontroler ESP32 yang mengintegrasikan FreeRTOS Queue, sensor ultrasonik sonar (`NewPing`), kendali aktuator katup servo (`ESP32Servo`), dan penanganan interupsi tombol darurat (`isr_sos_button`).*
![Basecode Sensor dan Aktuator Firmware C++](../media/Basecode%20Sensor%20dan%20Aktuator.png)

---

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Sinergi Lintas Bidang:** Menyelaraskan *schema payload* JSON antara simulator backend dengan kode firmware C++ Siti.
- **Pendampingan Hardware:** Terjun langsung membantu Ilman dalam perakitan kabel kelistrikan untuk mempercepat persiapan modul fisik.
- **Konsolidasi Manajemen Proyek:** Memastikan target integrasi pekan 4 tetap berjalan sesuai WBS dengan mendahulukan pematangan *full stack* perangkat lunak selagi perangkat keras dirakit.

**Kontribusi Pribadi**
Menuntaskan pilar *frontend* dan visualisasi data SCADA (EMS) secara mandiri sehingga tim kini memiliki antarmuka demonstrasi yang konkret dan siap dipresentasikan kapan pun.

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Pembangunan Web Dashboard SCADA (EMS) berjalan sukses dengan performa latensi sangat rendah (~15 ms) berkat arsitektur WebSocket Go.
- Troubleshooting teknis pada deduplikasi QoS 1 dan *rendering* Chart.js terselesaikan tuntas tanpa menghambat jadwal.
- Inisiatif membantu tim pada sektor *wiring* dan *firmware base code* membuktikan fleksibilitas peran teknik komputer.

**Yang Perlu Diperbaiki**
- Pengujian komunikasi fisik outdoor perlu segera dijadwalkan bersama seluruh anggota tim di pekan kelima agar data simulator dapat langsung digantikan dengan data transmisi riil.

**Pelajaran yang Didapat Minggu Ini**
- Ketiadaan perangkat keras fisik di tangan tidak boleh menghentikan kemajuan perangkat lunak. Pembuatan *dummy simulator* terbukti menjadi strategi rekayasa yang sangat ampuh (*best engineering practice*) untuk validasi integrasi sebelum *field deployment*.

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Uji integrasi End-to-End modul fisik di posko/lab (ESP32 Sensor $\rightarrow$ LoRa Gateway $\rightarrow$ Mosquitto $\rightarrow$ Web Dashboard). | Data riil sensor tertampil di Web EMS tanpa simulator. | 5 Jam |
| 2 | Pengujian fungsional downlink kendali katup pembilasan pada motor servo fisik. | Servo bergerak membuka 90° dan menutup 0° via tombol web. | 4 Jam |
| 3 | Pengambilan rekaman video demonstrasi prototype awal dan persiapan berkas Evaluasi Tengah Semester (Phase Gate 3). | Video demo 3-5 menit & slide presentasi. | 5 Jam |

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
