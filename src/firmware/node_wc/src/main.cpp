/**
 * =========================================================================================
 * Smart-Sanitation eSOS — FIRMWARE INTEGRASI NODE WC (ESP32 DevKit V1)
 * Subsystem    : Node WC (Bilik Sanitasi) — Integrasi Sensor, Aktuator Servo, & LoRa
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
 * Target Board : DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa LX6)
 * Sensors      : 
 *   - Winsen MQ-137 (Ammonia / NH3)  -> GPIO32 (ADC1_CH4)
 *   - Winsen MQ-136 (H2S)            -> GPIO33 (ADC1_CH5)
 *   - JSN-SR04T (Ultrasonic Tank)    -> TRIG: GPIO13, ECHO: GPIO12
 *   - Tombol Darurat SOS             -> GPIO27 (Input Pull-up)
 *   - Tegangan Baterai 1S Li-ion     -> GPIO34 (ADC1_CH6)
 * Actuator     : 
 *   - Servo Kunci / Katup TD-8120MG  -> GPIO26 (PWM via ESP32Servo / FreeRTOS TaskActuator)
 * Transceiver  : Ai-Thinker Ra-02 (Semtech SX1278, 433.175 MHz) -> SPI Hardware
 * Framework    : Arduino-ESP32 v2.0.17 (ESP-IDF v4.4) + Native Espressif FreeRTOS
 *
 * ARSITEKTUR KERNEL FREERTOS:
 * -----------------------------------------------------------------------------------------
 * 1. TaskSensors  (Core 0, Prioritas 1): Akuisisi periodik sensor gas MQ, ultrasonik, baterai, & SOS.
 * 2. TaskActuator (Core 0, Prioritas 2): Pengendali event-driven aktuator servo kunci/katup via antrean.
 * 3. TaskLoRaTx   (Core 1, Prioritas 3): Transmisi uplink telemetri 34-byte & jendela dengar downlink 10-byte.
 * 4. Antrean Komunikasi Antar-Task:
 *    - xQueueSensorData : Mengalirkan TelemetryPayload dari TaskSensors ke TaskLoRaTx.
 *    - xQueueActuatorCmd: Mengalirkan ActuatorCommand (dari Downlink LoRa / SOS lokal) ke TaskActuator.
 * =========================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Preferences.h>
#include <RadioLib.h>
#include <NewPing.h>
#include <ESP32Servo.h>
#include <math.h>
#include "config.h"

#ifdef IS_NODE_WC

// =========================================================================================
// 1. STRUKTUR PERSISTENSI NVS UNTUK SENSOR MQ (Schema v2)
// =========================================================================================
#define NVS_MQ_NAMESPACE        "mq_cal"
#define NVS_MQ_BLOB_KEY         "mq_rec"
#define NVS_MQ_MAGIC            0x4D513032  // ASCII 'MQ02'
#define NVS_MQ_VERSION          2

struct __attribute__((packed)) MQChannelRecord {
    float r_top_ohm;
    float r_bottom_ohm;
    float divider_k;
    float rl_nominal_ohm;
    float r0_clean_air_ohm;
    uint8_t is_divider_valid;
    uint8_t is_rl_valid;
    uint8_t is_r0_valid;
    uint8_t reserved;
};

struct __attribute__((packed)) MQNVSRecord {
    uint32_t magic;
    uint16_t schema_version;
    uint16_t reserved;
    MQChannelRecord ch137;
    MQChannelRecord ch136;
    uint32_t crc32;
};

// Parameter aktif sensor gas di RAM
static float g_r_top_137 = 0.0f;
static float g_r_bottom_137 = 0.0f;
static float g_k_137 = 0.0f;
static float g_rl_137 = 0.0f;
static float g_r0_137 = -1.0f;
static bool  g_div_valid_137 = false;
static bool  g_rl_valid_137 = false;
static bool  g_r0_valid_137 = false;

static float g_r_top_136 = 0.0f;
static float g_r_bottom_136 = 0.0f;
static float g_k_136 = 0.0f;
static float g_rl_136 = 0.0f;
static float g_r0_136 = -1.0f;
static bool  g_div_valid_136 = false;
static bool  g_rl_valid_136 = false;
static bool  g_r0_valid_136 = false;

// Tegangan loop nominal sensor gas (Winsen MQ Manual v1.6: Vc = 5.0V +/- 0.1V DC)
#define V_LOOP_SUPPLY_VOLTS     5.00f

// Helper Validasi Terpadu (Konsisten dengan Standalone Test)
static bool validateDivider(float r_top, float r_bottom, float* out_k) {
    if (!isfinite(r_top) || !isfinite(r_bottom) || 
        r_top < 100.0f || r_top > 10000000.0f || 
        r_bottom < 100.0f || r_bottom > 10000000.0f) {
        return false;
    }
    float sum = r_top + r_bottom;
    if (sum <= 0.0f) return false;
    float k = r_bottom / sum;
    if (k < 0.1000f || k > 0.6600f) return false;
    if (out_k != NULL) *out_k = k;
    return true;
}

static bool validateRL(float rl) {
    return (isfinite(rl) && rl >= 100.0f && rl <= 1000000.0f);
}

static bool validateR0(float r0) {
    return (isfinite(r0) && r0 >= 100.0f && r0 <= 10000000.0f);
}

// CRC32 IEEE 802.3
static uint32_t calculateCRC32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

// Memuat kalibrasi MQ dari NVS Flash dengan validasi terpadu
static void loadMQCalibration() {
    Preferences prefs;
    if (!prefs.begin(NVS_MQ_NAMESPACE, true)) {
        Serial.println(F("[NVS] Namespace 'mq_cal' tidak ditemukan. Menjalankan mode sirkuit belum terkonfirmasi."));
        return;
    }

    if (prefs.isKey(NVS_MQ_BLOB_KEY)) {
        MQNVSRecord rec;
        size_t read_bytes = prefs.getBytes(NVS_MQ_BLOB_KEY, &rec, sizeof(rec));
        if (read_bytes == sizeof(rec)) {
            uint32_t expected_crc = calculateCRC32((const uint8_t*)&rec, offsetof(MQNVSRecord, crc32));
            if (rec.magic == NVS_MQ_MAGIC && rec.schema_version == NVS_MQ_VERSION && rec.crc32 == expected_crc) {
                float k137 = 0.0f;
                if (rec.ch137.is_divider_valid && validateDivider(rec.ch137.r_top_ohm, rec.ch137.r_bottom_ohm, &k137)) {
                    g_r_top_137 = rec.ch137.r_top_ohm;
                    g_r_bottom_137 = rec.ch137.r_bottom_ohm;
                    g_k_137 = k137;
                    g_div_valid_137 = true;
                }
                if (rec.ch137.is_rl_valid && validateRL(rec.ch137.rl_nominal_ohm)) {
                    g_rl_137 = rec.ch137.rl_nominal_ohm;
                    g_rl_valid_137 = true;
                }
                if (rec.ch137.is_r0_valid && g_div_valid_137 && g_rl_valid_137 && validateR0(rec.ch137.r0_clean_air_ohm)) {
                    g_r0_137 = rec.ch137.r0_clean_air_ohm;
                    g_r0_valid_137 = true;
                }

                float k136 = 0.0f;
                if (rec.ch136.is_divider_valid && validateDivider(rec.ch136.r_top_ohm, rec.ch136.r_bottom_ohm, &k136)) {
                    g_r_top_136 = rec.ch136.r_top_ohm;
                    g_r_bottom_136 = rec.ch136.r_bottom_ohm;
                    g_k_136 = k136;
                    g_div_valid_136 = true;
                }
                if (rec.ch136.is_rl_valid && validateRL(rec.ch136.rl_nominal_ohm)) {
                    g_rl_136 = rec.ch136.rl_nominal_ohm;
                    g_rl_valid_136 = true;
                }
                if (rec.ch136.is_r0_valid && g_div_valid_136 && g_rl_valid_136 && validateR0(rec.ch136.r0_clean_air_ohm)) {
                    g_r0_136 = rec.ch136.r0_clean_air_ohm;
                    g_r0_valid_136 = true;
                }

                Serial.printf("[NVS] Kalibrasi MQ dimuat: MQ137(div=%s, RL=%s, R0=%.1f [%s]) | MQ136(div=%s, RL=%s, R0=%.1f [%s])\n",
                              g_div_valid_137 ? "OK" : "NO", g_rl_valid_137 ? "OK" : "NO", g_r0_137, g_r0_valid_137 ? "SAH" : "BELUM",
                              g_div_valid_136 ? "OK" : "NO", g_rl_valid_136 ? "OK" : "NO", g_r0_136, g_r0_valid_136 ? "SAH" : "BELUM");
            } else {
                Serial.println(F("[NVS] Data kalibrasi MQ korup (CRC mismatch). Menjalankan mode sirkuit belum terkonfirmasi."));
            }
        }
    } else {
        Serial.println(F("[NVS] Belum ada rekaman kalibrasi MQ di flash. Menjalankan mode diagnostik awal."));
    }
    prefs.end();
}

// =========================================================================================
// 2. KONSTANTA PERINTAH AKTUATOR SERVO & INSTANSIASI PERANGKAT KERAS
// =========================================================================================
// Definisi Command ID Sesuai Kontrak Sistem
#define CMD_LOCK_DOOR         1   // Kunci Pintu Bilik WC (Sudut default: 90 deg)
#define CMD_UNLOCK_DOOR       2   // Buka Kunci Pintu Bilik WC (Sudut default: 0 deg)
#define CMD_FLUSH             3   // Siram Katup Otomatis (Buka 90 deg, tahan 2.5s, tutup 0 deg)
#define CMD_DIRECT_ANGLE      4   // Setel Sudut Servo Langsung (0 - 180 deg via parameter)

#define SERVO_ANGLE_UNLOCKED  0   // Posisi Netral / Kunci Terbuka (Derajat)
#define SERVO_ANGLE_LOCKED   90   // Posisi Kunci Tertutup (Derajat)
#define SERVO_ANGLE_FLUSH    90   // Posisi Katup Siram Terbuka (Derajat)
#define SERVO_MOVE_SETTLE_MS 500  // Durasi rotasi mekanik sebelum PWM didetach untuk hemat daya (ms)

// Handle Antrean FreeRTOS
static QueueHandle_t xQueueSensorData = NULL;
static QueueHandle_t xQueueActuatorCmd = NULL;

// Objek Radio SX1278 (Hardware SPI Bus)
static SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE0));

// Objek Sensor Ultrasonik JSN-SR04T (Rentang ukur maksimum: 400 cm)
static NewPing sonar(PIN_TRIG_US, PIN_ECHO_US, 400);

// Objek Aktuator Servo (ESP32Servo)
static Servo g_servo_actuator;
static volatile uint8_t g_current_servo_angle = SERVO_ANGLE_UNLOCKED;

// Sequence Number Global Telemetri
static uint32_t g_telemetry_seq = 0;

// Task Handles
static TaskHandle_t xHandleTaskSensors = NULL;
static TaskHandle_t xHandleTaskLoRaTx = NULL;
static TaskHandle_t xHandleTaskActuator = NULL;

// Status Rangkaian Sensor Gas
enum SensorCircuitStatus {
    CIRCUIT_OK,
    CIRCUIT_UNCONFIRMED,
    CIRCUIT_SIGNAL_INVALID,
    CIRCUIT_RL_UNCONFIGURED
};

// =========================================================================================
// 3. FUNGSI PEMBACAAN DAN REKONSTRUKSI SENSOR GAS MQ
// =========================================================================================
static float sampleAndCalculateRs(uint8_t pin, bool div_valid, float r_top, float r_bottom, float k,
                                  bool rl_valid, float rl_nominal, float* out_v_pin_mv, float* out_v_ao_mv,
                                  SensorCircuitStatus* out_status = NULL) {
    const int SAMPLES = 8;
    uint32_t mv_sum = 0;

    for (int i = 0; i < SAMPLES; i++) {
        mv_sum += (uint32_t)analogReadMilliVolts(pin);
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    float v_pin_mv = (float)mv_sum / (float)SAMPLES;
    if (out_v_pin_mv != NULL) *out_v_pin_mv = v_pin_mv;

    if (!div_valid || k <= 0.05f) {
        if (out_v_ao_mv != NULL) *out_v_ao_mv = -1.0f;
        if (out_status != NULL) *out_status = CIRCUIT_UNCONFIRMED;
        return -1.0f;
    }

    float v_ao_mv = v_pin_mv / k;
    if (out_v_ao_mv != NULL) *out_v_ao_mv = v_ao_mv;

    if (v_ao_mv < 50.0f || v_ao_mv >= (V_LOOP_SUPPLY_VOLTS * 1000.0f - 50.0f)) {
        if (out_status != NULL) *out_status = CIRCUIT_SIGNAL_INVALID;
        return -1.0f;
    }

    if (!rl_valid || rl_nominal < 100.0f) {
        if (out_status != NULL) *out_status = CIRCUIT_RL_UNCONFIGURED;
        return -1.0f;
    }

    float v_ao_volts = v_ao_mv / 1000.0f;
    float r_divider_total = r_top + r_bottom;
    float rl_eff = (rl_nominal * r_divider_total) / (rl_nominal + r_divider_total);
    float rs = ((V_LOOP_SUPPLY_VOLTS / v_ao_volts) - 1.0f) * rl_eff;

    if (isfinite(rs) && rs > 0.0f) {
        if (out_status != NULL) *out_status = CIRCUIT_OK;
        return rs;
    } else {
        if (out_status != NULL) *out_status = CIRCUIT_SIGNAL_INVALID;
        return -1.0f;
    }
}

// =========================================================================================
// 4. PENGENDALI HARDWARE SERVO (Thread-Safe & Power-Saving)
// =========================================================================================
static void executeServoMove(uint8_t target_angle, uint32_t hold_duration_ms = 0) {
    if (target_angle > 180) target_angle = 180;

    Serial.printf("[SERVO HW] Mengaktifkan PWM GPIO %d -> Memutar ke %u derajat...\n", PIN_SERVO, target_angle);

    // Pasang PWM hardware LEDC jika belum terpasang
    if (!g_servo_actuator.attached()) {
        g_servo_actuator.setPeriodHertz(50); // Standar 50 Hz PWM
        g_servo_actuator.attach(PIN_SERVO, 500, 2400); // Pulse width: 500 us - 2400 us
    }

    g_servo_actuator.write(target_angle);
    g_current_servo_angle = target_angle;

    // Jeda non-blocking FreeRTOS agar motor mekanik (TD-8120MG / MG996R) tuntas berputar
    vTaskDelay(pdMS_TO_TICKS(SERVO_MOVE_SETTLE_MS));

    // Jika ada durasi hold khusus (misal katup siram flush valve)
    if (hold_duration_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(hold_duration_ms));
    }

    // Detach PWM pulse untuk menghemat konsumsi baterai dan menghilangkan getaran (jitter) motor
    g_servo_actuator.detach();
    Serial.printf("[SERVO HW OK] Posisi servo tercapai di %u derajat (PWM didetach untuk efisiensi daya).\n", target_angle);
}

// Helper Enqueue Perintah Aktuator (Thread-Safe untuk Dipanggil dari Task Manapun)
static bool enqueueActuatorCommand(uint8_t command_id, uint8_t parameter) {
    if (xQueueActuatorCmd == NULL) return false;
    ActuatorCommand cmd;
    memset(&cmd, 0, sizeof(ActuatorCommand));
    strncpy(cmd.node_code, NODE_CODE, sizeof(cmd.node_code) - 1);
    cmd.command_id = command_id;
    cmd.parameter = parameter;
    return (xQueueSend(xQueueActuatorCmd, &cmd, pdMS_TO_TICKS(100)) == pdTRUE);
}

// =========================================================================================
// 5. TASK 1: PEMBACAAN SELURUH SUBSISTEM SENSOR (CORE 0, PRIORITAS 1)
// =========================================================================================
void vTaskSensors(void *pvParameters) {
    (void)pvParameters;

    // Konfigurasi ADC1 (GPIO32, GPIO33, GPIO34)
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_MQ137_AO, ADC_11db);
    analogSetPinAttenuation(PIN_MQ136_AO, ADC_11db);
    analogSetPinAttenuation(PIN_BATT_VOLT, ADC_11db);

    // Memuat konfigurasi kalibrasi MQ dari flash NVS
    loadMQCalibration();

    for (;;) {
        TelemetryPayload payload;
        memset(&payload, 0, sizeof(TelemetryPayload));

        payload.schema_version = 1;
        strncpy(payload.node_code, NODE_CODE, sizeof(payload.node_code) - 1);
        payload.sequence_no = ++g_telemetry_seq;
        payload.uptime_seconds = (uint32_t)(millis() / 1000);

        // -------------------------------------------------------------
        // A. PENGUKURAN SENSOR ULTRASONIK JSN-SR04T (LEVEL AIR TANGKI)
        // -------------------------------------------------------------
        unsigned int dist = sonar.ping_cm();

        if (dist == 0) {
            payload.water_level_cm = -1.0f;
            Serial.println(F("[SENSOR US] JSN-SR04T: [NO_ECHO / OUT_OF_RANGE] (Ketinggian Air: INVALID)"));
        } 
        else if (dist < 25) {
            payload.water_level_cm = -1.0f;
            Serial.printf("[SENSOR US] JSN-SR04T: [BLIND_ZONE] Jarak: %u cm (< 25 cm zona buta) | Level Air: INVALID\n", dist);
        } 
        else {
            payload.water_level_cm = -1.0f;
            Serial.printf("[SENSOR US] JSN-SR04T: [ECHO_OK] Jarak Terbaca: %u cm | Level Air: N/A [TANK_GEOMETRY_UNCONFIGURED]\n", dist);
        }

        // -------------------------------------------------------------
        // B. PENGUKURAN SENSOR GAS MQ-137 (AMONIA / NH3)
        // -------------------------------------------------------------
        float v_pin_137_mv = 0.0f;
        float v_ao_137_mv = 0.0f;
        SensorCircuitStatus status_137 = CIRCUIT_UNCONFIRMED;
        float rs_137 = sampleAndCalculateRs(PIN_MQ137_AO, g_div_valid_137, g_r_top_137, g_r_bottom_137, g_k_137,
                                            g_rl_valid_137, g_rl_137, &v_pin_137_mv, &v_ao_137_mv, &status_137);

        payload.ammonia_ppm = -1.0f;

        if (status_137 == CIRCUIT_OK && g_r0_valid_137 && g_r0_137 > 0.0f) {
            float ratio_137 = rs_137 / g_r0_137;
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Rs/R0=%.2f (R0=%.0f) | PPM: N/A [PPM_MODEL_UNCONFIGURED]\n",
                          v_pin_137_mv, v_ao_137_mv, rs_137, ratio_137, g_r0_137);
        } else if (status_137 == CIRCUIT_OK) {
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Status: R0 BELUM TERKALIBRASI | PPM: N/A\n",
                          v_pin_137_mv, v_ao_137_mv, rs_137);
        } else if (status_137 == CIRCUIT_RL_UNCONFIGURED) {
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Status: RL BELUM DIKONFIGURASI | PPM: N/A\n",
                          v_pin_137_mv, v_ao_137_mv);
        } else if (status_137 == CIRCUIT_SIGNAL_INVALID) {
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Status: SINYAL DI LUAR BATAS | PPM: N/A\n",
                          v_pin_137_mv, v_ao_137_mv);
        } else {
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | Status: RANGKAIAN BELUM DIKONFIRMASI | PPM: N/A\n",
                          v_pin_137_mv);
        }

        // -------------------------------------------------------------
        // C. PENGUKURAN SENSOR GAS MQ-136 (HIDROGEN SULFIDA / H2S)
        // -------------------------------------------------------------
        float v_pin_136_mv = 0.0f;
        float v_ao_136_mv = 0.0f;
        SensorCircuitStatus status_136 = CIRCUIT_UNCONFIRMED;
        float rs_136 = sampleAndCalculateRs(PIN_MQ136_AO, g_div_valid_136, g_r_top_136, g_r_bottom_136, g_k_136,
                                            g_rl_valid_136, g_rl_136, &v_pin_136_mv, &v_ao_136_mv, &status_136);

        payload.h2s_ppm = -1.0f;

        if (status_136 == CIRCUIT_OK && g_r0_valid_136 && g_r0_136 > 0.0f) {
            float ratio_136 = rs_136 / g_r0_136;
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Rs/R0=%.2f (R0=%.0f) | PPM: N/A [PPM_MODEL_UNCONFIGURED]\n",
                          v_pin_136_mv, v_ao_136_mv, rs_136, ratio_136, g_r0_136);
        } else if (status_136 == CIRCUIT_OK) {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Status: R0 BELUM TERKALIBRASI | PPM: N/A\n",
                          v_pin_136_mv, v_ao_136_mv, rs_136);
        } else if (status_136 == CIRCUIT_RL_UNCONFIGURED) {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Status: RL BELUM DIKONFIGURASI | PPM: N/A\n",
                          v_pin_136_mv, v_ao_136_mv);
        } else if (status_136 == CIRCUIT_SIGNAL_INVALID) {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Status: SINYAL DI LUAR BATAS | PPM: N/A\n",
                          v_pin_136_mv, v_ao_136_mv);
        } else {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | Status: RANGKAIAN BELUM DIKONFIRMASI | PPM: N/A\n",
                          v_pin_136_mv);
        }

        // -------------------------------------------------------------
        // D. TEGANGAN CATU BATERAI (ADC1_CH6 / GPIO34)
        // -------------------------------------------------------------
        const bool BATT_DIVIDER_PHYSICALLY_INSTALLED = false;

        if (!BATT_DIVIDER_PHYSICALLY_INSTALLED) {
            payload.battery_voltage = -1.0f;
            Serial.println(F("[BATERAI] Rangkaian pembagi baterai fisik belum terpasang (GPIO34 floating). Nilai: N/A (-1.0V)"));
        } else {
            uint32_t batt_pin_mv = (uint32_t)analogReadMilliVolts(PIN_BATT_VOLT);
            float v_batt = ((float)batt_pin_mv * 2.0f) / 1000.0f;
            if (v_batt >= 2.80f && v_batt <= 4.35f) {
                payload.battery_voltage = v_batt;
                Serial.printf("[BATERAI] Tegangan Terbaca: %.2f V (Pin ADC: %u mV)\n", payload.battery_voltage, batt_pin_mv);
            } else {
                payload.battery_voltage = -1.0f;
                Serial.printf("[BATERAI] Tegangan di luar batas wajar: %.2f V -> INVALID (-1.0V)\n", v_batt);
            }
        }

        // -------------------------------------------------------------
        // E. DETEKSI TOMBOL DARURAT SOS (GPIO 27)
        // -------------------------------------------------------------
        if (digitalRead(PIN_BTN_SOS) == LOW) {
            payload.sos_triggered = 1;
            Serial.println(F("[EMERGENCY SOS] Tombol SOS ditekan! Mengirim perintah buka kunci otomatis ke servo..."));
            // Buka kunci pintu secara otomatis saat tombol darurat ditekan
            enqueueActuatorCommand(CMD_UNLOCK_DOOR, SERVO_ANGLE_UNLOCKED);
        } else {
            payload.sos_triggered = 0;
        }

        // -------------------------------------------------------------
        // F. KIRIM DATA KE ANTREAN TRANSMISI LORA
        // -------------------------------------------------------------
        if (xQueueSensorData != NULL) {
            if (xQueueSend(xQueueSensorData, &payload, pdMS_TO_TICKS(1000)) != pdTRUE) {
                Serial.println(F("[PERINGATAN] Antrean telemetri penuh! Paket di-drop."));
            }
        }

        UBaseType_t stackRem = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[RTOS] TaskSensors selesai sampling. Sisa Stack: %u Bytes\n", (unsigned int)stackRem);
        Serial.println(F("------------------------------------------------------------------------"));

        // Interval sampling telemetri: 10 detik
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// =========================================================================================
// 6. TASK 2: PENGENDALI AKTUATOR SERVO (CORE 0, PRIORITAS 2)
// =========================================================================================
void vTaskActuator(void *pvParameters) {
    (void)pvParameters;

    // 1. Inisialisasi awal servo pada boot: Kalibrasi posisi netral (Buka Kunci / 0 derajat)
    Serial.println(F("[ACTUATOR INIT] Memulai inisialisasi Servo TD-8120MG pada GPIO 26..."));
    executeServoMove(SERVO_ANGLE_UNLOCKED);
    Serial.println(F("[ACTUATOR INIT] Servo siap siaga menerima perintah FreeRTOS."));

    for (;;) {
        ActuatorCommand cmd;
        // Blokir task tanpa membebani CPU hingga ada perintah baru di antrean
        if (xQueueReceive(xQueueActuatorCmd, &cmd, portMAX_DELAY) == pdTRUE) {
            // Validasi target node (proses jika ditujukan untuk node ini atau broadcast "ALL")
            if (strncmp(cmd.node_code, NODE_CODE, sizeof(cmd.node_code)) != 0 &&
                strncmp(cmd.node_code, "ALL", 3) != 0) {
                Serial.printf("[ACTUATOR WARN] Perintah diabaikan (Target node: %.8s != %.8s)\n",
                              cmd.node_code, NODE_CODE);
                continue;
            }

            Serial.printf("[ACTUATOR CMD] Diterima perintah ID: %u | Parameter: %u\n",
                          cmd.command_id, cmd.parameter);

            switch (cmd.command_id) {
                case CMD_LOCK_DOOR: {
                    uint8_t angle = (cmd.parameter > 0 && cmd.parameter <= 180) ? cmd.parameter : SERVO_ANGLE_LOCKED;
                    Serial.printf("[ACTUATOR] Menjalankan perintah: KUNCI PINTU (Target: %u deg)\n", angle);
                    executeServoMove(angle);
                    break;
                }

                case CMD_UNLOCK_DOOR: {
                    uint8_t angle = (cmd.parameter <= 180) ? cmd.parameter : SERVO_ANGLE_UNLOCKED;
                    Serial.printf("[ACTUATOR] Menjalankan perintah: BUKA KUNCI PINTU (Target: %u deg)\n", angle);
                    executeServoMove(angle);
                    break;
                }

                case CMD_FLUSH: {
                    uint8_t flush_angle = (cmd.parameter > 0 && cmd.parameter <= 180) ? cmd.parameter : SERVO_ANGLE_FLUSH;
                    Serial.printf("[ACTUATOR] Menjalankan perintah: FLUSH VALVE (Buka %u deg, tahan 2.5s, lalu tutup)\n", flush_angle);
                    // Buka katup siram, tahan 2.5 detik secara non-blocking via FreeRTOS, lalu tutup kembali
                    executeServoMove(flush_angle, 2500);
                    executeServoMove(SERVO_ANGLE_UNLOCKED);
                    Serial.println(F("[ACTUATOR] Siklus siram flush valve selesai."));
                    break;
                }

                case CMD_DIRECT_ANGLE: {
                    if (cmd.parameter <= 180) {
                        Serial.printf("[ACTUATOR] Menjalankan perintah: SUDUT LANGSUNG (%u deg)\n", cmd.parameter);
                        executeServoMove(cmd.parameter);
                    } else {
                        Serial.printf("[ACTUATOR ERROR] Parameter sudut di luar batas: %u (> 180)\n", cmd.parameter);
                    }
                    break;
                }

                default:
                    Serial.printf("[ACTUATOR ERROR] Command ID %u tidak dikenal!\n", cmd.command_id);
                    break;
            }

            UBaseType_t stackRem = uxTaskGetStackHighWaterMark(NULL);
            Serial.printf("[RTOS] TaskActuator sisa stack: %u Bytes\n", (unsigned int)stackRem);
        }
    }
}

// =========================================================================================
// 7. TASK 3: TRANSMISI RADIO LORA SX1278 & JENDELA DENGAR DOWNLINK (CORE 1, PRIORITAS 3)
// =========================================================================================
void vTaskLoRaTx(void *pvParameters) {
    (void)pvParameters;
    TelemetryPayload txData;

    for (;;) {
        // Blokir sampai paket telemetri baru tersedia di antrean
        if (xQueueReceive(xQueueSensorData, &txData, portMAX_DELAY) == pdTRUE) {
            float estimatedToA = radio.getTimeOnAir(sizeof(TelemetryPayload)) / 1000.0f;

            Serial.println(F("\n========================================================================"));
            Serial.printf("[LORA TX] Mengirim Paket #%u ke Gateway Posko (Ukuran: %u Bytes)\n",
                          txData.sequence_no, (unsigned int)sizeof(TelemetryPayload));
            Serial.printf("  Node: %.8s | Uptime: %u s | Level Air: %.1f cm | NH3: %.2f | H2S: %.2f | Batt: %.2f V | SOS: %u\n",
                          txData.node_code, txData.uptime_seconds, txData.water_level_cm,
                          txData.ammonia_ppm, txData.h2s_ppm, txData.battery_voltage, txData.sos_triggered);
            Serial.printf("  Estimasi Time-on-Air (ToA): %.2f ms | Frek: %.3f MHz | SF%d | BW: %.1f kHz\n",
                          estimatedToA, LORA_FREQ, LORA_SF, LORA_BW);

            unsigned long startTxTime = millis();
            int state = radio.transmit((uint8_t*)&txData, sizeof(TelemetryPayload));
            unsigned long durationMs = millis() - startTxTime;

            if (state == RADIOLIB_ERR_NONE) {
                Serial.printf(">>> [TX SUKSES] Paket #%u terkirim secara lokal! Durasi: %lu ms\n",
                              txData.sequence_no, durationMs);
            } else {
                Serial.printf(">>> [TX GAGAL] Kode kesalahan RadioLib: %d\n", state);
            }

            // -------------------------------------------------------------
            // JENDELA DENGAR DOWNLINK (LoRa Class-A Single RX Window)
            // -------------------------------------------------------------
            // Jeda sejenak sebelum mengaktifkan penerimaan (100 ms settling)
            vTaskDelay(pdMS_TO_TICKS(100));

            Serial.println(F("[LORA DOWNLINK] Membuka jendela dengar perintah (RX Single Window)..."));
            uint8_t rxBuffer[sizeof(ActuatorCommand) + 2] = {0};
            int rxState = radio.receive(rxBuffer, sizeof(ActuatorCommand));

            if (rxState == RADIOLIB_ERR_NONE) {
                size_t rxLen = radio.getPacketLength();
                if (rxLen == sizeof(ActuatorCommand)) {
                    ActuatorCommand* rxCmd = (ActuatorCommand*)rxBuffer;
                    Serial.printf("[LORA DOWNLINK OK] Perintah diterima! Target: %.8s | CMD: %u | Param: %u | RSSI: %.1f dBm | SNR: %.1f dB\n",
                                  rxCmd->node_code, rxCmd->command_id, rxCmd->parameter,
                                  radio.getRSSI(), radio.getSNR());

                    if (xQueueActuatorCmd != NULL) {
                        if (xQueueSend(xQueueActuatorCmd, rxCmd, pdMS_TO_TICKS(100)) == pdTRUE) {
                            Serial.println(F("[LORA DOWNLINK OK] Perintah berhasil diteruskan ke TaskActuator."));
                        } else {
                            Serial.println(F("[LORA DOWNLINK WARN] Antrean TaskActuator penuh! Perintah di-drop."));
                        }
                    }
                } else {
                    Serial.printf("[LORA DOWNLINK] Ukuran paket tidak cocok (%u bytes != %u bytes). Diabaikan.\n",
                                  (unsigned int)rxLen, (unsigned int)sizeof(ActuatorCommand));
                }
            } else if (rxState == RADIOLIB_ERR_RX_TIMEOUT) {
                Serial.println(F("[LORA DOWNLINK] Tidak ada perintah downlink (Timeout normal)."));
            } else {
                Serial.printf("[LORA DOWNLINK] Kode status RadioLib: %d\n", rxState);
            }

            radio.standby(); // Kembali ke standby mode hemat daya
            Serial.println(F("========================================================================\n"));
        }
    }
}

// =========================================================================================
// 8. SETUP SISTEM UTAMA
// =========================================================================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Waktu stabilisasi Serial UART pada boot

    Serial.println();
    Serial.println(F("========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — Node WC Firmware Integrasi [v2.1]              "));
    Serial.println(F(" Subsystem: MQ-137, MQ-136, JSN-SR04T, Servo TD-8120MG, & LoRa SX1278  "));
    Serial.println(F(" Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           "));
    Serial.println(F("========================================================================"));

    // 1. Inisialisasi Pin Tombol Darurat SOS
    pinMode(PIN_BTN_SOS, INPUT_PULLUP);

    // 2. Inisialisasi Antrean FreeRTOS
    xQueueSensorData  = xQueueCreate(4, sizeof(TelemetryPayload));
    xQueueActuatorCmd = xQueueCreate(4, sizeof(ActuatorCommand));

    if (xQueueSensorData == NULL || xQueueActuatorCmd == NULL) {
        Serial.println(F("[FATAL] Gagal membuat antrean FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 3. Pre-inisialisasi NSS ke HIGH agar SX1278 tidak merespons noise boot ESP32
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    // 4. Hardware Reset SX1278 (LOW 20 ms, Settling time 100 ms)
    Serial.print(F("1. Melakukan Hardware Reset SX1278 (RST:15)... "));
    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100);
    Serial.println(F("[OK]"));

    // 5. Inisialisasi Bus Hardware SPI
    Serial.printf("2. Inisialisasi Hardware SPI (SCK:%d, MISO:%d, MOSI:%d, NSS:%d)... ",
                  PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);
    Serial.println(F("[OK]"));

    // 6. Inisialisasi Register Radio SX1278 dengan Retry Loop
    Serial.print(F("3. Menghubungi Register Chip Semtech SX1278... "));
    int initState = RADIOLIB_ERR_UNKNOWN;
    for (int attempt = 1; attempt <= 3; attempt++) {
        initState = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
        if (initState == RADIOLIB_ERR_NONE) break;
        delay(50);
    }

    if (initState == RADIOLIB_ERR_NONE) {
        Serial.println(F("[OK]"));
        radio.setCRC(true);
        Serial.println(F("   -> Radio SX1278 berhasil diinisialisasi (433.175 MHz)."));
    } else {
        Serial.printf("[GAGAL] Kode kesalahan: %d\n", initState);
        Serial.println(F("   -> Periksa kabel SPI dan jumper daya 3.3V ke SX1278."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 7. Pembuatan Task FreeRTOS dengan Verifikasi Alokasi Core & Prioritas
    Serial.println(F("4. Meluncurkan Task FreeRTOS..."));

    BaseType_t retSensors = xTaskCreatePinnedToCore(
        vTaskSensors,
        "TaskSensors",
        4096,
        NULL,
        1,
        &xHandleTaskSensors,
        0 // Core 0 (PRO_CPU)
    );

    BaseType_t retActuator = xTaskCreatePinnedToCore(
        vTaskActuator,
        "TaskActuator",
        4096,
        NULL,
        2,
        &xHandleTaskActuator,
        0 // Core 0 (PRO_CPU)
    );

    BaseType_t retLoRa = xTaskCreatePinnedToCore(
        vTaskLoRaTx,
        "TaskLoRaTx",
        4096,
        NULL,
        3,
        &xHandleTaskLoRaTx,
        1 // Core 1 (APP_CPU)
    );

    if (retSensors == pdPASS && retActuator == pdPASS && retLoRa == pdPASS) {
        Serial.println(F("[BOOT] Seluruh Task FreeRTOS berhasil diluncurkan!"));
        Serial.println(F("  - TaskSensors : Core 0, Prioritas 1 (MQ-137, MQ-136, JSN-SR04T, SOS)"));
        Serial.println(F("  - TaskActuator: Core 0, Prioritas 2 (Servo TD-8120MG pada GPIO 26)"));
        Serial.println(F("  - TaskLoRaTx  : Core 1, Prioritas 3 (Transmisi LoRa & Jendela Downlink)"));
    } else {
        Serial.println(F("[FATAL] Gagal membuat Task FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Serial.println(F("========================================================================\n"));
}

// =========================================================================================
// 9. ARDUINO LOOP (PENGHAPUSAN TASK LOOP UNTUK REKLAMASI MEMORI)
// =========================================================================================
void loop() {
    // Menghapus loopTask pada Core 1 agar memori 8KB stack direklamasi oleh Idle Task
    vTaskDelete(NULL);
}

#endif // IS_NODE_WC
