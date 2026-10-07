/*
 * =========================================================================================
 * Smart-Sanitation eSOS — UJI MANDIRI SENSOR ULTRASONIK JSN-SR04T (VERSI FREERTOS v2.0)
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
 * Target Board : DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa LX6)
 * Sensor       : JSN-SR04T (Waterproof Ultrasonic Transducer)
 * Library      : NewPing (v1.9.7) by Tim Leek
 * Framework    : Arduino-ESP32 v2.0.17 (ESP-IDF v4.4) + Native Espressif FreeRTOS
 *
 * CATATAN KETEKNIKAN & KEPATUHAN STANDAR ESPRESSIF:
 * -----------------------------------------------------------------------------------------
 * 1. INTEGRITAS DATA & INTERPRETASI PING (Zero-Fabrication):
 *    - Ketika sonar.ping_cm() mengembalikan 0, NewPing menandakan TIDAK ADA ECHO yang diterima
 *      sebelum batas waktu (timeout setara MAX_DISTANCE_CM = 400 cm). Hal ini terjadi jika:
 *      * Objek di luar jangkauan (> 400 cm).
 *      * Gelombang akustik diserap atau dipantulkan menyudut dari permukaan cairan/dinding.
 *      * Kabel transduser terputus atau modul tidak mendapatkan daya yang cukup.
 *    - DILARANG KERAS mengganti dist == 0 menjadi dist = BLIND_ZONE_CM (25 cm)!
 *      Menggantinya menjadi 25 cm adalah fabrikasi data yang dapat disalahartikan sebagai
 *      kondisi tangki penuh, padahal sensor kehilangan sinyal echo.
 *    - Status dilaporkan secara transparan: NO_ECHO / OUT_OF_RANGE.
 *
 * 2. ZONA BUTA FISIK (Blind Zone < 25 cm):
 *    - Transduser tunggal JSN-SR04T memiliki waktu redaman mekanik (piezoelectric ringing down)
 *      sekitar 1.2 - 1.5 ms setelah pengiriman pulsa emisi. Selama jeda ini, transduser belum
 *      dapat beralih menjadi mikrofon penerima echo.
 *    - Objek berjarak < 20-25 cm menghasilkan pantulan yang tiba sebelum ringing selesai,
 *      sehingga pembacaan tidak andal atau terbaca melompat.
 *    - Firmware mendeteksi kondisi ini dan menandainya sebagai [BLIND_ZONE_WARNING].
 *
 * 3. PERINGATAN STRAPPING PIN GPIO12 (MTDI):
 *    - GPIO12 adalah strapping pin pada ESP32 yang mengontrol tegangan internal flash SPI
 *      (VDD_SDIO: LOW = 3.3V, HIGH = 1.8V).
 *    - Modul JSN-SR04T beroperasi pada tegangan 5V, sehingga output pin ECHO berpotensi
 *      mengeluarkan level logika 5V. Jika pin ECHO menarik GPIO12 ke HIGH saat ESP32 reset,
 *      ESP32 akan salah memilih tegangan flash 1.8V dan mengalami kegagalan boot (bootloop).
 *    - PENTING TENTANG STRAPPING: Pembagi tegangan (misal 1k / 2k) membatasi 5V ke ~3.3V untuk
 *      keamanan input GPIO, namun jika pin ECHO aktif HIGH saat reset, 3.3V tetap terbaca
 *      sebagai logika HIGH. Pastikan jalur ECHO berlogika LOW (idle LOW) saat booting ESP32.
 *
 * 4. KEPATUHAN FREERTOS ESP-IDF:
 *    - Stack task dinyatakan dalam satuan BYTES pada ESP-IDF. Disediakan 3072 BYTES.
 *    - Nilai pengembalian xTaskCreatePinnedToCore() diverifikasi nilainya (pdPASS).
 *    - uxTaskGetStackHighWaterMark() menghasilkan satuan BYTES untuk pemantauan sisa stack.
 *    - loop() memanggil vTaskDelete(NULL) untuk menghapus loopTask pada Core 1 secara legal,
 *      sehingga memori stack 8KB loopTask direklamasi kembali oleh FreeRTOS.
 * =========================================================================================
 */

#include <Arduino.h>
#include <NewPing.h>

// =========================================================================================
// 1. DEFINISI PIN & KONSTANTA HARDWARE
// =========================================================================================
#define PIN_TRIG_US             13      // GPIO13 (Trigger Pulsa)
#define PIN_ECHO_US             12      // GPIO12 (Echo Pulsa - Perhatian: MTDI Strapping Pin!)
#define MAX_DISTANCE_CM         400     // Jangkauan batas maksimum deteksi (cm)
#define BLIND_ZONE_CM           25      // Batas fisik zona buta transduser tunggal (cm)

// FreeRTOS Task Configuration
#define JSN_TASK_CORE           0       // Pinned to Core 0 (PRO_CPU)
#define JSN_TASK_PRIORITY       1       // Priority 1
#define JSN_STACK_BYTES         3072    // 3072 bytes stack depth
#define JSN_SAMPLE_PERIOD_MS    1000    // Sampling setiap 1000 ms

