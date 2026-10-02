/**
 * =========================================================================================
 * Smart-Sanitation eSOS — ESP32 Dual Gas Sensor Standalone Test & Diagnostics
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
 * Target Board : DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa LX6)
 * Sensors      : Winsen MQ-137 (Ammonia / NH3) & Winsen MQ-136 (Hydrogen Sulfide / H2S)
 * Manual Ver.  : Winsen Manual v1.6 (Valid from: 2021-07-01)
 * Framework    : Arduino-ESP32 (v2.0.17 / ESP-IDF v4.4) + Native Espressif FreeRTOS
 * Author       : AI Engineering Agent (Antigravity) on behalf of Kelompok 4 FTUI
 * Date         : September 2026 (Refactored v2.2)
 *
 * CRITICAL ARCHITECTURAL & ELECTRICAL GOVERNANCE:
 * -----------------------------------------------------------------------------------------
 * 1. ESP-IDF FREERTOS CONVENTIONS:
 *    - In ESP-IDF FreeRTOS, task stack depths in xTaskCreatePinnedToCore() are specified in
 *      BYTES (not words). Allocated: 4096 BYTES for Sampler and Logger tasks.
 *    - uxTaskGetStackHighWaterMark() returns remaining stack in BYTES. No '* 4' multiplier!
 *
 * 2. WINSEN DATASHEET v1.6 BASELINE DEFINITION:
 *    - Per Winsen Manual v1.6 (Fig 3), R0 is explicitly defined as "resistance of sensor
 *      in clean air". Therefore, clean-air baseline calibration sets R0 = Rs_clean_air_mean.
 *      Arbitrary scale factors (like 3.6) are strictly prohibited.
 *
 * 3. ZERO-TRUST POLICY ON GAS PPM ESTIMATION:
 *    - Gas concentration conversion (ppm) is STRICTLY DISABLED in this diagnostic harness.
 *      Arbitrary curve-fit power-law coefficients are not accepted without empirical multi-point
 *      chamber regression data.
 *    - Measurable physical parameters reported:
 *      * ADC_mean: Arithmetic mean of 16 oversampled ADC readings in the current frame.
 *      * ADC_ema : Filtered Exponential Moving Average across consecutive frames.
 *      * ADC_min / ADC_max / ADC_stddev: Burst statistics for noise assessment.
 *      * V_pin   : Calibrated voltage at ESP32 pin (mV).
 *      * V_AO    : Reconstructed sensor output voltage (V_pin / k) in mV.
 *      * Rs      : Sensor resistance in Ohms (accounting for parallel divider loading).
 *      * Rs/R0   : Ratio relative to clean-air baseline.
 *
 * 4. ELECTRICAL SAFETY & UNIFIED RESISTOR DIVIDER VALIDATION:
 *    - Breakout Analog Output (AO) can reach 5.0V. ESP32 continuous safe pin limit is 3.3V.
 *    - Resistor divider (R_TOP, R_BOTTOM) is mandatory. Default template: 10k / 15k (k = 0.60).
 *    - Proposed divider is marked UNCONFIRMED at boot until explicitly set or loaded from valid NVS.
 *    - Unified validator validateDivider() enforces:
 *      * Physical range: 100 Ohm <= R <= 10 MOhm.
 *      * Safe ratio range: 0.1000 <= k <= 0.6600.
 *        Max k=0.6600 guarantees V_pin <= 3.3V when breakout AO reaches 5.0V.
 *    - Parallel loading effect: RL_eff = (RL * (R_TOP + R_BOTTOM)) / (RL + R_TOP + R_BOTTOM).
 *    - Any change to divider or RL immediately INVALIDATES existing R0 and cancels active calibration.
 *
 * 5. HONEST PERSISTENCE & NVS INTEGRITY:
 *    - TaskMQSampler is the SOLE OWNER of configuration, NVS flash operations, and ADC sampling.
 *    - Configuration persisted as an atomic versioned binary blob (MQNVSRecord, schema v2)
 *      with 32-bit CRC checksum. All return values verified.
 *    - Legacy v1 keys are detected and safely ignored (never silently erased without user command).
 *    - Explicit validity flags persisted per-channel; stale R0 resurrection is impossible.
 *
 * 6. STRICT SEPARATION OF RL, RS, AND RS/R0 VALIDITY:
 *    - MQ_FLAG_RL_OK indicates valid RL configuration.
 *    - MQ_FLAG_RS_VALID indicates successful, physically bounded Rs calculation.
 *    - Voltage bounds enforced: 50 mV <= V_AO <= 4950 mV. Out-of-bounds marked SIGNAL_INVALID.
 *    - Output prints explicit 'N/A [REASON]' instead of sentinel numeric values.
 * =========================================================================================
 */

#include <Arduino.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <math.h>
#include <stdarg.h>

// =========================================================================================
// 1. PIN DEFINITIONS & PHYSICAL CONSTANTS
// =========================================================================================
#define PIN_ADC_MQ137           32      // GPIO32 = ADC1_CH4 (Ammonia / NH3)
#define PIN_ADC_MQ136           33      // GPIO33 = ADC1_CH5 (Hydrogen Sulfide / H2S)

// Operating voltages (nominal)
#define V_LOOP_SUPPLY_VOLTS     5.00f   // Vc = 5.0V +/- 0.1V DC per Winsen datasheet

// FreeRTOS Task Configuration (ESP-IDF uses BYTES for stack depth)
#define SAMPLER_TASK_CORE       0       // Pinned to Core 0 (PRO_CPU)
#define SAMPLER_TASK_PRIORITY   2       // Priority 2 (Higher than logger)
#define SAMPLER_STACK_BYTES     4096    // 4096 bytes stack depth

#define LOGGER_TASK_CORE        0       // Pinned to Core 0 (PRO_CPU)
#define LOGGER_TASK_PRIORITY    1       // Priority 1
#define LOGGER_STACK_BYTES      4096    // 4096 bytes stack depth

#define FRAME_PERIOD_MS         1000    // Sampling frame every 1000 ms (1 Hz)
#define SAMPLES_PER_FRAME       16      // 16 oversampled readings per burst
#define INTER_SAMPLE_DELAY_MS   10      // 10 ms delay between successive ADC bursts

// Digital Filter Constant
#define EMA_ALPHA               0.25f   // Exponential Moving Average smoothing factor

// NVS Persistent Storage Configuration (Schema v2)
#define NVS_NAMESPACE           "mq_cal"
#define NVS_BLOB_KEY            "mq_rec"
#define NVS_MAGIC_HEADER        0x4D513032  // ASCII 'MQ02'
#define NVS_VERSION             2

// Special value when a parameter cannot be computed
#define MQ_VALUE_UNCONFIGURED   (-1.0f)

// Status Bitflags
#define MQ_FLAG_RAW_OK          (1 << 0)    // ADC sampled successfully
#define MQ_FLAG_DIVIDER_OK      (1 << 1)    // Voltage divider ratio confirmed by user/NVS
#define MQ_FLAG_RL_OK           (1 << 2)    // Load resistor RL configured
#define MQ_FLAG_R0_CALIBRATED   (1 << 3)    // Clean air baseline R0 calibrated
#define MQ_FLAG_ADC_SATURATED   (1 << 4)    // ADC clipped at raw >= 4095
#define MQ_FLAG_PREHEAT_WARN    (1 << 5)    // Uptime < 48 hours notice
#define MQ_FLAG_SIGNAL_INVALID  (1 << 6)    // Out of bounds (<50mV or >= Vc-50mV), open circuit, or saturated
#define MQ_FLAG_RS_VALID        (1 << 7)    // Rs calculated successfully and physically valid

// Calibration Requirements (Must have valid Rs calculation)
#define CALIBRATION_REQUIRED_FLAGS  ((uint8_t)(MQ_FLAG_DIVIDER_OK | MQ_FLAG_RL_OK | MQ_FLAG_RS_VALID))

// =========================================================================================
// 2. DATA STRUCTURES & PROTOCOLS
// =========================================================================================

