// Menyertakan library utama Arduino
#include <Arduino.h>

// Mendefinisikan bahwa kode ini berjalan untuk perangkat Gateway
#define IS_GATEWAY
#ifdef IS_GATEWAY

// Library untuk koneksi WiFi
#include <WiFi.h>
// Library untuk komunikasi dengan server MQTT
#include <PubSubClient.h>
// Library untuk memanipulasi format data JSON
#include <ArduinoJson.h>
// Library untuk sistem file internal ESP32 (menyimpan data secara permanen)
#include <LittleFS.h>
// Library untuk mengontrol modul radio LoRa
#include <RadioLib.h>
// Library bawaan ESP-IDF untuk manajemen daya (power management) ESP32
#include "esp_pm.h"
#include "esp_sleep.h"
// Library untuk komunikasi I2C (digunakan oleh RTC)
#include <Wire.h>
// Library untuk modul RTC (Real Time Clock) pembaca waktu
#include <RTClib.h>

// ==========================================
// KREDENSIAL & PARAMETER
// ==========================================
// Nama WiFi (SSID) dan Password yang akan dihubungkan oleh ESP32
const char* WIFI_SSID     = "POSKO_WIFI";
const char* WIFI_PASS     = "12345678";

// Pengaturan Server MQTT (IP Address, Port, Username, dan Password)
const char* MQTT_SERVER   = "192.168.0.100";
const int   MQTT_PORT     = 1883;
const char* MQTT_USER     = "esos_gateway";
const char* MQTT_PASS     = "gateway_secret";

// Topik MQTT untuk mengirim data sensor (TX/Uplink) dan menerima perintah (RX/Downlink)
const char* MQTT_TOPIC_TX = "esos/gateway_01/nodes/telemetry";
const char* MQTT_TOPIC_RX = "esos/+/+/command";

// Parameter pin untuk modul Radio LoRa (SX1278) di ESP32
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2
#define PIN_LORA_RESET    14
#define PIN_LORA_MISO     19
#define PIN_LORA_MOSI     23
#define PIN_LORA_SCK      18

// Konfigurasi frekuensi dan spesifikasi sinyal LoRa
#define LORA_FREQ         433.0 // Frekuensi 433 MHz
#define LORA_BW           125.0 // Bandwidth 125 kHz
#define LORA_SF           9     // Spreading Factor 9
#define LORA_CR           7     // Coding Rate 4/7
#define LORA_SYNC_WORD    0x12  // Sync word agar hanya berkomunikasi dengan perangkat yg sync wordnya sama
#define LORA_TX_POWER     17    // Daya pancar transmisi (17 dBm)

// ==========================================
// STRUKTUR DATA
// ==========================================

// Struktur data ini adalah format "paket" yang dikirim oleh Node Sensor via LoRa.
// Atribut 'packed' digunakan agar ukuran memori struktur ini padat dan efisien saat dikirim via radio.
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version;   // Versi format data
    char node_code[8];        // Kode identitas Node (misal: "NODE_01")
    uint32_t sequence_no;     // Nomor urut pesan
    uint32_t timestamp;       // Stempel waktu saat data diambil
    float water_level_cm;     // Data tinggi air
    float ammonia_ppm;        // Data kadar amonia
    float h2s_ppm;            // Data kadar H2S
    float battery_voltage;    // Data tegangan baterai Node
    uint8_t sos_triggered;    // Status apakah tombol SOS ditekan
};

// Objek untuk mengontrol modul RTC tipe DS3231
RTC_DS3231 rtc;

// Struktur data untuk menerima dan meneruskan perintah aktuator (seperti Servo/Katup).
struct __attribute__((packed)) ActuatorCommand {
    uint8_t command_id; // ID Perintah: 1 = BUKA, 2 = TUTUP, 3 = FLUSH
    uint8_t angle;      // Target sudut pergerakan servo (dalam derajat)
};

// ==========================================
// LITTLEFS CIRCULAR BUFFER (Store-and-Forward)
// ==========================================
// Bagian ini berfungsi untuk menyimpan data sensor ke memori internal (LittleFS)
// JIKA kebetulan WiFi/MQTT sedang mati. Saat WiFi hidup lagi, data akan dikirim.

