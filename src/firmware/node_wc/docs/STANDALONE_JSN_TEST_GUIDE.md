# PANDUAN PENGUJIAN STANDALONE SENSOR ULTRASONIK JSN-SR04T (FreeRTOS ESP32) [v2.0]
## Subkolektif: Node WC (Bilik Sanitasi) — Proyek Smart-Sanitation eSOS (Kelompok 4 FTUI)

---

## 1. Ringkasan Eksekutif & Batas Pengujian

Dokumen ini adalah panduan teknis resmi pengujian mandiri (*standalone test*) untuk subsistem pemantauan volume/ketinggian air tangki pada **Node WC**:
- **Sensor**: JSN-SR04T Waterproof Ultrasonic Transducer (Mode 0: Standard Pulse Trigger & Echo).
- **Mikrokontroler Target**: DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa 32-bit LX6).
- **Library**: `NewPing` (v1.9.7) by Tim Leek.
- **Framework**: Arduino-ESP32 v2.0.17 (ESP-IDF v4.4) dan native Espressif FreeRTOS.

### Batas Lingkup Pengujian Standalone (Revisi v2.0):
1. **Fokus Tunggal**: Validasi waktu pantul pulsa akustik (*time-of-flight*), eliminasi pemalsuan data (*zero-fabrication*), kepatuhan pin strapping ESP32, dan alokasi task FreeRTOS.
2. **Isolasi Subsistem**: Radio LoRa, aktuator servo valve, sensor gas MQ, dan periferal lainnya tidak diaktifkan pada pengujian ini.
3. **Integritas Pengukuran Akustik**:
   - Jika `sonar.ping_cm()` mengembalikan nilai `0`, firmware melaporkannya sebagai `[NO_ECHO / OUT_OF_RANGE]`. Firmware **DILARANG MEMALSUKAN** kegagalan echo sebagai `25 cm` (kondisi tangki penuh).
   - Nilai pantulan di bawah batas fisik transduser ($< 25\text{ cm}$) ditandai secara jujur sebagai `[BLIND_ZONE_WARNING]` akibat redaman osilasi piezoelektrik (*ringing time*).
   - Pengukuran dalam rentang sah ($25\text{ s.d. } 400\text{ cm}$) dilaporkan sebagai `[ECHO_VALID_IN_RANGE]`, bukan "Pengukuran Akurat" karena akurasi sebenarnya bergantung pada kompensasi suhu medium udara ($v = 331,3 + 0,606 \times T$).

---

## 2. Karakteristik Transduser JSN-SR04T & Fenomena Fisik Zona Buta

Modul JSN-SR04T menggunakan **transduser piezoelektrik tunggal** yang berfungsi ganda: sebagai pemancar (*transmitter*) pulsa ultrasonik 40 kHz dan sebagai penerima (*receiver*) gema pantulan.

### Mengapa Zona Buta Berada pada 20 – 25 cm?
1. Saat pin `TRIG` diberi pulsa trigger $10\ \mu\text{s}$, sirkuit modul menggetarkan kristal piezoelektrik dengan 8 siklus semburan 40 kHz.
2. Setelah pulsa berhenti, diafragma transduser tidak seketika berhenti bergetar karena inersia mekanik (*ringing effect*). Jeda redaman ini berlangsung selama $\approx 1,2 \text{ s.d. } 1,5\text{ ms}$.
3. Selama waktu redaman tersebut, transduser belum dapat mendeteksi pantulan balik.
4. Jarak tempuh suara bolak-balik selama $1,4\text{ ms}$:
   $$s = \frac{v \times t}{2} = \frac{343\text{ m/s} \times 0,0014\text{ s}}{2} \approx 0,24\text{ meter} = 24\text{ cm}$$
5. Oleh karena itu, objek yang berada lebih dekat dari $25\text{ cm}$ akan memantulkan gelombang saat transduser masih berdering, sehingga pantulan tertelan atau menghasilkan pembacaan melompat.

---

## 3. Peringatan Kritis Kelistrikan & Pin Strapping GPIO12 (MTDI)

### 3.1 Bahaya Bootloop pada GPIO12
- Pada ESP32, pin **GPIO12 (MTDI)** adalah salah satu dari 5 pin *strapping* internal.
- Level logika GPIO12 pada saat *power-on reset* (POR) atau hard reset menentukan regulator tegangan internal untuk flash SPI (`VDD_SDIO`):
  - **GPIO12 = LOW** (default pull-down internal): `VDD_SDIO = 3.3V` (sesuai chip ESP-WROOM-32).
  - **GPIO12 = HIGH**: `VDD_SDIO = 1.8V`.
- **Dampak Fatal**: Modul JSN-SR04T ditenagai dari rel 5,0V, sehingga output pin `ECHO` memiliki ayunan logika hingga 5,0V. Jika pin `ECHO` modul menarik GPIO12 ke level HIGH saat ESP32 melakukan boot/reset, mikrokontroler akan salah mengonfigurasi memori flash ke 1,8V dan mengalami **kegagalan boot seketika (*bootloop* / flash read error)**.

### 3.2 Solusi Rangkaian Pengkondisi Sinyal Echo:
Gunakan pembagi tegangan resistor sederhana pada jalur `ECHO` modul ke GPIO12 ESP32:

```
    JSN-SR04T Pin ECHO (0 - 5.0V)
                 |
               [1 kΩ]
                 |
                 +---------> Ke GPIO12 ESP32 (Tegangan aman ~3.3V)
                 |
               [2 kΩ]
                 |
                GND (Common Ground)
```

Faktor skala:
$$V_{\text{pin}} = 5,0\text{ V} \times \frac{2\text{ k}\Omega}{1\text{ k}\Omega + 2\text{ k}\Omega} = 3,33\text{ V}$$
Selain membatasi tegangan agar tidak membakar pin ESP32, resistor $2\text{ k}\Omega$ ke ground memastikan pin GPIO12 berada dalam kondisi **LOW saat boot**, menghilangkan risiko kegagalan regulator flash SPI.

---

## 4. Alokasi Pinout Hardware

| Jalur Sensor | Pin ESP32 | Arah Sinyal | Karakteristik & Catatan Proteksi |
| :--- | :---: | :---: | :--- |
| **TRIG** | **GPIO 13** | Output (ESP32 $\to$ JSN) | Pulsa trigger $10\ \mu\text{s}$ aktif HIGH. |
| **ECHO** | **GPIO 12** | Input (JSN $\to$ ESP32) | Pin MTDI Strapping! Wajib melalui pembagi tegangan 1k/2k. |
| **VCC** | **5V External / Vin** | Power | Modul JSN-SR04T membutuhkan catu daya 5,0V DC stabil. |
| **GND** | **GND ESP32** | Power Return | Satukan referensi ground (*common ground*). |

---

## 5. Arsitektur FreeRTOS & Kepatuhan Espressif

Firmware mengimplementasikan arsitektur task deterministik pada **Core 0**:
1. **`vTaskJsnSensor` (Core 0, Prioritas 1, Stack 3072 BYTES)**:
   - Dijalankan secara periodik setiap 1000 ms dengan delay non-blocking `vTaskDelay(pdMS_TO_TICKS(1000))`.
   - Mengambil sampel jarak via `sonar.ping_cm()`.
   - Memeriksa sisa stack menggunakan `uxTaskGetStackHighWaterMark(NULL)` (satuan bytes pada ESP-IDF v4.4).
2. **Verifikasi Return Code `xTaskCreatePinnedToCore`**:
   - Firmware memvalidasi status `result == pdPASS`. Jika gagal akibat kehabisan memori heap, sistem mencetak pesan galat fatal dan berhenti dengan aman.
3. **Reklamasi Memori `loop()`**:
   - `loop()` memanggil `vTaskDelete(NULL)` untuk menghapus `loopTask` bawaan Arduino pada Core 1, mereklamasi 8192 bytes stack yang dialokasikan oleh framework.

---

## 6. Prosedur Pengujian & Verifikasi Serial Monitor

### 6.1 Kompilasi & Flash Firmware (PlatformIO)
```powershell
pio run -d src/firmware/node_wc -e test_jsn_sr04t -t upload
```

### 6.2 Contoh Output Serial Monitor (115200 baud)

```text
========================================================================
 Smart-Sanitation eSOS — Uji Mandiri Ultrasonik JSN-SR04T [v2.0]        
 Subsystem: Sensor Ketinggian Air Tangki (Waterproof Ultrasonic)        
 Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           
========================================================================
Informasi Perangkat Keras & Proteksi:
  - Pin Trigger       : GPIO 13
  - Pin Echo          : GPIO 12 (Peringatan: MTDI Strapping Pin!)
  - Batas Zona Buta   : 25 cm (Ringing period transduser tunggal)
  - Jangkauan Maksimum: 400 cm
  - Catatan Strapping : Pastikan pin ECHO tidak bertegangan HIGH saat boot
                        agar tegangan flash ESP32 tidak salah pilih ke 1.8V.
========================================================================
[BOOT] Mendaftarkan dan meluncurkan TaskJsn ke Core 0...
[BOOT] TaskJsn BERHASIL diluncurkan pada Core 0 (PRO_CPU)!
========================================================================

[JSN #00001] Nilai: [ECHO_OK]     | Jarak:  48 cm | Status: [ECHO DITERIMA, DALAM RENTANG UKUR (25 - 400 cm)]
             [RTOS] Sisa Stack: 2184 Bytes | Core: 0
[JSN #00002] Nilai: [ECHO_OK]     | Jarak:  48 cm | Status: [ECHO DITERIMA, DALAM RENTANG UKUR (25 - 400 cm)]
             [RTOS] Sisa Stack: 2184 Bytes | Core: 0
[JSN #00003] Nilai: [BLIND_ZONE]  | Jarak:  18 cm | Status: [PERINGATAN: MASUK ZONA BUTA (< 25 cm) - TIDAK ANDAL]
             [RTOS] Sisa Stack: 2184 Bytes | Core: 0
[JSN #00004] Nilai: [NO_ECHO]     | Jarak:   0 cm | Status: [TIDAK ADA ECHO / DI LUAR JANGKAUAN (> 400 cm)]
             [RTOS] Sisa Stack: 2184 Bytes | Core: 0
```
