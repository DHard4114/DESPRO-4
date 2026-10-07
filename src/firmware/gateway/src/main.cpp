#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <RadioLib.h>
#include <Wire.h>
#include <RTClib.h>
#include "config.h"

// =========================================================================================
// SMART-SANITATION eSOS — GATEWAY ROUTER FIRMWARE (ESP32 DevKit V1)
// Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
// Framework    : Arduino-ESP32 v2.0.17 (ESP-IDF v4.4) + Native Espressif FreeRTOS
// Hardware     : DOIT ESP32 DevKit V1, SX1278 (Ai-Thinker Ra-02), DS3231 RTC
// Architecture : Single Radio Owner (TaskLoRaRx), LittleFS Store-and-Forward ("r+" mode),
//                Thread-Safe MQTT with Mutex, Custom Half-Duplex Post-Uplink Downlink Window.
// =========================================================================================

// ==========================================
// 1. OBJEK PERANGKAT KERAS & KERNEL FREERTOS
// ==========================================
// Argumen ke-4 Module adalah RADIOLIB_NC, oper objek SPI eksplisit
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI);

RTC_DS3231 rtc;
static bool g_rtc_available = false;

WiFiClient espClient;
PubSubClient mqtt(espClient);

// Handle FreeRTOS
QueueHandle_t xQueueTelemetry = NULL;
TaskHandle_t  TaskLoRaRxHandle = NULL;
SemaphoreHandle_t fsMutex = NULL;
SemaphoreHandle_t mqttMutex = NULL;

// ==========================================
// 1B. CUSTOM HALF-DUPLEX POST-UPLINK DOWNLINK QUEUE
// ==========================================
// Catatan Arsitektur: Ini adalah mekanisme half-duplex point-to-point / star kustom
// yang terinspirasi oleh prinsip jeda jendela RX setelah uplink, bukan LoRaWAN Class A resmi.
struct PendingDownlink {
    ActuatorCommand cmd;
    bool active;
    uint32_t queued_at;
};

#define MAX_PENDING_NODES 8
static PendingDownlink pendingDownlinks[MAX_PENDING_NODES];
static SemaphoreHandle_t pendingMutex = NULL;

static void queuePendingDownlink(const ActuatorCommand &cmd) {
    if (pendingMutex == NULL) return;
    xSemaphoreTake(pendingMutex, portMAX_DELAY);
    int slot = -1;
    for (int i = 0; i < MAX_PENDING_NODES; i++) {
        if (pendingDownlinks[i].active && strncmp(pendingDownlinks[i].cmd.node_code, cmd.node_code, 8) == 0) {
            slot = i;
            break;
        } else if (!pendingDownlinks[i].active && slot == -1) {
            slot = i;
        }
    }
    if (slot != -1) {
        pendingDownlinks[slot].cmd = cmd;
        pendingDownlinks[slot].active = true;
        pendingDownlinks[slot].queued_at = (uint32_t)(millis() / 1000);
        Serial.printf("[DOWNLINK QUEUE] Komando tertunda tersimpan untuk %.8s (CMD=%u, PARAM=%u)\n", 
                      cmd.node_code, cmd.command_id, cmd.parameter);
    } else {
        Serial.println(F("[DOWNLINK QUEUE ALERT] Antrean pending downlink penuh! Komando dibuang."));
    }
    xSemaphoreGive(pendingMutex);
}

static bool popPendingDownlink(const char* node_code, ActuatorCommand *outCmd) {
    if (pendingMutex == NULL || node_code == NULL || outCmd == NULL) return false;
    xSemaphoreTake(pendingMutex, portMAX_DELAY);
    bool found = false;
    for (int i = 0; i < MAX_PENDING_NODES; i++) {
        if (pendingDownlinks[i].active && strncmp(pendingDownlinks[i].cmd.node_code, node_code, 8) == 0) {
            *outCmd = pendingDownlinks[i].cmd;
            pendingDownlinks[i].active = false;
            found = true;
            Serial.printf("[DOWNLINK DISPATCH] Komando tertunda diambil untuk %.8s (CMD=%u)\n", 
                          node_code, outCmd->command_id);
            break;
        }
    }
    xSemaphoreGive(pendingMutex);
    return found;
}