#define MAX_BUFFER_SIZE 500 // Maksimal 500 data yang bisa ditampung saat offline
#define FILE_META "/meta.dat" // File untuk menyimpan posisi antrean (head/tail)
#define FILE_DATA "/buffer.dat" // File untuk menyimpan data sensor yang tertunda

// Struktur untuk melacak posisi antrean baca (head), antrean tulis (tail), dan total antrean (count)
struct BufferMeta {
    uint16_t head;
    uint16_t tail;
    uint16_t count;
};

BufferMeta meta = {0, 0, 0};
SemaphoreHandle_t fsMutex; // Kunci (mutex) agar pembacaan/penulisan file tidak bertabrakan antar-Task

// Fungsi untuk menyimpan posisi antrean (meta) ke memori (LittleFS)
void saveMeta() {
    File f = LittleFS.open(FILE_META, FILE_WRITE);
    if (f) {
        f.write((uint8_t*)&meta, sizeof(BufferMeta));
        f.close();
    }
}

// Fungsi untuk memuat ulang posisi antrean dari memori saat alat baru menyala
void loadMeta() {
    if (LittleFS.exists(FILE_META)) {
        File f = LittleFS.open(FILE_META, FILE_READ);
        if (f) {
            f.read((uint8_t*)&meta, sizeof(BufferMeta));
            f.close();
        }
    } else {
        saveMeta(); // Jika file belum ada, inisialisasi awal.
    }
}

// Fungsi untuk menjejalkan (push) data sensor ke dalam antrean file (buffer) jika offline
void pushToBuffer(TelemetryPayload data) {
    xSemaphoreTake(fsMutex, portMAX_DELAY); // Kunci akses file
    
    File f = LittleFS.open(FILE_DATA, FILE_WRITE); // Buka file untuk menulis
    uint32_t offset = meta.tail * sizeof(TelemetryPayload); // Hitung lokasi penulisan
    f.seek(offset, SeekSet); // Pindahkan kursor penulisan
    f.write((uint8_t*)&data, sizeof(TelemetryPayload)); // Tulis data ke memori
    f.close();

    // Geser posisi antrean belakang (tail)
    meta.tail = (meta.tail + 1) % MAX_BUFFER_SIZE;
    if (meta.count < MAX_BUFFER_SIZE) {
        meta.count++; // Tambah jumlah tumpukan data
    } else {
        meta.head = (meta.head + 1) % MAX_BUFFER_SIZE; // Jika kepenuhan, timpa data paling lama
    }
    saveMeta(); // Simpan pembaruan posisi antrean
    
    xSemaphoreGive(fsMutex); // Lepas kunci akses file
    Serial.printf("Buffer PUSH. Count: %d\n", meta.count);
}

// Fungsi untuk mengambil/mengeluarkan (pop) data dari memori untuk dikirim ke MQTT saat online
bool popFromBuffer(TelemetryPayload* data) {
    xSemaphoreTake(fsMutex, portMAX_DELAY); // Kunci akses file
    if (meta.count == 0) { // Jika tidak ada tumpukan data, batalkan
        xSemaphoreGive(fsMutex);
        return false;
    }

    File f = LittleFS.open(FILE_DATA, FILE_READ); // Buka file untuk membaca
    uint32_t offset = meta.head * sizeof(TelemetryPayload); // Cari lokasi data terlama
    f.seek(offset, SeekSet);
    f.read((uint8_t*)data, sizeof(TelemetryPayload)); // Baca datanya
    f.close();

    // Geser posisi antrean depan (head) dan kurangi total tumpukan (count)
    meta.head = (meta.head + 1) % MAX_BUFFER_SIZE;
    meta.count--;
    saveMeta(); // Simpan pembaruan posisi antrean
    
    xSemaphoreGive(fsMutex); // Lepas kunci akses file
    Serial.printf("Buffer POP. Count: %d\n", meta.count);
    return true;
}

// ==========================================
// GLOBAL HANDLES (Variabel Penghubung)
// ==========================================
QueueHandle_t xQueueTelemetry;       // Antrean (Queue) di RAM untuk data sensor LoRa ke MQTT
QueueHandle_t xQueueCommandDownlink; // Antrean (Queue) di RAM untuk perintah MQTT ke LoRa (Servo)
TaskHandle_t TaskLoRaRxHandle;       // Handle (ID) untuk Task penerima LoRa
esp_pm_lock_handle_t active_lock;    // Kunci daya (Power lock) agar ESP32 tidak tidur/sleep saat mengirim LoRa

