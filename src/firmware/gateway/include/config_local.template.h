#ifndef CONFIG_LOCAL_H
#define CONFIG_LOCAL_H

// =========================================================================
// SMART-SANITATION eSOS — TEMPLATE KREDENSIAL LOKAL GATEWAY
// =========================================================================
// Petunjuk Penggunaan:
// 1. Salin berkas ini menjadi 'config_local.h' di direktori yang sama:
//    src/firmware/gateway/include/config_local.h
// 2. Berkas 'config_local.h' diabaikan oleh Git (.gitignore) sehingga
//    kredensial nyata TIDAK akan ter-commit ke repositori publik/tim.
// 3. Ubah konstanta di bawah sesuai konfigurasi jaringan CPE220 / Laptop Anda.

#define WIFI_SSID           "CompEngQuiz-Server-Live"
#define WIFI_PASS           "MASUKKAN_PASSWORD_WIFI_DISINI"

// Konfigurasi IP Statis Gateway ESP32 (Subnet 192.168.101.0/24)
#define STATIC_IP_LOCAL     192, 168, 101, 11
#define STATIC_IP_GATEWAY   192, 168, 101, 1
#define STATIC_IP_SUBNET    255, 255, 255, 0
#define STATIC_IP_DNS       152, 118, 24, 4

// Konfigurasi Laptop / Broker Mosquitto MQTT
#define MQTT_SERVER         "192.168.101.10"
#define MQTT_PORT           1883
#define MQTT_USER           "esos_gateway"
#define MQTT_PASS           "MASUKKAN_PASSWORD_MQTT_DISINI"

#endif // CONFIG_LOCAL_H
