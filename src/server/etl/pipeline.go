package etl

import (
	"log"
	"math"
	"sync"
	"time"

	"github.com/google/uuid"
	"esos-server/database"
	"esos-server/models"
)

type HubBroadcaster interface {
	Broadcast(event models.WebSocketEvent)
}

// Pipeline represents the unified Streaming & Batch ETL Engine
type Pipeline struct {
	db          *database.DB
	broadcaster HubBroadcaster
	inStream    chan models.RawTelemetryPacket
	batchBuffer []models.TransformedTelemetry
	batchMu     sync.Mutex
	batchSize   int
	flushTimer  *time.Ticker
	stopChan    chan struct{}
}

// NewPipeline creates and starts the ETL background worker
func NewPipeline(db *database.DB, broadcaster HubBroadcaster, batchSize int, flushInterval time.Duration) *Pipeline {
	if batchSize <= 0 {
		batchSize = 20
	}
	if flushInterval <= 0 {
		flushInterval = 3 * time.Second
	}

	p := &Pipeline{
		db:          db,
		broadcaster: broadcaster,
		inStream:    make(chan models.RawTelemetryPacket, 1000),
		batchBuffer: make([]models.TransformedTelemetry, 0, batchSize),
		batchSize:   batchSize,
		flushTimer:  time.NewTicker(flushInterval),
		stopChan:    make(chan struct{}),
	}

	go p.runWorker()
	log.Printf("[ETL ENGINE] Streaming & Batch Pipeline started (BatchSize=%d, FlushInterval=%v, UUID Keys)", batchSize, flushInterval)
	return p
}

// Ingest streams raw sensor packets into the high-speed ingest channel
func (p *Pipeline) Ingest(raw models.RawTelemetryPacket) {
	select {
	case p.inStream <- raw:
	default:
		log.Printf("[WARN] Ingest buffer full! Dropping packet sequence #%d", raw.SequenceNo)
	}
}

// Close gracefully flushes all remaining records and shuts down the worker
func (p *Pipeline) Close() {
	close(p.stopChan)
	p.flushTimer.Stop()
	p.flushBatch()
}

func (p *Pipeline) runWorker() {
	for {
		select {
		case raw, ok := <-p.inStream:
			if !ok {
				return
			}
			// 1. Transform & Enrich
			transformed := p.transform(raw)

			// 2. Immediate Real-Time Stream Broadcast to WebSocket clients
			if p.broadcaster != nil {
				p.broadcaster.Broadcast(models.WebSocketEvent{
					Type:      "TELEMETRY_STREAM",
					Payload:   transformed,
					Timestamp: time.Now(),
				})
			}

			// 3. Append to Batch Buffer for Bulk Loading
			p.batchMu.Lock()
			p.batchBuffer = append(p.batchBuffer, transformed)
			if len(p.batchBuffer) >= p.batchSize {
				p.flushBatchLocked()
			}
			p.batchMu.Unlock()

		case <-p.flushTimer.C:
			p.flushBatch()

		case <-p.stopChan:
			return
		}
	}
}

// transform executes calibration formulas, metric calculations, and automated threshold alert generation
func (p *Pipeline) transform(raw models.RawTelemetryPacket) models.TransformedTelemetry {
	now := time.Now()

	// 1. Calculate water volume percentage (Assumes 100cm max height)
	tankHeight := 100.0
	volPct := math.Max(0.0, math.Min(100.0, (raw.WaterLevelCm/tankHeight)*100.0))

	// 2. Calculate battery percentage (3.0V cutoff = 0%, 4.2V max = 100%)
	batPct := math.Max(0.0, math.Min(100.0, ((raw.BatteryVoltage-3.0)/1.2)*100.0))

	// 3. Determine Air Quality Index (AQI) Category
	aqi := "GOOD"
	if raw.H2sPpm > 10.0 || raw.AmmoniaPpm > 25.0 {
		aqi = "HAZARDOUS"
	} else if raw.H2sPpm > 5.0 || raw.AmmoniaPpm > 15.0 {
		aqi = "MODERATE"
	}

	// 4. Anomaly detection flag
	anomaly := (raw.WaterLevelCm < 0 || raw.WaterLevelCm > 150.0 || raw.BatteryVoltage < 2.5)

	transformed := models.TransformedTelemetry{
		RecordID:            uuid.Must(uuid.NewV7()).String(), // UUIDv7 Time-Ordered (RFC 9562) for fast B-Tree indexing
		NodeID:              raw.NodeID,
		SequenceNo:          raw.SequenceNo,
		WaterLevelCm:        math.Round(raw.WaterLevelCm*10) / 10.0,
		WaterVolumePct:      math.Round(volPct*10) / 10.0,
		AmmoniaPpm:          math.Round(raw.AmmoniaPpm*10) / 10.0,
		H2sPpm:              math.Round(raw.H2sPpm*10) / 10.0,
		BatteryVoltage:      math.Round(raw.BatteryVoltage*100) / 100.0,
		BatteryPercentage:   math.Round(batPct*10) / 10.0,
		SolarChargingActive: raw.BatteryVoltage > 3.6,
		ValveServoOpen:      raw.ValveServoOpen,
		SosButtonTriggered:  raw.SosButtonTriggered,
		AirQualityIndex:     aqi,
		AnomalyDetected:     anomaly,
		RssiDbm:             raw.RssiDbm,
		SnrDb:               raw.SnrDb,
		ReceivedAt:          now,
	}

	// 5. Automated Alert Evaluation & Trigger
	p.evaluateAlerts(transformed)

	return transformed
}

