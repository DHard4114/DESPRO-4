# TEMPLATE LAPORAN PEKANAN INDIVIDU
**Capstone Project Desain Proyek 2**

### A. Identitas Mahasiswa
| Item | Keterangan |
|:---|:---|
| Nama | Daffa Hardhan |
| NPM | 2306161763 |
| Program Studi | Teknik Komputer |
| Kelompok | 4 (Empat) |
| Judul Proyek | Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana |
| Pekan ke- | 7 (Tujuh) |
| Periode | 5 Oktober 2026 – 10 Oktober 2026 |
| Tanggal | 10 Oktober 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

---

### B. Target Mingguan
Target yang direncanakan pada awal pekan berdasarkan WBS dan kesinambungan tindak lanjut dari Pekan 6 (di mana integrasi firmware Node WC dan LoRa TX telah berhasil diselesaikan di lab):

| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pengujian konektivitas jaringan lokal nirkabel (Wi-Fi STA) dan penetapan skema IP statis pada Node Gateway ESP32 | ESP32 Gateway terhubung stabil ke Access Point `CompEngQuiz-Server-Live` dengan IP statis `192.168.101.11` | ☑ Selesai &nbsp; ☐ Belum |
| 2 | Verifikasi keterhubungan antar-perangkat (*End-to-End ICMP Ping*) satu per satu antara PC Client, Broker MQTT Laptop, dan Node Gateway | Seluruh simpul jaringan dapat saling mem-ping satu per satu dengan *packet loss* 0% | ☑ Selesai &nbsp; ☐ Belum |
| 3 | Pengujian *transport layer* socket TCP dan integrasi protokol MQTT dari Gateway ke broker laptop (`192.168.101.100:1883`) | ESP32 Gateway mampu membuka soket TCP ke broker laptop (`connected=1`) dan parameter timeout diselaraskan | ☑ Selesai &nbsp; ☐ Belum |
| 4 | Diagnostik kendala inisialisasi perangkat keras radio LoRa SX1278 pada Gateway (mengatasi galat Kode: -2) | Modul SX1278 Gateway terdeteksi pada bus SPI dan lolos `radio.begin()` | ☐ Selesai &nbsp; ☑ Belum |
| 5 | Pengujian transmisi nirkabel LoRa-to-LoRa (menerima data telemetri 34-byte dari Node WC ke Gateway) | Paket telemetri dari Node WC tertangkap di Gateway via LoRa 433 MHz | ☐ Selesai &nbsp; ☑ Belum |

> **Catatan Ketercapaian:** Target 1, 2, dan 3 pada lapisan jaringan (*networking & TCP socket layer*) berhasil diselesaikan 100%. Namun, Target 4 dan 5 **belum selesai** karena radio LoRa SX1278 pada Gateway masih gagal inisialisasi (`Kode: -2`), sehingga transmisi data LoRa-to-LoRa antara Node WC dan Gateway belum dapat berjalan di Pekan 7 dan dilanjutkan ke Pekan 8.

---

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Senin, 5 Okt 2026 | Merancang topologi pengalamatan IP statis subnet `192.168.101.0/24` untuk integrasi posko darurat; mendefinisikan konstanta jaringan pada `config.h`. | 2.5 | Draf Skema Alamat IP & `config.h` |
| Selasa, 6 Okt 2026 | Mengonfigurasi Mosquitto MQTT Broker pada PC laptop (`192.168.101.100`), membuka port 1883 pada firewall Windows, dan menguji pub/sub client lokal. | 3 | Konfigurasi `mosquitto.conf` & Log Broker |
| Rabu, 7 Okt 2026 | Memprogram dan menguji modul Wi-Fi station pada ESP32 Gateway; memvalidasi penerapan IP statis `192.168.101.11` via AP `CompEngQuiz-Server-Live`. | 3 | Log Serial Monitor Wi-Fi Connection |
| Kamis, 8 Okt 2026 | Melakukan pengujian ICMP Ping satu per satu: dari PC ke broker lokal (`192.168.101.100`) dan dari PC ke ESP32 Node Gateway (`192.168.101.11`). | 2.5 | Tangkapan Layar PowerShell Ping Test |
| Jumat, 9 Okt 2026 | Menguji koneksi soket TCP dari ESP32 Gateway ke Broker Port 1883 (`connected=1` dalam 9–22 ms); mendiagnosis dan memperbaiki *timeout* MQTT `state=-4`. | 3 | Log Serial Monitor TCP Result |
| Sabtu, 10 Okt 2026 | Melakukan pengecekan fisik wiring SPI LoRa Gateway guna melacak galat `Kode: -2`; mendokumentasikan hasil pengujian jaringan dan kendala LoRa pada Laporan Pekan 7. | 3 | Pembaruan Firmware & Laporan Pekan 7 |

