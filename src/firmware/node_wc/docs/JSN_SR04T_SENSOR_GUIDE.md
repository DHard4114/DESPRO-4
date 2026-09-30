# PANDUAN INTEGRASI & PENGUJIAN SENSOR ULTRASONIK JSN-SR04T
## Proyek Capstone: Smart-Sanitation eSOS (Kelompok 4 - Desain Proyek 2 FTUI)

Dokumen ini berisi spesifikasi teknis, peringatan perangkat keras, dan panduan penggunaan sensor jarak ultrasonik kedap air (Waterproof) **JSN-SR04T** yang digunakan untuk mengukur ketinggian air (*Water Level*) pada tangki bilik sanitasi.

---

## 1. Spesifikasi Teknis & Karakteristik

Sensor JSN-SR04T dipilih karena *probe*-nya (kepala sensor) bersifat kedap air (waterproof), sehingga ideal dan aman untuk lingkungan yang lembap seperti bagian dalam tangki air.

* **Tegangan Kerja (VCC):** 5V DC (Penting: Kompatibilitas logika dibahas di bawah).
* **Arus Statis:** ~5 mA.
* **Sudut Deteksi (Beam Angle):** Kurang dari 50 derajat.
* **Jarak Pengukuran Efektif:** 25 cm hingga 450 cm (4.5 meter).
* **Blind Zone (Zona Buta):** **0 - 25 cm**. 
  > **⚠️ PERINGATAN BLIND ZONE:** Sensor tipe ini memiliki kelemahan di mana ia **tidak dapat membaca jarak di bawah 25 cm secara akurat**. Jika jarak air ke permukaan sensor kurang dari 25 cm, sensor akan memberikan data yang kacau/pantulan palsu. 

## 2. Tabel Pengkabelan Fisik (Wiring ESP32)

Sebagian besar sensor JSN-SR04T beroperasi pada tegangan 5V. Pastikan Anda menyambungkan VCC sensor ini ke pin **VIN atau 5V** pada ESP32, BUKAN pin 3V3.

| Pin JSN-SR04T | Pin ESP32 Node | Warna Kabel | Fungsi Utama |
| :--- | :--- | :--- | :--- |
| **5V / VCC** | **VIN / 5V** | Merah | Catu Daya Positif 5V dari adaptor/sistem. |
| **GND** | **GND** | Hitam | Ground (Wajib digabung dengan ground ESP32). |
| **TRIG (RX)** | **D13 (GPIO 13)** | Kuning/Putih | Pin Pemicu - ESP32 memberikan perintah ping. |
| **ECHO (TX)** | **D12 (GPIO 12)** | Biru/Hijau | Pin Pembaca Pantulan - Menerima sinyal balik. |

> *Catatan Elektrikal:* Jika papan JSN-SR04T Anda adalah versi awal yang ketat mengirimkan sinyal ECHO sebesar 5V, Anda mungkin memerlukan pembagi tegangan (*voltage divider*) pada kabel ECHO menuju pin D12 ESP32 agar ESP32 tidak cepat panas/rusak. Namun, banyak versi V3.0 baru yang sudah ramah 3.3V.

## 3. Logika Program & Pustaka (Library `NewPing`)

Pada firmware utama kita (`src/main.cpp`), sistem menggunakan pustaka populer **`NewPing`** agar pembacaan sensor tidak membekukan (*blocking*) program ESP32 saat menunggu pantulan sinyal, yang sangat penting karena kita menggunakan *FreeRTOS*.

**Potongan logika yang dipakai di program Anda:**
```cpp
#include <NewPing.h>

#define PIN_TRIG_US  13
#define PIN_ECHO_US  12
#define MAX_DISTANCE 400 // Jarak maksimum yang diizinkan (400 cm / 4 meter)

NewPing sonar(PIN_TRIG_US, PIN_ECHO_US, MAX_DISTANCE);

void bacaSensorAir() {
    // Menembakkan suara dan menunggu pantulannya dalam bentuk sentimeter (cm)
    unsigned int dist = sonar.ping_cm();
    
    // Fitur Mitigasi Blind Zone di kode Anda: 
    // Jika dist == 0 (Tidak terbaca/pantulan hilang) atau dist < 25 (Masuk ke Blind Zone)
    if (dist < 25 || dist == 0) {
        dist = 25; // Asumsikan air sedang sangat penuh / menyentuh batas aman 25 cm
    }
    
    Serial.printf("Ketinggian air terbaca: %u cm\n", dist);
}
```

## 4. Tips Penting Instalasi di Tangki Air (Praktik Lapangan)

1. **Posisi Mutlak Tegak Lurus:** Pemasangan *probe* harus benar-benar tegak lurus (90 derajat) menghadap langsung ke permukaan air. Sedikit saja kemiringan akan membuat gelombang ultrasonik memantul ke dinding tangki alih-alih memantul kembali ke atas.
2. **Jauhi Dinding Tangki:** Jangan memasang *probe* di pinggir yang menempel ketat dengan dinding tangki. Dinding pipa atau tangki dapat memantulkan sinyal (*false echo*) dan menyebabkan pembacaan meloncat-loncat. Pasang sedekat mungkin ke tengah.
3. **Patuhi Batas Aman 25 cm:** Buat dudukan (*mounting*) sensor yang menggantung atau menonjol ke atas setidaknya 25 cm dari batas maksimal air tertinggi agar air tidak pernah masuk ke *Blind Zone*.
