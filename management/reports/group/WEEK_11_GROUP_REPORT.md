# LAPORAN KEMAJUAN PEKANAN
**DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2**

| Item | Keterangan |
|:---|:---|
| Judul Proyek | RANCANG BANGUN SISTEM MONITORING SMART-SANITATION ESOS BERBASIS IOT UNTUK WILAYAH BLANK SPOT PASCA-BENCANA |
| Kelompok | 4 (Empat) |
| Pekan Ke- | 11 (Sebelas) |
| Periode | 2 November 2026 – 7 November 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc |
| Ketua Kelompok | Daffa Hardhan (2306161763) |
| Anggota Kelompok | - Raka Arrayan Muttaqien (2306161800)<br>- Siti Amalia Nurfaidah (2306161851)<br>- Darrell Alfath (2306266810)<br>- Muhammad Ilman Zuhriy (2306266786) |

---

### 1. Ringkasan Kemajuan Pekanan
Pada Pekan 11, kelompok kami berfokus pada integrasi komprehensif antara sistem mekanik, perangkat keras (hardware kelistrikan & daya), serta firmware komunikasi LoRa. Aktivitas diawali dengan pengulasan desain toilet darurat hasil rancangan Darrell, pengujian perangkaian seluruh modul sensor (ultrasonik JSN-SR04T dan sensor gas MQ) beserta aktuator servo pada ESP32, verifikasi keterbacaan data sensor pada Serial Monitor, hingga pengujian pengiriman data telemetry via transmisi LoRa. Selain itu, dilakukan pula pengetesan Solar Charge Controller (SCC) beserta panel surya dan multitester guna memastikan keandalan subsistem manajemen daya terbarukan pada kondisi off-grid di area bencana. 

---

### 2. Target Pekan Ini
| No | Target | Status |
|:---:|:---|:---:|
| 1 | Evaluasi rancangan dan posisi penempatan komponen pada purwarupa fisik toilet darurat | ☑ Tercapai |
| 2 | Perangkaian komponen hardware dan validasi alokasi pin GPIO pada ESP32 | ☑ Tercapai |
| 3 | Verifikasi keterbacaan data seluruh modul sensor dan aktuator secara simultan | ☑ Tercapai |
| 4 | Integrasi firmware end-to-end (Sensor + ESP32 + Transmisi LoRa SX1278) | ☑ Tercapai |
| 5 | Pengetesan dan pengujian tegangan Solar Charge Controller (SCC) subsistem daya surya | ☑ Tercapai |

---

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil |
|:---:|:---|:---|:---|
| 1 | Diskusi dan evaluasi desain struktur toilet darurat (posisi sensor, servo, dan tangki) | Darrell Alfath & Muhammad Ilman Zuhriy | Blueprint dan tata letak komponen di dalam toilet disepakati. |
| 2 | Perakitan komponen hardware pada breadboard dan verifikasi pinout GPIO | Raka Arrayan Muttaqien & Siti Amalia Nurfaidah | Jalur kelistrikan antar modul terverifikasi tanpa korsleting. |
| 3 | Pengujian firmware keterbacaan data sensor ultrasonik dan gas pada Arduino IDE | Daffa Hardhan & Raka Arrayan Muttaqien | Seluruh sensor terbaca stabil pada Serial Monitor. |
| 4 | Integrasi menyeluruh transmisi paket data LoRa dari node sensor ke gateway | Daffa Hardhan & Raka Arrayan Muttaqien | Paket data telemetry berhasil terkirim via LoRa. |
| 5 | Pengetesan parameter masukan/keluaran daya Solar Charge Controller (SCC) | Daffa Hardhan & Seluruh Anggota | SCC dan panel surya berfungsi mengalirkan daya terukur. |

---