**Total Jam Kerja Minggu Ini : 17 Jam**

---

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**  
Menindaklanjuti capaian Pekan 6 (di mana kode Node WC telah berhasil dikompilasi dan diuji kirim di lab), fokus pekerjaan di Pekan 7 diarahkan pada penyelarasan Gateway dan lapisan jaringan (*Networking Layer*):
- **Pengujian Jaringan Lokal dan Verifikasi Ping Satu per Satu (Selesai 100%):**  
  Berhasil menghubungkan ESP32 Node Gateway ke Access Point `CompEngQuiz-Server-Live` dengan IP statis `192.168.101.11`. Melakukan verifikasi konektivitas jaringan end-to-end melalui uji *ICMP Ping* satu per satu:
  1. Ping dari PC ke Broker MQTT Laptop (`192.168.101.100`): 4 paket terkirim, 4 diterima, **0% packet loss**, latensi $<1\text{ ms}$.
  2. Ping dari PC ke Node Gateway ESP32 (`192.168.101.11`): 4 paket terkirim, 4 diterima, **0% packet loss**, rata-rata latensi $78\text{ ms}$ ($66\text{ ms} - 89\text{ ms}$).
- **Pengujian Transport Layer Soket TCP ke Broker (Selesai 100%):**  
  Berhasil memvalidasi bahwa ESP32 Gateway mampu membuka koneksi soket TCP langsung ke Broker Mosquitto laptop (`192.168.101.100:1883`) dengan status `[TCP RESULT] connected=1` dalam durasi sangat singkat ($9\text{ ms} - 22\text{ ms}$).
- **Pemetaan dan Standarisasi Pengalamatan IP Sistem (Selesai 100%):**  
  Menetapkan dan mengunci definisi pengalamatan IP resmi pada `config.h`:
  - **MQTT Server (Broker Laptop Ethernet):** `192.168.101.100`
  - **Web Client / Host Laptop:** `192.168.101.10`
  - **Node Gateway ESP32:** `192.168.101.11`
- **Analisis & Perbaikan Timeout Handshake MQTT (Selesai 100%):**  
  Menemukan bahwa respons MQTT `state=-4` (*MQTT_CONNECTION_TIMEOUT*) terjadi akibat batas waktu `mqtt.setSocketTimeout(2)` yang terlalu sempit (2 detik) di atas jaringan nirkabel. Nilai ini telah diperbaiki menjadi 10 detik pada firmware.

**2. Aktivitas yang Tertahan / Belum Selesai (Kendala LoRa-to-LoRa)**  
- **Inisialisasi Radio LoRa SX1278 pada Gateway Gagal (`Kode: -2` / `CHIP_NOT_FOUND`):**  
  Saat Gateway dinyalakan, fungsi `radio.begin()` selalu mengembalikan `Kode: -2`. Meskipun pemetaan pinout SPI (SCK=21, MISO=19, MOSI=18, NSS=5) sudah disamakan persis dengan kode Node WC yang berjalan di Pekan 6, chip radio SX1278 pada Gateway tidak terbaca melalui bus SPI.
- **Transmisi LoRa-to-LoRa Belum Bisa Dilakukan:**  
  Karena radio LoRa pada Gateway belum berhasil diinisialisasi, Gateway belum bisa masuk ke mode penerimaan (*RX Continuous*). Akibatnya, **pengujian transmisi paket data LoRa-to-LoRa dari Node WC ke Gateway belum dapat berjalan di Pekan 7**. Pengujian baru sebatas konektivitas jaringan IP dan soket TCP. Penyelesaian kendala fisik radio LoRa dijadwalkan tuntas pada Pekan 8.

