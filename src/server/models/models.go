package models

import "time"

// RawTelemetryPacket represents the raw streaming data packet received from LoRa Gateway or HTTP Node
type RawTelemetryPacket struct {
	NodeID             string  `json:"node_id"`
	SequenceNo         int64   `json:"sequence_no"`
	WaterLevelCm       float64 `json:"water_level_cm"`
	AmmoniaPpm         float64 `json:"ammonia_ppm"`
	H2sPpm             float64 `json:"h2s_ppm"`
	BatteryVoltage     float64 `json:"battery_voltage"`
	SosButtonTriggered bool    `json:"sos_button_triggered"`
	ValveServoOpen     bool    `json:"valve_servo_open"`
	RssiDbm            int     `json:"rssi_dbm"`
	SnrDb              float64 `json:"snr_db"`
}

// TransformedTelemetry represents data after passing through the ETL pipeline (calibrated, normalized, enriched)
// Uses UUIDv4 Primary Key for seamless multi-posko synchronization
type TransformedTelemetry struct {
	RecordID             string    `json:"record_id"` // UUIDv4 Primary Key
	NodeID               string    `json:"node_id"`   // Node UUID / Code
	SequenceNo           int64     `json:"sequence_no"`
	WaterLevelCm         float64   `json:"water_level_cm"`
	WaterVolumePct       float64   `json:"water_volume_percentage"`
	AmmoniaPpm           float64   `json:"ammonia_ppm"`
	H2sPpm               float64   `json:"h2s_ppm"`
	BatteryVoltage       float64   `json:"battery_voltage"`
	BatteryPercentage    float64   `json:"battery_percentage"`
	SolarChargingActive  bool      `json:"solar_charging_active"`
	ValveServoOpen       bool      `json:"valve_servo_open"`
	SosButtonTriggered   bool      `json:"sos_button_triggered"`
	AirQualityIndex      string    `json:"air_quality_index"` // GOOD, MODERATE, HAZARDOUS
	AnomalyDetected      bool      `json:"anomaly_detected"`
	RssiDbm              int       `json:"rssi_dbm"`
	SnrDb                float64   `json:"snr_db"`
	ReceivedAt           time.Time `json:"received_at"`
}

// IncidentAlert represents emergency and safety violation events
// Uses UUIDv4 Primary Key for distributed alarm tracking
type IncidentAlert struct {
	AlertID     string     `json:"alert_id"` // UUIDv4 Primary Key
	NodeID      string     `json:"node_id"`
	AlertType   string     `json:"alert_type"` // SOS_BUTTON, GAS_LETHAL, WATER_EMPTY, POWER_FAIL
	Severity    string     `json:"severity"`   // INFO, WARNING, CRITICAL, EMERGENCY
	Description string     `json:"description"`
	IsResolved  bool       `json:"is_resolved"`
	ResolvedAt  *time.Time `json:"resolved_at,omitempty"`
	ResolvedBy  string     `json:"resolved_by,omitempty"`
	CreatedAt   time.Time  `json:"created_at"`
}

// SanitationNode represents registered physical stations
type SanitationNode struct {
	NodeID           string    `json:"node_id"` // UUIDv4 Primary Key
	NodeCode         string    `json:"node_code"` // e.g. 'NODE_SANITATION_01'
	LocationName     string    `json:"location_name"`
	Latitude         float64   `json:"latitude"`
	Longitude        float64   `json:"longitude"`
	InstallationDate time.Time `json:"installation_date"`
	Status           string    `json:"status"` // ACTIVE, INACTIVE, ALERT
}

// ActuationCommand represents valve and hardware action logs
type ActuationCommand struct {
	CommandID   string    `json:"command_id"` // UUIDv4 Primary Key
	NodeID      string    `json:"node_id"`
	CommandType string    `json:"command_type"` // OPEN_VALVE, CLOSE_VALVE, RESET_ALARM
	TriggeredBy string    `json:"triggered_by"` // SENSOR_AUTO, OPERATOR_MANUAL
	ExecutedAt  time.Time `json:"executed_at"`
	Status      string    `json:"status"`
}

// WebSocketEvent defines streaming packet format for live dashboard push
type WebSocketEvent struct {
	Type      string      `json:"type"` // "TELEMETRY_STREAM", "EMERGENCY_ALERT", "VALVE_STATE"
	Payload   interface{} `json:"payload"`
	Timestamp time.Time   `json:"timestamp"`
}
