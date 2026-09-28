/*
 * ======================================================================================
 * PROYEK CAPSTONE: SMART-SANITATION eSOS (DESAIN PROYEK 2 - KELOMPOK 4 FTUI)
 * SKRIP UJI PENERIMA / GATEWAY: ESP32 DEVKIT V1 + Ai-Thinker Ra-02 (Semtech SX1278)
 * ======================================================================================
 *
 * ARSITEKTUR & CATATAN KEANDALAN:
 * 1. Skrip ini memisahkan tugas pembacaan radio (Core 1 / APP_CPU) dari pemrosesan/serial
 *    (Core 0 / PRO_CPU) menggunakan FreeRTOS Queue dan Task Notification.
 * 2. PENTING: Penggunaan FreeRTOS mempermudah pemisahan beban kerja asinkron, namun BUKAN
 *    jaminan mutlak paket tidak pernah hilang. Paket tetap dapat hilang akibat tabrakan
 *    di udara (LoRa ALOHA), redaman material/path loss, atau jika queue penuh.
 * 3. Buffer FIFO internal SX1278 (256 byte) berbeda dari Queue FreeRTOS di RAM ESP32.
 *    Paket baru di udara yang tiba sebelum pembacaan register selesai dapat menimpa buffer radio.
 *
 * PINOUT PENGKABELAN (Ai-Thinker Ra-02 -> ESP32 DevKit V1 30-pin):
 * - VCC  -> 3V3 (3.3V WAJIB, jangan 5V!)
 * - GND  -> GND
 * - NSS  -> GPIO 5
 * - MOSI -> GPIO 23
 * - MISO -> GPIO 19
 * - SCK  -> GPIO 18
 * - RST  -> GPIO 14
 * - DIO0 -> GPIO 2 (Interupsi RX_DONE)
 * ======================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// 1. DEFINISI PIN PERANGKAT KERAS (ESP32 DevKit V1)
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2     // DIO0 SX1278 (Interupsi RX_DONE)
#define PIN_LORA_RESET    14
#define PIN_LORA_MISO     19
#define PIN_LORA_MOSI     23
#define PIN_LORA_SCK      18

// 2. PARAMETER RADIO RESMI (Kominfo Permen No. 2/2019: 433,050 - 434,790 MHz)
#define LORA_FREQ         433.175 // Frekuensi tengah legal (MHz)
#define LORA_BW           125.0   // Bandwidth kanal (kHz)
#define LORA_SF           9       // Spreading Factor
#define LORA_CR           7       // Coding Rate 4/7 (CR = 3)
#define LORA_SYNC_WORD    0x12    // Sync Word Jaringan Privat eSOS

// 3. KONTRAK STRUKTUR PAYLOAD BINER MUTLAK (34 Bytes)
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte  : Versi skema biner (Selalu 1)
    char node_code[8];      // Offset  1 | 8 bytes : "WC_01\0\0\0"
    uint32_t sequence_no;   // Offset  9 | 4 bytes : Nomor urut paket (Little-Endian)
    uint32_t uptime_seconds;// Offset 13 | 4 bytes : Waktu operasional sejak boot (Detik)
    float water_level_cm;   // Offset 17 | 4 bytes : Nilai sensor air tangki (IEEE-754)
    float ammonia_ppm;      // Offset 21 | 4 bytes : Nilai sensor gas amonia (IEEE-754)
    float h2s_ppm;          // Offset 25 | 4 bytes : Nilai sensor gas H2S (IEEE-754)
    float battery_voltage;  // Offset 29 | 4 bytes : Nilai tegangan baterai 18650 (IEEE-754)
    uint8_t sos_triggered;  // Offset 33 | 1 byte  : Flag status darurat (0/1)
};
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

// Wrapper data untuk antrean gateway
struct GatewayPacketWrapper {
    TelemetryPayload payload;
    float rssi;
    float snr;
    size_t raw_length;
};

// Objek Hardware SX1278 (Argumen ke-4 RADIOLIB_NC, bukan MISO)
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC);

// Handle FreeRTOS
QueueHandle_t xGatewayQueue = NULL;
TaskHandle_t xLoRaTaskHandle = NULL;

// Flag interupsi radio
volatile bool packetReceivedFlag = false;

// Counter diagnostik drop paket pada antrean penuh
volatile uint32_t droppedQueueFullCount = 0;

// ISR (Interrupt Service Routine) dipicu saat sinyal RX_DONE aktif di pin DIO0
void IRAM_ATTR setRxFlag() {
    packetReceivedFlag = true;
    if (xLoRaTaskHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(xLoRaTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ===================================================================
// TASK 1: LORA RX LISTENER (CORE 1 / APP_CPU - PRIORITAS 3)
// ===================================================================
void vTaskLoRaRxListener(void *pvParameters) {
    for (;;) {
        // Blokir task sampai ada notifikasi dari ISR pin DIO0
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (packetReceivedFlag) {
            packetReceivedFlag = false;

            // 1. Verifikasi panjang paket aktual dari radio sebelum membaca payload
            size_t packetLen = radio.getPacketLength();

            if (packetLen != sizeof(TelemetryPayload)) {
                // Panjang paket tidak sesuai kontrak (paket asing atau korup)
                Serial.printf("[RX CORE 1 - REJECT] Panjang paket tidak valid: %u byte (ekspektasi: %u byte). Menguras FIFO...\n",
                              (unsigned int)packetLen, (unsigned int)sizeof(TelemetryPayload));

                // Baca dan buang data agar buffer FIFO SX1278 bersih
                uint8_t dummyBuf[256];
                radio.readData(dummyBuf, packetLen);
            } else {
                // 2. Panjang tepat 34 byte, baca payload biner
                GatewayPacketWrapper incoming;
                memset(&incoming, 0, sizeof(GatewayPacketWrapper));
                incoming.raw_length = packetLen;

                int state = radio.readData((uint8_t*)&incoming.payload, sizeof(TelemetryPayload));

                if (state == RADIOLIB_ERR_NONE) {
                    // 3. Validasi isi header payload biner
                    bool isValidSchema = (incoming.payload.schema_version == 1);
                    
                    // Pastikan node_code memiliki karakter ASCII tercetak dan terminator null
                    bool isValidNodeCode = (incoming.payload.node_code[0] >= 32 && incoming.payload.node_code[0] <= 126);
                    incoming.payload.node_code[sizeof(incoming.payload.node_code) - 1] = '\0'; // Enforce null-terminate

                    if (isValidSchema && isValidNodeCode) {
                        incoming.rssi = radio.getRSSI();
                        incoming.snr = radio.getSNR();

                        // 4. Masukkan ke antrean tanpa menunggu (timeout 0) agar RX segera di-rearm
                        if (xQueueSend(xGatewayQueue, &incoming, 0) != pdTRUE) {
                            droppedQueueFullCount++;
                            Serial.printf("[RX CORE 1 - DROP] Antrean gateway penuh! Paket #%u dari %s terbuang. Total dropped: %u\n",
                                          incoming.payload.sequence_no, incoming.payload.node_code, droppedQueueFullCount);
                        }
                    } else {
                        Serial.printf("[RX CORE 1 - REJECT] Validasi header gagal: Schema=%u, Node=%s\n",
                                      incoming.payload.schema_version, incoming.payload.node_code);
                    }

                } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
                    Serial.println("[RX CORE 1 - CRC ERROR] Paket terdeteksi korup di udara (CRC Mismatch)!");
                } else {
                    Serial.printf("[RX CORE 1 - READ ERROR] Kode galat readData: %d\n", state);
                }
            }

            // 5. Segera aktifkan kembali mode RX asinkron agar SX1278 siap menangkap paket berikutnya
            int rearmState = radio.startReceive();
            if (rearmState != RADIOLIB_ERR_NONE) {
                Serial.printf("[RX CORE 1 - FATAL] Gagal re-arm startReceive! Kode galat: %d\n", rearmState);
            }
        }
    }
}

// ===================================================================
// PELACAK NOMOR URUT PER-NODE (MULTI-NODE TRACKER)
// ===================================================================
#define MAX_TRACKED_NODES 8

struct NodeTracker {
    char node_code[8];
    uint32_t last_seq;
    uint32_t total_received;
    uint32_t observed_gaps;
    bool active;
};

static NodeTracker trackedNodes[MAX_TRACKED_NODES];

static NodeTracker* getOrRegisterNode(const char *code) {
    // Cari node yang sudah terdaftar
    for (int i = 0; i < MAX_TRACKED_NODES; i++) {
        if (trackedNodes[i].active && strncmp(trackedNodes[i].node_code, code, 7) == 0) {
            return &trackedNodes[i];
        }
    }
    // Jika belum terdaftar, alokasikan slot kosong baru
    for (int i = 0; i < MAX_TRACKED_NODES; i++) {
        if (!trackedNodes[i].active) {
            strncpy(trackedNodes[i].node_code, code, 7);
            trackedNodes[i].node_code[7] = '\0';
            trackedNodes[i].last_seq = 0;
            trackedNodes[i].total_received = 0;
            trackedNodes[i].observed_gaps = 0;
            trackedNodes[i].active = true;
            return &trackedNodes[i];
        }
    }
    return NULL; // Slot habis
}

// ===================================================================
// TASK 2: GATEWAY DISPATCH / SERIAL LOGGER (CORE 0 / PRO_CPU - PRIORITAS 1)
// ===================================================================
void vTaskGatewayDispatch(void *pvParameters) {
    GatewayPacketWrapper item;

    for (;;) {
        // Ambil paket dari antrean buffer (Task tidur jika antrean kosong)
        if (xQueueReceive(xGatewayQueue, &item, portMAX_DELAY) == pdTRUE) {
            
            NodeTracker *node = getOrRegisterNode(item.payload.node_code);
            uint32_t currentGap = 0;

            if (node != NULL) {
                node->total_received++;
                if (node->last_seq > 0) {
                    if (item.payload.sequence_no > node->last_seq + 1) {
                        currentGap = item.payload.sequence_no - (node->last_seq + 1);
                        node->observed_gaps += currentGap;
                    } else if (item.payload.sequence_no <= node->last_seq) {
                        // Terjadi restart node atau pembungkus integer (wrap-around)
                        Serial.printf("[DISPATCH] Informasi: Nomor urut node %s mundur/restart (Sebelumnya #%u, kini #%u)\n",
                                      node->node_code, node->last_seq, item.payload.sequence_no);
                    }
                }
                node->last_seq = item.payload.sequence_no;
            }

            Serial.println("=================================================================");
            Serial.printf("[GATEWAY DISPATCH] Paket Valid Diterima dari Node: %s\n", item.payload.node_code);
            Serial.printf("  Nomor Urut    : #%u (Total Diterima dari node ini: %u)\n", 
                          item.payload.sequence_no, node ? node->total_received : 1);
            if (currentGap > 0) {
                Serial.printf("  PERINGATAN    : Celah nomor urut teramati sebanyak %u paket (Total celah: %u)\n",
                              currentGap, node ? node->observed_gaps : currentGap);
                Serial.println("                  (Catatan: Celah dapat disebabkan drop di udara, tabrakan, atau reboot)");
            }
            Serial.printf("  Metrik RF     : RSSI = %.1f dBm | SNR = %.2f dB\n", item.rssi, item.snr);
            Serial.printf("  Uptime Node   : %u detik\n", item.payload.uptime_seconds);
            Serial.printf("  Telemetri     : Air=%.1f cm | NH3=%.1f ppm | H2S=%.1f ppm | Batt=%.2f V\n",
                          item.payload.water_level_cm, item.payload.ammonia_ppm, 
                          item.payload.h2s_ppm, item.payload.battery_voltage);
            
            if (item.payload.sos_triggered) {
                Serial.println("  >>> STATUS    : [DARURAT] TOMBOL SOS AKTIF DARI TOILET INI! <<<");
            } else {
                Serial.println("  >>> STATUS    : Normal");
            }
            Serial.println("=================================================================\n");
        }
    }
}

// ===================================================================
// SETUP (INISIALISASI SISTEM & URUTAN AMAN BEBAS RACE CONDITION)
// ===================================================================
void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n=================================================================");
    Serial.println("  SMART-SANITATION eSOS - GATEWAY RECEIVER TEST (ESP32 DevKit V1)");
    Serial.println("=================================================================");

    // Inisialisasi pelacak node
    memset(trackedNodes, 0, sizeof(trackedNodes));

    // 1. Inisialisasi Bus SPI Perangkat Keras
    Serial.print("1. Inisialisasi Hardware SPI Bus... ");
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    Serial.println("[OK]");

    // 2. Inisialisasi Radio SX1278
    Serial.print("2. Inisialisasi Chip Semtech SX1278... ");
    int initState = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, 10);
    if (initState != RADIOLIB_ERR_NONE) {
        Serial.printf("[GAGAL] Kode Galat: %d. Periksa wiring & catu daya 3.3V.\n", initState);
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    // 3. Konfigurasi CRC Hardware secara Eksplisit pada RadioLib
    Serial.print("3. Mengaktifkan Hardware CRC Check... ");
    int crcState = radio.setCRC(true);
    if (crcState != RADIOLIB_ERR_NONE) {
        Serial.printf("[GAGAL] Kode Galat CRC: %d\n", crcState);
        while (true) delay(1000);
    }
    Serial.println("[OK] CRC Enabled.");

    // 4. Alokasi Antrean FreeRTOS dan Verifikasi Hasil
    Serial.print("4. Mengalokasikan Antrean FreeRTOS (Kapasitas: 10 Paket)... ");
    xGatewayQueue = xQueueCreate(10, sizeof(GatewayPacketWrapper));
    if (xGatewayQueue == NULL) {
        Serial.println("[FATAL GAGAL] Memori heap tidak mencukupi untuk Queue!");
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    // 5. Buat Task FreeRTOS dan Verifikasi Status
    Serial.println("5. Memasang Task FreeRTOS ke Dual Core ESP32:");
    
    // Core 0 (PRO_CPU): Dispatcher Serial / Jaringan
    BaseType_t dispatchStatus = xTaskCreatePinnedToCore(
        vTaskGatewayDispatch, 
        "TaskDispatch", 
        4096, 
        NULL, 
        1, 
        NULL, 
        0       // Core 0 (PRO_CPU)
    );

    // Core 1 (APP_CPU): Radio Listener
    BaseType_t rxTaskStatus = xTaskCreatePinnedToCore(
        vTaskLoRaRxListener, 
        "TaskLoRaRx", 
        4096, 
        NULL, 
        3, 
        &xLoRaTaskHandle, 
        1       // Core 1 (APP_CPU)
    );

    if (dispatchStatus != pdPASS || rxTaskStatus != pdPASS || xLoRaTaskHandle == NULL) {
        Serial.println("[FATAL GAGAL] Gagal membuat salah satu Task FreeRTOS! Periksa alokasi heap.");
        while (true) delay(1000);
    }
    Serial.println("   -> TaskDispatch (Core 0, Prio 1) : [AKTIF]");
    Serial.println("   -> TaskLoRaRx   (Core 1, Prio 3) : [AKTIF]");

    // 6. URUTAN AMAN: Pasang Callback Interupsi SETELAH Task Siap
    // Mencegah race condition di mana ISR memanggil vTaskNotifyGiveFromISR pada handle NULL
    Serial.print("6. Mendaftarkan Callback Interupsi DIO0... ");
    radio.setPacketReceivedAction(setRxFlag);
    Serial.println("[OK]");

    // 7. Mulai Mendengarkan Paket di Udara secara Asinkron
    Serial.print("7. Mengaktifkan Radio Mode RX Asinkron (433.175 MHz)... ");
    int rxStartState = radio.startReceive();
    if (rxStartState != RADIOLIB_ERR_NONE) {
        Serial.printf("[GAGAL] Kode Galat startReceive: %d\n", rxStartState);
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    Serial.println("=================================================================");
    Serial.println(">>> GATEWAY SIAP MENERIMA PAKET DI 433.175 MHz <<<");
    Serial.println("=================================================================\n");
}

void loop() {
    // Biarkan loopTask tidur hemat daya tanpa membebani sistem
    vTaskDelay(pdMS_TO_TICKS(10000));
}
