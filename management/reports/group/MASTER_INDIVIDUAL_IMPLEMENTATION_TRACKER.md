# MASTER IMPLEMENTATION & DEPENDENCY TRACKER
**(Smart-Sanitation eSOS - Kelompok 4)**

Dokumen ini adalah acuan kerja teknis tingkat individu (*Individual Task Masterlist*) dari Pekan 3 hingga Pekan 14. Anggota dapat melakukan *Fast-Tracking* (mengerjakan tugas masa depan) berdasarkan rincian fase di bawah ini.

---

## 1. PETA KETERGANTUNGAN GLOBAL (CRITICAL PATH)
1. **Hardware Handshake (Ilman → Darrel):** Darrel tidak dapat memfinalisasi dimensi cetak 3D Casing sebelum Ilman mengonfirmasi ukuran blok baterai 1S4P dan dimensi rakitan *perfboard*.
2. **Firmware Merge (Amel + Raka):** Kode logika Sensor (Amel) dan kode logika Servo (Raka) harus di-*merge* ke dalam satu mikrokontroler ESP32 utama.
3. **Data Link (Amel → Daffa):** Format JSON yang dikirimkan oleh LoRa ESP32 (Amel) harus sama persis dengan spesifikasi *endpoint* yang dirancang oleh Server Go (Daffa).

---

## 2. DAFFA HARDHAN (PM, Backend, & Network)
*Fokus: "Otak Pusat", Server, Database, Dashboard, dan Jaringan.*

### Fase 1: Pre-Hardware Readiness (Pekan 3)
- [ ] Menerbitkan Dokumen Spesifikasi API (JSON Contract) untuk pedoman *coding* Amel.
- [ ] Menulis script Python (*Dummy Data Generator*) untuk menembak API lokal secara looping.
- [ ] Mengonfigurasi jaringan LAN/WLAN pada **Router TP-Link CPE220** (Static IP).

### Fase 2: Server Implementation & Database (Pekan 4-6)
- [ ] Merampungkan *coding* backend **Go Server** (`esos-server.exe`) untuk fitur WebSockets.
- [ ] Menguji skema *Database* PostgreSQL 16 (UUIDv7) dengan penulisan ribuan *dummy data* per detik.
- [ ] Mengembangkan **Web Dashboard UI** (HTML/Tailwind/Chart.js) untuk visualisasi live H2S, Amonia, dan Water Level.

### Fase 3: Integration & Finalization (Pekan 7-14)
- [ ] Menghubungkan modul Receiver LoRa Gateway (via Serial/USB) ke program Go Server.
- [ ] Memimpin pembekuan kode (*Code Freeze*) sebelum demonstrasi UTS.
- [ ] Menyusun arsitektur sistem komprehensif pada Laporan Akhir (UAS).

---

## 3. RAKA ARRAYAN (Procurement, Actuator Firmware, QA)
*Fokus: Logistik, Kendali Motor Servo (ESP32), dan Uji Kualitas Lapangan.*

### Fase 1: Procurement & Finance (Pekan 3)
- [ ] Eksekusi *Checkout* Master BoM (15 Komponen) di e-commerce pilihan.
- [ ] Memastikan tanda terima barang di lab dan mengumpulkan seluruh Kuitansi/Invoice digital.
- [ ] Mengajukan formulir administrasi klaim *reimbursement* (sebelum tenggat 25 hari).

### Fase 2: Actuator Firmware (Pekan 4-5)
- [ ] Melakukan setup *PlatformIO* dan *library* ESP32Servo.
- [ ] Menulis logika C++ (PWM) untuk **Motor Servo MG996R** yang akan berfungsi sebagai aktuator katup gas pembuangan septic tank.
- [ ] Melakukan integrasi (*Code Merge*) kode Servo ke dalam program utama ESP32 bersama Amel.

### Fase 3: Field Quality Assurance (Pekan 6-10)
- [ ] Menguji jangkauan nirkabel (*Packet Loss*) LoRa dari jarak >500 meter di wilayah *blank spot*.
- [ ] Memimpin pengujian ketahanan cuaca IP54 (Water spray test) bersama Darrel.
- [ ] Menulis laporan hasil Eksekusi Skenario Pengujian Mutu (TC-01 s.d. TC-06).

---

## 4. SITI AMALIA / AMEL (Firmware Sensors & LoRa)
*Fokus: Memprogram ESP32 untuk pembacaan kondisi Septic Tank dan Telemetri Data.*

### Fase 1: Input Drivers (Pekan 3-4)
- [ ] Menulis *driver* PulseIn untuk membaca *Water Level* dari Sensor Ultrasonik JSN-SR04T.
- [ ] Menulis *driver* ADC (Analog) untuk mengubah voltase Sensor MQ-137 menjadi satuan **ppm (Amonia)**.
- [ ] Menulis *driver* ADC (Analog) untuk Sensor MQ-136 menjadi satuan **ppm (H2S)**.

### Fase 2: Telemetry & Serialization (Pekan 5-6)
- [ ] Merangkum data JSN, MQ137, dan MQ136 menjadi bentuk teks struktur **JSON**.
- [ ] Mengonfigurasi modul **LoRa RA-02 433MHz** via komunikasi protokol SPI pada ESP32.
- [ ] Melakukan transmisi paket JSON via antena LoRa ke *Gateway* secara periodik.

### Fase 3: Firmware Handshake (Pekan 6)
- [ ] Melakukan penyesuaian (*tuning*) Bandwidth & Spreading Factor LoRa agar sinyal kuat menembus dinding beton septic tank.

---

## 5. M. ILMAN ZUHRIY (Hardware Power & Analog Circuit)
*Fokus: Sumber Daya Baterai/Surya, Pre-Heating Sensor, dan Rangkaian Kelistrikan.*

### Fase 1: Power Assembly (Pekan 3-4)
- [ ] Merakit formasi blok **Baterai 18650 (1S4P)** dan menyambungkannya dengan Modul BMS TP4056.
- [ ] Melakukan setup rangkaian *Solar Charge Controller* 10Wp dan modul step-up/down (Buck-Boost).

### Fase 2: Analog Wiring & Calibration (Pekan 4-5)
- [ ] Menyolder seluruh jalur kelistrikan utama di atas papan *Perfboard* (non-Custom PCB).
- [ ] Menyuplai arus 5V ke pemanas (*Heater*) MQ-137 dan MQ-136 secara nonstop selama **24 jam (Pre-Heating)**.
- [ ] Memutar potensio (trimpot) analog di belakang sensor MQ untuk kalibrasi sensitivitas deteksi gas septic tank.

### Fase 3: Endurance Test (Pekan 8-9)
- [ ] Menguji nyala alat 24 jam nonstop untuk menghitung kurva penurunan baterai (*discharge rate*) melawan kurva pengisian tenaga surya.

---

## 6. DARREL ALFATH (Mechanical & 3D Engineering)
*Fokus: Perancangan Enclosure IP54, Mekanisme Servo, dan Waterproofing Septic Tank.*

### Fase 1: CAD & Measurements (Pekan 3-4)
- [ ] Meminta ukuran absolut rakitan baterai 1S4P (dari Ilman) dan ukuran modul ESP32 (dari Amel).
- [ ] Merancang bentuk 3D *Enclosure* (Casing) tahan air di software Fusion 360 / SolidWorks.
- [ ] Merancang tuas/mekanisme dudukan mekanis untuk Servo MG996R (agar bisa memutar katup gas buang).

### Fase 2: Fabrication (Pekan 4-5)
- [ ] Melakukan proses *slicing* (G-Code) dan mencetak casing 3D menggunakan bahan **PETG**.
- [ ] Menyiapkan letak *cable gland* kedap air untuk probe sensor ultrasonik JSN-SR04T.

### Fase 3: Final Assembly & Sealing (Pekan 6-7)
- [ ] Merakit fisik (memasukkan perfboard, baterai, sensor, dan servo) ke dalam casing.
- [ ] Menerapkan *Sealing Gasket* (Silikon RTV) di celah penutup agar kedap cairan/gas septik (Sertifikasi standar IP54).
- [ ] Membantu Raka dalam eksekusi penyemprotan air (Uji ketahanan fisik casing).
