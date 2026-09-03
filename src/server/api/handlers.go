package api

import (
	"encoding/json"
	"net/http"
	"strconv"
	"time"

	"esos-server/database"
	"esos-server/etl"
	"esos-server/models"
)

type Handler struct {
	db       *database.DB
	pipeline *etl.Pipeline
	hub      *Hub
}

func NewHandler(db *database.DB, pipeline *etl.Pipeline, hub *Hub) *Handler {
	return &Handler{
		db:       db,
		pipeline: pipeline,
		hub:      hub,
	}
}

// IngestTelemetry handles streaming telemetry packets from LoRa Gateway or HTTP sensor nodes
func (h *Handler) IngestTelemetry(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var raw models.RawTelemetryPacket
	if err := json.NewDecoder(r.Body).Decode(&raw); err != nil {
		http.Error(w, "Invalid JSON payload: "+err.Error(), http.StatusBadRequest)
		return
	}

	if raw.NodeID == "" {
		raw.NodeID = "NODE_SANITATION_01"
	}

	// Send to high-speed ETL pipeline (non-blocking)
	h.pipeline.Ingest(raw)

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"status":      "ACCEPTED",
		"node_id":     raw.NodeID,
		"sequence_no": raw.SequenceNo,
		"ingested_at": time.Now(),
	})
}

// GetLatestTelemetry returns the latest telemetry snapshot for each node
func (h *Handler) GetLatestTelemetry(w http.ResponseWriter, r *http.Request) {
	nodeID := r.URL.Query().Get("node_id")
	records, err := h.db.GetLatestTelemetry(nodeID)
	if err != nil {
		http.Error(w, "Failed to query telemetry: "+err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(records)
}

// GetHistoricalTelemetry returns time-series data for live charts
func (h *Handler) GetHistoricalTelemetry(w http.ResponseWriter, r *http.Request) {
	nodeID := r.URL.Query().Get("node_id")
	limitStr := r.URL.Query().Get("limit")
	limit := 50
	if limitStr != "" {
		if l, err := strconv.Atoi(limitStr); err == nil {
			limit = l
		}
	}

	records, err := h.db.GetHistoricalTelemetry(nodeID, limit)
	if err != nil {
		http.Error(w, "Failed to query history: "+err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(records)
}

// GetActiveAlerts returns active emergency alerts
func (h *Handler) GetActiveAlerts(w http.ResponseWriter, r *http.Request) {
	alerts, err := h.db.GetActiveAlerts()
	if err != nil {
		http.Error(w, "Failed to query alerts: "+err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(alerts)
}

// ResolveAlert marks an alert as resolved by its UUID
func (h *Handler) ResolveAlert(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	alertID := r.URL.Query().Get("alert_id")
	if alertID == "" {
		http.Error(w, "Missing alert_id parameter", http.StatusBadRequest)
		return
	}

	resolvedBy := r.URL.Query().Get("resolved_by")
	if resolvedBy == "" {
		resolvedBy = "Petugas Posko"
	}

	if err := h.db.ResolveAlert(alertID, resolvedBy); err != nil {
		http.Error(w, "Failed to resolve alert: "+err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]interface{}{
		"status":      "RESOLVED",
		"alert_id":    alertID,
		"resolved_by": resolvedBy,
	})
}
