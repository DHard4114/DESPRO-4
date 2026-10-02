/**
 * =========================================================================================
 * Smart-Sanitation eSOS — FIRMWARE INTEGRASI NODE WC (ESP32 DevKit V1)
 * Subsystem    : Node WC (Bilik Sanitasi) — Integrasi Sensor & Transmisi LoRa
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
 * Target Board : DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa LX6)
 * Sensors      : 
 *   - Winsen MQ-137 (Ammonia / NH3)  -> GPIO32 (ADC1_CH4)
 *   - Winsen MQ-136 (H2S)            -> GPIO33 (ADC1_CH5)
 *   - JSN-SR04T (Ultrasonic Tank)    -> TRIG: GPIO13, ECHO: GPIO12
 * Transceiver  : Ai-Thinker Ra-02 (Semtech SX1278, 433.175 MHz) -> SPI Hardware
 * Framework    : Arduino-ESP32 v2.0.17 (ESP-IDF v4.4) + Native Espressif FreeRTOS
 *
 * TAHAP INTEGRASI AKTIF:
 * -----------------------------------------------------------------------------------------
 * Sesuai batas pengujian tervalidasi:
 * 1. Akuisisi Gas Ganda MQ-137 & MQ-136 (Sampling ADC1, Rekonstruksi V_AO, Kalkulasi Rs,
 *    dan pemuatan baseline R0 dari NVS flash jika tersedia).
 * 2. Sensor Ultrasonik JSN-SR04T (Penanganan echo jujur: dist == 0 dilaporkan sebagai NO_ECHO,
 *    tanpa manipulasi nilai 25 cm; deteksi zona buta < 25 cm).
 * 3. Transmisi Radio LoRa SX1278 (Pita 433 MHz Permenkomdigi No. 2 Tahun 2025, paket biner 34 byte).
 * 4. Aktuator servo dan downlink kontrol dinonaktifkan sementara sampai tahap uji mandiri aktuator.
 * =========================================================================================
 */
#define IS_NODE_WC 
#include <Arduino.h>
#include <SPI.h>
#include <Preferences.h>
#include <RadioLib.h>
#include <NewPing.h>
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

// Parameter aktif sensor gas di RAM (Aman, default awal tidak terkonfirmasi)
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
    if (k < 0.1000f || k > 0.6600f) return false; // k_max = 0.66 melindungi pin ADC ESP32 (5.0V * 0.66 = 3.3V)
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
                // Salin & validasi MQ-137
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

                // Salin & validasi MQ-136
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
        Serial.println(F("[NVS] Belum ada rekaman kalibrasi MQ di flash. Menjalankan mode diagnostik awal (sirkuit belum terkonfirmasi)."));
    }
    prefs.end();
}

// =========================================================================================
// 2. INSTANSIASI PERANGKAT KERAS & ANTAR-TASK QUEUE
// =========================================================================================
// Handle Antrean FreeRTOS
static QueueHandle_t xQueueSensorData = NULL;

// Objek Radio SX1278 (Hardware SPI Bus)
// Menggunakan SPI 1 MHz untuk kestabilan sinyal tinggi pada jalur jumper breadboard
static SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE0));

// Objek Sensor Ultrasonik JSN-SR04T (Rentang ukur maksimum: 400 cm)
static NewPing sonar(PIN_TRIG_US, PIN_ECHO_US, 400);

// Sequence Number Global Telemetri
static uint32_t g_telemetry_seq = 0;

// Task Handles
static TaskHandle_t xHandleTaskSensors = NULL;
static TaskHandle_t xHandleTaskLoRaTx = NULL;

// =========================================================================================
// 3. FUNGSI PEMBACAAN DAN REKONSTRUKSI SENSOR GAS MQ
// =========================================================================================
static float sampleAndCalculateRs(uint8_t pin, bool div_valid, float r_top, float r_bottom, float k,
                                  bool rl_valid, float rl_nominal, float* out_v_pin_mv, float* out_v_ao_mv) {
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
        return -1.0f; // Pembagi tegangan belum dikonfirmasi fisik
    }

    float v_ao_mv = v_pin_mv / k;
    if (out_v_ao_mv != NULL) *out_v_ao_mv = v_ao_mv;

    // Pengecekan batas sinyal fisik: 50 mV <= V_AO <= (Vc - 50 mV)
    if (v_ao_mv < 50.0f || v_ao_mv >= (V_LOOP_SUPPLY_VOLTS * 1000.0f - 50.0f)) {
        return -1.0f; // Sinyal out of range / saturasi / putus
    }

    if (!rl_valid || rl_nominal < 100.0f) {
        return -1.0f; // Resistor beban RL belum dikonfigurasi fisik
    }

    float v_ao_volts = v_ao_mv / 1000.0f;
    float r_divider_total = r_top + r_bottom; // Pembebanan aktual Rtop + Rbottom
    float rl_eff = (rl_nominal * r_divider_total) / (rl_nominal + r_divider_total);
    float rs = ((V_LOOP_SUPPLY_VOLTS / v_ao_volts) - 1.0f) * rl_eff;

    return (isfinite(rs) && rs > 0.0f) ? rs : -1.0f;
}