// Inisialisasi objek Radio LoRa
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, PIN_LORA_MISO);
// Inisialisasi koneksi WiFi Client
WiFiClient espClient;
// Inisialisasi koneksi MQTT dengan menggunakan WiFi Client
PubSubClient mqtt(espClient);

// ==========================================
// CALLBACK & ISR (Fungsi yang dipanggil otomatis oleh Trigger)
// ==========================================

// Fungsi Interupsi (Interrupt) Hardware: Dipanggil O-T-O-M-A-T-I-S oleh chip saat LoRa menerima paket baru
void IRAM_ATTR isr_lora_rx() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Beri sinyal (Notify) ke Task LoRa RX bahwa ada data baru untuk diambil
    vTaskNotifyGiveFromISR(TaskLoRaRxHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(); // Perpindahan konteks (context switch) secara cepat
    }
}

// Fungsi Callback MQTT: Dipanggil otomatis saat ada pesan/perintah masuk dari Server/Aplikasi
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // 1. Jika pesan masuk adalah perintah "sinkronisasi waktu (time_sync)"
    if (strstr(topic, "time_sync") != nullptr) {
        String ts = "";
        for (int i = 0; i < length; i++) ts += (char)payload[i];
        rtc.adjust(DateTime(ts.toInt())); // Perbarui waktu RTC dengan waktu server
        Serial.printf("RTC Synced: %s\n", ts.c_str());
        return;
    }

    // 2. Jika bukan sinkronisasi waktu, anggap ini perintah Servo/Aktuator berformat JSON
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) return; // Jika format JSON rusak, batalkan

    ActuatorCommand cmd;
    const char* c_type = doc["command_type"]; // Ambil jenis perintah dari JSON
    
    // Konversi jenis perintah teks menjadi angka (Command ID)
    if (c_type != nullptr) {
        if (strcmp(c_type, "OPEN_VALVE") == 0) cmd.command_id = 1;
        else if (strcmp(c_type, "CLOSE_VALVE") == 0) cmd.command_id = 2;
        else if (strcmp(c_type, "FLUSH_TANK") == 0) cmd.command_id = 3;
        else return; // Perintah tidak dikenal
        
        // Ambil besar sudut servo dari JSON
        cmd.angle = doc["target_angle_deg"] | 0;

        // Masukkan perintah yang sudah diformat ke dalam Antrean Downlink LoRa
        xQueueSend(xQueueCommandDownlink, &cmd, 0); 
    }
}

// ==========================================
// TASK 1: LORA RX (Berjalan terus di CPU Core 1)
// ==========================================
// Fungsi: Membaca secara fisik data LoRa yang tertangkap antena
void vTaskLoRaRx(void *pvParameters) {
    for (;;) {
        // Task akan "tertidur" di sini sampai dibangunkan oleh Fungsi Interupsi (isr_lora_rx)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TelemetryPayload rxData;
        // Baca data dari modul radio ke variabel rxData
        int state = radio.readData((uint8_t*)&rxData, sizeof(TelemetryPayload));

        if (state == RADIOLIB_ERR_NONE) { // Jika pembacaan sukses tanpa error
            rxData.timestamp = rtc.now().unixtime(); // Tambahkan waktu (timestamp) saat ini dari RTC
            
            // Masukkan data ke Antrean (Queue) MQTT. Jika antrean RAM penuh, simpan ke Memori/Buffer (LittleFS)
            if (xQueueSend(xQueueTelemetry, &rxData, 0) != pdPASS) {
                pushToBuffer(rxData);
            }
        }
        
        // Perintahkan modul radio untuk kembali ke mode "Mendengar" (Listen/Receive)
        radio.startReceive();
    }
}

