// Package database menyediakan koneksi thread-safe ke PostgreSQL
// menggunakan pgxpool untuk menunjang Goroutine Worker Pool ETL.
// Kepatuhan: ADR-06 (PostgreSQL + TimescaleDB)
package database

import (
	"context"
	"fmt"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
)

// DB membungkus pgxpool.Pool agar dapat diinjeksi ke seluruh
// layer aplikasi (ETL, API, WebSocket) secara thread-safe.
type DB struct {
	Pool *pgxpool.Pool
}

// InitDB membuat koneksi pool ke PostgreSQL dengan konfigurasi
// standar produksi. Parameter databaseURL menggunakan format DSN:
// "postgres://user:pass@host:5432/esos_db?sslmode=disable"
//
// Kredensial WAJIB dibaca dari environment variable (.env),
// DILARANG di-hardcode di source code manapun.
func InitDB(databaseURL string) (*DB, error) {
	config, err := pgxpool.ParseConfig(databaseURL)
	if err != nil {
		return nil, fmt.Errorf("database: gagal parsing DSN: %w", err)
	}

	// === Konfigurasi Pool Standar Produksi ===
	config.MaxConns = 20
	config.MinConns = 5
	config.MaxConnLifetime = 1 * time.Hour
	config.MaxConnIdleTime = 30 * time.Minute
	config.HealthCheckPeriod = 1 * time.Minute

	// === Koneksi dengan Timeout ===
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	pool, err := pgxpool.NewWithConfig(ctx, config)
	if err != nil {
		return nil, fmt.Errorf("database: gagal membuat pool: %w", err)
	}

	// === Verifikasi Koneksi ===
	if err := pool.Ping(ctx); err != nil {
		pool.Close()
		return nil, fmt.Errorf("database: gagal ping PostgreSQL: %w", err)
	}

	return &DB{Pool: pool}, nil
}

// Close memutus seluruh koneksi di pool secara anggun (graceful shutdown).
// Panggil fungsi ini di main() saat aplikasi menerima sinyal SIGTERM/SIGINT.
func (db *DB) Close() {
	if db.Pool != nil {
		db.Pool.Close()
	}
}
