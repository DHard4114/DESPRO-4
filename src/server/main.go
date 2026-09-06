package main

import (
	"context"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"smart-sanitation-esos/server/api"
	"smart-sanitation-esos/server/config"
	"smart-sanitation-esos/server/database"
	"smart-sanitation-esos/server/etl"
	mqttclient "smart-sanitation-esos/server/mqtt"
)

// @title Smart-Sanitation eSOS API
// @version 1.0
// @description API Enterprise-grade untuk manajemen posko sanitasi darurat (Open Intranet).
// @host 192.168.0.100:8000
// @BasePath /api/v1
func main() {
	log.Println("=======================================")
	log.Println("Memulai Smart-Sanitation eSOS Server...")
	log.Println("=======================================")

	// 1. Muat Konfigurasi (.env)
	cfg := config.LoadConfig()

	// 2. Inisialisasi Database (PostgreSQL via pgxpool)
	db, err := database.InitDB(cfg.DBURL)
	if err != nil {
		log.Fatalf("Gagal inisialisasi database: %v", err)
	}
	defer db.Close()
	log.Println("Berhasil terhubung ke PostgreSQL.")

	// 3. Inisialisasi In-Memory Threshold Cache & Listen/Notify
	if err := etl.GlobalThresholdCache.LoadAll(db.Pool); err != nil {
		log.Fatalf("Gagal memuat threshold cache: %v", err)
	}
	log.Println("Threshold Cache berhasil dimuat ke memory.")
	etl.GlobalThresholdCache.StartListenNotify(db.Pool)

	// 4. Inisialisasi & Jalankan WebSocket Hub
	wsHub := api.NewWSHub()
	go wsHub.Run()
	log.Println("WebSocket Hub berjalan.")

	// 5. Inisialisasi Mesin ETL (Pipeline)
	pipeline := etl.NewPipeline(db.Pool)
	pipeline.StartWorkers(8) // 8 Goroutine Workers
	pipeline.StartBatchInserter()
	log.Println("Mesin ETL Pipeline berjalan.")

	// 6. Inisialisasi & Hubungkan Klien MQTT (Subscriber)
	mqttConn, err := mqttclient.SubscribeToTelemetry(cfg, pipeline)
	if err != nil {
		log.Fatalf("Gagal menjalankan MQTT Subscriber: %v", err)
	}
	defer mqttConn.Disconnect(250)

	// 7. Setup & Jalankan HTTP Server (REST API + WS)
	router := api.SetupRouter(db.Pool, wsHub)
	server := &http.Server{
		Addr:    ":8000",
		Handler: router,
	}

	go func() {
		log.Println("HTTP Server (REST + WebSocket) mendengarkan di port :8000")
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("Kesalahan HTTP Server: %v", err)
		}
	}()

	// 8. Mekanisme Graceful Shutdown
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit // Blokir sampai sinyal ditangkap

	log.Println("\nMenerima sinyal shutdown, mematikan sistem secara anggun...")
	
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	if err := server.Shutdown(ctx); err != nil {
		log.Printf("Kesalahan saat shutdown HTTP Server: %v", err)
	}

	log.Println("Sistem berhasil dimatikan. Sampai jumpa.")
}
