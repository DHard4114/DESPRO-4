/*
 * =========================================================================================
 * Smart-Sanitation eSOS — UJI MANDIRI SENSOR ULTRASONIK JSN-SR04T (VERSI FREERTOS)
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI)
 * Target Board : DOIT ESP32 DevKit V1
 * Sensor       : JSN-SR04T (Waterproof Ultrasonic)
 * =========================================================================================
 */

#include <Arduino.h>
#include <NewPing.h> // Pastikan library NewPing sudah terpasang

// ==========================================
// 1. DEFINISI PIN & KONSTANTA
// ==========================================
#define PIN_TRIG_US 13
#define PIN_ECHO_US 12
#define MAX_DISTANCE_CM 400 
#define BLIND_ZONE_CM 25

// ==========================================
// 2. INISIALISASI OBJEK SENSOR
// ==========================================
NewPing sonar(PIN_TRIG_US, PIN_ECHO_US, MAX_DISTANCE_CM);

// ==========================================
// 3. TASK FREERTOS: PEMBACAAN SENSOR JSN
// ==========================================
// Ini adalah "Fungsi/Task" yang akan berjalan paralel
void vTaskJsnSensor(void *pvParameters) {
    for (;;) {
        // Melakukan proses "Ping" dan meminta hasil dalam besaran Centimeter (cm)
        unsigned int dist = sonar.ping_cm();

        Serial.print("[DATA RTOS] Jarak Terbaca : ");
        
        // Penanganan Zona Buta & Error
        if (dist == 0) {
            Serial.print("0 cm -> [ERROR] Terlalu Dekat / Out of Range!");
            dist = BLIND_ZONE_CM; 
        } 
        else if (dist < BLIND_ZONE_CM) {
            Serial.printf("%u cm -> [PERINGATAN] Masuk Zona Buta (< 25 cm)!", dist);
            dist = BLIND_ZONE_CM; 
        } 
        else {
            Serial.printf("%u cm -> [OK] Pengukuran Akurat.", dist);
        }

        Serial.printf(" | Final: %u cm\n", dist);

        // Tidur selama 1 detik (1000 ms).
        // Menggunakan vTaskDelay (bukan delay biasa) agar CPU bisa dialihkan 
        // ke pekerjaan lain jika ada (Non-blocking).
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

// ==========================================
// 4. SETUP UTAMA
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Tunggu Serial Monitor siap

    Serial.println("\n=======================================================");
    Serial.println("  UJI MANDIRI SENSOR JSN-SR04T (FREERTOS) - KELOMPOK 4");
    Serial.println("=======================================================");
    Serial.println("Menyiapkan Sistem Operasi FreeRTOS ESP32...");

    // Mendaftarkan dan Meluncurkan Task FreeRTOS ke dalam Core 0
    xTaskCreatePinnedToCore(
        vTaskJsnSensor,   // Nama fungsi Task yang mau dijalankan
        "TaskJsn",        // Nama identitas Task
        2048,             // Ukuran alokasi memori/stack dalam bytes (2KB)
        NULL,             // Parameter untuk task (kosong)
        1,                // Prioritas Task (1 = Normal/Rendah)
        NULL,             // Task Handle (kosong)
        0                 // Dijalankan secara eksklusif di Core 0 (PRO_CPU)
    );
    
    Serial.println("TaskJsn berhasil diluncurkan di Core 0!");
    Serial.println("=======================================================\n");
}

// ==========================================
// 5. LOOP ARDUINO
// ==========================================
void loop() {
    // Karena kita sudah 100% menggunakan arsitektur FreeRTOS, 
    // fungsi loop() bawaan Arduino tidak kita butuhkan lagi.
    // Kita hapus saja loop ini agar memori ESP32 menjadi lebih efisien.
    vTaskDelete(NULL);
}
