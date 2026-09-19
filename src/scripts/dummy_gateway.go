package main

import (
	"encoding/json"
	"log"
	"math/rand"
	"time"

	paho "github.com/eclipse/paho.mqtt.golang"
)

// ESP32 Payload Structure (Sesuai dengan kode firmware Siti)
type TelemetryPayload struct {
	NodeCode       string  `json:"node_code"`
	Timestamp      int64   `json:"timestamp"`
	WaterLevelCM   float64 `json:"water_level_cm"`
	AmmoniaPPM     float64 `json:"ammonia_ppm"`
	H2SPPM         float64 `json:"h2s_ppm"`
	BatteryVoltage float64 `json:"battery_voltage"`
	SOSTriggered   int     `json:"sos_triggered"`
}

func main() {
	log.Println("=== Memulai Simulator Gateway eSOS (EMS Dummy Data) ===")

	opts := paho.NewClientOptions()
	opts.AddBroker("tcp://localhost:1883")
	opts.SetClientID("EMS_Simulator_Node")
	
	client := paho.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("Gagal menyambung ke Mosquitto: %v", token.Error())
	}
	defer client.Disconnect(250)

	log.Println("Tersambung ke MQTT Broker. Memulai injeksi data palsu setiap 2 detik...")
	log.Println("Buka browser ke http://localhost:8000 untuk melihat pergerakan grafik EMS!")

	// Beritahu server bahwa Gateway sedang ONLINE
	client.Publish("esos/gateway_01/status", 1, false, "ONLINE")

	rand.Seed(time.Now().UnixNano())

	// Nilai dasar simulasi
	waterLevel := 50.0
	ammonia := 5.0
	h2s := 0.5
	batt := 4.0

	for {
		// Buat pergerakan grafik terlihat realistis (random walk)
		waterLevel += (rand.Float64() * 4) - 2
		ammonia += (rand.Float64() * 2) - 1
		h2s += (rand.Float64() * 0.5) - 0.25
		batt -= 0.001 // Baterai perlahan turun

		// Jaga dalam batas logis
		if waterLevel < 0 { waterLevel = 0 }
		if waterLevel > 200 { waterLevel = 200 }
		if ammonia < 0 { ammonia = 0 }
		if ammonia > 30 { ammonia = 30 } // Bisa jadi merah (Danger)
		if h2s < 0 { h2s = 0 }
		if h2s > 6 { h2s = 6 }
		if batt < 3.2 { batt = 4.2 } // Baterai dicharge ulang jika habis

		payload := TelemetryPayload{
			NodeCode:       "WC_01",
			Timestamp:      time.Now().Unix(),
			WaterLevelCM:   waterLevel,
			AmmoniaPPM:     ammonia,
			H2SPPM:         h2s,
			BatteryVoltage: batt,
			SOSTriggered:   0,
		}

		// Simulasi orang menekan tombol darurat (1% peluang)
		if rand.Intn(100) > 98 {
			payload.SOSTriggered = 1
			log.Println("⚠️ MENGIRIM SINYAL SOS DARURAT!")
		}

		bytes, _ := json.Marshal(payload)
		topic := "esos/gateway_01/WC_01/telemetry"
		
		token := client.Publish(topic, 1, false, bytes)
		token.Wait()

		log.Printf("Mengirim ke %s -> Water: %.1f cm | NH3: %.1f ppm | Baterai: %.2f V\n", topic, waterLevel, ammonia, batt)
		
		time.Sleep(2 * time.Second)
	}
}
