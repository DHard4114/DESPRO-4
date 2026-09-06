#include <Arduino.h>
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
QueueHandle_t xQueueSensorData;
QueueHandle_t xQueueCommand;
esp_pm_lock_handle_t active_lock;

SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, PIN_LORA_MISO);
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
        TelemetryPayload payload = {0};
        payload.schema_version = 1;
        strncpy(payload.node_code, NODE_CODE, sizeof(payload.node_code) - 1);
        payload.sequence_no = ++global_sequence_no;
        payload.sos_triggered = 0;

        // Baca Ultrasonik
        unsigned int dist = sonar.ping_cm();
        
        // Kompensasi Blind Zone JSN-SR04T (< 25cm = Penuh)
        if (dist < 25 || dist == 0) {
            dist = 25;
        }
        payload.water_level_cm = (float)dist;

        // Dummy/Raw bacaan ADC untuk Gas & Baterai
        payload.ammonia_ppm = (float)analogRead(PIN_MQ137_AO) * 0.1;
        payload.h2s_ppm = (float)analogRead(PIN_MQ136_AO) * 0.1;
        payload.battery_voltage = (float)analogRead(PIN_BATT_VOLT) * (3.3 / 4095.0) * 2.0;

        // Kirim ke LoRa Task
        xQueueSend(xQueueSensorData, &payload, portMAX_DELAY);

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
            
            // Acquire Power Lock untuk stabilitas Clock/SPI [ADR-09]
            esp_pm_lock_acquire(active_lock);
            
            // Jeda Stabilisasi PLL Clock [ADR-09]
            vTaskDelay(pdMS_TO_TICKS(10));

            // Transmisi LoRa Biner (Sinkron/Blocking sementara untuk keandalan awal)
            int state = radio.transmit((uint8_t*)&txData, sizeof(TelemetryPayload));
            
            if (state == RADIOLIB_ERR_NONE) {
                // Berhasil Tx. Buka RX Window 2000ms (Mekanisme Class A)
                radio.startReceive();
                
                TickType_t rx_start = xTaskGetTickCount();
                bool received = false;
                ActuatorCommand rxCmd;
                
                while ((xTaskGetTickCount() - rx_start) * portTICK_PERIOD_MS < 2000) {
                    if (radio.readData((uint8_t*)&rxCmd, sizeof(ActuatorCommand)) == RADIOLIB_ERR_NONE) {
                        received = true;
                        break;
                    }
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                radio.standby();

                if (received) {
                    xQueueSend(xQueueCommand, &rxCmd, portMAX_DELAY);
                }
            }

            // Release Power Lock agar ESP32 bisa Light-Sleep [ADR-09]
            esp_pm_lock_release(active_lock);
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
            // Aktuator tidak diputar dari ISR [ADR-01]
            valveServo.write(cmd.angle);
        }
    }
}

// ==========================================
// INIT (SETUP)
// ==========================================
void setup() {
    Serial.begin(115200);
    
    // Inisialisasi Power Management (Tickless Idle & DFS) [ADR-09]
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "active_lock", &active_lock);

    // Inisialisasi Queues
    xQueueSensorData = xQueueCreate(10, sizeof(TelemetryPayload));
    xQueueCommand = xQueueCreate(5, sizeof(ActuatorCommand));

    // Inisialisasi Hardware
    valveServo.attach(PIN_SERVO);
    pinMode(PIN_BTN_SOS, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_SOS), isr_sos_button, FALLING);

    // Inisialisasi Radio
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("LoRa INIT FAILED!");
        while (true) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Pembuatan Task FreeRTOS sesuai Pemetaan Topologi Task [ADR-01]
    xTaskCreatePinnedToCore(vTaskSensors,  "TaskSensors",  2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskActuator, "TaskActuator", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLoRaTx,   "TaskLoRaTx",   4096, NULL, 3, NULL, 1);
}

void loop() {
    // Loop kosong, di-delete agar memory hemat [ADR-01]
    vTaskDelete(NULL);
}

#endif // IS_NODE_WC