// ==========================================
// TASK 2: LORA TX DOWNLINK (Berjalan di Core 1)
// ==========================================
// Fungsi: Meneruskan perintah aktuator dari antrean RAM (dari MQTT) menembak lewat sinyal LoRa
void vTaskLoRaTxDownlink(void *pvParameters) {
    ActuatorCommand cmd;
    for (;;) {
        // Menunggu jika ada antrean perintah masuk (di-blok/menunggu di sini)
        if (xQueueReceive(xQueueCommandDownlink, &cmd, portMAX_DELAY) == pdTRUE) {
            
            esp_pm_lock_acquire(active_lock); // Kunci Power agar voltase stabil saat memancarkan LoRa
            radio.standby(); // Hentikan mode "Mendengar", ubah ke mode Standby

            // Pancarkan (Transmit) paket perintah Servo
            int state = radio.transmit((uint8_t*)&cmd, sizeof(ActuatorCommand));
            if (state == RADIOLIB_ERR_NONE) {
                Serial.printf("LoRa Downlink Terkirim! CMD: %d, Sudut: %d\n", cmd.command_id, cmd.angle);
            } else {
                Serial.printf("LoRa Downlink Gagal (rc=%d)\n", state);
            }
            
            radio.startReceive(); // Kembali ke mode "Mendengar"
            esp_pm_lock_release(active_lock); // Lepas Kunci Power
        }
    }
}

// ==========================================
// TASK 3: MQTT TX & BUFFER FLUSH (Berjalan di Core 0)
// ==========================================
// Fungsi: Mengambil data dari antrean (LoRa) dan mempublikasikannya ke MQTT Server
void vTaskMqttTx(void *pvParameters) {
    for (;;) {
        TelemetryPayload data;
        bool hasData = false;

        // 1. Cek apakah ada data langsung dari memori RAM (Queue)
        if (xQueueReceive(xQueueTelemetry, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            hasData = true;
        } 
        // 2. Jika di RAM kosong, tapi MQTT terhubung dan masih ada data 'nunggak' di LittleFS, ambil dari situ
        else if (mqtt.connected() && meta.count > 0) {
            hasData = popFromBuffer(&data);
        }

        if (hasData) {
            if (mqtt.connected()) {
                // Bungkus (Format) data berstruktur menjadi tipe JSON
                StaticJsonDocument<256> doc;
                doc["schema_version"] = data.schema_version;
                doc["node_code"] = data.node_code;
                doc["sequence_no"] = data.sequence_no;
                doc["timestamp"] = data.timestamp;
                doc["water_level_cm"] = data.water_level_cm;
                doc["ammonia_ppm"] = data.ammonia_ppm;
                doc["h2s_ppm"] = data.h2s_ppm;
                doc["battery_voltage"] = data.battery_voltage;
                doc["sos_triggered"] = data.sos_triggered;

                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer); // Ubah JSON ke string karakter

                // Terbitkan (Publish) ke server. Jika Publish GAGAL, masukkan data kembali ke Buffer (Memori)
                if (!mqtt.publish(MQTT_TOPIC_TX, jsonBuffer, true)) {
                    pushToBuffer(data);
                }
            } else {
                // Jika MQTT sedang terputus, simpan data ke Buffer (Memori LittleFS)
                pushToBuffer(data);
            }
        }
    }
}

// ==========================================
// TASK 4: WIFI & MQTT SUPERVISOR (Berjalan di Core 0)
// ==========================================
// Fungsi: Memantau dan memastikan koneksi WiFi dan MQTT selalu terhubung.
void vTaskWiFiSupervisor(void *pvParameters) {
    uint32_t backoff = 3000; // Waktu tunggu awal jika gagal terhubung
    
    for (;;) {
        // Jika WiFi Putus, coba hubungkan kembali
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi Terputus. Menghubungkan...");
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            
            // Beri waktu (timeout) koneksi hingga 10 percobaan (5 detik total)
            int retries = 10;
            while (WiFi.status() != WL_CONNECTED && retries > 0) {
                vTaskDelay(pdMS_TO_TICKS(500)); // Jeda 500ms
                retries--;
            }
        }

        // Jika WiFi Terhubung tapi MQTT terputus, hubungkan MQTT
        if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
            Serial.println("MQTT Terputus. Menghubungkan...");
            mqtt.setServer(MQTT_SERVER, MQTT_PORT);
            mqtt.setCallback(mqttCallback); // Atur fungsi callback saat ada pesan masuk
            
            // Coba login/connect ke server MQTT (disertai Last Will "OFFLINE" jika ESP mati)
            if (mqtt.connect("Gateway_01", MQTT_USER, MQTT_PASS, "esos/gateway_01/status", 1, true, "OFFLINE")) {
                Serial.println("MQTT Terhubung!");
                mqtt.publish("esos/gateway_01/status", "ONLINE", true); // Laporkan status online
                mqtt.subscribe(MQTT_TOPIC_RX, 1); // Berlangganan topik untuk perintah Servo
                mqtt.subscribe("esos/gateway_01/time_sync", 1); // Berlangganan topik sinkronisasi waktu
                backoff = 3000; // Reset waktu backoff
            } else {
                // Jika gagal terhubung MQTT, beri jeda dengan sistem eksponensial (waktu tunggu bertambah lama)
                Serial.printf("MQTT Gagal (rc=%d). Backoff %d ms\n", mqtt.state(), backoff);
                vTaskDelay(pdMS_TO_TICKS(backoff));
                if (backoff < 12000) backoff *= 2; 
            }
        }
        
        // Panggil fungsi rutin internal library MQTT untuk menjaga koneksi tetap hidup
        if (mqtt.connected()) {
            mqtt.loop();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Beri nafas ke CPU (jangan dipantau non-stop 0 detik)
    }
}

