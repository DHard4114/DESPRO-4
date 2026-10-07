/*
 * =========================================================================================
 * Smart-Sanitation eSOS — UJI MANDIRI AKTUATOR SERVO TD-8120MG (VERSI FREERTOS)
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI)
 * Target Board : DOIT ESP32 DevKit V1
 * Aktuator     : Motor Servo Digital High-Torque (TD-8120MG / MG996R)
 * =========================================================================================
 * 
 * PANDUAN PENGKABELAN (WIRING) DENGAN STEP-DOWN:
 * -----------------------------------------------------------------------------------------
 * 1. BATERAI 18650 -> Modul Step-Down IN+ dan IN- (Tegangan Output Step-Down WAJIB diset 5V/6V)
 * 2. KABEL MERAH (Servo)    -> OUT+ (5V/6V) pada Modul Step-Down
 * 3. KABEL HITAM (Servo)    -> OUT- (GND) pada Modul Step-Down
 * 4. KABEL ORANYE (Sinyal)  -> Pin D26 (GPIO 26) pada ESP32
 * 5. KABEL GROUND BERSAMA   -> Tarik satu kabel dari OUT- Step-Down menuju pin GND ESP32
 * 6. POWER ESP32            -> Gunakan kabel USB dari Laptop (untuk upload & Serial Monitor)
 * 
 * =========================================================================================
 */

#include <Arduino.h>
#include <ESP32Servo.h> // Pastikan library "ESP32Servo" karya Kevin Harrington sudah diinstal

// ==========================================
// 1. DEFINISI PIN & OBJEK
// ==========================================
#define PIN_SERVO 26

Servo valveServo;

// ==========================================
// 2. TASK FREERTOS: PENGUJIAN MOTOR SERVO
// ==========================================
void vTaskServoTest(void *pvParameters) {
    for (;;) {
        // --- POSISI 0 DERAJAT (TERBUKA PENUH) ---
        Serial.println("[AKTUATOR] Bergerak ke posisi: 0 Derajat (Pintu Terbuka)");
        valveServo.write(0);
        vTaskDelay(pdMS_TO_TICKS(2000)); // Tunggu 2 detik agar servo selesai bergerak

        // --- POSISI 90 DERAJAT (TERKUNCI) ---
        Serial.println("[AKTUATOR] Bergerak ke posisi: 90 Derajat (Pintu Terkunci)");
        valveServo.write(90);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // --- POSISI 180 DERAJAT (MAKSIMAL) ---
        Serial.println("[AKTUATOR] Bergerak ke posisi: 180 Derajat (Batas Maksimal)");
        valveServo.write(180);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // --- KEMBALI KE POSISI 90 DERAJAT ---
        Serial.println("[AKTUATOR] Kembali ke posisi netral: 90 Derajat");
        valveServo.write(90);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ==========================================
// 3. SETUP UTAMA
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=======================================================");
    Serial.println(" UJI MANDIRI SERVO TD-8120MG (FREERTOS) - KELOMPOK 4");
    Serial.println("=======================================================");

    // Inisialisasi Timer Hardware khusus untuk ESP32Servo
    // (Penting agar PWM sinyal servo stabil dan tidak bergetar)
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Frekuensi standar motor servo adalah 50 Hz
    valveServo.setPeriodHertz(50); 
    
    // Attach Servo ke Pin 26
    // Parameter 500 dan 2500 adalah rentang pulsa (microseconds) yang umum untuk servo torsi besar
    valveServo.attach(PIN_SERVO, 500, 2500); 

    Serial.println("Konfigurasi PWM Servo berhasil!");

    // Meluncurkan Task Servo ke Core 0
    xTaskCreatePinnedToCore(
        vTaskServoTest,   // Fungsi Task
        "TaskServo",      // Identitas Task
        2048,             // Alokasi memori
        NULL,             // Parameter
        2,                // Prioritas 2 (Medium)
        NULL,             // Handle Task
        0                 // Dijalankan di Core 0
    );
    
    Serial.println("TaskServo berhasil diluncurkan. Memulai siklus putaran...\n");
}

// ==========================================
// 4. LOOP (DIHAPUS UNTUK RTOS)
// ==========================================
void loop() {
    vTaskDelete(NULL);
}
