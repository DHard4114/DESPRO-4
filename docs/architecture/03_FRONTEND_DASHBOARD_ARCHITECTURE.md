# DOKUMEN ARSITEKTUR ANTARMUKA WEB DASHBOARD (FRONTEND USER INTERFACE)
## Smart-Sanitation eSOS (Emergency Sanitation Operating System) — Pusat Kendali & Pemantauan Visual Posko Darurat

Status Dokumen: ARSITEKTUR ANTARMUKA TERKENDALI (CONTROLLED BASELINE)  
Teknologi: Hypertext Markup Language 5 (HTML5), Tailwind Cascading Style Sheets (Tailwind CSS), Chart.js, Native WebSocket Client  
Sifat Operasi: Offline-Ready (Dirancang Khusus untuk Jaringan Lokal Posko Bencana Tanpa Internet Publik)  
Author: Daffa Hardhan (Manajer Proyek & Penanggung Jawab Full-Stack Server/Dashboard)  
Institusi: Departemen Teknik Elektro, Fakultas Teknik Universitas Indonesia (DTE FTUI)  

---

## 1. Glosarium Singkatan & Istilah Lengkap Antarmuka (Glossary of Terms)

Untuk memudahkan pemahaman seluruh anggota tim dan tim penguji, berikut adalah daftar kepanjangan resmi dan definisi dari setiap singkatan teknis pada antarmuka dashboard:

| Singkatan | Kepanjangan Lengkap (Full Term) | Penjelasan Sederhana & Fungsi dalam Sistem eSOS |
|:---|:---|:---|
| **UI** | *User Interface* (Antarmuka Pengguna) | Tampilan visual grafis pada layar monitor peramban yang berinteraksi langsung dengan petugas posko. |
| **UX** | *User Experience* (Pengalaman Pengguna) | Kenyamanan, kemudahan navigasi, dan kecepatan pemahaman informasi oleh petugas saat terjadi kondisi darurat bencana. |
| **HTML5** | *Hypertext Markup Language version 5* | Bahasa standar penataan struktur kerangka dasar halaman web modern. |
| **CSS** | *Cascading Style Sheets* | Bahasa tata letak visual untuk mengatur warna, tata letak, ukuran font, dan responsivitas tampilan. |
| **JS** | *JavaScript* | Bahasa pemrograman sisi peramban web untuk mengolah logika interaktif, grafik animasi, dan penerimaan paket data secara *real-time*. |
| **WS / WebSocket** | *WebSocket Protocol (RFC 6455)* | Protokol komunikasi dua arah (*full-duplex*) berlatensi sangat rendah ($< 1\text{ ms}$) antara server Go dan peramban web tanpa perlu memuat ulang (*refresh*) halaman. |
| **JSON** | *JavaScript Object Notation* | Format pertukaran data standar berbasis teks yang ringan dan mudah dibaca oleh manusia maupun komputer. |
| **DOM** | *Document Object Model* | Struktur hirarki pohon elemen di dalam peramban web yang diperbarui secara dinamis oleh JavaScript. |
| **CPE** | *Customer Premises Equipment* | Perangkat pemancar nirkabel luar ruangan (*outdoor router access point*) TP-Link CPE220. |
| **AP** | *Access Point* | Titik pemancar sinyal nirkabel lokal (Wi-Fi) tempat mikrokontroler ESP32 terhubung. |
| **SOS** | *Save Our Souls / Sinyal Darurat Bencana* | Tombol fisik darurat di dalam bilik sanitasi yang ditekan oleh pengungsi saat membutuhkan bantuan medis atau pertolongan darurat. |
| **PPM** | *Parts Per Million* (Bagian per Sejuta) | Satuan konsentrasi gas amonia ($NH_3$) dan hidrogen sulfida ($H_2S$) di udara. |
| **AQI** | *Air Quality Index* (Indeks Kualitas Udara) | Indikator tingkatan mutu udara di bilik sanitasi (*Baik*, *Sedang*, atau *Berbahaya*). |
| **IP** | *Internet Protocol Address* | Alamat numerik identitas perangkat dalam jaringan komputer lokal (contoh: `192.168.0.100`). |

---

## 2. Filosofi Desain & Kebutuhan Khusus Posko Bencana

Antarmuka Web Dashboard [src/server/static/index.html](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/static/index.html) dirancang dengan 4 prinsip rekayasa utama:
1. **Kemandirian Total Tanpa Instalasi Tambahan (*Zero Build Dependency*):** Berbasis HTML5 murni tanpa tahapan kompilasi framework yang rumit, sehingga dapat langsung dimuat dalam hitungan milidetik di peramban web laptop posko bencana.
2. **Kemandirian Jaringan Lokal (*Offline-First Operation*):** Dashboard dapat diakses $100\%$ melalui jaringan Wi-Fi lokal TP-Link CPE220 (`http://192.168.0.100:8000/`) tanpa membutuhkan kuota data atau koneksi internet publik.
3. **Pembaruan Data Instan (*Sub-Second Latency*):** Memanfaatkan koneksi WebSocket untuk memperbarui jarum level air tangki, grafik gas, dan status baterai seketika data dipancarkan oleh sensor ($< 1\text{ ms}$).
4. **Ergonomi Kontras Tinggi (*High-Contrast Dark Mode*):** Menggunakan palet tema gelap (Slate-950) berlatar kontras tinggi untuk menjaga fokus dan mencegah kelelahan mata petugas jaga selama shift pemantauan 24 jam.