// ==========================================
// FUNGSI SETUP (Dipanggil 1 kali saat ESP32 pertama kali menyala)
// ==========================================
void setup() {
    Serial.begin(115200); // Mulai jalur komunikasi Serial Monitor

    Wire.begin(21, 22); // Mulai jalur I2C di Pin 21 & 22
    if (!rtc.begin()) { // Cek apakah modul RTC/jam terhubung
        Serial.println("RTC tidak ditemukan");
    }

    // Inisialisasi Manajemen Daya (Power Management) ESP-IDF
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 80, // Frekuensi prosesor maks (diturunkan dari 240mhz untuk hemat daya)
        .min_freq_mhz = 10, // Frekuensi prosesor min
        .light_sleep_enable = true // Boleh tidur ringan saat nganggur
    };
    esp_pm_configure(&pm_config);
    esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "active_lock", &active_lock);

    // Inisialisasi Mutex & Queues (RAM)
    fsMutex = xSemaphoreCreateMutex();
    xQueueTelemetry = xQueueCreate(20, sizeof(TelemetryPayload)); // Kapasitas 20 antrean
    xQueueCommandDownlink = xQueueCreate(5, sizeof(ActuatorCommand)); // Kapasitas 5 antrean

    // Inisialisasi LittleFS (Sistem penyimpanan internal permanen)
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    loadMeta(); // Muat posisi jejak tumpukan data dari memori

    // Inisialisasi Modul Radio LoRa
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state == RADIOLIB_ERR_NONE) {
        radio.setPacketReceivedAction(isr_lora_rx); // Daftarkan fungsi interupsi jika ada paket datang
        radio.startReceive(); // Perintahkan LoRa untuk mulai mendengarkan
    } else {
        Serial.println("LoRa INIT FAILED!");
    }

    // Pembuatan Task-Task Multi-threading menggunakan FreeRTOS
    // Format: Nama Fungsi, Nama Bebas, Ukuran Memori, Argumen, Prioritas (1 terendah, 3 tertinggi), Handle, Nomor Core CPU (0/1)
    xTaskCreatePinnedToCore(vTaskWiFiSupervisor, "TaskWiFi",       4096, NULL, 1, NULL, 0); // Core 0
    xTaskCreatePinnedToCore(vTaskMqttTx,         "TaskMqtt",       4096, NULL, 2, NULL, 0); // Core 0
    xTaskCreatePinnedToCore(vTaskLoRaTxDownlink, "TaskLoRaTxDown", 4096, NULL, 2, NULL, 1); // Core 1
    
    // Simpan Handle ID untuk Task LoRaRX agar bisa disinyal oleh fungsi interupsi (ISR)
    xTaskCreatePinnedToCore(vTaskLoRaRx,         "TaskLoRaRx",     4096, NULL, 3, &TaskLoRaRxHandle, 1); // Core 1
}

// ==========================================
// FUNGSI LOOP
// ==========================================
void loop() {
    // Menghapus/mematikan fungsi loop() kosong karena di ESP32 FreeRTOS, fungsi ini menghabiskan memory.
    // Semua aktivitas program ini dikerjakan di Task (vTask...) bukan di Loop.
    vTaskDelete(NULL); 
}

#endif // IS_GATEWAY