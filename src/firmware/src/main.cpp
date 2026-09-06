#include <Arduino.h>
#include "config.h"

// Task Handles
TaskHandle_t TaskLoRaHandle = NULL;
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskMqttHandle = NULL;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=========================================");
    Serial.printf("STARTING SYSTEM: %s\n", NODE_ID);
    Serial.println("=========================================");

#ifdef IS_NODE_WC
    Serial.println("MODE: NODE WC (LoRa Tx Only)");
    // TODO: Init FreeRTOS Queues (xQueueSensorData)
    // TODO: Init Sensors & Attach SOS Interrupt (DIO0, BTN_SOS)
    // TODO: Create vTaskSensors, vTaskLoRaTx, vTaskActuator
#elif defined(IS_GATEWAY)
    Serial.println("MODE: GATEWAY ROUTER (LoRa Rx + Wi-Fi/MQTT)");
    // TODO: Init FreeRTOS Queues (xQueueTelemetry, xQueueCommand)
    // TODO: Init LittleFS for Store-and-Forward Ring Buffer
    // TODO: Init Wi-Fi & MQTT
    // TODO: Create vTaskLoRaRx, vTaskMqttTx, vTaskMqttRx
#else
    #error "PlatformIO Environment is missing build flag IS_NODE_WC or IS_GATEWAY!"
#endif
}

void loop() {
    // Kosong. Arsitektur murni FreeRTOS.
    // Gunakan vTaskDelete(NULL) jika loop() tidak dipakai sama sekali untuk menghemat memori.
    vTaskDelay(portMAX_DELAY);
}