**3. Luaran yang Dihasilkan**
- Pembaruan firmware Gateway (`main.ino` dan `config.h`) dengan perbaikan socket timeout dan konfigurasi IP resmi.
- Bukti tangkapan layar pengujian *ICMP Ping* ke `192.168.101.100` (Broker) dengan hasil 0% loss.
- Bukti tangkapan layar pengujian *ICMP Ping* ke `192.168.101.11` (Node Gateway) dengan hasil 0% loss.
- Bukti log Serial Monitor keberhasilan koneksi Wi-Fi dan soket TCP layer transport (`connected=1`) sekaligus rekaman galat inisialisasi radio LoRa (`Kode: -2`).
- Dokumen Laporan Kemajuan Pekanan Individu Pekan 7.

---

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan (Lingkup Daffa Hardhan)**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware (LoRa Gateway) | Diagnostik & perbaikan galat inisialisasi SX1278 Kode: -2 | Radio belum terdeteksi via SPI; kendala fisik jumper/daya belum tuntas | 30% |
| Transmisi LoRa-to-LoRa | Uji kirim-terima paket 34-byte dari Node WC ke Gateway | Tertahan; belum bisa dilakukan karena modul radio Gateway offline | 20% |
| Software (FW Gateway) | Implementasi Wi-Fi STA, IP statis, & soket TCP/MQTT Gateway | IP statis aktif, Wi-Fi stabil, soket TCP ke port 1883 sukses (`connected=1`) | 90% |
| Software (Host/Broker) | Pengujian Mosquitto MQTT Broker pada laptop client | Broker aktif di port 1883, menerima soket TCP dari Gateway | 90% |
| Pengujian Jaringan | Uji ICMP Ping satu per satu & verifikasi transport layer | Uji Ping 100% tembus (0% loss) dan soket TCP sukses terbentuk | 100% |
| Dokumentasi | Penyusunan logbook & Laporan Individu Pekan 7 | Laporan dan bukti tangkapan layar terarsip lengkap | 100% |

**Ringkasan Kemajuan**
Pada Pekan 7, fondasi lapisan jaringan (*networking & transport layer*) berhasil dibuktikan secara nyata. Seluruh perangkat utama (PC laptop, broker Mosquitto, dan ESP32 Node Gateway) telah terhubung dalam subnet nirkabel lokal `192.168.101.0/24` dengan latensi stabil dan *zero packet loss*. Pembentukan soket TCP dari Gateway ke laptop membuktikan pipa ke broker telah siap. Namun, pengujian fungsional nirkabel LoRa-to-LoRa antara Node WC dan Gateway belum dapat dilaksanakan karena inisialisasi radio SX1278 pada Gateway mengalami kendala fisik (Kode: -2) yang memerlukan perbaikan kabel jumper dan rel daya pada Pekan 8.

---

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | **Galat Inisialisasi Radio SX1278 Gateway (`Kode: -2` / `CHIP_NOT_FOUND`):** Saat booting, fungsi `radio.begin()` pada ESP32 Gateway selalu mengembalikan kode galat -2. | **Transmisi LoRa-to-LoRa dari Node WC ke Gateway tertahan (belum bisa dilakukan).** Gateway tidak dapat memasuki mode dengar (*RX Continuous*) untuk menangkap data sensor dari lapangan. | **Tinggi** |
| 2 | **Timeout Handshake MQTT (`state = -4`):** Setelah soket TCP berhasil dibuka (`connected=1`), pemanggilan `mqtt.connect()` sempat mengalami *timeout* tepat pada durasi 2003 ms akibat batasan timeout default yang terlalu sempit. | ESP32 Gateway sempat gagal menyelesaikan proses *handshake* CONNACK dengan broker Mosquitto sebelum batas timeout dinaikkan. | **Sedang** |
| 3 | **Fluktuasi Catu Daya (*Brownout / Power Reset*):** Log boot ESP32 sempat mencatat `rst:0x1 (POWERON_RESET)` secara berulang saat Wi-Fi memancarkan daya RF tinggi. | Mikrokontroler me-restart mandiri jika arus puncak Wi-Fi tidak ditopang jalur daya USB yang memadai. | **Rendah** |