// ==========================================
// 2. LITTLEFS CIRCULAR BUFFER (Store-and-Forward) [ADR-03]
// ==========================================
#define MAX_BUFFER_RECORDS 500
#define FILE_META "/meta.dat"
#define FILE_DATA "/buffer.dat"
#define BUFFER_MAGIC 0x47574231 // 'GWB1'
#define BUFFER_VERSION 1

struct __attribute__((packed)) BufferMeta {
    uint32_t magic;
    uint16_t version;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint32_t dropped_count;
    uint32_t crc32;
};

static BufferMeta meta = {BUFFER_MAGIC, BUFFER_VERSION, 0, 0, 0, 0, 0};

static uint32_t calculateMetaCRC(const BufferMeta &m) {
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *data = (const uint8_t*)&m;
    size_t length = offsetof(BufferMeta, crc32);
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

static void saveMeta() {
    meta.magic = BUFFER_MAGIC;
    meta.version = BUFFER_VERSION;
    meta.crc32 = calculateMetaCRC(meta);
    File f = LittleFS.open(FILE_META, "w");
    if (f) {
        f.write((const uint8_t*)&meta, sizeof(BufferMeta));
        f.flush();
        f.close();
    } else {
        Serial.println(F("[LITTLEFS ERROR] Gagal menulis metadata buffer!"));
    }
}

static void initEmptyDataFile() {
    if (!LittleFS.exists(FILE_DATA)) {
        File f = LittleFS.open(FILE_DATA, "w");
        if (f) {
            f.close();
        }
    }
}

static void loadMeta() {
    initEmptyDataFile();
    bool valid = false;
    if (LittleFS.exists(FILE_META)) {
        File f = LittleFS.open(FILE_META, "r");
        if (f) {
            BufferMeta temp;
            if (f.read((uint8_t*)&temp, sizeof(BufferMeta)) == sizeof(BufferMeta)) {
                if (temp.magic == BUFFER_MAGIC && temp.version == BUFFER_VERSION &&
                    temp.crc32 == calculateMetaCRC(temp) &&
                    temp.head < MAX_BUFFER_RECORDS && temp.tail < MAX_BUFFER_RECORDS &&
                    temp.count <= MAX_BUFFER_RECORDS) {
                    meta = temp;
                    valid = true;
                }
            }
            f.close();
        }
    }
    if (!valid) {
        Serial.println(F("[LITTLEFS] Metadata buffer tidak valid atau baru, inisialisasi awal..."));
        meta.magic = BUFFER_MAGIC;
        meta.version = BUFFER_VERSION;
        meta.head = 0;
        meta.tail = 0;
        meta.count = 0;
        meta.dropped_count = 0;
        saveMeta();
    }
}

static void pushToBuffer(const TelemetryPayload &data) {
    if (fsMutex == NULL) return;
    xSemaphoreTake(fsMutex, portMAX_DELAY);
    
    // Buka mode "r+" untuk update posisi seek acak tanpa mengosongkan/memotong isi file
    File f = LittleFS.open(FILE_DATA, "r+");
    if (!f) {
        f = LittleFS.open(FILE_DATA, "w+");
    }
    
    if (f) {
        uint32_t slot = meta.tail;
        uint32_t offset = slot * sizeof(TelemetryPayload);
        if (f.seek(offset, SeekSet)) {
            size_t written = f.write((const uint8_t*)&data, sizeof(TelemetryPayload));
            f.flush();
            if (written == sizeof(TelemetryPayload)) {
                meta.tail = (meta.tail + 1) % MAX_BUFFER_RECORDS;
                if (meta.count < MAX_BUFFER_RECORDS) {
                    meta.count++;
                } else {
                    // Buffer penuh: timpa slot tertua dan majukan pointer head
                    meta.head = (meta.head + 1) % MAX_BUFFER_RECORDS;
                    meta.dropped_count++;
                    Serial.printf("[STORE-AND-FORWARD ALERT] Buffer flash penuh (%u rekaman). Slot tertua ditimpa! Total dropped: %u\n",
                                  MAX_BUFFER_RECORDS, meta.dropped_count);
                }
                saveMeta();
                Serial.printf("[STORE-AND-FORWARD] Data disimpan ke LittleFS (Slot %u, Antrean: %u)\n",
                              slot, meta.count);
            } else {
                Serial.println(F("[LITTLEFS ERROR] Gagal menulis seluruh byte payload ke flash!"));
            }
        } else {
            Serial.println(F("[LITTLEFS ERROR] Seek gagal pada pushToBuffer!"));
        }
        f.close();
    } else {
        Serial.println(F("[LITTLEFS ERROR] Gagal membuka file data untuk write!"));
    }
    xSemaphoreGive(fsMutex);
}

static bool popFromBuffer(TelemetryPayload *data) {
    if (fsMutex == NULL || meta.count == 0 || data == NULL) return false;
    xSemaphoreTake(fsMutex, portMAX_DELAY);

    bool success = false;
    File f = LittleFS.open(FILE_DATA, "r");
    if (f) {
        uint32_t slot = meta.head;
        uint32_t offset = slot * sizeof(TelemetryPayload);
        if (f.seek(offset, SeekSet)) {
            if (f.read((uint8_t*)data, sizeof(TelemetryPayload)) == sizeof(TelemetryPayload)) {
                meta.head = (meta.head + 1) % MAX_BUFFER_RECORDS;
                meta.count--;
                saveMeta();
                success = true;
                Serial.printf("[STORE-AND-FORWARD] Data di-pop dari LittleFS (Slot %u, Sisa antrean: %u)\n",
                              slot, meta.count);
            }
        }
        f.close();
    }
    xSemaphoreGive(fsMutex);
    return success;
}

// ==========================================
// 3. ISR INTERRUPT DIO0 (HARDWARE RX_DONE)
// ==========================================
void IRAM_ATTR isr_lora_rx() {
    if (TaskLoRaRxHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(TaskLoRaRxHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ==========================================
// 4. TASK: PENERIMA LORA RX ASINKRON (Core 1, Prio 3) — SOLE RADIO OWNER
// ==========================================
void vTaskLoRaRx(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        // Blokir sampai ada sinyal interupsi hardware DIO0
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TelemetryPayload rxData;
        memset(&rxData, 0, sizeof(TelemetryPayload));

        int state = radio.readData((uint8_t*)&rxData, sizeof(TelemetryPayload));

        if (state == RADIOLIB_ERR_NONE) {
            // Berikan True Timestamp dari RTC DS3231 jika sinkron
            if (g_rtc_available) {
                rxData.timestamp = rtc.now().unixtime();
            } else {
                rxData.timestamp = 0; // Sentinel 0: Waktu RTC offline/belum terkalibrasi
            }
            
            Serial.printf("[LORA RX] Paket Diterima: Node=%.8s | Seq=#%u | Uptime=%u s | Air=%.1f cm | RSSI=%.1f dBm | SNR=%.1f dB\n",
                          rxData.node_code, rxData.sequence_no, rxData.timestamp, rxData.water_level_cm, 
                          radio.getRSSI(), radio.getSNR());

            // Kirim ke antrean telemetri RAM. Jika penuh, simpan ke LittleFS
            if (xQueueSend(xQueueTelemetry, &rxData, 0) != pdPASS) {
                pushToBuffer(rxData);
            }

            // [Half-Duplex Post-Uplink RX Window]:
            // Node WC membuka RX Window sesaat setelah transmit selesai.
            // Cek apakah ada komando tertunda untuk node ini:
            ActuatorCommand pendingCmd;
            if (popPendingDownlink(rxData.node_code, &pendingCmd)) {
                vTaskDelay(pdMS_TO_TICKS(60)); // Waktu peralihan Node WC ke mode dengar
                radio.standby();
                Serial.printf("[DOWNLINK TX] Memancarkan Komando ke %.8s (CMD=%u, PARAM=%u) dalam Jendela RX...\n", 
                              pendingCmd.node_code, pendingCmd.command_id, pendingCmd.parameter);
                int txState = radio.transmit((uint8_t*)&pendingCmd, sizeof(ActuatorCommand));
                if (txState == RADIOLIB_ERR_NONE) {
                    Serial.println(F("[DOWNLINK TX] Komando Berhasil Dipancarkan ke Jendela RX Node!"));
                } else {
                    Serial.printf("[DOWNLINK TX] Gagal Memancarkan Downlink (Kode: %d)\n", txState);
                }
            }
        }

        // Segera aktifkan kembali mode RX Continuous
        radio.startReceive();
    }
}

// ==========================================
// 5. TASK: MQTT DISPATCH & STORE-AND-FORWARD (Core 0, Prio 2)
// ==========================================
void vTaskMqttTx(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        TelemetryPayload data;
        bool hasData = false;

        // Cek antrean RAM
        if (xQueueReceive(xQueueTelemetry, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            hasData = true;
        } 
        // Jika antrean RAM kosong, kuras dari LittleFS saat MQTT online
        else {
            bool isMqttOnline = false;
            if (mqttMutex != NULL) {
                xSemaphoreTake(mqttMutex, portMAX_DELAY);
                isMqttOnline = mqtt.connected();
                xSemaphoreGive(mqttMutex);
            }
            if (isMqttOnline && meta.count > 0) {
                hasData = popFromBuffer(&data);
            }
        }

        if (hasData) {
            bool published = false;
            if (mqttMutex != NULL) {
                xSemaphoreTake(mqttMutex, portMAX_DELAY);
                if (mqtt.connected()) {
                    StaticJsonDocument<384> doc;
                    doc["schema_version"]  = data.schema_version;
                    char safe_node[9] = {0};
                    memcpy(safe_node, data.node_code, 8);
                    doc["node_code"]       = safe_node;
                    doc["sequence_no"]     = data.sequence_no;
                    doc["timestamp"]       = data.timestamp;
                    doc["water_level_cm"]  = data.water_level_cm;
                    doc["ammonia_ppm"]     = data.ammonia_ppm;
                    doc["h2s_ppm"]         = data.h2s_ppm;
                    doc["battery_voltage"] = data.battery_voltage;
                    doc["sos_triggered"]   = data.sos_triggered;

                    char jsonBuffer[384];
                    size_t jsonLen = serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

                    if (mqtt.publish(MQTT_TOPIC_TELEMETRY, (const uint8_t*)jsonBuffer, (unsigned int)jsonLen, true)) {
                        Serial.printf("[MQTT TX] Berhasil Publish ke %s: %s\n", MQTT_TOPIC_TELEMETRY, jsonBuffer);
                        published = true;
                    } else {
                        Serial.println(F("[MQTT TX] Publish gagal! Menyimpan ke buffer Flash."));
                    }
                }
                xSemaphoreGive(mqttMutex);
            }

            if (!published) {
                // Jaringan offline, amankan ke flash disk LittleFS
                pushToBuffer(data);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==========================================
// 6. TASK: SUPERVISOR WIFI & MQTT (Core 0, Prio 1)
// ==========================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    (void)topic;
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (!error) {
        ActuatorCommand cmd;
        memset(&cmd, 0, sizeof(ActuatorCommand));
        const char* node = doc["node_code"] | "WC_01";
        strncpy(cmd.node_code, node, sizeof(cmd.node_code) - 1);
        cmd.command_id = doc["command_id"] | 1;
        cmd.parameter  = doc["parameter"] | 0;

        Serial.printf("[MQTT RX] Komando diterima dari server untuk %.8s: CMD=%u (Param=%u)\n", 
                      cmd.node_code, cmd.command_id, cmd.parameter);

        // Masukkan ke antrean pending downlink agar dipancarkan saat node membuka jendela RX
        queuePendingDownlink(cmd);
    }
}

void vTaskWiFiSupervisor(void *pvParameters) {
    (void)pvParameters;
    uint32_t backoff = 3000;
    for (;;) {
        // Monitor Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            Serial.println(F("[WIFI] Menyambungkan ke WiFi Posko..."));
            vTaskDelay(pdMS_TO_TICKS(5000));
        }

        // Monitor MQTT jika Wi-Fi sudah tersambung
        bool isConnected = false;
        if (mqttMutex != NULL) {
            xSemaphoreTake(mqttMutex, portMAX_DELAY);
            isConnected = mqtt.connected();
            xSemaphoreGive(mqttMutex);
        }

        if (WiFi.status() == WL_CONNECTED && !isConnected) {
            if (mqttMutex != NULL) {
                xSemaphoreTake(mqttMutex, portMAX_DELAY);
                mqtt.setServer(MQTT_SERVER, MQTT_PORT);
                mqtt.setCallback(mqttCallback);

                String clientId = "GatewayPosko-" + String(WiFi.macAddress());
                Serial.printf("[MQTT] Menghubungi Broker %s:%d...\n", MQTT_SERVER, MQTT_PORT);

                if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
                    Serial.println(F("[MQTT] Terhubung ke Broker!"));
                    mqtt.publish(MQTT_TOPIC_STATUS, "ONLINE", true);
                    mqtt.subscribe(MQTT_TOPIC_COMMAND, 1);
                    backoff = 3000;
                } else {
                    Serial.printf("[MQTT] Gagal (rc=%d). Backoff %u ms\n", mqtt.state(), backoff);
                    if (backoff < 30000) backoff *= 2;
                }
                xSemaphoreGive(mqttMutex);
            }
            vTaskDelay(pdMS_TO_TICKS(backoff));
        }

        if (mqttMutex != NULL) {
            xSemaphoreTake(mqttMutex, portMAX_DELAY);
            if (mqtt.connected()) {
                mqtt.loop();
            }
            xSemaphoreGive(mqttMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ==========================================
// 7. SETUP UTAMA GATEWAY
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println(F("================================================================="));
    Serial.println(F("  SMART-SANITATION eSOS - GATEWAY ROUTER FIRMWARE (ESP32 DevKit) "));
    Serial.println(F("================================================================="));

    // Inisialisasi Bus I2C RTC DS3231 (SDA:4, SCL:22)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (!rtc.begin()) {
        Serial.println(F("[RTC PERINGATAN] Modul RTC DS3231 tidak ditemukan pada I2C!"));
        g_rtc_available = false;
    } else if (rtc.lostPower()) {
        Serial.println(F("[RTC PERINGATAN] DS3231 lost power / osilator berhenti, waktu belum sinkron!"));
        g_rtc_available = false;
    } else {
        Serial.println(F("[RTC OK] Modul RTC DS3231 aktif dan waktu valid."));
        g_rtc_available = true;
    }

    // Inisiasi Mutex & Queues
    fsMutex = xSemaphoreCreateMutex();
    mqttMutex = xSemaphoreCreateMutex();
    pendingMutex = xSemaphoreCreateMutex();
    xQueueTelemetry = xQueueCreate(20, sizeof(TelemetryPayload));

    if (fsMutex == NULL || mqttMutex == NULL || pendingMutex == NULL || xQueueTelemetry == NULL) {
        Serial.println(F("[FATAL] Gagal membuat kernel primitives FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // Konfigurasi kapasitas buffer PubSubClient (default 128/256B terlalu kecil untuk JSON 384B)
    mqtt.setBufferSize(512);

    // Inisiasi LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println(F("[FATAL] LittleFS Mount Failed!"));
    } else {
        loadMeta();
        Serial.printf("[OK] LittleFS aktif. Antrean pesan tersimpan: %u (Dropped: %u)\n", 
                      meta.count, meta.dropped_count);
    }

    // Inisialisasi Hardware SPI Bus untuk SX1278
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100);

    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);

    // Inisiasi Radio SX1278 via RadioLib
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state == RADIOLIB_ERR_NONE) {
        radio.setCRC(true);
        radio.setDio0Action(isr_lora_rx, RISING);
        radio.startReceive();
        Serial.println(F("[OK] Radio SX1278 aktif & Siaga di 433.175 MHz."));
    } else {
        Serial.printf("[GAGAL] Inisialisasi Radio SX1278 gagal (Kode: %d)\n", state);
    }

    // Pasang Task FreeRTOS ke Dual Core ESP32
    // Core 0: Wi-Fi/MQTT Supervisor dan MQTT Dispatcher
    // Core 1: Sole Radio Owner (TaskLoRaRx)
    xTaskCreatePinnedToCore(vTaskWiFiSupervisor, "TaskWiFi", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskMqttTx,         "TaskMqtt", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLoRaRx,         "TaskLoRaRx", 4096, NULL, 3, &TaskLoRaRxHandle, 1);

    Serial.println(F("=================================================================\n"));
}

void loop() {
    vTaskDelete(NULL);
}