---

## 3. Arsitektur Komunikasi UI & Sinkronisasi (Flowchart & Sequence)

Dashboard eSOS dirancang untuk melakukan *rendering* secepat kilat menggunakan *Vanilla JavaScript* dan *WebSockets*. 

### 3.1. Diagram Sekuensial: Pembaruan DOM Real-Time
`mermaid
sequenceDiagram
    autonumber
    participant Browser as Web Browser (DOM)
    participant JS as Logika Frontend (JS)
    participant WS as Go WebSocket Server
    participant Chart as Chart.js Canvas

    Browser->>JS: Halaman Dimuat (window.onload)
    JS->>WS: Buka Koneksi ws://192.168.0.100/ws
    WS-->>JS: 101 Switching Protocols (Tersambung)
    
    loop Setiap Data LoRa Masuk
        WS->>JS: Push Pesan JSON {gas: 150}
        JS->>Browser: Update Teks HTML (innerHTML)
        JS->>Chart: chart.update(150)
        Chart-->>Browser: Render Ulang Kanvas Grafik
    end
`

### 3.2. Pemenuhan Prinsip ACID di Sisi Klien (Frontend)
Meskipun ACID adalah konsep inti *Database*, UI Dashboard meniru logika keandalan tersebut untuk menghindari *glitch* visual:
1. **Atomicity:** Pembaruan UI dilakukan secara serentak (Nilai Teks dan Grafik diperbarui dalam satu fungsi JS atomic). Jika JSON gagal di- *parse*, seluruh visualisasi untuk detik tersebut dibatalkan (tidak ada UI setengah-*update*).
2. **Consistency:** Skema JSON selalu divalidasi oleh blok 	ry...catch di JavaScript sebelum elemen HTML DOM diubah, mencegah angka NaN atau *layout* rusak.
3. **Isolation:** Komunikasi WebSockets untuk Node WC 1 diisolasi ke dalam blok fungsi spesifik WC 1, sehingga ledakan data dari WC 2 tidak akan menyilang dan merusak grafik WC 1.
4. **Durability:** Jika koneksi WebSocket terputus akibat router/Wi-Fi mati, fungsi *Auto-Reconnect JS* (berjalan setiap 3 detik) memastikan UI akan memulihkan *state* visual secara otomatis begitu jaringan pulih tanpa intervensi petugas.

---

## 4. Struktur Tata Letak Komponen Antarmuka (ASCII Component Layout)

Tata letak antarmuka dirancang tersusun secara vertikal intuitif dari atas ke bawah untuk memudahkan pemantauan hierarkis:

```text
+=====================================================================================================+
| 1. BILAH NAVIGASI UTAMA ATAS (TOP NAVIGATION HEADER)                                                |
|    * Status Jaringan AP: TP-Link CPE220 (192.168.0.254)   * Status LoRa Link: 433 MHz SX1278 (Live)  |
|    * Identitas Proyek: Smart-Sanitation eSOS Posko 1      * Jam Operasional Real-Time Posko Bencana |
+=====================================================================================================+
| 2. KARTU PERINGATAN DARURAT SOS (EMERGENCY SOS ALERT BANNER - BERKEDIP MERAH SAAT AKTIF)           |
|    [PERINGATAN DARURAT]: Tombol Bantuan Darurat SOS Ditekan oleh Pengungsi di Bilik Sanitasi!       |
|    Tindakan Petugas: [Verifikasi Lapangan & Matikan Alarm Darurat]                                  |
+=====================================================================================================+
| 3. EMPAT KARTU METRIK TELEMETRI UTAMA (KEY TELEMETRY METRIC CARDS)                                  |
| +-------------------------+ +-------------------------+ +-------------------------+ +-------------+ |
| | LEVEL AIR TANGKI BERSIH | | GAS AMONIA (NH3)        | | GAS HIDROGEN SULFIDA    | | DAYA BATERAI| |
| | Nilai: 65.4 cm (Normal) | | Nilai: 8.2 ppm (Aman)   | | Nilai: 3.1 ppm (Aman)   | | 3.95 V (79%)| |
| | Bar Kapasitas Volume %  | | Batas Bahaya: 25.0 ppm  | | Batas Bahaya: 10.0 ppm  | | Panel Surya | |
| +-------------------------+ +-------------------------+ +-------------------------+ +-------------+ |
+=====================================================================================================+
| 4. GRAFIK DERET WAKTU REAL-TIME (TIME-SERIES LINE CHARTS - CHART.JS RENDERING)                      |
| +---------------------------------------------------+ +-------------------------------------------+ |
| | GRAFIK 1: Tren Kualitas Udara Live (NH3 vs H2S)   | | GRAFIK 2: Tren Ketersediaan Level Air (cm)| |
| | * Garis Hijau : Gas Amonia (NH3)                  | | * Garis Biru: Ketinggian Air Tangki       | |
| | * Garis Oranye: Gas Beracun H2S                   | | * Garis Merah Putus: Batas Kritis < 15 cm | |
| +---------------------------------------------------+ +-------------------------------------------+ |
+=====================================================================================================+
| 5. PANEL KENDALI AKTUATOR & ALAT UJI (ACTUATOR CONTROL & TESTING SUITE)                             |
| +---------------------------------------------------+ +-------------------------------------------+ |
| | KENDALI MANUAL KATUP NIR-SENTUH (SERVO OVERRIDE)  | | SIMULATOR PIPELINE DATA SENSOR KE DATABASE| |
| | Status Saat Ini: TERTUTUP (0 Derajat)             | | Input Uji: Air, Gas NH3, H2S, Voltase, SOS| |
| | [Tombol Buka Katup 90°]  [Tombol Tutup Katup 0°]  | | [Tombol Kirim Paket Uji ke Database Go]   | |
| +---------------------------------------------------+ +-------------------------------------------+ |
+=====================================================================================================+
| 6. TABEL LOG ALIRAN REKAMAN TELEMETRI (LIVE TELEMETRY RECORDS TABLE - SQLITE / POSTGRESQL)          |
|    Waktu Penerimaan | Node ID | Nomor Urut | Level Air | Gas NH3 | Gas H2S | Baterai | Status SOS   |
|    06:25:00 WIB     | NODE_01 | Paket #105 | 65.4 cm   | 8.2 ppm | 3.1 ppm | 3.95 V  | STANDBY (AMAN|
+=====================================================================================================+
```

---

## 4. Logika Pembaruan Data Sisi Klien (Client-Side Logic)

### 4.1 Manajemen Koneksi WebSocket & Auto-Reconnect
Klien peramban web membuka koneksi ke alamat `ws://192.168.0.100:8000/ws`. Jika terjadi gangguan sinyal radio nirkabel di lapangan, skrip JavaScript secara otomatis menjalankan mekanisme penyambungan ulang (*reconnection timer*) setiap 3 detik:

```javascript
// Logika Rekoneksi Otomatis Klien WebSocket
let soketWeb;
function sambungkanWebSocket() {
    const protokol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const alamatWs = `${protokol}//${window.location.host}/ws`;
    
    soketWeb = new WebSocket(alamatWs);
    
    soketWeb.onopen = () => {
        document.getElementById('loraStatus').innerText = '433 MHz SX1278 (WebSocket Siaran Langsung)';
        document.getElementById('loraStatus').className = 'text-emerald-400 font-semibold';
    };
    
    soketWeb.onmessage = (peristiwa) => {
        const dataMasuk = JSON.parse(peristiwa.data);
        if (dataMasuk.type === 'TELEMETRY_STREAM') {
            perbaruiTampilanDashboard(dataMasuk.payload);
        } else if (dataMasuk.type === 'EMERGENCY_ALERT') {
            aktifkanBannerDaruratSOS(dataMasuk.payload);
        }
    };
    
    soketWeb.onclose = () => {
        document.getElementById('loraStatus').innerText = '433 MHz SX1278 (Menyambung Ulang...)';
        document.getElementById('loraStatus').className = 'text-amber-400 font-semibold';
        setTimeout(sambungkanWebSocket, 3000); // Otomatis mencoba menyambung kembali setiap 3 detik
    };
}
```

---

## 5. Prosedur Uji Coba Antarmuka Melalui Simulator Bawaan

Dashboard dilengkapi simulator internal di bagian bawah layar untuk memvalidasi alur data secara mandiri tanpa memerlukan perangkat keras fisik:
1. Masukkan nilai ketinggian air tangki pada kolom **Air (cm)** (contoh: `12.0` untuk menguji kondisi air kritis).
2. Masukkan nilai gas amonia pada kolom **NH3 (ppm)** dan gas beracun pada kolom **H2S (ppm)**.
3. Centang kotak pilihan **Simulasi Tombol SOS Ditekan** untuk menguji mekanisme alarm darurat.
4. Klik tombol **Kirim Paket Sensor ke Database**:
   - Server Go akan menerima data via HTTP POST.
   - Mesin ETL akan memproses kalkulasi dan mendeteksi status bahaya.
   - WebSocket Hub akan memancarkan data secara instan kembali ke layar dashboard.
   - Banner darurat merah akan berkedip dan baris data baru akan muncul pada tabel log.
