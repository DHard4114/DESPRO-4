package config

import (
	"log"
	"os"

	"github.com/joho/godotenv"
)

// AppConfig menampung variabel lingkungan
type AppConfig struct {
	MQTTBrokerURL string
	MQTTUser      string
	MQTTPass      string
	DBURL         string
}

// LoadConfig memuat file .env dan mengembalikan AppConfig
func LoadConfig() *AppConfig {
	err := godotenv.Load()
	if err != nil {
		log.Println("Peringatan: File .env tidak ditemukan, menggunakan environment OS bawaan.")
	}

	return &AppConfig{
		MQTTBrokerURL: getEnv("MQTT_BROKER_URL", "tcp://192.168.0.100:1883"),
		// DILARANG hardcode kredensial Mosquitto [ADR-02 Backend Env]
		MQTTUser: getEnv("MQTT_USER", ""),
		MQTTPass: getEnv("MQTT_PASS", ""),
		DBURL:    getEnv("DB_URL", "postgres://user:pass@localhost:5432/esos_db?sslmode=disable"),
	}
}

// getEnv membaca environment dengan nilai default
func getEnv(key, fallback string) string {
	if value, exists := os.LookupEnv(key); exists {
		return value
	}
	return fallback
}
