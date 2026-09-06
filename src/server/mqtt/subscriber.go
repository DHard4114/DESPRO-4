package mqtt

import (
	"fmt"
	"log"

	"smart-sanitation-esos/server/config"
	"smart-sanitation-esos/server/etl"
	"smart-sanitation-esos/server/models"
	paho "github.com/eclipse/paho.mqtt.golang"
)

// SubscribeToTelemetry melakukan inisialisasi koneksi ke Mosquitto
// dan menjalankan fungsi callback OnMessage yang bersifat non-blocking [ADR-01].
func SubscribeToTelemetry(cfg *config.AppConfig, pipeline *etl.Pipeline) (paho.Client, error) {
	opts := paho.NewClientOptions()
	opts.AddBroker(cfg.MQTTBrokerURL)
	opts.SetClientID("Go_ETL_Server_01")
	opts.SetUsername(cfg.MQTTUser)
	opts.SetPassword(cfg.MQTTPass)

	// CleanSession = false agar pesan yang terlewat saat server mati
	// tetap dikirim oleh Mosquitto setelah server online kembali (QoS 1).
	opts.SetCleanSession(false)

	// MUTLAK: Handler ini DILARANG memproses JSON.
	// Tugasnya hanya mendorong raw byte ke channel InStream.
	opts.SetDefaultPublishHandler(func(client paho.Client, msg paho.Message) {
		rawMsg := models.RawMQTTMessage{
			Topic:   msg.Topic(),
			Payload: msg.Payload(),
		}
		// Push secepat kilat ke Buffered Channel tanpa blocking (jika channel tidak penuh)
		select {
		case pipeline.InStream <- rawMsg:
			// Sukses push
		default:
			log.Println("Peringatan: Channel InStream PENUH! Pesan MQTT didrop.")
		}
	})

	opts.OnConnect = func(c paho.Client) {
		log.Println("MQTT Subscriber terhubung ke broker.")
		// Subscribe ke wildcard kluster gateway dengan QoS 1
		topic := "esos/+/+/telemetry"
		if token := c.Subscribe(topic, 1, nil); token.Wait() && token.Error() != nil {
			log.Fatalf("Gagal subscribe ke %s: %v", topic, token.Error())
		}
		log.Printf("Berhasil subscribe ke topik: %s", topic)
	}

	opts.OnConnectionLost = func(c paho.Client, err error) {
		log.Printf("Koneksi MQTT terputus: %v", err)
	}

	client := paho.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		return nil, fmt.Errorf("gagal menyambung ke MQTT: %w", token.Error())
	}

	return client, nil
}
