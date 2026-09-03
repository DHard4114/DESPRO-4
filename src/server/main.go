package main

import (
	"log"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"esos-server/api"
	"esos-server/database"
	"esos-server/etl"
)

func main() {
	log.Println("==========================================================")
	log.Println(" SMART-SANITATION eSOS — HIGH-PERFORMANCE GO BACKEND")
	log.Println(" Streaming Ingestion, Batch ETL & Real-Time Web Server")
	log.Println(" Lead: Daffa Hardhan (Project Manager & Backend Lead)")
	log.Println(" Course: Desain Proyek 2 (DTE FTUI Gasal 2026/2027)")
	log.Println("==========================================================")

	// Database Path Multi-Path Resolver
	dbPath := os.Getenv("DATABASE_PATH")
	if dbPath == "" {
		possibleDbPaths := []string{
			filepath.Join("src", "data", "esos_telemetry.db"),
			filepath.Join("..", "data", "esos_telemetry.db"),
			filepath.Join("data", "esos_telemetry.db"),
		}
		dbPath = possibleDbPaths[0]
		for _, p := range possibleDbPaths {
			if _, err := os.Stat(filepath.Dir(p)); err == nil {
				dbPath = p
				break
			}
		}
	}

	// 1. Initialize SQLite Database (WAL Mode)
	db, err := database.InitDB(dbPath)
	if err != nil {
		log.Fatalf("[FATAL] Failed to initialize database: %v", err)
	}
	defer db.Close()

	// 2. Initialize Real-Time WebSocket Streaming Hub
	wsHub := api.NewHub()

	// 3. Initialize Streaming & Batch ETL Engine
	// Flushes every 20 records or every 3 seconds to optimize SQLite write throughput
	pipeline := etl.NewPipeline(db, wsHub, 20, 3*time.Second)
	defer pipeline.Close()

	// 4. Initialize REST API Handlers
	handler := api.NewHandler(db, pipeline, wsHub)

	// 5. Setup HTTP Routing
	mux := http.NewServeMux()

	// WebSocket Endpoint
	mux.HandleFunc("/ws", wsHub.ServeWS)

	// REST API Endpoints
	mux.HandleFunc("/api/telemetry", handler.IngestTelemetry)
	mux.HandleFunc("/api/telemetry/latest", handler.GetLatestTelemetry)
	mux.HandleFunc("/api/telemetry/history", handler.GetHistoricalTelemetry)
	mux.HandleFunc("/api/alerts/active", handler.GetActiveAlerts)
	mux.HandleFunc("/api/alerts/resolve", handler.ResolveAlert)

	// Static Web Dashboard Multi-Path Resolver
	possibleDirs := []string{
		filepath.Join("src", "server", "static"),
		filepath.Join("..", "server", "static"),
		filepath.Join("..", "src", "server", "static"),
		"static",
	}
	staticDir := filepath.Join("src", "server", "static")
	for _, dir := range possibleDirs {
		if info, err := os.Stat(dir); err == nil && info.IsDir() {
			staticDir = dir
			break
		}
	}
	log.Printf("[STATIC] Serving Web Dashboard assets from: %s", staticDir)
	fs := http.FileServer(http.Dir(staticDir))
	mux.Handle("/", fs)

	// 6. Start HTTP Server
	port := os.Getenv("SERVER_PORT")
	if port == "" {
		port = "8000"
	}

	server := &http.Server{
		Addr:         ":" + port,
		Handler:      mux,
		ReadTimeout:  10 * time.Second,
		WriteTimeout: 10 * time.Second,
		IdleTimeout:  60 * time.Second,
	}

	log.Printf("[SERVER] Starting Go HTTP & WebSocket Server on http://0.0.0.0:%s", port)
	log.Printf("[DASHBOARD] Web Dashboard accessible at http://localhost:%s/", port)

	if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		log.Fatalf("[FATAL] Server terminated unexpectedly: %v", err)
	}
}