// Sensor Channel Hardware & Calibration Parameters
struct MQChannelConfig {
    const char* gas_name;           // e.g. "NH3 (MQ137)"
    uint8_t pin;                    // GPIO Pin number
    float r_top_ohm;                // Resistor from AO to ESP32 ADC pin
    float r_bottom_ohm;             // Resistor from ESP32 ADC pin to GND
    float divider_k;                // k = R_bottom / (R_top + R_bottom)
    float rl_nominal_ohm;           // Breakout module load resistor (0 = unconfigured)
    float r0_clean_air_ohm;         // Baseline sensor resistance in clean air (<= 0 = uncalibrated)
    bool is_divider_valid;          // false until explicitly set or loaded from valid NVS
    bool is_rl_valid;
    bool is_r0_valid;
};

// Immutable Snapshot of Channel Configuration for Thread-Safe Reporting
struct MQChannelConfigSnapshot {
    char gas_name[16];
    uint8_t pin;
    float r_top_ohm;
    float r_bottom_ohm;
    float divider_k;
    float rl_nominal_ohm;
    float r0_clean_air_ohm;
    bool is_divider_valid;
    bool is_rl_valid;
    bool is_r0_valid;
};

// Snapshot Measurement for a single sensor channel
struct MQChannelReading {
    uint16_t adc_raw_mean;          // Arithmetic mean of 16 raw ADC burst samples (frame saat ini)
    uint16_t adc_raw_ema;           // Filtered Exponential Moving Average across frames
    uint16_t adc_raw_min;           // Minimum sample in burst
    uint16_t adc_raw_max;           // Maximum sample in burst
    float adc_raw_stddev;           // Standard deviation in burst
    float v_adc_mv;                 // Voltage at ESP32 pin (millivolts, via factory eFuse cal)
    float v_ao_mv;                  // Reconstructed AO voltage (V_ADC / k) in millivolts
    float rs_ohm;                   // Calculated sensor resistance in Ohms
    float ratio_rs_r0;              // Rs / R0 ratio (only if R0 calibrated)
    uint8_t status_flags;           // Bitmask of MQ_FLAG_*
};

// Complete Dual-Sensor Telemetry Snapshot Frame (Fixed Size, Memory Safe)
struct MQDataFrame {
    uint32_t frame_id;
    uint64_t timestamp_ms;
    uint32_t dropped_frames_count;
    MQChannelReading mq137;
    MQChannelReading mq136;
    MQChannelConfigSnapshot cfg137_snap;
    MQChannelConfigSnapshot cfg136_snap;
    uint32_t sampler_stack_watermark_bytes;
    uint32_t logger_stack_watermark_bytes;
};

// Serial Command IPC message
enum CommandType {
    CMD_NONE = 0,
    CMD_CALIBRATE_MQ137,
    CMD_CALIBRATE_MQ136,
    CMD_SET_DIVIDER_MQ137,
    CMD_SET_DIVIDER_MQ136,
    CMD_SET_RL_MQ137,
    CMD_SET_RL_MQ136,
    CMD_SAVE_NVS,
    CMD_RESET_CALIBRATION
};

struct MQCommand {
    CommandType type;
    float param1;                   // e.g. R_TOP or RL
    float param2;                   // e.g. R_BOTTOM
};

// Asynchronous Log Message from Sampler to Logger (Sole Writer Principle)
struct LogMessage {
    char text[128];
};

// Calibration State Machine (accumulates 10 stable frames in clean air)
struct CalibrationState {
    bool active;
    uint8_t target_sensor;          // 137 or 136
    uint8_t frame_count;
    float rs_accumulator;
    float last_rs_readings[10];
};

// Packed Binary Record for Atomic NVS Storage (Schema v2)
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
    uint32_t magic;                 // 0x4D513032 ('MQ02')
    uint16_t schema_version;        // 2
    uint16_t reserved;
    MQChannelRecord ch137;
    MQChannelRecord ch136;
    uint32_t crc32;                 // CRC32 of preceding fields
};

// =========================================================================================
// 3. GLOBAL VARIABLES & RTOS HANDLES
// =========================================================================================
static QueueHandle_t xQueueMQFrames = NULL;
static QueueHandle_t xQueueMQCommands = NULL;
static QueueHandle_t xQueueLogMessages = NULL;

static TaskHandle_t xHandleMQSampler = NULL;
static TaskHandle_t xHandleMQLogger = NULL;

// Channel configurations owned EXCLUSIVELY by TaskMQSampler
// Note: Default proposed divider is marked UNCONFIRMED (is_divider_valid = false)
static MQChannelConfig config137 = {
    .gas_name = "NH3 (MQ137)",
    .pin = PIN_ADC_MQ137,
    .r_top_ohm = 10000.0f,          // 10 kOhm proposed default template
    .r_bottom_ohm = 15000.0f,       // 15 kOhm proposed default template
    .divider_k = 0.6000f,           // 15 / (10 + 15) = 0.6000
    .rl_nominal_ohm = 0.0f,         // 0 = unconfigured
    .r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED,
    .is_divider_valid = false,      // Requires explicit confirmation or NVS load
    .is_rl_valid = false,
    .is_r0_valid = false
};

static MQChannelConfig config136 = {
    .gas_name = "H2S (MQ136)",
    .pin = PIN_ADC_MQ136,
    .r_top_ohm = 10000.0f,          // 10 kOhm proposed default template
    .r_bottom_ohm = 15000.0f,       // 15 kOhm proposed default template
    .divider_k = 0.6000f,           // 15 / (10 + 15) = 0.6000
    .rl_nominal_ohm = 0.0f,         // 0 = unconfigured
    .r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED,
    .is_divider_valid = false,      // Requires explicit confirmation or NVS load
    .is_rl_valid = false,
    .is_r0_valid = false
};

static CalibrationState calState = {
    .active = false,
    .target_sensor = 0,
    .frame_count = 0,
    .rs_accumulator = 0.0f,
    .last_rs_readings = {0}
};

static uint32_t g_dropped_frames = 0;

// Cached Snapshot accessible to Logger CLI for instant 'config' command
static MQDataFrame s_last_valid_frame;
static bool s_has_received_frame = false;

// =========================================================================================
// 4. FORWARD DECLARATIONS & VALIDATION HELPERS
// =========================================================================================
static void TaskMQSampler(void* pvParameters);
static void TaskMQLogger(void* pvParameters);

static void samplerLog(const char* fmt, ...);
static void loadConfigurationFromNVS();
static void saveConfigurationToNVS();
static void resetCalibrationInNVS();

static bool validateDivider(float r_top, float r_bottom, float* out_k, char* err_buf, size_t err_len);
static bool validateRL(float rl, char* err_buf, size_t err_len);
static bool validateR0(float r0, char* err_buf, size_t err_len);
static uint32_t calculateCRC32(const uint8_t* data, size_t length);

static void processChannelSampling(MQChannelConfig* cfg, MQChannelReading* reading, float* ema_val);
static void handleIncomingCommand(const MQCommand* cmd);
static void parseSerialInput(const char* line);
static void printCommandHelp();
static void printActiveConfig(const MQChannelConfigSnapshot* c137, const MQChannelConfigSnapshot* c136);
static void populateConfigSnapshot(MQChannelConfigSnapshot* snap, const MQChannelConfig* cfg);

/**
 * Standard IEEE 802.3 CRC32 Calculation
 */
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

/**
 * Unified Voltage Divider Validation
 *
 * Enforces:
 *  1. Physical resistor range: 100 Ohm <= R <= 10 MOhm.
 *  2. Safe electrical ratio: 0.1000 <= k <= 0.6600.
 *     k_max = 0.6600 guarantees that a 5.0V breakout AO swing produces V_pin <= 3.3V,
 *     protecting the ESP32 GPIO from overvoltage damage.
 */
static bool validateDivider(float r_top, float r_bottom, float* out_k, char* err_buf, size_t err_len) {
    if (!isfinite(r_top) || !isfinite(r_bottom) ||
        r_top < 100.0f || r_top > 10000000.0f ||
        r_bottom < 100.0f || r_bottom > 10000000.0f) {
        if (err_buf && err_len > 0) {
            snprintf(err_buf, err_len, "Resistors must be finite and within 100 Ohm to 10 MOhm");
        }
        return false;
    }
    float sum = r_top + r_bottom;
    if (sum <= 0.0f) {
        if (err_buf && err_len > 0) {
            snprintf(err_buf, err_len, "Resistor sum must be positive");
        }
        return false;
    }
    float k = r_bottom / sum;
    if (k < 0.1000f || k > 0.6600f) {
        if (err_buf && err_len > 0) {
            snprintf(err_buf, err_len, "Divider ratio k=%.4f out of safe bounds [0.1000, 0.6600]. Max k=0.66 ensures V_pin <= 3.3V at 5.0V AO", k);
        }
        return false;
    }
    if (out_k != NULL) {
        *out_k = k;
    }
    return true;
}