func (p *Pipeline) evaluateAlerts(t models.TransformedTelemetry) {
	if t.SosButtonTriggered {
		alert := models.IncidentAlert{
			AlertID:     uuid.Must(uuid.NewV7()).String(),
			NodeID:      t.NodeID,
			AlertType:   "SOS_BUTTON",
			Severity:    "EMERGENCY",
			Description: "Peringatan Darurat: Tombol SOS bilik sanitasi ditekan oleh pengungsi!",
			CreatedAt:   t.ReceivedAt,
		}
		p.db.InsertAlert(alert)
		if p.broadcaster != nil {
			p.broadcaster.Broadcast(models.WebSocketEvent{
				Type:      "EMERGENCY_ALERT",
				Payload:   alert,
				Timestamp: time.Now(),
			})
		}
	} else if t.AmmoniaPpm > 25.0 {
		alert := models.IncidentAlert{
			AlertID:     uuid.Must(uuid.NewV7()).String(),
			NodeID:      t.NodeID,
			AlertType:   "GAS_AMMONIA_HIGH",
			Severity:    "WARNING",
			Description: "Konsentrasi gas amonia (NH3) melebihi batas ambang aman (25 ppm)!",
			CreatedAt:   t.ReceivedAt,
		}
		p.db.InsertAlert(alert)
	} else if t.H2sPpm > 10.0 {
		alert := models.IncidentAlert{
			AlertID:     uuid.Must(uuid.NewV7()).String(),
			NodeID:      t.NodeID,
			AlertType:   "GAS_H2S_HIGH",
			Severity:    "CRITICAL",
			Description: "Bahaya: Konsentrasi gas beracun H2S terdeteksi di atas batas aman (10 ppm)!",
			CreatedAt:   t.ReceivedAt,
		}
		p.db.InsertAlert(alert)
	} else if t.WaterLevelCm < 15.0 {
		alert := models.IncidentAlert{
			AlertID:     uuid.Must(uuid.NewV7()).String(),
			NodeID:      t.NodeID,
			AlertType:   "WATER_EMPTY",
			Severity:    "WARNING",
			Description: "Ketersediaan air bersih pada tangki sanitasi berada pada tingkat kritis (< 15 cm)!",
			CreatedAt:   t.ReceivedAt,
		}
		p.db.InsertAlert(alert)
	}
}

func (p *Pipeline) flushBatch() {
	p.batchMu.Lock()
	defer p.batchMu.Unlock()
	p.flushBatchLocked()
}

func (p *Pipeline) flushBatchLocked() {
	if len(p.batchBuffer) == 0 {
		return
	}

	records := make([]models.TransformedTelemetry, len(p.batchBuffer))
	copy(records, p.batchBuffer)
	p.batchBuffer = p.batchBuffer[:0] // Reset slice length

	// Execute bulk load in database
	go func(recs []models.TransformedTelemetry) {
		if err := p.db.BatchInsertTelemetry(recs); err != nil {
			log.Printf("[ETL ERROR] Failed to flush batch of %d records to DB: %v", len(recs), err)
		} else {
			log.Printf("[ETL LOAD] Successfully committed batch of %d records to SQLite", len(recs))
		}
	}(records)
}