// =========================================================================================
// 4. TASK 1: PEMBACAAN SELURUH SUBSISTEM SENSOR (CORE 0, PRIORITY 1)
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
        payload.sos_triggered = 0;

        // -------------------------------------------------------------
        // A. PENGUKURAN SENSOR ULTRASONIK JSN-SR04T (LEVEL AIR TANGKI)
        // -------------------------------------------------------------
        unsigned int dist = sonar.ping_cm();

        if (dist == 0) {
            // NewPing menghasilkan 0 jika timeout tercapai (tidak ada echo)
            // DILARANG MEMALSUKAN dist = 25 cm! Nilai -1.0f menandakan NO_ECHO / Out of Range
            payload.water_level_cm = -1.0f;
            Serial.println(F("[SENSOR US] JSN-SR04T: [NO_ECHO / OUT_OF_RANGE] (Ketinggian Air: INVALID)"));
        } 
        else if (dist < 25) {
            // Objek berada di dalam zona buta transduser tunggal (< 25 cm akibat ringing osilasi)
            // Pembacaan di bawah minimum range TIDAK BOLEH diteruskan sebagai level air valid!
            payload.water_level_cm = -1.0f;
            Serial.printf("[SENSOR US] JSN-SR04T: [BLIND_ZONE] Jarak: %u cm (< 25 cm zona buta) | Level Air: INVALID\n", dist);
        } 
        else {
            // Tinggi total tangki dari dasar hingga ke batas pemasangan sensor
            const float TINGGI_TANGKI_CM = 60.0f; 
            
            // Kalkulasi tinggi/volume air yang tersisa di dalam tangki
            float level_air = TINGGI_TANGKI_CM - (float)dist;

            // Keamanan: Jika tangki sangat kosong (jarak pantulan > tinggi tangki)
            if (level_air < 0) {
                level_air = 0.0f;
            }

            payload.water_level_cm = level_air; 
            Serial.printf("[SENSOR US] JSN-SR04T: [ECHO_OK] Jarak Terbaca: %u cm | Level Air: %.1f cm\n", dist, level_air);
        }

        // -------------------------------------------------------------
        // B. PENGUKURAN SENSOR GAS MQ-137 (AMONIA / NH3)
        // -------------------------------------------------------------
        float v_pin_137_mv = 0.0f;
        float v_ao_137_mv = 0.0f;
        float rs_137 = sampleAndCalculateRs(PIN_MQ137_AO, g_div_valid_137, g_r_top_137, g_r_bottom_137, g_k_137,
                                           g_rl_valid_137, g_rl_137, &v_pin_137_mv, &v_ao_137_mv);

        // DILARANG mengisi ammonia_ppm menggunakan rasio Rs/R0!
        // Sentinel -1.0f menandakan PPM belum tersedia (memerlukan kalibrasi chamber gas)
        payload.ammonia_ppm = -1.0f;

        if (rs_137 > 0.0f && g_r0_valid_137 && g_r0_137 > 0.0f) {
            float ratio_137 = rs_137 / g_r0_137;
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Rs/R0=%.2f (R0=%.0f) | PPM: N/A [CHAMBER_CAL_REQUIRED]\n",
                          v_pin_137_mv, v_ao_137_mv, rs_137, ratio_137, g_r0_137);
        } else if (rs_137 > 0.0f) {
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Status: R0 BELUM TERKALIBRASI | PPM: N/A\n",
                          v_pin_137_mv, v_ao_137_mv, rs_137);
        } else if (v_ao_137_mv > 0.0f) {
            Serial.printf("[SENSOR GAS] MQ-137: V_pin=%.0f mV | V_AO=%.0f mV | Status: SINYAL DI LUAR BATAS (Putus/Jenuh) | PPM: N/A\n",
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
        float rs_136 = sampleAndCalculateRs(PIN_MQ136_AO, g_div_valid_136, g_r_top_136, g_r_bottom_136, g_k_136,
                                           g_rl_valid_136, g_rl_136, &v_pin_136_mv, &v_ao_136_mv);

        // DILARANG mengisi h2s_ppm menggunakan rasio Rs/R0!
        // Sentinel -1.0f menandakan PPM belum tersedia (memerlukan kalibrasi chamber gas)
        payload.h2s_ppm = -1.0f;

        if (rs_136 > 0.0f && g_r0_valid_136 && g_r0_136 > 0.0f) {
            float ratio_136 = rs_136 / g_r0_136;
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Rs/R0=%.2f (R0=%.0f) | PPM: N/A [CHAMBER_CAL_REQUIRED]\n",
                          v_pin_136_mv, v_ao_136_mv, rs_136, ratio_136, g_r0_136);
        } else if (rs_136 > 0.0f) {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Rs=%.0f Ohm | Status: R0 BELUM TERKALIBRASI | PPM: N/A\n",
                          v_pin_136_mv, v_ao_136_mv, rs_136);
        } else if (v_ao_136_mv > 0.0f) {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | V_AO=%.0f mV | Status: SINYAL DI LUAR BATAS (Putus/Jenuh) | PPM: N/A\n",
                          v_pin_136_mv, v_ao_136_mv);
        } else {
            Serial.printf("[SENSOR GAS] MQ-136: V_pin=%.0f mV | Status: RANGKAIAN BELUM DIKONFIRMASI | PPM: N/A\n",
                          v_pin_136_mv);
        }

        // -------------------------------------------------------------
        // D. TEGANGAN CATU BATERAI (ADC1_CH6 / GPIO34)
        // -------------------------------------------------------------
        uint32_t batt_pin_mv = (uint32_t)analogReadMilliVolts(PIN_BATT_VOLT);
        if (batt_pin_mv < 100) {
            payload.battery_voltage = -1.0f; // Divider baterai belum terpasang / floating
            Serial.println(F("[BATERAI] Pin ADC < 100 mV (Rangkaian pembagi baterai belum terpasang / floating)"));
        } else {
            // Pembagi tegangan baterai 1:1 (R1 = R2 = 100k) -> V_batt = V_pin * 2
            payload.battery_voltage = ((float)batt_pin_mv * 2.0f) / 1000.0f;
            Serial.printf("[BATERAI] Tegangan Terbaca: %.2f V (Pin ADC: %u mV)\n", payload.battery_voltage, batt_pin_mv);
        }

        // -------------------------------------------------------------
        // E. KIRIM KE ANTREAN TRANSMISI LORA
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
// 5. TASK 2: TRANSMISI RADIO LORA SX1278 (CORE 1, PRIORITY 3)
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
            Serial.printf("  Node: %.8s | Uptime: %u s | Level Air: %.1f cm | NH3: %.2f | H2S: %.2f | Batt: %.2f V\n",
                          txData.node_code, txData.uptime_seconds, txData.water_level_cm,
                          txData.ammonia_ppm, txData.h2s_ppm, txData.battery_voltage);
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

            radio.standby(); // Kembali ke standby mode hemat daya
            Serial.println(F("========================================================================\n"));
        }
    }
}

