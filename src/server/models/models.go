package models

import "time"

// TelemetryPayload merepresentasikan JSON Envelope yang dikirim dari Gateway.
// Field disesuaikan dengan C-Struct __attribute__((packed)) [ADR-01]
type TelemetryPayload struct {
	SchemaVersion  uint8   `json:"schema_version"`
	NodeCode       string  `json:"node_code"` // Contoh: "WC_01"
	SequenceNo     uint32  `json:"sequence_no"`
	WaterLevelCM   float32 `json:"water_level_cm"`
	AmmoniaPPM     float32 `json:"ammonia_ppm"`
	H2SPPM         float32 `json:"h2s_ppm"`
	BatteryVoltage float32 `json:"battery_voltage"`
	SOSTriggered   uint8   `json:"sos_triggered"`
}

// ThresholdConfig menampung data dari tabel node_threshold_configs [ADR-07]
type ThresholdConfig struct {
	NodeID        string
	MetricName    string
	WarningValue  float64
	CriticalValue float64
}

// TelemetryRecord merepresentasikan data yang siap diinsert ke PostgreSQL.
type TelemetryRecord struct {
	NodeID         string // UUIDv7 Format
	NodeCode       string
	SequenceNo     uint32
	WaterLevelCM   float32
	AmmoniaPPM     float32
	H2SPPM         float32
	BatteryVoltage float32
	SOSTriggered   bool
	ReceivedAt     time.Time
}

// RawMQTTMessage digunakan untuk menampung pesan MQTT sebelum diolah pekerja ETL.
type RawMQTTMessage struct {
	Topic   string
	Payload []byte
}