**Analisis Penyebab**
1. Galat `Kode: -2` pada RadioLib membuktikan bahwa register Chip Version (0x42) pada modul SX1278 mengembalikan nilai 0x00 atau 0xFF. Karena konfigurasi pinout SPI pada kode Gateway (`config.h`: SCK=21, MISO=19, MOSI=18, NSS=5) sudah dibuat identik dengan Node WC yang sukses di Pekan 6, kendala ini murni bersumber dari aspek fisik: resistansi kontak kabel jumper breadboard yang longgar atau tegangan 3.3V drop saat radio diinisialisasi bersamaan dengan radio Wi-Fi.
2. Nilai konfigurasi `mqtt.setSocketTimeout(2)` pada library `PubSubClient` hanya memberi toleransi waktu 2 detik. Pada jaringan nirkabel lapangan, paket `CONNACK` membutuhkan toleransi waktu hingga beberapa detik sehingga memicu kegagalan semu.

---

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Inisialisasi Radio LoRa Gateway (`Kode: -2`) | Melakukan perombakan pengkabelan bus SPI fisik (mengganti jumper dengan kabel pendek berkualitas/solder langsung ke perfboard) serta mengukur kestabilan rel 3.3V menggunakan multimeter digital saat inisialisasi. | Daffa Hardhan | **Pekan 8 (Prioritas Utama)** |
| Timeout Handshake MQTT (`state = -4`) | Meningkatkan `mqtt.setSocketTimeout` dari 2 detik menjadi 10 detik, serta menaikkan batas timeout TCP menjadi 3000 ms pada firmware Gateway. | Daffa Hardhan | **Selesai (Pekan 7)** |
| Fluktuasi Daya (*Brownout*) | Memindahkan sambungan kabel data USB ke port daya berdaya tinggi (USB 3.0) serta merapatkan rel daya 3.3V dan Ground pada breadboard. | Daffa Hardhan | **Selesai (Pekan 7)** |

---

### H. Dokumentasi Kemajuan
Berikut adalah bukti autentik realisasi pekerjaan pekan ketujuh yang mencakup uji keterhubungan ICMP Ping satu per satu serta log Serial Monitor pembentukan soket TCP:

**1. Bukti Uji Ping Laptop ke Broker MQTT (`192.168.101.100`)**  
*Tangkapan layar terminal PowerShell membuktikan koneksi loopback/lokal ke broker Mosquitto di laptop berjalan sempurna: 4 paket terkirim, 4 diterima, **0% packet loss**, dengan waktu respons $<1\text{ ms}$.*  
![Uji Ping Broker Laptop](../../media/Ping_Broker_Laptop_W7.png)

---

**2. Bukti Uji Ping Laptop ke ESP32 Node Gateway (`192.168.101.11`)**  
*Tangkapan layar terminal PowerShell membuktikan keterhubungan nirkabel PC Client ke Node Gateway ESP32: 4 paket terkirim, 4 diterima, **0% packet loss**, dengan rata-rata waktu respons $78\text{ ms}$ (minimal $66\text{ ms}$, maksimal $89\text{ ms}$).*  
![Uji Ping ESP32 Gateway](../../media/Ping_Gateway_ESP32_W7.png)

---

**3. Bukti Serial Monitor Gateway: Koneksi Wi-Fi & Soket TCP Sukses (`connected=1`) serta Rekaman Kendala LoRa (`Kode: -2`)**  
*Tangkapan layar Serial Monitor ESP32 (115200 baud) yang mendokumentasikan dua kondisi riil sistem:*  
1. **Sisi Lapisan Jaringan & Transport Layer (SUKSES):** IP statis `192.168.101.11` terhubung ke SSID `CompEngQuiz-Server-Live` (RSSI -56 dBm), dan lapisan transport berhasil membuka soket TCP ke Broker `192.168.101.100:1883` dengan durasi $22\text{ ms}$ dan $9\text{ ms}$ (`[TCP RESULT] connected=1`).  
2. **Sisi Radio LoRa Gateway (BELUM SELESAI):** Terekam log peringatan `[RADIO ERROR] Inisialisasi Radio SX1278 gagal (Kode: -2). Memeriksa jumper SPI...` yang menjadi bukti otentik mengapa pengujian transmisi LoRa-to-LoRa dari Node WC belum bisa dieksekusi pada pekan ini.  
![Serial Monitor Gateway TCP Connect](../../media/Serial_Monitor_Gateway_TCP_W7.png)

