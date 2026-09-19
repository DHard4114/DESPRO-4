package api

import (
	"net/http"

	"github.com/gorilla/mux"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/rs/cors"
)

// SetupRouter merangkai router, middleware, REST endpoint, dan WebSocket
func SetupRouter(dbPool *pgxpool.Pool, wsHub *WSHub) http.Handler {
	router := mux.NewRouter()

	apiHandler := &API{
		DBPool: dbPool,
	}

	// Middleware CORS Terbuka (Open Intranet)
	c := cors.New(cors.Options{
		AllowedOrigins: []string{"*"}, // Mengizinkan semua karena ini jaringan lokal posko
		AllowedMethods: []string{"GET", "POST", "OPTIONS"},
		AllowedHeaders: []string{"Content-Type", "Idempotency-Key"},
	})

	// Subrouter untuk /api/v1
	v1 := router.PathPrefix("/api/v1").Subrouter()

	// REST API Endpoint
	v1.HandleFunc("/nodes/{node_code}/telemetry/latest", apiHandler.GetLatestTelemetry).Methods("GET")
	v1.HandleFunc("/actuator/commands", apiHandler.PostActuatorCommand).Methods("POST")

	// WebSocket Endpoint
	v1.HandleFunc("/ws", wsHub.ServeWS)

	// Sajikan file statis Dashboard UI (index.html) pada root "/"
	router.PathPrefix("/").Handler(http.FileServer(http.Dir("./static")))

	// Terapkan CORS
	handler := c.Handler(router)

	return handler
}