// =========================================================================================
// 2. INSTANSIASI OBJEK SENSOR
// =========================================================================================
NewPing sonar(PIN_TRIG_US, PIN_ECHO_US, MAX_DISTANCE_CM);

// Handle Task FreeRTOS
static TaskHandle_t xHandleTaskJsn = NULL;

// =========================================================================================
// 3. TASK FREERTOS: PEMBACAAN PERIODIK JSN-SR04T (CORE 0)
// =========================================================================================
void vTaskJsnSensor(void *pvParameters) {
    (void)pvParameters;

    uint32_t sample_seq = 0;

    for (;;) {
        sample_seq++;

        // Melakukan proses ping akustik dan menghitung jarak dalam sentimeter
        unsigned int dist = sonar.ping_cm();

        // 1. Evaluasi Status Pengukuran Tanpa Manipulasi Data
        if (dist == 0) {
            // NewPing menghasilkan 0 jika timeout tercapai sebelum echo diterima (tidak ada pantulan)
            Serial.printf("[JSN #%05u] Status: [NO_ECHO]    | Jarak Diagnostik:   0 cm | Keterangan: Tidak ada echo / di luar jangkauan (> %u cm)\n",
                          sample_seq, MAX_DISTANCE_CM);
        } 
        else if (dist < BLIND_ZONE_CM) {
            // Objek berada di dalam rentang zona buta transduser
            Serial.printf("[JSN #%05u] Status: [BLIND_ZONE] | Jarak Diagnostik: %3u cm | Keterangan: Masuk zona buta (< %u cm) - tidak andal\n",
                          sample_seq, dist, BLIND_ZONE_CM);
        } 
        else {
            // Echo diterima secara sah di dalam rentang kerja transduser
            Serial.printf("[JSN #%05u] Status: [ECHO_OK]    | Jarak Sensor: %3u cm | Catatan: Jarak sensor-ke-target (bukan tinggi air tanpa geometri tangki)\n",
                          sample_seq, dist);
        }

        // 2. Pemantauan Resource Memori Stack (ESP-IDF v4.4 mengembalikan satuan BYTES)
        UBaseType_t stackRemBytes = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("             [RTOS] Sisa Stack: %u Bytes | Core: %d\n",
                      (unsigned int)stackRemBytes, xPortGetCoreID());

        // 3. Delay Non-blocking
        vTaskDelay(pdMS_TO_TICKS(JSN_SAMPLE_PERIOD_MS));
    }
}

// =========================================================================================
// 4. SETUP SISTEM
// =========================================================================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Waktu stabilisasi Serial UART pada setup() Arduino-ESP32

    Serial.println();
    Serial.println(F("========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — Uji Mandiri Ultrasonik JSN-SR04T [v2.0]        "));
    Serial.println(F(" Subsystem: Sensor Ketinggian Air Tangki (Waterproof Ultrasonic)        "));
    Serial.println(F(" Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           "));
    Serial.println(F("========================================================================"));
    Serial.println(F("Informasi Perangkat Keras & Proteksi:"));
    Serial.printf("  - Pin Trigger       : GPIO %d\n", PIN_TRIG_US);
    Serial.printf("  - Pin Echo          : GPIO %d (Peringatan: MTDI Strapping Pin!)\n", PIN_ECHO_US);
    Serial.printf("  - Batas Zona Buta   : %d cm (Ringing period transduser tunggal)\n", BLIND_ZONE_CM);
    Serial.printf("  - Jangkauan Maksimum: %d cm\n", MAX_DISTANCE_CM);
    Serial.println(F("  - Catatan Strapping : Pastikan pin ECHO berlogika LOW (idle LOW) saat boot/reset"));
    Serial.println(F("                        agar VDD_SDIO ESP32 tidak salah konfigurasi ke flash 1.8V."));
    Serial.println(F("========================================================================"));
    Serial.println(F("[BOOT] Mendaftarkan dan meluncurkan TaskJsn ke Core 0..."));

    // Mendaftarkan task dengan verifikasi return code eksplisit
    BaseType_t result = xTaskCreatePinnedToCore(
        vTaskJsnSensor,
        "TaskJsn",
        JSN_STACK_BYTES,
        NULL,
        JSN_TASK_PRIORITY,
        &xHandleTaskJsn,
        JSN_TASK_CORE
    );

    if (result == pdPASS) {
        Serial.println(F("[BOOT] TaskJsn BERHASIL diluncurkan pada Core 0 (PRO_CPU)!"));
    } else {
        Serial.printf("[FATAL ERROR] Gagal membuat TaskJsn! Kode error: %d (Heap tidak cukup).\n", (int)result);
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println(F("========================================================================\n"));
}

// =========================================================================================
// 5. ARDUINO LOOP (PENGHAPUSAN TASK LOOP UNTUK REKLAMASI MEMORI)
// =========================================================================================
void loop() {
    // Karena seluruh eksekusi diserahkan kepada FreeRTOS scheduler,
    // loopTask pada Core 1 dihapus agar memori stack 8KB dapat direklamasi oleh Idle Task.
    vTaskDelete(NULL);
}