/**
 * Unified Load Resistor (RL) Validation
 *
 * Enforces: 100 Ohm <= RL <= 1 MOhm.
 */
static bool validateRL(float rl, char* err_buf, size_t err_len) {
    if (!isfinite(rl) || rl < 100.0f || rl > 1000000.0f) {
        if (err_buf && err_len > 0) {
            snprintf(err_buf, err_len, "RL must be finite and within 100 Ohm to 1 MOhm");
        }
        return false;
    }
    return true;
}

/**
 * Unified Baseline Resistance (R0) Validation
 *
 * Enforces: 100 Ohm <= R0 <= 10 MOhm.
 */
static bool validateR0(float r0, char* err_buf, size_t err_len) {
    if (!isfinite(r0) || r0 < 100.0f || r0 > 10000000.0f) {
        if (err_buf && err_len > 0) {
            snprintf(err_buf, err_len, "R0 must be finite and within 100 Ohm to 10 MOhm");
        }
        return false;
    }
    return true;
}

// =========================================================================================
// 5. ARDUINO SETUP & SYSTEM INITIALIZATION
// =========================================================================================
void setup() {
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Allow Serial monitor to connect
    
    Serial.println();
    Serial.println(F("========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — ESP32 Dual Gas Sensor Diagnostics [v2.2]       "));
    Serial.println(F(" Subsystem: Winsen MQ-137 (NH3) & Winsen MQ-136 (H2S) Test Harness      "));
    Serial.println(F(" Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           "));
    Serial.println(F("========================================================================"));
    Serial.println(F("[BOOT] Initializing ADC1 conditioning..."));

    // 1. Hardware ADC Configuration (ADC1 Only)
    analogReadResolution(12); // 12-bit: 0 - 4095
    analogSetPinAttenuation(PIN_ADC_MQ137, ADC_11db);
    analogSetPinAttenuation(PIN_ADC_MQ136, ADC_11db);

    // 2. Create FreeRTOS Queues
    xQueueMQFrames = xQueueCreate(4, sizeof(MQDataFrame));
    xQueueMQCommands = xQueueCreate(4, sizeof(MQCommand));
    xQueueLogMessages = xQueueCreate(8, sizeof(LogMessage));

    if (xQueueMQFrames == NULL || xQueueMQCommands == NULL || xQueueLogMessages == NULL) {
        Serial.println(F("[FATAL] Queue creation failed! Insufficient heap. Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 3. Spawn Tasks with Byte-Specified Stack Depth and Explicit Core Affinity
    BaseType_t retSampler = xTaskCreatePinnedToCore(
        TaskMQSampler,
        "TaskMQSampler",
        SAMPLER_STACK_BYTES,
        NULL,
        SAMPLER_TASK_PRIORITY,
        &xHandleMQSampler,
        SAMPLER_TASK_CORE
    );

    BaseType_t retLogger = xTaskCreatePinnedToCore(
        TaskMQLogger,
        "TaskMQLogger",
        LOGGER_STACK_BYTES,
        NULL,
        LOGGER_TASK_PRIORITY,
        &xHandleMQLogger,
        LOGGER_TASK_CORE
    );

    if (retSampler != pdPASS || retLogger != pdPASS) {
        Serial.println(F("[FATAL] FreeRTOS Task creation failed! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Serial.println(F("[BOOT] System initialized. TaskMQSampler & TaskMQLogger active on Core 0."));
    Serial.println(F("[BOOT] Type 'help' for interactive serial commands."));
    Serial.println(F("========================================================================"));
}

// =========================================================================================
// 6. ARDUINO LOOP (YIELD TO SCHEDULER)
// =========================================================================================
void loop() {
    // In Arduino-ESP32, loop() runs inside loopTask on Core 1 by default.
    // Core 1 is reserved for future LoRa radio tasks.
    // Strictly delay loop() to yield CPU cycles.
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// =========================================================================================
// 7. TASK: MQ SAMPLER (CORE 0, PRIORITY 2) — SOLE RESOURCE & NVS OWNER
// =========================================================================================
static void TaskMQSampler(void* pvParameters) {
    (void)pvParameters;

    // Load persisted parameters from NVS strictly inside Sampler task context
    loadConfigurationFromNVS();

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(FRAME_PERIOD_MS);

    uint32_t frame_seq = 0;
    float ema_mq137 = -1.0f;
    float ema_mq136 = -1.0f;

    MQDataFrame frame;

    for (;;) {
        // Precise periodic timing using vTaskDelayUntil (no cumulative drift)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        frame_seq++;

        // 1. Process any incoming commands from Logger CLI
        MQCommand cmd;
        if (xQueueReceive(xQueueMQCommands, &cmd, 0) == pdTRUE) {
            handleIncomingCommand(&cmd);
        }

        // 2. Populate Frame Metadata
        frame.frame_id = frame_seq;
        frame.timestamp_ms = (uint64_t)(esp_timer_get_time() / 1000ULL);
        frame.dropped_frames_count = g_dropped_frames;

        // 3. Sample Sensor 1: MQ137 (Ammonia)
        processChannelSampling(&config137, &frame.mq137, &ema_mq137);

        // 4. Sample Sensor 2: MQ136 (Hydrogen Sulfide)
        processChannelSampling(&config136, &frame.mq136, &ema_mq136);

        // 5. Handle Active Clean-Air Baseline Calibration State Machine
        if (calState.active) {
            MQChannelReading* targetReading = (calState.target_sensor == 137) ? &frame.mq137 : &frame.mq136;
            MQChannelConfig* targetCfg = (calState.target_sensor == 137) ? &config137 : &config136;

            // Strictly require valid Rs calculation before accumulating sample
            if (targetReading->status_flags & MQ_FLAG_RS_VALID) {
                calState.last_rs_readings[calState.frame_count] = targetReading->rs_ohm;
                calState.rs_accumulator += targetReading->rs_ohm;
                calState.frame_count++;

                samplerLog("[CAL] %s: Step %u/10, Current Rs = %.1f Ohm",
                           targetCfg->gas_name, calState.frame_count, targetReading->rs_ohm);

                if (calState.frame_count >= 10) {
                    float rs_mean = calState.rs_accumulator / 10.0f;
                    float sum_sq_diff = 0.0f;
                    for (int i = 0; i < 10; i++) {
                        float diff = calState.last_rs_readings[i] - rs_mean;
                        sum_sq_diff += diff * diff;
                    }
                    float rs_stddev = sqrtf(sum_sq_diff / 10.0f);
                    float cv_percent = (rs_stddev / rs_mean) * 100.0f;

                    // Stability threshold: Coefficient of variation must be < 5.0%
                    if (cv_percent < 5.0f) {
                        char r0_err[80] = {0};
                        if (validateR0(rs_mean, r0_err, sizeof(r0_err))) {
                            // Per Winsen Manual v1.6: R0 is explicitly defined as Rs in clean air
                            targetCfg->r0_clean_air_ohm = rs_mean;
                            targetCfg->is_r0_valid = true;


                            samplerLog(">>> [CAL SUCCESS] %s Clean-Air Baseline Established!", targetCfg->gas_name);
                            samplerLog("    Rs(clean air mean) = %.1f Ohm, StdDev = %.1f Ohm (CV: %.2f%%)",
                                       rs_mean, rs_stddev, cv_percent);
                            samplerLog("    Established Baseline R0 = %.1f Ohm", targetCfg->r0_clean_air_ohm);
                            samplerLog("    Type 'save' to commit baseline to NVS flash.");
                        } else {
                            samplerLog(">>> [CAL REJECTED] %s Mean Rs out of range [100 Ohm - 10 MOhm]: %.1f Ohm",
                                       targetCfg->gas_name, rs_mean);
                        }
                    } else {
                        samplerLog(">>> [CAL REJECTED] %s Readings too unstable! CV = %.2f%% (> 5.0%% limit).",
                                   targetCfg->gas_name, cv_percent);
                        samplerLog("    Ensure sensor is preheated and placed in still, unpolluted air.");
                    }
                    calState.active = false;
                }
            } else {
                samplerLog("[CAL ABORTED] %s: Circuit unconfirmed, ADC saturated, or Rs invalid!",
                           targetCfg->gas_name);
                calState.active = false;
            }
        }

        // 6. Capture Stack High Water Mark (ESP-IDF FreeRTOS returns BYTES)
        frame.sampler_stack_watermark_bytes = (uint32_t)uxTaskGetStackHighWaterMark(NULL);
        frame.logger_stack_watermark_bytes = (xHandleMQLogger != NULL) ? 
            (uint32_t)uxTaskGetStackHighWaterMark(xHandleMQLogger) : 0;

        // 7. Attach Configuration Snapshots
        populateConfigSnapshot(&frame.cfg137_snap, &config137);
        populateConfigSnapshot(&frame.cfg136_snap, &config136);

        // 8. Enqueue Snapshot Frame to Logger Task (Drop oldest if queue full to prevent stall)
        if (xQueueSend(xQueueMQFrames, &frame, 0) != pdTRUE) {
            MQDataFrame dummy;
            xQueueReceive(xQueueMQFrames, &dummy, 0); // Drop stale frame
            g_dropped_frames++;
            frame.dropped_frames_count = g_dropped_frames;
            xQueueSend(xQueueMQFrames, &frame, 0);
        }
    }
}

// =========================================================================================
// 8. SENSOR SAMPLING & ELECTRICAL DSP PIPELINE
// =========================================================================================
static void processChannelSampling(MQChannelConfig* cfg, MQChannelReading* reading, float* ema_val) {
    uint32_t raw_sum = 0;
    uint32_t mv_sum = 0;
    uint16_t sample_min = 4095;
    uint16_t sample_max = 0;
    uint16_t samples[SAMPLES_PER_FRAME];

    reading->status_flags = 0;

    // Check preheat warning (< 48 hours notice per Winsen v1.6)
    uint64_t uptime_sec = esp_timer_get_time() / 1000000ULL;
    if (uptime_sec < (48ULL * 3600ULL)) {
        reading->status_flags |= MQ_FLAG_PREHEAT_WARN;
    }

    // Burst oversampling: 16 samples separated by 10 ms
    for (int i = 0; i < SAMPLES_PER_FRAME; i++) {
        uint16_t raw = (uint16_t)analogRead(cfg->pin);
        uint32_t mv = (uint32_t)analogReadMilliVolts(cfg->pin); // Factory calibrated mV

        samples[i] = raw;
        raw_sum += raw;
        mv_sum += mv;

        if (raw < sample_min) sample_min = raw;
        if (raw > sample_max) sample_max = raw;

        vTaskDelay(pdMS_TO_TICKS(INTER_SAMPLE_DELAY_MS));
    }

    // 1. Raw Statistics & Explicit Mean vs EMA Separation
    float raw_mean = (float)raw_sum / (float)SAMPLES_PER_FRAME;
    reading->adc_raw_mean = (uint16_t)roundf(raw_mean);
    reading->adc_raw_min = sample_min;
    reading->adc_raw_max = sample_max;
    reading->v_adc_mv = (float)mv_sum / (float)SAMPLES_PER_FRAME;

    // Check for ADC Saturation (Clamped at 4095)
    if (sample_max >= 4095) {
        reading->status_flags |= (MQ_FLAG_ADC_SATURATED | MQ_FLAG_SIGNAL_INVALID);
    }

    // Standard deviation of raw samples in burst
    float sq_diff_sum = 0.0f;
    for (int i = 0; i < SAMPLES_PER_FRAME; i++) {
        float diff = (float)samples[i] - raw_mean;
        sq_diff_sum += diff * diff;
    }
    reading->adc_raw_stddev = sqrtf(sq_diff_sum / (float)SAMPLES_PER_FRAME);

    // 2. Exponential Moving Average (EMA) Smoothing Across Frames
    if (*ema_val < 0.0f) {
        *ema_val = raw_mean; // Initialize on first frame
    } else {
        *ema_val = (EMA_ALPHA * raw_mean) + ((1.0f - EMA_ALPHA) * (*ema_val));
    }
    reading->adc_raw_ema = (uint16_t)roundf(*ema_val);
    reading->status_flags |= MQ_FLAG_RAW_OK;

    // 3. Voltage Reconstruction (V_AO = V_ADC / k)
    float k = 0.0f;
    if (cfg->is_divider_valid && validateDivider(cfg->r_top_ohm, cfg->r_bottom_ohm, &k, NULL, 0)) {
        reading->status_flags |= MQ_FLAG_DIVIDER_OK;
        reading->v_ao_mv = reading->v_adc_mv / k;
    } else {
        reading->v_ao_mv = MQ_VALUE_UNCONFIGURED;
    }

    // 4. Physical Signal Bounds Checking:
    // If divider confirmed, check if reconstructed V_AO is physically plausible:
    // Open circuit / disconnected: V_AO < 50 mV
    // Short circuit / saturated near rail: V_AO >= (Vc - 50 mV)
    if (reading->status_flags & MQ_FLAG_DIVIDER_OK) {
        if (reading->v_ao_mv < 50.0f || reading->v_ao_mv >= (V_LOOP_SUPPLY_VOLTS * 1000.0f - 50.0f)) {
            reading->status_flags |= MQ_FLAG_SIGNAL_INVALID;
        }
    }

    // 5. Validate RL Configuration
    if (cfg->is_rl_valid && validateRL(cfg->rl_nominal_ohm, NULL, 0)) {
        reading->status_flags |= MQ_FLAG_RL_OK;
    }

    // 6. Sensor Resistance Calculation (Rs)
    // Formula: Rs = ((Vc / V_AO) - 1) * RL_eff
    // Parallel divider loading: RL_eff = (RL * (R_top + R_bottom)) / (RL + R_top + R_bottom)
    const uint8_t req_circuit = MQ_FLAG_DIVIDER_OK | MQ_FLAG_RL_OK;
    if (((reading->status_flags & req_circuit) == req_circuit) &&
        !(reading->status_flags & (MQ_FLAG_SIGNAL_INVALID | MQ_FLAG_ADC_SATURATED)) &&
        reading->v_ao_mv >= 50.0f) {

        float v_ao_volts = reading->v_ao_mv / 1000.0f;
        float r_divider_total = cfg->r_top_ohm + cfg->r_bottom_ohm;
        float rl_eff = (cfg->rl_nominal_ohm * r_divider_total) / (cfg->rl_nominal_ohm + r_divider_total);

        float calculated_rs = ((V_LOOP_SUPPLY_VOLTS / v_ao_volts) - 1.0f) * rl_eff;
        if (isfinite(calculated_rs) && calculated_rs > 0.0f) {
            reading->rs_ohm = calculated_rs;
            reading->status_flags |= MQ_FLAG_RS_VALID;
        } else {
            reading->rs_ohm = MQ_VALUE_UNCONFIGURED;
            reading->status_flags |= MQ_FLAG_SIGNAL_INVALID;
        }
    } else {
        reading->rs_ohm = MQ_VALUE_UNCONFIGURED;
    }

    // 7. Ratio Rs / R0 (Clean Air Baseline Ratio)
    // Strictly requires MQ_FLAG_RS_VALID and valid R0
    if ((reading->status_flags & MQ_FLAG_RS_VALID) && cfg->is_r0_valid &&
        cfg->r0_clean_air_ohm > 0.0f && !(reading->status_flags & MQ_FLAG_SIGNAL_INVALID)) {
        reading->status_flags |= MQ_FLAG_R0_CALIBRATED;
        reading->ratio_rs_r0 = reading->rs_ohm / cfg->r0_clean_air_ohm;
    } else {
        reading->ratio_rs_r0 = MQ_VALUE_UNCONFIGURED;
    }
}

// =========================================================================================
// 9. ASYNCHRONOUS SAMPLER LOGGING (ROUTED TO LOGGER TASK)
// =========================================================================================
static void samplerLog(const char* fmt, ...) {
    LogMessage msg;
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.text, sizeof(msg.text), fmt, args);
    va_end(args);

    if (xQueueLogMessages != NULL) {
        xQueueSend(xQueueLogMessages, &msg, 0);
    }
}

// =========================================================================================
// 10. SAMPLER-SIDE COMMAND EXECUTION (THREAD-SAFE MUTATIONS & NVS)
// =========================================================================================
static void handleIncomingCommand(const MQCommand* cmd) {
    switch (cmd->type) {
        case CMD_CALIBRATE_MQ137:
            if (!config137.is_divider_valid || !config137.is_rl_valid) {
                samplerLog("[CAL ABORTED] Configure divider and RL first: setdiv137 & setrl137");
            } else {
                calState.active = true;
                calState.target_sensor = 137;
                calState.frame_count = 0;
                calState.rs_accumulator = 0.0f;
                samplerLog("[CAL] Starting 10-second clean-air baseline calibration for MQ137...");
            }
            break;

        case CMD_CALIBRATE_MQ136:
            if (!config136.is_divider_valid || !config136.is_rl_valid) {
                samplerLog("[CAL ABORTED] Configure divider and RL first: setdiv136 & setrl136");
            } else {
                calState.active = true;
                calState.target_sensor = 136;
                calState.frame_count = 0;
                calState.rs_accumulator = 0.0f;
                samplerLog("[CAL] Starting 10-second clean-air baseline calibration for MQ136...");
            }
            break;

        case CMD_SET_DIVIDER_MQ137: {
            float k = 0.0f;
            if (validateDivider(cmd->param1, cmd->param2, &k, NULL, 0)) {
                config137.r_top_ohm = cmd->param1;
                config137.r_bottom_ohm = cmd->param2;
                config137.divider_k = k;
                config137.is_divider_valid = true;
                // Circuit modified: Invalidate old R0 calibration and cancel any running calibration
                config137.is_r0_valid = false;
                config137.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
                if (calState.active && calState.target_sensor == 137) calState.active = false;
                samplerLog("[CONFIG] MQ137 Divider Confirmed: R_top=%.0f Ohm, R_bot=%.0f Ohm -> k = %.4f",
                           config137.r_top_ohm, config137.r_bottom_ohm, config137.divider_k);
                samplerLog("[WARNING] MQ137 baseline R0 invalidated due to circuit change. Recalibration required.");
            } else {
                samplerLog("[ERROR] MQ137 divider parameters rejected by validator.");
            }
            break;
        }

        case CMD_SET_DIVIDER_MQ136: {
            float k = 0.0f;
            if (validateDivider(cmd->param1, cmd->param2, &k, NULL, 0)) {
                config136.r_top_ohm = cmd->param1;
                config136.r_bottom_ohm = cmd->param2;
                config136.divider_k = k;
                config136.is_divider_valid = true;
                // Circuit modified: Invalidate old R0 calibration and cancel any running calibration
                config136.is_r0_valid = false;
                config136.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
                if (calState.active && calState.target_sensor == 136) calState.active = false;
                samplerLog("[CONFIG] MQ136 Divider Confirmed: R_top=%.0f Ohm, R_bot=%.0f Ohm -> k = %.4f",
                           config136.r_top_ohm, config136.r_bottom_ohm, config136.divider_k);
                samplerLog("[WARNING] MQ136 baseline R0 invalidated due to circuit change. Recalibration required.");
            } else {
                samplerLog("[ERROR] MQ136 divider parameters rejected by validator.");
            }
            break;
        }

        case CMD_SET_RL_MQ137: {
            if (validateRL(cmd->param1, NULL, 0)) {
                config137.rl_nominal_ohm = cmd->param1;
                config137.is_rl_valid = true;
                // Circuit modified: Invalidate old R0 calibration and cancel any running calibration
                config137.is_r0_valid = false;
                config137.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
                if (calState.active && calState.target_sensor == 137) calState.active = false;
                samplerLog("[CONFIG] MQ137 RL set to %.0f Ohm.", config137.rl_nominal_ohm);
                samplerLog("[WARNING] MQ137 baseline R0 invalidated due to circuit change. Recalibration required.");
            } else {
                samplerLog("[ERROR] MQ137 RL parameter rejected by validator.");
            }
            break;
        }

        case CMD_SET_RL_MQ136: {
            if (validateRL(cmd->param1, NULL, 0)) {
                config136.rl_nominal_ohm = cmd->param1;
                config136.is_rl_valid = true;
                // Circuit modified: Invalidate old R0 calibration and cancel any running calibration
                config136.is_r0_valid = false;
                config136.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
                if (calState.active && calState.target_sensor == 136) calState.active = false;
                samplerLog("[CONFIG] MQ136 RL set to %.0f Ohm.", config136.rl_nominal_ohm);
                samplerLog("[WARNING] MQ136 baseline R0 invalidated due to circuit change. Recalibration required.");
            } else {
                samplerLog("[ERROR] MQ136 RL parameter rejected by validator.");
            }
            break;
        }

        case CMD_SAVE_NVS:
            saveConfigurationToNVS();
            break;

        case CMD_RESET_CALIBRATION:
            resetCalibrationInNVS();
            break;

        default:
            break;
    }
}

// =========================================================================================
// 11. NVS PERSISTENCE (SOLE OWNER: TaskMQSampler, ATOMIC BINARY RECORD WITH CRC32)
// =========================================================================================
static void loadConfigurationFromNVS() {
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) { // Read-only mode
        samplerLog("[NVS] Failed to open NVS namespace. Using unconfigured defaults.");
        return;
    }

    if (prefs.isKey(NVS_BLOB_KEY)) {
        MQNVSRecord rec;
        size_t read_bytes = prefs.getBytes(NVS_BLOB_KEY, &rec, sizeof(rec));
        if (read_bytes == sizeof(rec)) {
            uint32_t expected_crc = calculateCRC32((const uint8_t*)&rec, offsetof(MQNVSRecord, crc32));
            if (rec.magic == NVS_MAGIC_HEADER && rec.schema_version == NVS_VERSION && rec.crc32 == expected_crc) {
                samplerLog("[NVS] Valid binary calibration record found (v%u, CRC: 0x%08X). Validating parameters...",
                           rec.schema_version, rec.crc32);

                // Unpack and validate MQ137
                float k137 = 0.0f;
                if (rec.ch137.is_divider_valid &&
                    validateDivider(rec.ch137.r_top_ohm, rec.ch137.r_bottom_ohm, &k137, NULL, 0)) {
                    config137.r_top_ohm = rec.ch137.r_top_ohm;
                    config137.r_bottom_ohm = rec.ch137.r_bottom_ohm;
                    config137.divider_k = k137;
                    config137.is_divider_valid = true;
                } else {
                    config137.is_divider_valid = false;
                }

                if (rec.ch137.is_rl_valid && validateRL(rec.ch137.rl_nominal_ohm, NULL, 0)) {
                    config137.rl_nominal_ohm = rec.ch137.rl_nominal_ohm;
                    config137.is_rl_valid = true;
                } else {
                    config137.is_rl_valid = false;
                }

                if (rec.ch137.is_r0_valid && config137.is_divider_valid && config137.is_rl_valid &&
                    validateR0(rec.ch137.r0_clean_air_ohm, NULL, 0)) {
                    config137.r0_clean_air_ohm = rec.ch137.r0_clean_air_ohm;
                    config137.is_r0_valid = true;
                } else {
                    config137.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
                    config137.is_r0_valid = false;
                }

                // Unpack and validate MQ136
                float k136 = 0.0f;
                if (rec.ch136.is_divider_valid &&
                    validateDivider(rec.ch136.r_top_ohm, rec.ch136.r_bottom_ohm, &k136, NULL, 0)) {
                    config136.r_top_ohm = rec.ch136.r_top_ohm;
                    config136.r_bottom_ohm = rec.ch136.r_bottom_ohm;
                    config136.divider_k = k136;
                    config136.is_divider_valid = true;
                } else {
                    config136.is_divider_valid = false;
                }

                if (rec.ch136.is_rl_valid && validateRL(rec.ch136.rl_nominal_ohm, NULL, 0)) {
                    config136.rl_nominal_ohm = rec.ch136.rl_nominal_ohm;
                    config136.is_rl_valid = true;
                } else {
                    config136.is_rl_valid = false;
                }

                if (rec.ch136.is_r0_valid && config136.is_divider_valid && config136.is_rl_valid &&
                    validateR0(rec.ch136.r0_clean_air_ohm, NULL, 0)) {
                    config136.r0_clean_air_ohm = rec.ch136.r0_clean_air_ohm;
                    config136.is_r0_valid = true;
                } else {
                    config136.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
                    config136.is_r0_valid = false;
                }

                samplerLog("[NVS] Parameters validated and loaded successfully.");
            } else {
                samplerLog("[NVS ERROR] Binary record corrupted or version mismatch! CRC: 0x%08X vs 0x%08X. Running in unconfirmed mode.",
                           rec.crc32, expected_crc);
            }
        } else {
            samplerLog("[NVS ERROR] Binary record read size mismatch (%u != %u). Running in unconfirmed mode.",
                       (uint32_t)read_bytes, (uint32_t)sizeof(rec));
        }
    } else {
        // Safe inspection of legacy keys: do not delete, inform user
        if (prefs.isKey("magic") || prefs.isKey("div_ok_137")) {
            samplerLog("[NVS NOTICE] Legacy v1 schema detected without v2 record. Ignoring legacy data; running in unconfirmed diagnostic mode.");
        } else {
            samplerLog("[NVS] No persisted configuration found. Running in unconfirmed diagnostic mode.");
        }
    }
    prefs.end();
}

static void saveConfigurationToNVS() {
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) { // Read-write mode
        samplerLog("[NVS ERROR] Could not open flash namespace for writing!");
        return;
    }

    MQNVSRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.magic = NVS_MAGIC_HEADER;
    rec.schema_version = NVS_VERSION;
    rec.reserved = 0;

    // Pack MQ137
    rec.ch137.r_top_ohm = config137.r_top_ohm;
    rec.ch137.r_bottom_ohm = config137.r_bottom_ohm;
    rec.ch137.divider_k = config137.divider_k;
    rec.ch137.rl_nominal_ohm = config137.rl_nominal_ohm;
    rec.ch137.r0_clean_air_ohm = config137.r0_clean_air_ohm;
    rec.ch137.is_divider_valid = config137.is_divider_valid ? 1 : 0;
    rec.ch137.is_rl_valid = config137.is_rl_valid ? 1 : 0;
    rec.ch137.is_r0_valid = config137.is_r0_valid ? 1 : 0;
    rec.ch137.reserved = 0;

    // Pack MQ136
    rec.ch136.r_top_ohm = config136.r_top_ohm;
    rec.ch136.r_bottom_ohm = config136.r_bottom_ohm;
    rec.ch136.divider_k = config136.divider_k;
    rec.ch136.rl_nominal_ohm = config136.rl_nominal_ohm;
    rec.ch136.r0_clean_air_ohm = config136.r0_clean_air_ohm;
    rec.ch136.is_divider_valid = config136.is_divider_valid ? 1 : 0;
    rec.ch136.is_rl_valid = config136.is_rl_valid ? 1 : 0;
    rec.ch136.is_r0_valid = config136.is_r0_valid ? 1 : 0;
    rec.ch136.reserved = 0;

    // CRC32 calculated over all bytes preceding the crc32 field
    rec.crc32 = calculateCRC32((const uint8_t*)&rec, offsetof(MQNVSRecord, crc32));

    size_t written = prefs.putBytes(NVS_BLOB_KEY, &rec, sizeof(rec));
    prefs.end();

    if (written == sizeof(rec)) {
        samplerLog("[NVS SUCCESS] Binary configuration record committed to flash (CRC32: 0x%08X)!", rec.crc32);
    } else {
        samplerLog("[NVS ERROR] Write failed! Wrote %u of %u bytes.", (uint32_t)written, (uint32_t)sizeof(rec));
    }
}

static void resetCalibrationInNVS() {
    Preferences prefs;
    bool clear_ok = false;
    if (prefs.begin(NVS_NAMESPACE, false)) {
        clear_ok = prefs.clear();
        prefs.end();
    }

    config137.r_top_ohm = 10000.0f;
    config137.r_bottom_ohm = 15000.0f;
    config137.divider_k = 0.6000f;
    config137.rl_nominal_ohm = 0.0f;
    config137.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
    config137.is_divider_valid = false;
    config137.is_rl_valid = false;
    config137.is_r0_valid = false;

    config136.r_top_ohm = 10000.0f;
    config136.r_bottom_ohm = 15000.0f;
    config136.divider_k = 0.6000f;
    config136.rl_nominal_ohm = 0.0f;
    config136.r0_clean_air_ohm = MQ_VALUE_UNCONFIGURED;
    config136.is_divider_valid = false;
    config136.is_rl_valid = false;
    config136.is_r0_valid = false;

    if (calState.active) calState.active = false;

    if (clear_ok) {
        samplerLog("[NVS SUCCESS] Flash namespace cleared! Restored to unconfirmed diagnostic defaults.");
    } else {
        samplerLog("[NVS ERROR] Flash clear operation failed! Restored defaults in RAM only.");
    }
}

// =========================================================================================
// 12. TASK: MQ LOGGER & CLI PARSER (CORE 0, PRIORITY 1) — SOLE SERIAL WRITER
// =========================================================================================
static void TaskMQLogger(void* pvParameters) {
    (void)pvParameters;

    char serial_rx_buf[64];
    uint8_t rx_idx = 0;
    MQDataFrame frame;

    for (;;) {
        // 1. Drain and print any asynchronous log messages from Sampler
        LogMessage logMsg;
        while (xQueueReceive(xQueueLogMessages, &logMsg, 0) == pdTRUE) {
            Serial.printf(">>> %s\n", logMsg.text);
        }

        // 2. Finite-timeout wait for telemetry frame from Sampler (100 ms)
        if (xQueueReceive(xQueueMQFrames, &frame, pdMS_TO_TICKS(100)) == pdTRUE) {
            s_last_valid_frame = frame;
            s_has_received_frame = true;

            uint32_t uptime_s = (uint32_t)(frame.timestamp_ms / 1000ULL);
            uint32_t hours = uptime_s / 3600;
            uint32_t minutes = (uptime_s % 3600) / 60;
            uint32_t seconds = uptime_s % 60;

            Serial.printf("----------------------------------------------------------------------------------------\n");
            Serial.printf("[FRAME #%05u] Monotonic Uptime: %02uh:%02um:%02us (%llu ms) | Dropped: %u\n",
                          frame.frame_id, hours, minutes, seconds, frame.timestamp_ms, frame.dropped_frames_count);

            // Channel 1: MQ137 (Ammonia) — Explicit Mean vs EMA
            Serial.printf("  CH1 [MQ137-NH3]: ADC_mean = %4u (min:%4u, max:%4u, dev:%4.1f) | ADC_ema = %4u | V_pin = %4.0f mV\n",
                          frame.mq137.adc_raw_mean, frame.mq137.adc_raw_min, frame.mq137.adc_raw_max,
                          frame.mq137.adc_raw_stddev, frame.mq137.adc_raw_ema, frame.mq137.v_adc_mv);
            
            // Reconstructed V_AO
            if (frame.mq137.status_flags & MQ_FLAG_DIVIDER_OK) {
                Serial.printf("                 Reconstructed V_AO = %4.0f mV (k=%.4f)",
                              frame.mq137.v_ao_mv, frame.cfg137_snap.divider_k);
            } else {
                Serial.print(F("                 Reconstructed V_AO = N/A [DIVIDER_UNCONFIRMED]"));
            }

            // Sensor Resistance Rs
            if (frame.mq137.status_flags & MQ_FLAG_RS_VALID) {
                Serial.printf(" | Rs = %6.0f Ohm", frame.mq137.rs_ohm);
            } else if (!(frame.mq137.status_flags & MQ_FLAG_DIVIDER_OK)) {
                Serial.print(F(" | Rs = N/A [DIVIDER_UNCONFIRMED]"));
            } else if (!(frame.mq137.status_flags & MQ_FLAG_RL_OK)) {
                Serial.print(F(" | Rs = N/A [RL_UNCONFIGURED]"));
            } else if (frame.mq137.status_flags & MQ_FLAG_ADC_SATURATED) {
                Serial.print(F(" | Rs = N/A [ADC_SATURATED]"));
            } else if (frame.mq137.status_flags & MQ_FLAG_SIGNAL_INVALID) {
                Serial.print(F(" | Rs = N/A [SIGNAL_INVALID]"));
            } else {
                Serial.print(F(" | Rs = N/A [UNAVAILABLE]"));
            }

            // Ratio Rs / R0
            if (frame.mq137.status_flags & MQ_FLAG_R0_CALIBRATED) {
                Serial.printf(" | Rs/R0 = %4.2f (R0=%.0f)\n",
                              frame.mq137.ratio_rs_r0, frame.cfg137_snap.r0_clean_air_ohm);
            } else if (!(frame.mq137.status_flags & MQ_FLAG_RS_VALID)) {
                Serial.print(F(" | Rs/R0 = N/A [RS_INVALID]\n"));
            } else if (!frame.cfg137_snap.is_r0_valid) {
                Serial.print(F(" | Rs/R0 = N/A [R0_NOT_CALIBRATED]\n"));
            } else {
                Serial.print(F(" | Rs/R0 = N/A [UNAVAILABLE]\n"));
            }

            // Channel 2: MQ136 (Hydrogen Sulfide) — Explicit Mean vs EMA
            Serial.printf("  CH2 [MQ136-H2S]: ADC_mean = %4u (min:%4u, max:%4u, dev:%4.1f) | ADC_ema = %4u | V_pin = %4.0f mV\n",
                          frame.mq136.adc_raw_mean, frame.mq136.adc_raw_min, frame.mq136.adc_raw_max,
                          frame.mq136.adc_raw_stddev, frame.mq136.adc_raw_ema, frame.mq136.v_adc_mv);

            // Reconstructed V_AO
            if (frame.mq136.status_flags & MQ_FLAG_DIVIDER_OK) {
                Serial.printf("                 Reconstructed V_AO = %4.0f mV (k=%.4f)",
                              frame.mq136.v_ao_mv, frame.cfg136_snap.divider_k);
            } else {
                Serial.print(F("                 Reconstructed V_AO = N/A [DIVIDER_UNCONFIRMED]"));
            }

            // Sensor Resistance Rs
            if (frame.mq136.status_flags & MQ_FLAG_RS_VALID) {
                Serial.printf(" | Rs = %6.0f Ohm", frame.mq136.rs_ohm);
            } else if (!(frame.mq136.status_flags & MQ_FLAG_DIVIDER_OK)) {
                Serial.print(F(" | Rs = N/A [DIVIDER_UNCONFIRMED]"));
            } else if (!(frame.mq136.status_flags & MQ_FLAG_RL_OK)) {
                Serial.print(F(" | Rs = N/A [RL_UNCONFIGURED]"));
            } else if (frame.mq136.status_flags & MQ_FLAG_ADC_SATURATED) {
                Serial.print(F(" | Rs = N/A [ADC_SATURATED]"));
            } else if (frame.mq136.status_flags & MQ_FLAG_SIGNAL_INVALID) {
                Serial.print(F(" | Rs = N/A [SIGNAL_INVALID]"));
            } else {
                Serial.print(F(" | Rs = N/A [UNAVAILABLE]"));
            }

            // Ratio Rs / R0
            if (frame.mq136.status_flags & MQ_FLAG_R0_CALIBRATED) {
                Serial.printf(" | Rs/R0 = %4.2f (R0=%.0f)\n",
                              frame.mq136.ratio_rs_r0, frame.cfg136_snap.r0_clean_air_ohm);
            } else if (!(frame.mq136.status_flags & MQ_FLAG_RS_VALID)) {
                Serial.print(F(" | Rs/R0 = N/A [RS_INVALID]\n"));
            } else if (!frame.cfg136_snap.is_r0_valid) {
                Serial.print(F(" | Rs/R0 = N/A [R0_NOT_CALIBRATED]\n"));
            } else {
                Serial.print(F(" | Rs/R0 = N/A [UNAVAILABLE]\n"));
            }

            // Diagnostic Warnings & FreeRTOS Resource Metrics (Stack watermark in BYTES)
            if (frame.mq137.status_flags & MQ_FLAG_PREHEAT_WARN) {
                Serial.print(F("  [NOTE] Uptime ESP32 < 48 jam. Datasheet Winsen v1.6 mensyaratkan pemanasan 48 jam sebelum baseline stabil. Pastikan heater 5V telah menyala sesuai durasi preheat.\n"));
            }
            if ((frame.mq137.status_flags | frame.mq136.status_flags) & MQ_FLAG_ADC_SATURATED) {
                Serial.print(F("  [ALERT] ADC SATURATION DETECTED (raw >= 4095)! Sinyal tidak sah.\n"));
            }

            Serial.printf("  [RTOS] Stack High Water Mark: Sampler = %u B remaining, Logger = %u B remaining\n",
                          frame.sampler_stack_watermark_bytes, frame.logger_stack_watermark_bytes);
        }

        // 3. Non-blocking Serial Command CLI Parser
        while (Serial.available() > 0) {
            char ch = (char)Serial.read();
            if (ch == '\r' || ch == '\n') {
                if (rx_idx > 0) {
                    serial_rx_buf[rx_idx] = '\0';
                    parseSerialInput(serial_rx_buf);
                    rx_idx = 0;
                }
            } else if (rx_idx < (sizeof(serial_rx_buf) - 1)) {
                serial_rx_buf[rx_idx++] = ch;
            }
        }
    }
}

// =========================================================================================
// 13. CLI COMMAND PARSING & DISPATCH WITH ENQUEUE VERIFICATION
// =========================================================================================
static void parseSerialInput(const char* line) {
    while (*line == ' ') line++; // Trim leading whitespace
    if (strlen(line) == 0) return;

    Serial.printf("\n[CMD] Received command: '%s'\n", line);

    MQCommand cmd;
    cmd.type = CMD_NONE;
    cmd.param1 = 0.0f;
    cmd.param2 = 0.0f;

    if (strcasecmp(line, "help") == 0 || strcmp(line, "?") == 0) {
        printCommandHelp();
        return;
    } else if (strcasecmp(line, "status") == 0) {
        uint32_t uptime_s = (uint32_t)(esp_timer_get_time() / 1000000ULL);
        Serial.println(F("\n--- [SYSTEM & RTOS STATUS] ---"));
        Serial.printf("  Uptime            : %u seconds (%u min, %u hrs)\n",
                      uptime_s, uptime_s / 60, uptime_s / 3600);
        Serial.printf("  Free Heap         : %u bytes (Historic Min: %u bytes)\n",
                      esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
        Serial.printf("  Core Allocation   : TaskMQSampler -> Core %d, TaskMQLogger -> Core %d\n",
                      SAMPLER_TASK_CORE, LOGGER_TASK_CORE);
        if (xHandleMQSampler) {
            Serial.printf("  Sampler Stack HWM : %u bytes remaining\n",
                          (uint32_t)uxTaskGetStackHighWaterMark(xHandleMQSampler));
        }
        if (xHandleMQLogger) {
            Serial.printf("  Logger Stack HWM  : %u bytes remaining\n",
                          (uint32_t)uxTaskGetStackHighWaterMark(xHandleMQLogger));
        }
        Serial.println(F("------------------------------\n"));
        return;
    } else if (strcasecmp(line, "config") == 0) {
        // Read configuration from the cached snapshot frame
        if (s_has_received_frame) {
            printActiveConfig(&s_last_valid_frame.cfg137_snap, &s_last_valid_frame.cfg136_snap);
        } else {
            Serial.println(F("[CONFIG] Telemetry frame not yet received from Sampler. Please wait 1 second."));
        }
        return;
    } else if (strcasecmp(line, "cal137") == 0) {
        cmd.type = CMD_CALIBRATE_MQ137;
    } else if (strcasecmp(line, "cal136") == 0) {
        cmd.type = CMD_CALIBRATE_MQ136;
    } else if (strncasecmp(line, "setdiv137", 9) == 0) {
        float rtop = 0.0f, rbot = 0.0f, k = 0.0f;
        char err_buf[96] = {0};
        if (sscanf(line + 9, "%f %f", &rtop, &rbot) == 2 &&
            validateDivider(rtop, rbot, &k, err_buf, sizeof(err_buf))) {
            cmd.type = CMD_SET_DIVIDER_MQ137;
            cmd.param1 = rtop;
            cmd.param2 = rbot;
        } else {
            if (strlen(err_buf) > 0) {
                Serial.printf("[ERROR] %s\n", err_buf);
            } else {
                Serial.println(F("[ERROR] Usage: setdiv137 <R_TOP_OHM> <R_BOTTOM_OHM> (e.g. setdiv137 10000 15000)"));
            }
            return;
        }
    } else if (strncasecmp(line, "setdiv136", 9) == 0) {
        float rtop = 0.0f, rbot = 0.0f, k = 0.0f;
        char err_buf[96] = {0};
        if (sscanf(line + 9, "%f %f", &rtop, &rbot) == 2 &&
            validateDivider(rtop, rbot, &k, err_buf, sizeof(err_buf))) {
            cmd.type = CMD_SET_DIVIDER_MQ136;
            cmd.param1 = rtop;
            cmd.param2 = rbot;
        } else {
            if (strlen(err_buf) > 0) {
                Serial.printf("[ERROR] %s\n", err_buf);
            } else {
                Serial.println(F("[ERROR] Usage: setdiv136 <R_TOP_OHM> <R_BOTTOM_OHM> (e.g. setdiv136 10000 15000)"));
            }
            return;
        }
    } else if (strncasecmp(line, "setrl137", 8) == 0) {
        float rl = 0.0f;
        char err_buf[96] = {0};
        if (sscanf(line + 8, "%f", &rl) == 1 && validateRL(rl, err_buf, sizeof(err_buf))) {
            cmd.type = CMD_SET_RL_MQ137;
            cmd.param1 = rl;
        } else {
            if (strlen(err_buf) > 0) {
                Serial.printf("[ERROR] %s\n", err_buf);
            } else {
                Serial.println(F("[ERROR] Usage: setrl137 <RL_OHM> (100 Ohm <= RL <= 1 MOhm)"));
            }
            return;
        }
    } else if (strncasecmp(line, "setrl136", 8) == 0) {
        float rl = 0.0f;
        char err_buf[96] = {0};
        if (sscanf(line + 8, "%f", &rl) == 1 && validateRL(rl, err_buf, sizeof(err_buf))) {
            cmd.type = CMD_SET_RL_MQ136;
            cmd.param1 = rl;
        } else {
            if (strlen(err_buf) > 0) {
                Serial.printf("[ERROR] %s\n", err_buf);
            } else {
                Serial.println(F("[ERROR] Usage: setrl136 <RL_OHM> (100 Ohm <= RL <= 1 MOhm)"));
            }
            return;
        }
    } else if (strcasecmp(line, "save") == 0) {
        cmd.type = CMD_SAVE_NVS;
    } else if (strcasecmp(line, "resetcal") == 0) {
        cmd.type = CMD_RESET_CALIBRATION;
    } else {
        Serial.printf("[ERROR] Unknown command '%s'. Type 'help' for available commands.\n", line);
        return;
    }

    // Verified enqueue to TaskMQSampler
    if (cmd.type != CMD_NONE) {
        if (xQueueSend(xQueueMQCommands, &cmd, pdMS_TO_TICKS(50)) != pdTRUE) {
            Serial.println(F("[FATAL QUEUE ERROR] Command queue full! Command rejected."));
        }
    }
}

// =========================================================================================
// 14. HELPER FUNCTIONS
// =========================================================================================
static void populateConfigSnapshot(MQChannelConfigSnapshot* snap, const MQChannelConfig* cfg) {
    strncpy(snap->gas_name, cfg->gas_name, sizeof(snap->gas_name) - 1);
    snap->gas_name[sizeof(snap->gas_name) - 1] = '\0';
    snap->pin = cfg->pin;
    snap->r_top_ohm = cfg->r_top_ohm;
    snap->r_bottom_ohm = cfg->r_bottom_ohm;
    snap->divider_k = cfg->divider_k;
    snap->rl_nominal_ohm = cfg->rl_nominal_ohm;
    snap->r0_clean_air_ohm = cfg->r0_clean_air_ohm;
    snap->is_divider_valid = cfg->is_divider_valid;
    snap->is_rl_valid = cfg->is_rl_valid;
    snap->is_r0_valid = cfg->is_r0_valid;
}

static void printCommandHelp() {
    Serial.println(F("\n========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — Serial CLI Command Reference [v2.2]            "));
    Serial.println(F("========================================================================"));
    Serial.println(F("  help                      : Display this reference menu"));
    Serial.println(F("  status                    : Show RTOS runtime metrics, memory, and stack"));
    Serial.println(F("  config                    : Print active circuit & baseline calibration state"));
    Serial.println(F("  setdiv137 <rtop> <rbot>   : Confirm divider resistors for MQ137 (Ohm)"));
    Serial.println(F("  setdiv136 <rtop> <rbot>   : Confirm divider resistors for MQ136 (Ohm)"));
    Serial.println(F("  setrl137 <ohm>            : Configure breakout load resistor RL for MQ137"));
    Serial.println(F("  setrl136 <ohm>            : Configure breakout load resistor RL for MQ136"));
    Serial.println(F("  cal137                    : Perform 10-second clean-air baseline (R0) cal"));
    Serial.println(F("  cal136                    : Perform 10-second clean-air baseline (R0) cal"));
    Serial.println(F("  save                      : Commit active configuration & R0 to NVS flash"));
    Serial.println(F("  resetcal                  : Erase NVS flash and reset to default diagnostics"));
    Serial.println(F("========================================================================\n"));
}

static void printActiveConfig(const MQChannelConfigSnapshot* c137, const MQChannelConfigSnapshot* c136) {
    Serial.println(F("\n--- [ACTIVE CIRCUIT & CALIBRATION CONFIG] ---"));
    Serial.printf("  MQ137 (Ammonia):\n");
    Serial.printf("    Pin GPIO        : %u (ADC1_CH4)\n", c137->pin);
    Serial.printf("    Divider Status  : %s (R_top=%.0f Ohm, R_bot=%.0f Ohm -> k=%.4f)\n",
                  c137->is_divider_valid ? "CONFIRMED" : "UNCONFIRMED_DEFAULT",
                  c137->r_top_ohm, c137->r_bottom_ohm, c137->divider_k);
    if (c137->is_rl_valid) {
        Serial.printf("    Breakout RL     : CONFIGURED (%.0f Ohm)\n", c137->rl_nominal_ohm);
    } else {
        Serial.println(F("    Breakout RL     : UNCONFIGURED (N/A)"));
    }
    if (c137->is_r0_valid) {
        Serial.printf("    Clean-Air R0    : CALIBRATED (%.1f Ohm)\n", c137->r0_clean_air_ohm);
    } else {
        Serial.println(F("    Clean-Air R0    : NOT_CALIBRATED (N/A)"));
    }
    Serial.println(F("    PPM Estimation  : DISABLED [PPM_MODEL_UNCONFIGURED] (Sensitivity curves PPM = a*(Rs/R0)^b require empirical lab chamber data)"));

    Serial.printf("  MQ136 (Hydrogen Sulfide):\n");
    Serial.printf("    Pin GPIO        : %u (ADC1_CH5)\n", c136->pin);
    Serial.printf("    Divider Status  : %s (R_top=%.0f Ohm, R_bot=%.0f Ohm -> k=%.4f)\n",
                  c136->is_divider_valid ? "CONFIRMED" : "UNCONFIRMED_DEFAULT",
                  c136->r_top_ohm, c136->r_bottom_ohm, c136->divider_k);
    if (c136->is_rl_valid) {
        Serial.printf("    Breakout RL     : CONFIGURED (%.0f Ohm)\n", c136->rl_nominal_ohm);
    } else {
        Serial.println(F("    Breakout RL     : UNCONFIGURED (N/A)"));
    }
    if (c136->is_r0_valid) {
        Serial.printf("    Clean-Air R0    : CALIBRATED (%.1f Ohm)\n", c136->r0_clean_air_ohm);
    } else {
        Serial.println(F("    Clean-Air R0    : NOT_CALIBRATED (N/A)"));
    }
    Serial.println(F("    PPM Estimation  : DISABLED [PPM_MODEL_UNCONFIGURED] (Sensitivity curves PPM = a*(Rs/R0)^b require empirical lab chamber data)"));
    Serial.println(F("--------------------------------------------\n"));
}
