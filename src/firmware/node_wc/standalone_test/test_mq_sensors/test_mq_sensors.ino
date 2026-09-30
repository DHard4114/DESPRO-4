/**
 * =========================================================================================
 * Smart-Sanitation eSOS — ESP32 Dual Gas Sensor Standalone Test & Diagnostics
 * Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
 * Target Board : DOIT ESP32 DevKit V1 (ESP32-WROOM-32, Dual-Core Xtensa LX6)
 * Sensors      : Winsen MQ-137 (Ammonia / NH3) & Winsen MQ-136 (Hydrogen Sulfide / H2S)
 * Manual Ver.  : Winsen Manual v1.6 (Valid from: 2021-07-01)
 * Framework    : Arduino-ESP32 (v2.0.17 / ESP-IDF v4.4) + Native Espressif FreeRTOS
 * Author       : AI Engineering Agent (Antigravity) on behalf of Kelompok 4 FTUI
 * Date         : September 2026
 *
 * CRITICAL HARDWARE & ELECTRICAL SAFETY NOTICE:
 * -----------------------------------------------------------------------------------------
 * 1. SENSOR HEATER SUPPLY:
 *    - Both sensors require Vc = 5.0V +/- 0.1V and Vh = 5.0V +/- 0.1V DC.
 *    - Combined heater power consumption can reach 2 x 950 mW = 1.9 Watts (~380 mA @ 5V).
 *    - DO NOT power the sensor heaters directly from ESP32 3.3V or USB 3.3V LDO.
 *    - Common Ground (GND) must be tied securely between 5V PSU, breakout modules, and ESP32.
 *
 * 2. ADC VOLTAGE LIMITS & LEVEL CONVERSION:
 *    - Breakout Analog Output (AO) can swing up to 5.0V.
 *    - ESP32 GPIO absolute maximum rating is VDD + 0.3V (~3.6V).
 *    - Connecting 5.0V directly to ESP32 ADC pins will cause permanent silicon breakdown!
 *    - Internal ADC attenuation (11 dB) only scales the ADC measurement range; it DOES NOT
 *      protect the physical silicon input stage against overvoltage damage.
 *    - A dedicated resistor divider (or analog buffer) is MANDATORY on each AO line.
 *      Proposed Divider: R_TOP = 10 kOhm, R_BOTTOM = 15 kOhm
 *      Divider Ratio: k = R_BOTTOM / (R_TOP + R_BOTTOM) = 15 / (10 + 15) = 0.60
 *      At max AO = 5.0V -> V_ADC = 3.00V (well within safe limit and ADC1 linear window).
 *    - Divider Loading: The divider resistance (R_TOP + R_BOTTOM = 25 kOhm) appears in parallel
 *      with the breakout load resistor (RL). Effective load: RL_eff = (RL * (R_TOP + R_BOTTOM)) / (RL + R_TOP + R_BOTTOM).
 *
 * 3. PIN ALLOCATION (ADC1 ONLY):
 *    - MQ137 AO -> GPIO 32 (ADC1_CH4)
 *    - MQ136 AO -> GPIO 33 (ADC1_CH5)
 *    - Using ADC1 prevents hardware conflicts with Wi-Fi / Radio SAR ADC arbiter.
 *
 * 4. ZERO-TRUST & CALIBRATION POLICY:
 *    - Standard preheat requirement per Winsen datasheet is OVER 48 HOURS.
 *    - In clean air, the typical resistance ratio is Rs/R0 ~ 3.6 for both sensors.
 *    - Initial diagnostic state: PPM calculation is STRICTLY DISABLED (PPM_NOT_CALIBRATED)
 *      until the user explicitly configures RL, confirms divider ratio k, and conducts
 *      a stabilized clean-air baseline calibration (R0). No arbitrary assumptions allowed!
 * =========================================================================================
 */

#include <Arduino.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <math.h>

// =========================================================================================
// 1. PIN DEFINITIONS & PHYSICAL CONSTANTS
// =========================================================================================
#define PIN_ADC_MQ137           32      // GPIO32 = ADC1_CH4 (Ammonia / NH3)
#define PIN_ADC_MQ136           33      // GPIO33 = ADC1_CH5 (Hydrogen Sulfide / H2S)

// Operating voltages (nominal)
#define V_LOOP_SUPPLY_VOLTS     5.00f   // Vc = 5.0V +/- 0.1V DC per Winsen datasheet

// FreeRTOS Task Configuration
#define SAMPLER_TASK_CORE       0       // Pinned to Core 0 (PRO_CPU)
#define SAMPLER_TASK_PRIORITY   2       // Priority 2 (Higher than logger)
#define SAMPLER_STACK_WORDS     1024    // 1024 words = 4096 bytes on Xtensa

#define LOGGER_TASK_CORE        0       // Pinned to Core 0 (PRO_CPU)
#define LOGGER_TASK_PRIORITY    1       // Priority 1
#define LOGGER_STACK_WORDS      1024    // 1024 words = 4096 bytes on Xtensa

#define FRAME_PERIOD_MS         1000    // Sampling frame every 1000 ms (1 Hz)
#define SAMPLES_PER_FRAME       16      // 16 oversampled readings per sensor frame
#define INTER_SAMPLE_DELAY_MS   10      // 10 ms delay between successive ADC bursts

// Digital Filter Constant
#define EMA_ALPHA               0.25f   // Exponential Moving Average smoothing factor

// NVS Persistent Storage Configuration
#define NVS_NAMESPACE           "mq_cal"
#define NVS_MAGIC_HEADER        0x4D513031  // ASCII 'MQ01'
#define NVS_VERSION             1