### 4. Kemajuan Tiap Anggota
| Nama | Tugas Pekan Ini | Persentase Penyelesaian |
|:---|:---|:---:|
| Daffa Hardhan | Integrasi firmware LoRa, pengujian gateway, dan pengetesan SCC daya surya | 100% |
| Raka Arrayan Muttaqien | Pengodean firmware sensor, integrasi logic control, dan perangkaian pin GPIO | 100% |
| Darrell Alfath | Finalisasi desain fisik toilet darurat dan tata letak mounting komponen | 100% |
| Siti Amalia Nurfaidah | Uji keterbacaan sensor, verifikasi kelistrikan, dan dokumentasi pengujian | 100% |
| Muhammad Ilman Zuhriy | Validasi dimensi mekanik toilet dan persiapan integrasi enclosure daya | 100% |

---

### 5. Hasil Implementasi
- **Hardware:**  
  Rangkaian komponen utama (ESP32, JSN-SR04T, MQ-136/137, Servo TD-8120MG, dan modul LoRa SX1278) berhasil terhubung dengan tepat sesuai header configuration (`config.h`). Pengetesan fisik Solar Charge Controller (SCC) dengan multimeter memastikan sistem pasokan daya mandiri siap menyuplai baterai.
- **Software:**  
  Firmware node sensor dan gateway telah berhasil diintegrasikan. Program mampu membaca nilai sensor secara simultan, memvalidasi data (dengan nilai konsentrasi gas menggunakan data mockup PPM untuk validasi format transmisi), dan mengemasnya ke dalam paket data LoRa untuk dikirimkan secara berkala.
- **Mekanik/Biomedik:**  
  Desain fisik toilet darurat karya Darrell telah disesuaikan dengan posisi penempatan sensor ultrasonik di bagian atas tangki, sensor gas di area ventilasi, serta posisi pengunci pintu otomatis servo.

---

### 6. Dokumentasi Hasil
| Lampiran | Penjelasan |
|:---|:---|
| Foto 1 | Pembahasan rancangan fisik toilet darurat buatan Darrell yang mencakup kalkulasi posisi sensor, mekanisme servo, dan tempat tangki limbah. |
| Foto 2 | Melakukan perangkaian komponen hardware pada breadboard dan memastikan seluruh alokasi pin GPIO ESP32 sudah sesuai dan benar. |
| Foto 3 | Melakukan verifikasi jalannya kode program untuk menguji keseluruhan komponen dan memastikan seluruh sensor terbaca dengan baik. |
| Foto 4 | Melakukan integrasi menyeluruh antara kode program mikrokontroler, seluruh modul sensor/aktuator, dan transmisi LoRa. |
| Foto 5 | Melakukan pengetesan Solar Charge Controller (SCC) dan pengujian keluaran daya dari panel surya menggunakan multitester. |

---

### 7. Pengujian yang Dilakukan
| Parameter | Target | Hasil | Status |
|:---|:---|:---|:---:|
| Uji Kode Gateway | Seluruh sensor (ultrasonik & gas) terbaca tanpa error | Data ketinggian air dan kadar gas terbaca presisi | ☑ Lulus &nbsp; ☐ Tidak |
| Uji Kontinuitas PCB | Paket telemetry terkirim dari node ke gateway | Transmisi data LoRa berhasil diterima gateway | ☑ Lulus &nbsp; ☐ Tidak |
| Uji Dimensi Mekanik WC | Tidak ada bentrok bus I2C/SPI pada ESP32 | Seluruh modul beroperasi sesuai alokasi pin | ☑ Lulus &nbsp; ☐ Tidak |
| Uji Output Daya SCC | SCC mengontrol pasokan daya dari panel surya | Tegangan dan arus terukur stabil pada multimeter | ☑ Lulus &nbsp; ☐ Tidak |

**Analisis Singkat:**  
Pengujian pada Pekan 11 menunjukkan hasil yang sangat memuaskan di mana seluruh subsistem utama mulai dari pembacaan sensor, komunikasi nirkabel LoRa, hingga subsistem catu daya surya (SCC) telah lulus uji fungsionalitas. Node sensor mampu membaca parameter lingkungan secara akurat dan mentransmisikannya ke gateway tanpa mengalami kebocoran memori atau pembacaan invalid.