---

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Penetapan Skema IP Jaringan:** Mengomunikasikan pemetaan alamat IP jaringan statis subnet `192.168.101.0/24` kepada seluruh anggota tim (Node Gateway `192.168.101.11`, Laptop Broker `192.168.101.100`, Laptop Client `192.168.101.10`) agar konsisten dengan antarmuka web monitoring.
- **Penyelarasan Kendala LoRa Gateway:** Menginformasikan kepada Amel dan Raka bahwa transmisi nirkabel dari unit Node WC (yang sudah selesai diuji di Pekan 6) belum dapat ditangkap di Gateway karena modul SX1278 Gateway masih mengalami galat SPI Kode: -2, sehingga sesi integrasi gabungan dijadwalkan ulang setelah perbaikan kabel fisik tuntas.

**Kontribusi Pribadi**
Mengonfigurasi dan menguji seluruh lapisan jaringan (Wi-Fi STA, IP statis, verifikasi ICMP Ping satu per satu, pembukaan soket TCP), mengisolasi parameter timeout MQTT, serta menjaga keselarasan repositori Git dan dokumentasi WBS tim.

---

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Seluruh simpul perangkat jaringan berhasil saling terhubung dan terbukti melalui uji ICMP ping satu per satu dengan *zero packet loss* (0% paket hilang).
- Lapisan transport TCP pada ESP32 berhasil membuka koneksi ke port broker 1883 dengan waktu respons sangat cepat ($<25\text{ ms}$).
- Isolasi akar masalah pada timeout MQTT berhasil diidentifikasi secara presisi berdasarkan analisis log Serial Monitor dan diperbaiki pada kode.

**Yang Perlu Diperbaiki / Belum Tercapai**
- Transmisi nirkabel LoRa-to-LoRa antara Node WC dan Gateway belum berhasil dieksekusi karena inisialisasi radio SX1278 pada Gateway mengalami kendala fisik (galat Kode: -2).
- Ketergantungan pada kabel jumper di atas breadboard terbukti rentan terhadap hambatan kontak; perbaikan fisik wiring menjadi pekerjaan rumah wajib yang harus dituntaskan di awal Pekan 8.

**Pelajaran yang Didapat Minggu Ini**
- Pengujian sistem bertingkat (*layered testing*) sangat krusial: dengan memverifikasi lapisan jaringan (Ping dan TCP) terlebih dahulu, kita dapat memastikan bahwa saat masalah radio LoRa terselesaikan, pipa data ke broker dan server sudah siap 100% tanpa hambatan jaringan.

---

### K. Rencana Pekan Berikutnya (Pekan 8)
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Menyelesaikan perbaikan fisik wiring modul LoRa SX1278 pada Gateway (mengatasi galat Kode: -2 hingga lolos inisialisasi SPI) | Radio LoRa Gateway aktif normal dalam mode RX Continuous (`[OK] Radio SX1278 aktif`) | 5 Jam |
| 2 | Melakukan pengujian transmisi nirkabel LoRa-to-LoRa langsung dari Node WC (hasil Pekan 6) ke Gateway | Gateway berhasil menerima dan mendekode paket telemetri biner 34-byte dari Node WC | 5 Jam |
| 3 | Menguji publikasi data telemetri MQTT penuh dari Gateway ke Mosquitto Broker laptop (`192.168.101.100`) dan verifikasi tampilan di Web Dashboard | Data telemetri mengalir utuh dari sensor lapangan $\to$ LoRa $\to$ Gateway $\to$ MQTT Broker $\to$ Web Client | 6 Jam |

---

### L. Persetujuan
**Mahasiswa**  
Nama: Daffa Hardhan  
Tanda tangan: *(Digital)*  
Tanggal: 10 Oktober 2026  

**Ketua Kelompok**  
Nama: Daffa Hardhan  
Tanda tangan: *(Digital)*  
Tanggal: 10 Oktober 2026  

**Catatan Pembimbing**  
....................................................................................  
....................................................................................  
....................................................................................  