// Special value when PPM or Rs cannot be computed
#define MQ_VALUE_UNCONFIGURED   (-1.0f)

// Status Bitflags
#define MQ_FLAG_RAW_OK          (1 << 0)    // ADC sampled successfully
#define MQ_FLAG_DIVIDER_OK      (1 << 1)    // Voltage divider ratio configured
#define MQ_FLAG_RL_OK           (1 << 2)    // Load resistor RL configured
#define MQ_FLAG_R0_CALIBRATED   (1 << 3)    // Clean air baseline R0 calibrated
#define MQ_FLAG_PPM_VALID       (1 << 4)    // Mathematical conditions met for PPM output
#define MQ_FLAG_ADC_SATURATED   (1 << 5)    // ADC clipped at raw 4095
#define MQ_FLAG_PREHEAT_WARN    (1 << 6)    // Uptime < 48 hours notice

// =========================================================================================
// 2. DATA STRUCTURES & PROTOCOLS
// =========================================================================================

// Sensor Channel Hardware & Calibration Parameters
struct MQChannelConfig {
    const char* gas_name;           // e.g. "NH3" or "H2S"
    uint8_t pin;                    // GPIO Pin number
    float r_top_ohm;                // Resistor from AO to ESP32 ADC pin
    float r_bottom_ohm;             // Resistor from ESP32 ADC pin to GND
    float divider_k;                // k = R_bottom / (R_top + R_bottom)
    float rl_nominal_ohm;           // Breakout module load resistor (0 = unconfigured)
    float r0_clean_air_ohm;         // Baseline sensor resistance in clean air (<= 0 = uncalibrated)
    float curve_a;                  // Sensitivity curve parameter A: ppm = A * (Rs/R0)^B
    float curve_b;                  // Sensitivity curve parameter B
    float clean_air_ratio;          // Datasheet typical Rs/R0 in clean air (~3.6)
    bool is_divider_valid;
    bool is_rl_valid;
    bool is_r0_valid;
};

// Snapshot Measurement for a single sensor channel
struct MQChannelReading {
    uint16_t adc_raw_avg;           // Arithmetic mean of 16 raw ADC samples (0-4095)
    uint16_t adc_raw_min;           // Minimum sample in burst
    uint16_t adc_raw_max;           // Maximum sample in burst
    float adc_raw_stddev;           // Standard deviation (stability check)
    float v_adc_mv;                 // Voltage at ESP32 pin (millivolts, via factory eFuse cal)
    float v_ao_mv;                  // Reconstructed AO voltage (V_ADC / k) in millivolts
    float rs_ohm;                   // Calculated sensor resistance in Ohms
    float ratio_rs_r0;              // Rs / R0 ratio
    float ppm_estimate;             // Estimated gas concentration in ppm (-1.0 if not calibrated)
    uint8_t status_flags;           // Bitmask of MQ_FLAG_*
};

// Complete Dual-Sensor Telemetry Snapshot Frame (Fixed Size, Memory Safe)
struct MQDataFrame {
    uint32_t frame_id;
    uint64_t timestamp_ms;
    MQChannelReading mq137;
    MQChannelReading mq136;
    uint32_t sampler_stack_watermark_words;
    uint32_t logger_stack_watermark_words;
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
    CMD_RESET_CALIBRATION,
    CMD_PRINT_CONFIG,
    CMD_PRINT_STATUS,
    CMD_PRINT_HELP
};

struct MQCommand {
    CommandType type;
    float param1;                   // e.g. R_TOP or k or RL
    float param2;                   // e.g. R_BOTTOM
};

// Calibration State Machine (accumulates 10 stable frames in clean air)
struct CalibrationState {
    bool active;
    uint8_t target_sensor;          // 137 or 136
    uint8_t frame_count;
    float rs_accumulator;
    float last_rs_readings[10];
};

// =========================================================================================
// 3. GLOBAL VARIABLES & RTOS HANDLES
// =========================================================================================
static QueueHandle_t xQueueMQFrames = NULL;
static QueueHandle_t xQueueMQCommands = NULL;

static TaskHandle_t xHandleMQSampler = NULL;
static TaskHandle_t xHandleMQLogger = NULL;

static Preferences prefs;

// Channel configurations (Default: Proposed divider, RL unconfigured, R0 uncalibrated)
static MQChannelConfig config137 = {
    .gas_name = "NH3 (MQ137)",
    .pin = PIN_ADC_MQ137,
    .r_top_ohm = 10000.0f,          // 10 kOhm proposed
    .r_bottom_ohm = 15000.0f,       // 15 kOhm proposed
    .divider_k = 0.60f,             // 15 / (10 + 15) = 0.60
    .rl_nominal_ohm = 0.0f,         // 0 = unconfigured (diagnostic mode)
    .r0_clean_air_ohm = -1.0f,      // -1 = uncalibrated
    .curve_a = 50.0f,               // Winsen v1.6 Fig 3 curve fit: ppm = 50 * (Rs/R0)^(-2.513)
    .curve_b = -2.513f,
    .clean_air_ratio = 3.60f,       // Datasheet clean air ratio Rs/R0 ~ 3.6
    .is_divider_valid = true,
    .is_rl_valid = false,
    .is_r0_valid = false
};