// =========================================================================================
// 6. SETUP SISTEM UTAMA
// =========================================================================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Waktu stabilisasi Serial UART pada boot

    Serial.println();
    Serial.println(F("========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — Node WC Firmware Integrasi [v2.0]              "));
    Serial.println(F(" Subsystem: MQ-137, MQ-136, JSN-SR04T, & Transmisi LoRa SX1278          "));
    Serial.println(F(" Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           "));
    Serial.println(F("========================================================================"));

    // 1. Inisialisasi Antrean FreeRTOS
    xQueueSensorData = xQueueCreate(4, sizeof(TelemetryPayload));
    if (xQueueSensorData == NULL) {
        Serial.println(F("[FATAL] Gagal membuat antrean FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 2. Pre-inisialisasi NSS ke HIGH agar SX1278 tidak merespons noise boot ESP32
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    // 3. Hardware Reset SX1278 (LOW 20 ms, Settling time 100 ms)
    Serial.print(F("1. Melakukan Hardware Reset SX1278 (RST:15)... "));
    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100);
    Serial.println(F("[OK]"));

    // 4. Inisialisasi Bus Hardware SPI
    Serial.printf("2. Inisialisasi Hardware SPI (SCK:%d, MISO:%d, MOSI:%d, NSS:%d)... ",
                  PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);
    Serial.println(F("[OK]"));

    // 5. Inisialisasi Register Radio SX1278 dengan Retry Loop
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

    // 6. Pembuatan Task FreeRTOS dengan Verifikasi
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

    BaseType_t retLoRa = xTaskCreatePinnedToCore(
        vTaskLoRaTx,
        "TaskLoRaTx",
        4096,
        NULL,
        3,
        &xHandleTaskLoRaTx,
        1 // Core 1 (APP_CPU)
    );

    if (retSensors == pdPASS && retLoRa == pdPASS) {
        Serial.println(F("[BOOT] Seluruh Task FreeRTOS berhasil diluncurkan!"));
        Serial.println(F("  - TaskSensors: Core 0, Prioritas 1 (MQ-137, MQ-136, JSN-SR04T)"));
        Serial.println(F("  - TaskLoRaTx : Core 1, Prioritas 3 (Transmisi LoRa SX1278)"));
    } else {
        Serial.println(F("[FATAL] Gagal membuat Task FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Serial.println(F("========================================================================\n"));
}

// =========================================================================================
// 7. ARDUINO LOOP (PENGHAPUSAN TASK LOOP UNTUK REKLAMASI MEMORI)
// =========================================================================================
void loop() {
    // Menghapus loopTask pada Core 1 agar memori 8KB stack direklamasi oleh Idle Task
    vTaskDelete(NULL);
}

#endif // IS_NODE_WC
