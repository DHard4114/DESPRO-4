package api

import (
	"context"
	"encoding/json"
	"net/http"
	"time"

	"github.com/google/uuid"
	"github.com/gorilla/mux"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

// API struct menampung dependensi koneksi DB
type API struct {
	DBPool *pgxpool.Pool
}

// ErrorEnvelope adalah standar error response REST API [ADR-05]
type ErrorEnvelope struct {
	Status string `json:"status"` // Selalu "error"
	Error  struct {
		Code    string `json:"code"`
		Message string `json:"message"`
		TraceID string `json:"trace_id"`
	} `json:"error"`
}

// respondError memformat response error menjadi Error Envelope
func respondError(w http.ResponseWriter, statusCode int, code, message string) {
	traceID, _ := uuid.NewV7() // Menggunakan UUIDv7 untuk TraceID
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(statusCode)
	json.NewEncoder(w).Encode(ErrorEnvelope{
		Status: "error",
		Error: struct {
			Code    string `json:"code"`
			Message string `json:"message"`
			TraceID string `json:"trace_id"`
		}{
			Code:    code,
			Message: message,
			TraceID: traceID.String(),
		},
	})
}

// GetLatestTelemetry godoc
// @Summary Ambil telemetri terbaru
// @Description Mengambil satu rekor data telemetri terbaru berdasarkan node_code
// @Tags Telemetry
// @Produce json
// @Param node_code path string true "Kode Node WC (misal: WC_01)"
// @Success 200 {object} map[string]interface{}
// @Failure 404 {object} ErrorEnvelope
// @Failure 500 {object} ErrorEnvelope
// @Router /api/v1/nodes/{node_code}/telemetry/latest [get]
func (api *API) GetLatestTelemetry(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	nodeCode := vars["node_code"]

	ctx, cancel := context.WithTimeout(r.Context(), 5*time.Second)
	defer cancel()

	// Query data terakhir menggunakan node_code
	// Bergantung pada join atau subquery, di sini kita gunakan node_code
	// secara langsung yang ada di tabel telemetry_records (sesuai ERD Fase 0).
	query := `
		SELECT sequence_no, water_level_cm, ammonia_ppm, h2s_ppm, battery_voltage, sos_triggered, received_at
		FROM telemetry_records
		WHERE node_code = $1
		ORDER BY received_at DESC
		LIMIT 1
	`

	var rec struct {
		SequenceNo     uint32    `json:"sequence_no"`
		WaterLevelCM   float32   `json:"water_level_cm"`
		AmmoniaPPM     float32   `json:"ammonia_ppm"`
		H2SPPM         float32   `json:"h2s_ppm"`
		BatteryVoltage float32   `json:"battery_voltage"`
		SOSTriggered   bool      `json:"sos_triggered"`
		ReceivedAt     time.Time `json:"received_at"`
	}

	err := api.DBPool.QueryRow(ctx, query, nodeCode).Scan(
		&rec.SequenceNo, &rec.WaterLevelCM, &rec.AmmoniaPPM,
		&rec.H2SPPM, &rec.BatteryVoltage, &rec.SOSTriggered, &rec.ReceivedAt,
	)

	if err != nil {
		if err == pgx.ErrNoRows {
			respondError(w, http.StatusNotFound, "NOT_FOUND", "Data telemetri tidak ditemukan untuk node_code tersebut.")
			return
		}
		respondError(w, http.StatusInternalServerError, "DB_ERROR", "Gagal mengeksekusi query database.")
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"status": "success",
		"data":   rec,
	})
}

// ActuationCommandPayload mendefinisikan JSON request body untuk perintah servo
type ActuationCommandPayload struct {
	NodeCode       string `json:"node_code"`
	CommandType    string `json:"command_type"` // OPEN_VALVE, CLOSE_VALVE, FLUSH_TANK
	TargetAngleDeg int    `json:"target_angle_deg,omitempty"`
}

// PostActuatorCommand godoc
// @Summary Kirim perintah aktuator (Servo)
// @Description Endpoint menerima perintah aktuator untuk node tertentu dan menyimpannya ke antrean database.
// @Tags Actuator
// @Accept json
// @Produce json
// @Param Idempotency-Key header string true "UUID Idempotency Key untuk mencegah eksekusi ganda"
// @Param payload body ActuationCommandPayload true "Detail Perintah"
// @Success 202 {object} map[string]interface{}
// @Failure 400 {object} ErrorEnvelope
// @Failure 409 {object} ErrorEnvelope
// @Failure 500 {object} ErrorEnvelope
// @Router /api/v1/actuator/commands [post]
func (api *API) PostActuatorCommand(w http.ResponseWriter, r *http.Request) {
	// Wajib mengecek header Idempotency-Key [ADR-05]
	idempotencyKeyStr := r.Header.Get("Idempotency-Key")
	if idempotencyKeyStr == "" {
		respondError(w, http.StatusBadRequest, "MISSING_HEADER", "Header Idempotency-Key (UUID) wajib disertakan.")
		return
	}

	idempotencyKey, err := uuid.Parse(idempotencyKeyStr)
	if err != nil {
		respondError(w, http.StatusBadRequest, "INVALID_UUID", "Idempotency-Key harus berformat UUID.")
		return
	}

	var payload ActuationCommandPayload
	if err := json.NewDecoder(r.Body).Decode(&payload); err != nil {
		respondError(w, http.StatusBadRequest, "INVALID_JSON", "Format JSON payload tidak valid.")
		return
	}

	ctx, cancel := context.WithTimeout(r.Context(), 5*time.Second)
	defer cancel()

	// Resolve node_code menjadi node_id untuk konsistensi Relational Integrity
	var nodeID uuid.UUID
	err = api.DBPool.QueryRow(ctx, "SELECT node_id FROM sanitation_nodes WHERE node_code = $1", payload.NodeCode).Scan(&nodeID)
	if err != nil {
		if err == pgx.ErrNoRows {
			respondError(w, http.StatusBadRequest, "NODE_NOT_FOUND", "Node code tidak ditemukan.")
			return
		}
		respondError(w, http.StatusInternalServerError, "DB_ERROR", "Gagal memvalidasi node code.")
		return
	}

	// Insert ke tabel actuation_commands (Akan gagal jika Idempotency Key sama [ADR-05])
	insertQuery := `
		INSERT INTO actuation_commands (node_id, idempotency_key, command_type, status)
		VALUES ($1, $2, $3, 'PENDING')
		RETURNING command_id
	`
	var commandID uuid.UUID
	err = api.DBPool.QueryRow(ctx, insertQuery, nodeID, idempotencyKey, payload.CommandType).Scan(&commandID)
	if err != nil {
		// Asumsi error adalah constraint violation (Idempotency)
		respondError(w, http.StatusConflict, "IDEMPOTENCY_CONFLICT", "Perintah dengan Idempotency-Key ini sudah pernah diproses.")
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusAccepted) // HTTP 202 Accepted
	json.NewEncoder(w).Encode(map[string]interface{}{
		"status": "success",
		"data": map[string]interface{}{
			"command_id": commandID.String(),
			"message":    "Perintah diterima dan sedang diproses (PENDING).",
		},
	})
}
