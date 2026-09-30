#include <Arduino.h>
#include <SPI.h>
#include "config.h"

#ifdef IS_NODE_WC

#include <RadioLib.h>
#include <NewPing.h>
#include <ESP32Servo.h>
#include "esp_pm.h"
#include "esp_sleep.h"

// ==========================================
// GLOBAL HANDLES & VARIABLES
// ==========================================
QueueHandle_t xQueueSensorData = NULL;
QueueHandle_t xQueueCommand = NULL;
esp_pm_lock_handle_t active_lock = NULL;

// Koreksi RadioLib: Argumen ke-4 Module(cs, irq, rst, gpio) adalah GPIO tambahan (DIO1),
// BUKAN MISO! Gunakan RADIOLIB_NC. MISO dikonfigurasi via bus SPIClass (oper objek SPI eksplisit).
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI);
NewPing sonar(PIN_TRIG_US, PIN_ECHO_US, 400);
Servo valveServo;

uint32_t global_sequence_no = 0;
volatile TickType_t last_sos_time = 0;

// ==========================================
// ISR: TOMBOL SOS (EXTI)
// ==========================================
void IRAM_ATTR isr_sos_button() {
    TickType_t now = xTaskGetTickCountFromISR();
    // Software Debouncing 300ms [ADR-09]
    if ((now - last_sos_time) * portTICK_PERIOD_MS > 300) {
        last_sos_time = now;
        
        TelemetryPayload alert = {0};
        alert.sos_triggered = 1;
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        // Bypass antrean normal, kirim ke depan antrean
        xQueueSendToFrontFromISR(xQueueSensorData, &alert, &xHigherPriorityTaskWoken);
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

// ==========================================
// TASK: PEMBACAAN SENSOR (Core 0, Prio 1)
// ==========================================
void vTaskSensors(void *pvParameters) {
    for (;;) {
        TelemetryPayload payload;
        memset(&payload, 0, sizeof(TelemetryPayload));

        payload.schema_version = 1;
        strncpy(payload.node_code, NODE_CODE, sizeof(payload.node_code) - 1);
        payload.sequence_no = ++global_sequence_no;
        payload.uptime_seconds = (uint32_t)(millis() / 1000);
        payload.sos_triggered = 0;

        // Baca Ultrasonik
        unsigned int dist = sonar.ping_cm();
        
        // Kompensasi Blind Zone JSN-SR04T (< 25cm = Penuh)
        if (dist < 25 || dist == 0) {
            dist = 25;
        }
        payload.water_level_cm = (float)dist;

        // Dummy/Raw bacaan ADC untuk Gas & Baterai
        payload.ammonia_ppm = (float)analogRead(PIN_MQ137_AO) * 0.1f;
        payload.h2s_ppm = (float)analogRead(PIN_MQ136_AO) * 0.1f;
        payload.battery_voltage = (float)analogRead(PIN_BATT_VOLT) * (3.3f / 4095.0f) * 2.0f;

        // Kirim ke LoRa Task
        if (xQueueSensorData != NULL) {
            xQueueSend(xQueueSensorData, &payload, portMAX_DELAY);
        }

        // Tidur 10 detik [DILARANG delay()]
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// ==========================================
// TASK: TRANSMISI LORA & RX WINDOW (Core 1, Prio 3)
// ==========================================
void vTaskLoRaTx(void *pvParameters) {
    TelemetryPayload txData;
    
    for (;;) {
        // Block menunggu data dari Queue
        if (xQueueReceive(xQueueSensorData, &txData, portMAX_DELAY) == pdTRUE) {
            
            #if defined(CONFIG_PM_ENABLE)
            // Acquire Power Lock untuk stabilitas Clock/SPI [ADR-09]
            if (active_lock != NULL) esp_pm_lock_acquire(active_lock);
            vTaskDelay(pdMS_TO_TICKS(10));
            #endif

            // Transmisi LoRa Biner (Sinkron/Blocking sementara untuk keandalan awal)
            int state = radio.transmit((uint8_t*)&txData, sizeof(TelemetryPayload));
            
            if (state == RADIOLIB_ERR_NONE) {
                // Berhasil Tx. Buka RX Window 2000ms (Mekanisme Class A)
                radio.startReceive();
                
                TickType_t rx_start = xTaskGetTickCount();
                bool received = false;
                ActuatorCommand rxCmd;
                
                // Tunggu instruksi aktuator dari server posko selama 2000 ms (LoRaWAN Class A RX Window)
                while ((xTaskGetTickCount() - rx_start) * portTICK_PERIOD_MS < 2000) {
                    if (radio.readData((uint8_t*)&rxCmd, sizeof(ActuatorCommand)) == RADIOLIB_ERR_NONE) {
                        // [ZERO-TRUST FILTER]: Hanya eksekusi jika ditujukan khusus untuk NODE_CODE ini!
                        if (strncmp(rxCmd.node_code, NODE_CODE, sizeof(rxCmd.node_code)) == 0) {
                            received = true;
                            Serial.printf("[LORA RX WINDOW] Menemukan komando valid untuk %s: CMD=%u, PARAM=%u\n",
                                          rxCmd.node_code, rxCmd.command_id, rxCmd.parameter);
                            break;
                        } else {
                            Serial.printf("[ZERO-TRUST] Mengabaikan komando untuk node lain: %s\n", rxCmd.node_code);
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                radio.standby(); // Kembali ke standby mode untuk efisiensi baterai

                if (received && xQueueCommand != NULL) {
                    xQueueSend(xQueueCommand, &rxCmd, portMAX_DELAY);
                }
            }

            #if defined(CONFIG_PM_ENABLE)
            // Release Power Lock agar ESP32 bisa Light-Sleep [ADR-09]
            if (active_lock != NULL) esp_pm_lock_release(active_lock);
            #endif
        }
    }
}

// ==========================================
// TASK: AKTUATOR (Core 0, Prio 2)
// ==========================================
void vTaskActuator(void *pvParameters) {
    ActuatorCommand cmd;
    for (;;) {
        if (xQueueReceive(xQueueCommand, &cmd, portMAX_DELAY) == pdTRUE) {
            Serial.printf("[TASK ACTUATOR] Eksekusi komando untuk %s: CMD=%u (Param=%u)\n", 
                          cmd.node_code, cmd.command_id, cmd.parameter);
            // 1 = LOCK_DOOR (Kunci Bilik WC)
            if (cmd.command_id == 1) {
                uint8_t angle = (cmd.parameter > 0) ? cmd.parameter : 90;
                valveServo.write(angle);
                Serial.printf("[TASK ACTUATOR] Pintu Dikunci! Sudut Servo: %u°\n", angle);
            } 
            // 2 = UNLOCK_DOOR (Buka Kunci Bilik WC)
            else if (cmd.command_id == 2) {
                uint8_t angle = (cmd.parameter > 0) ? cmd.parameter : 0;
                valveServo.write(angle);
                Serial.printf("[TASK ACTUATOR] Pintu Dibuka! Sudut Servo: %u°\n", angle);
            } 
            // 3 = FLUSH / Custom angle
            else {
                valveServo.write(cmd.parameter);
            }
        }
    }
}

// ==========================================
// INIT (SETUP)
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    #if defined(CONFIG_PM_ENABLE)
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "active_lock", &active_lock);
    #endif

    // Inisialisasi Queues
    xQueueSensorData = xQueueCreate(10, sizeof(TelemetryPayload));
    xQueueCommand = xQueueCreate(5, sizeof(ActuatorCommand));

    // Inisialisasi Hardware
    valveServo.attach(PIN_SERVO);
    pinMode(PIN_BTN_SOS, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_SOS), isr_sos_button, FALLING);
    // [STRATEGI 2: EVENT-DRIVEN WAKEUP] Daftarkan pin SOS sebagai pemicu bangun Light-Sleep seketika
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_SOS, 0); // 0 = Pemicu LOW level (saat tombol ditekan ke GND)

    // Inisialisasi Bus SPI secara eksplisit untuk ESP32 DevKit V1
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);

    // Inisialisasi Radio
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("LoRa INIT FAILED! Error code: %d\n", state);
        while (true) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Serial.println("LoRa INIT SUCCESS (433.175 MHz)!");
    radio.setCRC(true);

    // Pembuatan Task FreeRTOS sesuai Pemetaan Topologi Task [ADR-01]
    xTaskCreatePinnedToCore(vTaskSensors,  "TaskSensors",  3072, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskActuator, "TaskActuator", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLoRaTx,   "TaskLoRaTx",   4096, NULL, 3, NULL, 1);
}

void loop() {
    // Loop kosong, di-delete agar memory loopTask direklamasi oleh Idle Task [ADR-01]
    vTaskDelete(NULL);
}

#endif // IS_NODE_WC