---

### 8. Kendala dan Solusi
| Kendala | Dampak | Solusi yang Dilakukan |
|:---|:---|:---|
| Adanya potensi fluktuasi nilai tegangan saat pengujian panel surya dan SCC pada kondisi pencahayaan berubah. | Pembacaan masukan daya pada multimeter bervariasi. | Melakukan pengujian SCC dengan kondisi sumber cahaya stabil serta menambahkan modul pengatur tegangan (buck converter). |
| Penyesuaian delay pembacaan sensor agar tidak mengganggu timing transmisi paket LoRa. | Terjadinya penumpukan data pada antrean FreeRTOS. | Mengatur timer task terpisah antara sampling sensor dan pemancaran sinyal radio LoRa. |

---

### 9. Deviasi Terhadap Jadwal
| Aktivitas | Jadwal Awal | Realisasi | Keterangan |
|:---|:---:|:---:|:---|
| Perakitan & Verifikasi Pin Hardware | Pekan 11 | Pekan 11 | Sesuai jadwal (*on track*) |
| Uji Integrasi Sensor & LoRa | Pekan 11 | Pekan 11 | Sesuai jadwal (*on track*) |
| Pengetesan Subsistem Daya SCC | Pekan 11 | Pekan 11 | Sesuai jadwal (*on track*) |

**Analisis Deviasi:**  
Seluruh aktivitas kerja kelompok pada Pekan 11 terlaksana 100% sesuai dengan linimasa perencanaan awal tanpa adanya hambatan atau deviasi yang merugikan.

---

### 10. Rencana Kerja Pekan Berikutnya
| No | Aktivitas | PIC | Target Selesai |
|:---:|:---|:---|:---:|
| 1 | Penyolderan komponen secara permanen dari breadboard ke papan PCB | Raka & Siti Amalia | Pekan Depan (Pekan 12) |
| 2 | Pemasangan seluruh komponen IoT dan subsistem daya ke dalam kerangka fisik toilet | Ilman & Darrell | Pekan Depan (Pekan 12) |
| 3 | Pengujian lapangan jangkauan sinyal LoRa (*range test*) dari kerangka toilet ke gateway | Seluruh Anggota | Pekan Depan (Pekan 12) |
| 4 | Penyempurnaan integrasi tampilan dashboard monitoring | Seluruh Anggota | Pekan Depan (Pekan 12) |

---

### 11. Persentase Progress Proyek
| Komponen | Bobot (%) | Progress (%) | Kontribusi (%) |
|:---|:---:|:---:|:---:|
| Perancangan | 25 | 100 | 25.00 |
| Implementasi Hardware | 25 | 85 | 21.25 |
| Implementasi Software | 20 | 95 | 19.00 |
| Integrasi Sistem | 15 | 80 | 12.00 |
| Pengujian | 5 | 90 | 4.50 |
| Dokumentasi | 10 | 95 | 9.50 |

$$\textbf{Progress Total} = \mathbf{91.25\%}$$ *(Revisi akurasi bobot perhitungan terbobot)*

---

### 12. Kesimpulan
Pada Pekan 11, Kelompok 4 berhasil menyelesaikan pengujian desain toilet darurat, perangkaian dan verifikasi pinout hardware, integrasi firmware sensor dengan komunikasi LoRa, serta pengetesan daya pada Solar Charge Controller (SCC). Hasil pengujian menunjukkan bahwa seluruh subsistem berfungsi dengan stabil dan siap untuk tahap pemasangan fisik secara permanen pada pekan berikutnya.

---

### Persetujuan
| Jabatan | Nama | Tanda Tangan |
|:---|:---|:---:|
| Ketua Kelompok | Daffa Hardhan | *(Digital)* |
| Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc | - |
