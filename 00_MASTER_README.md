# Smart-Sanitation eSOS 🚀

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-5.1%20Ultimate-blue)
![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20Go%20%7C%20PostgreSQL-lightgrey)

## 📖 Deskripsi Singkat
**Smart-Sanitation eSOS** (*Emergency Sanitation Operating System*) adalah sistem rekayasa sanitasi cerdas modular dan *Offline-First* yang dirancang khusus untuk fasilitas tanggap darurat pasca-bencana di wilayah terisolir (*blank spot*). Sistem ini mengkombinasikan teknologi radio jarak jauh bebas kuota dengan arsitektur perangkat lunak skala *enterprise* untuk menjamin keselamatan pengungsi tanpa bergantung pada infrastruktur internet.

---

## ✨ Sorotan Arsitektur Super Canggih
Kami mengadopsi standar industri tertinggi untuk memastikan keandalan sistem dalam kondisi ekstrem:

*   🔋 **Ultra-Low Power Firmware:** Mikrokontroler ESP32 berjalan di atas kernel **FreeRTOS** dengan implementasi *Light-Sleep*, *Tickless Idle*, dan *Power Management Locks* (PM Locks). Menjamin efisiensi ekstrem untuk operasi kontinu 24 jam dengan suplai panel surya 20 Wp.
*   🕰️ **True Timestamping & Store-and-Forward:** Sistem didesain tangguh terhadap pemadaman jaringan (*network partition*). Menggunakan modul **RTC DS3231 (I2C)** untuk mencap waktu absolut, serta **LittleFS Circular Buffer** untuk menahan payload data secara persisten di *flash memory* Gateway saat Wi-Fi mati.
*   🚀 **High-Performance Go Backend:** Mesin server ditulis murni menggunakan **Golang** dengan pola *Lambda Architecture*. Dilengkapi *Goroutine Worker Pool* untuk menelan ribuan pesan MQTT secara konkuren, dipadukan dengan **PostgreSQL** yang memanfaatkan *UUIDv7* untuk jaminan 100% ACID (*Atomicity, Consistency, Isolation, Durability*) dan arsitektur *Trigger Listen/Notify*.
*   🌐 **Offline-Ready Web Dashboard:** Antarmuka visual operator dibangun dengan *Tailwind CSS* & *Chart.js* dalam satu paket tunggal (*single-file*). Didukung oleh koneksi **Native WebSocket** dengan fitur *Auto-Reconnect* (3 detik) dan keamanan API tingkat tinggi via *Idempotency-Key* untuk mencegah komando aktuator ganda.

---

## 📡 Topologi Perangkat Keras Jaringan
Alur data end-to-end *(Zero-Trust Pipeline)* terjamin dari hilir ke hulu:

**[Node Bilik WC]** 
👉 Menggunakan Radio **LoRa (TX Only)** untuk transmisi paket biner terenkapsulasi (Bebas Wi-Fi).
↓
**[ESP32 Gateway]** 
👉 Menjembatani dua dunia: Menerima biner via **LoRa (RX)** dan mentransmisikan JSON via **Wi-Fi**.
↓
**[Router TP-Link CPE220]** 
👉 Memancarkan sinyal Wi-Fi luar ruangan berdaya tinggi (High-Power 2.4GHz) ke area tenda darurat.
↓
**[Laptop Server Posko]** 
👉 Menjalankan Mosquitto Broker, Database PostgreSQL, dan Backend Go Server secara *Offline-Local*.

---

## ⚡ Cara Menjalankan (Quick Start)

Sistem dirancang untuk *deployment* instan di lapangan. Cukup lakukan 3 langkah berikut pada Server Laptop Posko:

1. **Jalankan Broker MQTT (Mosquitto)**
   ```powershell
   mosquitto -c src/config/mosquitto.conf -v
   ```
2. **Siapkan Kredensial Lingkungan**
   Buat file `.env` di root direktori proyek berdasarkan `.env.example`.
   ```env
   DB_HOST=localhost
   DB_PORT=5432
   DB_USER=postgres
   DB_PASS=rahasia
   DB_NAME=esos_db
   ```
3. **Jalankan Mesin Go Server**
   ```powershell
   cd src/server
   go run main.go
   ```
   *Dashboard langsung dapat diakses di browser pada alamat `http://localhost:8000` atau IP lokal Router CPE220.*

---
*Dibuat oleh Kelompok 4 - Desain Proyek 2, DTE FTUI (2026).*