static MQChannelConfig config136 = {
    .gas_name = "H2S (MQ136)",
    .pin = PIN_ADC_MQ136,
    .r_top_ohm = 10000.0f,          // 10 kOhm proposed
    .r_bottom_ohm = 15000.0f,       // 15 kOhm proposed
    .divider_k = 0.60f,             // 15 / (10 + 15) = 0.60
    .rl_nominal_ohm = 0.0f,         // 0 = unconfigured (diagnostic mode)
    .r0_clean_air_ohm = -1.0f,      // -1 = uncalibrated
    .curve_a = 10.0f,               // Winsen v1.6 Fig 3 curve fit: ppm = 10 * (Rs/R0)^(-2.900)
    .curve_b = -2.900f,
    .clean_air_ratio = 3.60f,       // Datasheet clean air ratio Rs/R0 ~ 3.6
    .is_divider_valid = true,
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

// =========================================================================================
// 4. FORWARD DECLARATIONS
// =========================================================================================
static void TaskMQSampler(void* pvParameters);
static void TaskMQLogger(void* pvParameters);

static void loadConfigurationFromNVS();
static void saveConfigurationToNVS();
static void resetCalibrationInNVS();

static void processChannelSampling(MQChannelConfig* cfg, MQChannelReading* reading, float* ema_val);
static void handleIncomingCommand(const MQCommand* cmd);
static void parseSerialInput(const char* line);
static void printCommandHelp();
static void printSystemStatus();
static void printActiveConfig();

// =========================================================================================
// 5. ARDUINO SETUP & SYSTEM INITIALIZATION
// =========================================================================================
void setup() {
    Serial.begin(115200);
    
    // Give time for Serial monitor connection
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    Serial.println();
    Serial.println(F("========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — ESP32 Dual Gas Sensor Standalone Diagnostics  "));
    Serial.println(F(" Subsystem: Winsen MQ-137 (NH3) & Winsen MQ-136 (H2S) Test Harness      "));
    Serial.println(F(" Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           "));
    Serial.println(F("========================================================================"));
    Serial.println(F("[BOOT] Initializing ADC1 hardware conditioning..."));

    // 1. Hardware ADC Configuration (ADC1 Only)
    // 12-bit resolution: 0 - 4095
    analogReadResolution(12);
    // 11 dB attenuation gives measurable range from ~150 mV up to ~2600-3100 mV
    analogSetPinAttenuation(PIN_ADC_MQ137, ADC_11db);
    analogSetPinAttenuation(PIN_ADC_MQ136, ADC_11db);

    // 2. Load Persisted Calibration & Divider parameters from NVS
    loadConfigurationFromNVS();

    // 3. Create FreeRTOS Inter-Task Communication Queues
    // Frame queue depth: 4 frames (holds 4 seconds of telemetry buffer)
    xQueueMQFrames = xQueueCreate(4, sizeof(MQDataFrame));
    if (xQueueMQFrames == NULL) {
        Serial.println(F("[FATAL] Failed to create xQueueMQFrames! Insufficient heap."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // Command queue depth: 4 commands
    xQueueMQCommands = xQueueCreate(4, sizeof(MQCommand));
    if (xQueueMQCommands == NULL) {
        Serial.println(F("[FATAL] Failed to create xQueueMQCommands! Insufficient heap."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 4. Spawn Tasks with explicit Core Affinity and Priorities
    // TaskMQSampler: Core 0, Priority 2 (Single owner of ADC bus & DSP filtering)
    BaseType_t retSampler = xTaskCreatePinnedToCore(
        TaskMQSampler,
        "TaskMQSampler",
        SAMPLER_STACK_WORDS,
        NULL,
        SAMPLER_TASK_PRIORITY,
        &xHandleMQSampler,
        SAMPLER_TASK_CORE
    );

    // TaskMQLogger: Core 0, Priority 1 (Serial presentation and non-blocking CLI)
    BaseType_t retLogger = xTaskCreatePinnedToCore(
        TaskMQLogger,
        "TaskMQLogger",
        LOGGER_STACK_WORDS,
        NULL,
        LOGGER_TASK_PRIORITY,
        &xHandleMQLogger,
        LOGGER_TASK_CORE
    );

    if (retSampler != pdPASS || retLogger != pdPASS) {
        Serial.println(F("[FATAL] FreeRTOS Task creation failed! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Serial.println(F("[BOOT] All tasks spawned successfully on Core 0."));
    Serial.println(F("[BOOT] Type 'help' in Serial Monitor for interactive CLI commands."));
    Serial.println(F("========================================================================"));
}

// =========================================================================================
// 6. ARDUINO LOOP (YIELD TO SCHEDULER)
// =========================================================================================
void loop() {
    // In Arduino-ESP32, loop() runs inside loopTask on Core 1 by default.
    // Core 1 is reserved for future LoRa radio tasks.
    // We strictly delay loop() to avoid wasting CPU cycles in an idle spinlock.
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// =========================================================================================
// 7. TASK: MQ SAMPLER (CORE 0, PRIORITY 2)
// =========================================================================================
static void TaskMQSampler(void* pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(FRAME_PERIOD_MS);

    uint32_t frame_seq = 0;
    float ema_mq137 = -1.0f;
    float ema_mq136 = -1.0f;

    MQDataFrame frame;

    for (;;) {
        // Precise periodic timing using vTaskDelayUntil (no drift)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        frame_seq++;

        // 1. Process any incoming command from logger CLI
        MQCommand cmd;
        if (xQueueReceive(xQueueMQCommands, &cmd, 0) == pdTRUE) {
            handleIncomingCommand(&cmd);
        }

        // 2. Populate Frame Metadata
        frame.frame_id = frame_seq;
        frame.timestamp_ms = (uint64_t)(esp_timer_get_time() / 1000ULL);

        // 3. Sample Sensor 1: MQ137 (Ammonia)
        processChannelSampling(&config137, &frame.mq137, &ema_mq137);

        // 4. Sample Sensor 2: MQ136 (Hydrogen Sulfide)
        processChannelSampling(&config136, &frame.mq136, &ema_mq136);

        // 5. Handle Active Clean-Air Baseline Calibration State Machine
        if (calState.active) {
            MQChannelReading* targetReading = (calState.target_sensor == 137) ? &frame.mq137 : &frame.mq136;
            MQChannelConfig* targetCfg = (calState.target_sensor == 137) ? &config137 : &config136;

            // Calibration requires that circuit (RL & divider) is valid and AO is measurable
            if ((targetReading->status_flags & (MQ_FLAG_DIVIDER_OK | MQ_FLAG_RL_OK)) && targetReading->rs_ohm > 0.0f) {
                calState.last_rs_readings[calState.frame_count] = targetReading->rs_ohm;
                calState.rs_accumulator += targetReading->rs_ohm;
                calState.frame_count++;

                Serial.printf("[CAL] Sensor %s: Step %u/10, Current Rs = %.1f Ohm\n",
                              targetCfg->gas_name, calState.frame_count, targetReading->rs_ohm);

                if (calState.frame_count >= 10) {
                    // Compute mean and standard deviation of 10 calibration frames
                    float rs_mean = calState.rs_accumulator / 10.0f;
                    float sum_sq_diff = 0.0f;
                    for (int i = 0; i < 10; i++) {
                        float diff = calState.last_rs_readings[i] - rs_mean;
                        sum_sq_diff += diff * diff;
                    }
                    float rs_stddev = sqrtf(sum_sq_diff / 10.0f);
                    float coefficient_of_variation = (rs_stddev / rs_mean) * 100.0f;

                    // Stability threshold: Coefficient of variation must be < 5.0%
                    if (coefficient_of_variation < 5.0f) {
                        // Formula per datasheet: In clean air, Rs/R0 = clean_air_ratio (~3.6)
                        // Therefore: R0 = Rs_clean_air / clean_air_ratio
                        targetCfg->r0_clean_air_ohm = rs_mean / targetCfg->clean_air_ratio;
                        targetCfg->is_r0_valid = true;

                        Serial.printf("\n>>> [CAL SUCCESS] %s Calibration Complete!\n", targetCfg->gas_name);
                        Serial.printf("    Rs(clean air mean) = %.1f Ohm, StdDev = %.1f Ohm (%.2f%%)\n",
                                      rs_mean, rs_stddev, coefficient_of_variation);
                        Serial.printf("    Calculated Baseline R0 = %.1f Ohm (using ratio %.2f)\n",
                                      targetCfg->r0_clean_air_ohm, targetCfg->clean_air_ratio);
                        Serial.println(F("    Type 'save' to commit new calibration to NVS flash.\n"));
                    } else {
                        Serial.printf("\n>>> [CAL REJECTED] %s Readings too unstable! CV = %.2f%% (> 5%% limit).\n",
                                      targetCfg->gas_name, coefficient_of_variation);
                        Serial.println(F("    Please ensure sensor is preheated and in still, clean air.\n"));
                    }
                    calState.active = false;
                }
            } else {
                Serial.printf("[CAL ERROR] Cannot calibrate %s: RL or Divider not configured or AO zero!\n",
                              targetCfg->gas_name);
                calState.active = false;
            }
        }

        // 6. Capture Stack High Water Mark (measured in words on Xtensa)
        frame.sampler_stack_watermark_words = uxTaskGetStackHighWaterMark(NULL);
        if (xHandleMQLogger != NULL) {
            frame.logger_stack_watermark_words = uxTaskGetStackHighWaterMark(xHandleMQLogger);
        } else {
            frame.logger_stack_watermark_words = 0;
        }

        // 7. Enqueue Snapshot Frame to Logger Task (Drop oldest if queue full to prevent stall)
        if (xQueueSend(xQueueMQFrames, &frame, 0) != pdTRUE) {
            // Queue full: dequeue one old frame and enqueue newest
            MQDataFrame dummy;
            xQueueReceive(xQueueMQFrames, &dummy, 0);
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

    // 1. Raw Statistics
    float raw_mean = (float)raw_sum / (float)SAMPLES_PER_FRAME;
    reading->adc_raw_min = sample_min;
    reading->adc_raw_max = sample_max;
    reading->v_adc_mv = (float)mv_sum / (float)SAMPLES_PER_FRAME;

    // Check for ADC Saturation
    if (sample_max >= 4095) {
        reading->status_flags |= MQ_FLAG_ADC_SATURATED;
    }

    // Standard deviation of raw samples in burst
    float sq_diff_sum = 0.0f;
    for (int i = 0; i < SAMPLES_PER_FRAME; i++) {
        float diff = (float)samples[i] - raw_mean;
        sq_diff_sum += diff * diff;
    }
    reading->adc_raw_stddev = sqrtf(sq_diff_sum / (float)SAMPLES_PER_FRAME);

    // 2. Exponential Moving Average (EMA) Smoothing
    if (*ema_val < 0.0f) {
        *ema_val = raw_mean; // Initialize on first frame
    } else {
        *ema_val = (EMA_ALPHA * raw_mean) + ((1.0f - EMA_ALPHA) * (*ema_val));
    }
    reading->adc_raw_avg = (uint16_t)roundf(*ema_val);
    reading->status_flags |= MQ_FLAG_RAW_OK;

    // 3. Voltage Reconstruction (V_AO = V_ADC / k)
    if (cfg->is_divider_valid && cfg->divider_k > 0.001f) {
        reading->status_flags |= MQ_FLAG_DIVIDER_OK;
        reading->v_ao_mv = reading->v_adc_mv / cfg->divider_k;
    } else {
        reading->v_ao_mv = MQ_VALUE_UNCONFIGURED;
    }

    // 4. Sensor Resistance Calculation (Rs)
    // Formula: Rs = ((Vc / V_AO) - 1) * RL_eff
    // Accounting for parallel divider loading:
    // RL_eff = (RL * (R_top + R_bottom)) / (RL + R_top + R_bottom)
    if ((reading->status_flags & MQ_FLAG_DIVIDER_OK) && cfg->is_rl_valid && cfg->rl_nominal_ohm > 0.0f) {
        reading->status_flags |= MQ_FLAG_RL_OK;

        float v_ao_volts = reading->v_ao_mv / 1000.0f;
        float r_divider_total = cfg->r_top_ohm + cfg->r_bottom_ohm;
        float rl_eff = (cfg->rl_nominal_ohm * r_divider_total) / (cfg->rl_nominal_ohm + r_divider_total);

        if (v_ao_volts >= 0.050f && v_ao_volts < (V_LOOP_SUPPLY_VOLTS - 0.020f)) {
            reading->rs_ohm = ((V_LOOP_SUPPLY_VOLTS / v_ao_volts) - 1.0f) * rl_eff;
        } else if (v_ao_volts >= (V_LOOP_SUPPLY_VOLTS - 0.020f)) {
            reading->rs_ohm = 0.0f; // Saturated maximum conductivity
        } else {
            reading->rs_ohm = 1e6f; // High resistance / near zero conductivity
        }
    } else {
        reading->rs_ohm = MQ_VALUE_UNCONFIGURED;
    }

    // 5. Ratio and Gas Estimation (PPM)
    // Strict Zero-Trust condition: PPM is ONLY valid if R0 is calibrated and Rs is computed
    if ((reading->status_flags & MQ_FLAG_RL_OK) && cfg->is_r0_valid && cfg->r0_clean_air_ohm > 0.0f && reading->rs_ohm > 0.0f) {
        reading->status_flags |= MQ_FLAG_R0_CALIBRATED;
        reading->ratio_rs_r0 = reading->rs_ohm / cfg->r0_clean_air_ohm;

        // Power-law formula: ppm = A * (Rs/R0)^B
        float calc_ppm = cfg->curve_a * powf(reading->ratio_rs_r0, cfg->curve_b);

        // Sanity boundaries per datasheet nominal limits
        if (calc_ppm < 0.1f) calc_ppm = 0.1f;
        reading->ppm_estimate = calc_ppm;
        reading->status_flags |= MQ_FLAG_PPM_VALID;
    } else {
        reading->ratio_rs_r0 = MQ_VALUE_UNCONFIGURED;
        reading->ppm_estimate = MQ_VALUE_UNCONFIGURED;
    }
}

// =========================================================================================
// 9. TASK: MQ LOGGER & CLI COMMAND PROCESSOR (CORE 0, PRIORITY 1)
// =========================================================================================
static void TaskMQLogger(void* pvParameters) {
    (void)pvParameters;

    char serial_rx_buf[64];
    uint8_t rx_idx = 0;
    MQDataFrame frame;

    for (;;) {
        // 1. Finite-timeout wait for telemetry frame from Sampler (100 ms)
        if (xQueueReceive(xQueueMQFrames, &frame, pdMS_TO_TICKS(100)) == pdTRUE) {
            uint32_t uptime_s = (uint32_t)(frame.timestamp_ms / 1000ULL);
            uint32_t hours = uptime_s / 3600;
            uint32_t minutes = (uptime_s % 3600) / 60;
            uint32_t seconds = uptime_s % 60;

            Serial.printf("----------------------------------------------------------------------------------------\n");
            Serial.printf("[FRAME #%05u] Monotonic Uptime: %02uh:%02um:%02us (%llu ms)\n",
                          frame.frame_id, hours, minutes, seconds, frame.timestamp_ms);

            // Channel 1: MQ137 (Ammonia)
            Serial.printf("  CH1 [MQ137-NH3]: ADC_raw = %4u (min:%4u, max:%4u, dev:%4.1f) | V_pin = %4.0f mV\n",
                          frame.mq137.adc_raw_avg, frame.mq137.adc_raw_min, frame.mq137.adc_raw_max,
                          frame.mq137.adc_raw_stddev, frame.mq137.v_adc_mv);
            if (frame.mq137.v_ao_mv > 0.0f) {
                Serial.printf("                 Reconstructed V_AO = %4.0f mV (k=%.2f)",
                              frame.mq137.v_ao_mv, config137.divider_k);
            } else {
                Serial.print(F("                 Reconstructed V_AO = [DIVIDER_NOT_SET]"));
            }

            if (frame.mq137.status_flags & MQ_FLAG_RL_OK) {
                Serial.printf(" | Rs = %6.0f Ohm", frame.mq137.rs_ohm);
            } else {
                Serial.print(F(" | Rs = [RL_UNCONFIGURED]"));
            }

            if (frame.mq137.status_flags & MQ_FLAG_PPM_VALID) {
                Serial.printf(" | Ratio = %4.2f | Est. NH3 = %6.2f ppm\n",
                              frame.mq137.ratio_rs_r0, frame.mq137.ppm_estimate);
            } else {
                Serial.print(F(" | Est. NH3 = [PPM_NOT_CALIBRATED]\n"));
            }

            // Channel 2: MQ136 (Hydrogen Sulfide)
            Serial.printf("  CH2 [MQ136-H2S]: ADC_raw = %4u (min:%4u, max:%4u, dev:%4.1f) | V_pin = %4.0f mV\n",
                          frame.mq136.adc_raw_avg, frame.mq136.adc_raw_min, frame.mq136.adc_raw_max,
                          frame.mq136.adc_raw_stddev, frame.mq136.v_adc_mv);
            if (frame.mq136.v_ao_mv > 0.0f) {
                Serial.printf("                 Reconstructed V_AO = %4.0f mV (k=%.2f)",
                              frame.mq136.v_ao_mv, config136.divider_k);
            } else {
                Serial.print(F("                 Reconstructed V_AO = [DIVIDER_NOT_SET]"));
            }

            if (frame.mq136.status_flags & MQ_FLAG_RL_OK) {
                Serial.printf(" | Rs = %6.0f Ohm", frame.mq136.rs_ohm);
            } else {
                Serial.print(F(" | Rs = [RL_UNCONFIGURED]"));
            }

            if (frame.mq136.status_flags & MQ_FLAG_PPM_VALID) {
                Serial.printf(" | Ratio = %4.2f | Est. H2S = %6.2f ppm\n",
                              frame.mq136.ratio_rs_r0, frame.mq136.ppm_estimate);
            } else {
                Serial.print(F(" | Est. H2S = [PPM_NOT_CALIBRATED]\n"));
            }

            // Diagnostic Warnings & FreeRTOS Resource Stats
            if (frame.mq137.status_flags & MQ_FLAG_PREHEAT_WARN) {
                Serial.print(F("  [NOTE] Preheat duration < 48 hours. Values may drift until fully conditioned.\n"));
            }
            if ((frame.mq137.status_flags | frame.mq136.status_flags) & MQ_FLAG_ADC_SATURATED) {
                Serial.print(F("  [WARNING] ADC input is saturated (raw >= 4095)! Check divider ratio.\n"));
            }

            Serial.printf("  [RTOS] Stack High Water Mark: Sampler = %u words (%u B), Logger = %u words (%u B)\n",
                          frame.sampler_stack_watermark_words, frame.sampler_stack_watermark_words * 4,
                          frame.logger_stack_watermark_words, frame.logger_stack_watermark_words * 4);
        }

        // 2. Non-blocking Serial Command CLI Parser
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
// 10. NON-BLOCKING CLI COMMAND PARSING & DISPATCH
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
    } else if (strcasecmp(line, "status") == 0) {
        printSystemStatus();
    } else if (strcasecmp(line, "config") == 0) {
        printActiveConfig();
    } else if (strcasecmp(line, "cal137") == 0) {
        cmd.type = CMD_CALIBRATE_MQ137;
        xQueueSend(xQueueMQCommands, &cmd, 0);
    } else if (strcasecmp(line, "cal136") == 0) {
        cmd.type = CMD_CALIBRATE_MQ136;
        xQueueSend(xQueueMQCommands, &cmd, 0);
    } else if (strncasecmp(line, "setdiv137", 9) == 0) {
        float rtop = 0.0f, rbot = 0.0f;
        if (sscanf(line + 9, "%f %f", &rtop, &rbot) == 2 && (rtop + rbot) > 0.0f) {
            cmd.type = CMD_SET_DIVIDER_MQ137;
            cmd.param1 = rtop;
            cmd.param2 = rbot;
            xQueueSend(xQueueMQCommands, &cmd, 0);
        } else {
            Serial.println(F("[ERROR] Usage: setdiv137 <R_TOP_OHM> <R_BOTTOM_OHM> (e.g. setdiv137 10000 15000)"));
        }
    } else if (strncasecmp(line, "setdiv136", 9) == 0) {
        float rtop = 0.0f, rbot = 0.0f;
        if (sscanf(line + 9, "%f %f", &rtop, &rbot) == 2 && (rtop + rbot) > 0.0f) {
            cmd.type = CMD_SET_DIVIDER_MQ136;
            cmd.param1 = rtop;
            cmd.param2 = rbot;
            xQueueSend(xQueueMQCommands, &cmd, 0);
        } else {
            Serial.println(F("[ERROR] Usage: setdiv136 <R_TOP_OHM> <R_BOTTOM_OHM> (e.g. setdiv136 10000 15000)"));
        }
    } else if (strncasecmp(line, "setrl137", 8) == 0) {
        float rl = 0.0f;
        if (sscanf(line + 8, "%f", &rl) == 1 && rl > 0.0f) {
            cmd.type = CMD_SET_RL_MQ137;
            cmd.param1 = rl;
            xQueueSend(xQueueMQCommands, &cmd, 0);
        } else {
            Serial.println(F("[ERROR] Usage: setrl137 <RL_OHM> (e.g. setrl137 4700 or setrl137 10000)"));
        }
    } else if (strncasecmp(line, "setrl136", 8) == 0) {
        float rl = 0.0f;
        if (sscanf(line + 8, "%f", &rl) == 1 && rl > 0.0f) {
            cmd.type = CMD_SET_RL_MQ136;
            cmd.param1 = rl;
            xQueueSend(xQueueMQCommands, &cmd, 0);
        } else {
            Serial.println(F("[ERROR] Usage: setrl136 <RL_OHM> (e.g. setrl136 4700 or setrl136 10000)"));
        }
    } else if (strcasecmp(line, "save") == 0) {
        saveConfigurationToNVS();
    } else if (strcasecmp(line, "resetcal") == 0) {
        cmd.type = CMD_RESET_CALIBRATION;
        xQueueSend(xQueueMQCommands, &cmd, 0);
    } else {
        Serial.printf("[ERROR] Unknown command '%s'. Type 'help' for available commands.\n", line);
    }
}

// =========================================================================================
// 11. SAMPLER-SIDE COMMAND EXECUTION (THREAD-SAFE STATE UPDATES)
// =========================================================================================
static void handleIncomingCommand(const MQCommand* cmd) {
    switch (cmd->type) {
        case CMD_CALIBRATE_MQ137:
            if (!config137.is_rl_valid) {
                Serial.println(F("[CAL ABORTED] Please configure RL first using: setrl137 <ohm>"));
            } else {
                calState.active = true;
                calState.target_sensor = 137;
                calState.frame_count = 0;
                calState.rs_accumulator = 0.0f;
                Serial.println(F("[CAL] Starting 10-second baseline calibration for MQ137 in clean air..."));
            }
            break;

        case CMD_CALIBRATE_MQ136:
            if (!config136.is_rl_valid) {
                Serial.println(F("[CAL ABORTED] Please configure RL first using: setrl136 <ohm>"));
            } else {
                calState.active = true;
                calState.target_sensor = 136;
                calState.frame_count = 0;
                calState.rs_accumulator = 0.0f;
                Serial.println(F("[CAL] Starting 10-second baseline calibration for MQ136 in clean air..."));
            }
            break;

        case CMD_SET_DIVIDER_MQ137:
            config137.r_top_ohm = cmd->param1;
            config137.r_bottom_ohm = cmd->param2;
            config137.divider_k = cmd->param2 / (cmd->param1 + cmd->param2);
            config137.is_divider_valid = true;
            Serial.printf("[CONFIG] MQ137 Divider Updated: R_top=%.1f Ohm, R_bot=%.1f Ohm -> k = %.4f\n",
                          config137.r_top_ohm, config137.r_bottom_ohm, config137.divider_k);
            break;

        case CMD_SET_DIVIDER_MQ136:
            config136.r_top_ohm = cmd->param1;
            config136.r_bottom_ohm = cmd->param2;
            config136.divider_k = cmd->param2 / (cmd->param1 + cmd->param2);
            config136.is_divider_valid = true;
            Serial.printf("[CONFIG] MQ136 Divider Updated: R_top=%.1f Ohm, R_bot=%.1f Ohm -> k = %.4f\n",
                          config136.r_top_ohm, config136.r_bottom_ohm, config136.divider_k);
            break;

        case CMD_SET_RL_MQ137:
            config137.rl_nominal_ohm = cmd->param1;
            config137.is_rl_valid = true;
            Serial.printf("[CONFIG] MQ137 Breakout RL set to %.1f Ohm.\n", config137.rl_nominal_ohm);
            break;

        case CMD_SET_RL_MQ136:
            config136.rl_nominal_ohm = cmd->param1;
            config136.is_rl_valid = true;
            Serial.printf("[CONFIG] MQ136 Breakout RL set to %.1f Ohm.\n", config136.rl_nominal_ohm);
            break;

        case CMD_RESET_CALIBRATION:
            resetCalibrationInNVS();
            break;

        default:
            break;
    }
}

// =========================================================================================
// 12. NVS PERSISTENCE (NON-VOLATILE STORAGE VIA PREFERENCES)
// =========================================================================================
static void loadConfigurationFromNVS() {
    prefs.begin(NVS_NAMESPACE, true); // Read-only mode
    uint32_t magic = prefs.getUInt("magic", 0);

    if (magic == NVS_MAGIC_HEADER) {
        Serial.println(F("[NVS] Valid calibration record found. Loading parameters..."));

        config137.r_top_ohm = prefs.getFloat("r_top_137", config137.r_top_ohm);
        config137.r_bottom_ohm = prefs.getFloat("r_bot_137", config137.r_bottom_ohm);
        config137.divider_k = prefs.getFloat("k_137", config137.divider_k);
        config137.rl_nominal_ohm = prefs.getFloat("rl_137", config137.rl_nominal_ohm);
        config137.r0_clean_air_ohm = prefs.getFloat("r0_137", config137.r0_clean_air_ohm);
        config137.is_rl_valid = (config137.rl_nominal_ohm > 0.0f);
        config137.is_r0_valid = (config137.r0_clean_air_ohm > 0.0f);

        config136.r_top_ohm = prefs.getFloat("r_top_136", config136.r_top_ohm);
        config136.r_bottom_ohm = prefs.getFloat("r_bot_136", config136.r_bottom_ohm);
        config136.divider_k = prefs.getFloat("k_136", config136.divider_k);
        config136.rl_nominal_ohm = prefs.getFloat("rl_136", config136.rl_nominal_ohm);
        config136.r0_clean_air_ohm = prefs.getFloat("r0_136", config136.r0_clean_air_ohm);
        config136.is_rl_valid = (config136.rl_nominal_ohm > 0.0f);
        config136.is_r0_valid = (config136.r0_clean_air_ohm > 0.0f);
    } else {
        Serial.println(F("[NVS] No prior calibration record found. Using factory diagnostic defaults."));
    }
    prefs.end();
}

static void saveConfigurationToNVS() {
    prefs.begin(NVS_NAMESPACE, false); // Read-write mode
    prefs.putUInt("magic", NVS_MAGIC_HEADER);
    prefs.putUInt("version", NVS_VERSION);

    prefs.putFloat("r_top_137", config137.r_top_ohm);
    prefs.putFloat("r_bot_137", config137.r_bottom_ohm);
    prefs.putFloat("k_137", config137.divider_k);
    prefs.putFloat("rl_137", config137.rl_nominal_ohm);
    prefs.putFloat("r0_137", config137.r0_clean_air_ohm);

    prefs.putFloat("r_top_136", config136.r_top_ohm);
    prefs.putFloat("r_bot_136", config136.r_bottom_ohm);
    prefs.putFloat("k_136", config136.divider_k);
    prefs.putFloat("rl_136", config136.rl_nominal_ohm);
    prefs.putFloat("r0_136", config136.r0_clean_air_ohm);

    prefs.end();
    Serial.println(F("[NVS] All active configuration & calibration parameters saved successfully!"));
}

static void resetCalibrationInNVS() {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.clear();
    prefs.end();

    config137.rl_nominal_ohm = 0.0f;
    config137.r0_clean_air_ohm = -1.0f;
    config137.is_rl_valid = false;
    config137.is_r0_valid = false;

    config136.rl_nominal_ohm = 0.0f;
    config136.r0_clean_air_ohm = -1.0f;
    config136.is_rl_valid = false;
    config136.is_r0_valid = false;

    Serial.println(F("[NVS] Calibration cleared from flash! Returned to diagnostic uncalibrated state."));
}

// =========================================================================================
// 13. INFORMATIONAL CLI HELPERS
// =========================================================================================
static void printCommandHelp() {
    Serial.println(F("\n========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — Serial CLI Command Reference                   "));
    Serial.println(F("========================================================================"));
    Serial.println(F("  help                      : Display this reference menu"));
    Serial.println(F("  status                    : Show RTOS runtime metrics, memory, and stack"));
    Serial.println(F("  config                    : Print active circuit & calibration coefficients"));
    Serial.println(F("  setdiv137 <rtop> <rbot>   : Configure divider resistors for MQ137 (Ohm)"));
    Serial.println(F("  setdiv136 <rtop> <rbot>   : Configure divider resistors for MQ136 (Ohm)"));
    Serial.println(F("  setrl137 <ohm>            : Configure breakout load resistor RL for MQ137"));
    Serial.println(F("  setrl136 <ohm>            : Configure breakout load resistor RL for MQ136"));
    Serial.println(F("  cal137                    : Perform 10-second clean-air baseline (R0) cal"));
    Serial.println(F("  cal136                    : Perform 10-second clean-air baseline (R0) cal"));
    Serial.println(F("  save                      : Commit active configuration & R0 to NVS flash"));
    Serial.println(F("  resetcal                  : Erase NVS flash and reset to default diagnostics"));
    Serial.println(F("========================================================================\n"));
}

static void printSystemStatus() {
    uint32_t uptime_s = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();

    Serial.println(F("\n--- [SYSTEM & RTOS STATUS] ---"));
    Serial.printf("  Uptime            : %u seconds (%u min, %u hrs)\n",
                  uptime_s, uptime_s / 60, uptime_s / 3600);
    Serial.printf("  Free Heap         : %u bytes (Historic Min: %u bytes)\n",
                  free_heap, min_free_heap);
    Serial.printf("  Core Allocation   : TaskMQSampler -> Core %d, TaskMQLogger -> Core %d\n",
                  SAMPLER_TASK_CORE, LOGGER_TASK_CORE);
    if (xHandleMQSampler) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xHandleMQSampler);
        Serial.printf("  Sampler Stack HWM : %u words (%u bytes remaining)\n", hwm, hwm * 4);
    }
    if (xHandleMQLogger) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xHandleMQLogger);
        Serial.printf("  Logger Stack HWM  : %u words (%u bytes remaining)\n", hwm, hwm * 4);
    }
    Serial.println(F("------------------------------\n"));
}

static void printActiveConfig() {
    Serial.println(F("\n--- [ACTIVE CIRCUIT & CALIBRATION CONFIG] ---"));
    Serial.printf("  MQ137 (Ammonia):\n");
    Serial.printf("    Pin GPIO        : %u (ADC1_CH4)\n", config137.pin);
    Serial.printf("    Divider         : R_top = %.0f Ohm, R_bot = %.0f Ohm -> k = %.4f\n",
                  config137.r_top_ohm, config137.r_bottom_ohm, config137.divider_k);
    Serial.printf("    Breakout RL     : %s (%.0f Ohm)\n",
                  config137.is_rl_valid ? "CONFIGURED" : "UNCONFIGURED", config137.rl_nominal_ohm);
    Serial.printf("    Baseline R0     : %s (%.1f Ohm, clean-air ratio: %.2f)\n",
                  config137.is_r0_valid ? "CALIBRATED" : "NOT_CALIBRATED",
                  config137.r0_clean_air_ohm, config137.clean_air_ratio);
    Serial.printf("    Curve Fit Model : ppm = %.1f * (Rs/R0)^(%.3f)\n",
                  config137.curve_a, config137.curve_b);

    Serial.printf("  MQ136 (Hydrogen Sulfide):\n");
    Serial.printf("    Pin GPIO        : %u (ADC1_CH5)\n", config136.pin);
    Serial.printf("    Divider         : R_top = %.0f Ohm, R_bot = %.0f Ohm -> k = %.4f\n",
                  config136.r_top_ohm, config136.r_bottom_ohm, config136.divider_k);
    Serial.printf("    Breakout RL     : %s (%.0f Ohm)\n",
                  config136.is_rl_valid ? "CONFIGURED" : "UNCONFIGURED", config136.rl_nominal_ohm);
    Serial.printf("    Baseline R0     : %s (%.1f Ohm, clean-air ratio: %.2f)\n",
                  config136.is_r0_valid ? "CALIBRATED" : "NOT_CALIBRATED",
                  config136.r0_clean_air_ohm, config136.clean_air_ratio);
    Serial.printf("    Curve Fit Model : ppm = %.1f * (Rs/R0)^(%.3f)\n",
                  config136.curve_a, config136.curve_b);
    Serial.println(F("--------------------------------------------\n"));
}
