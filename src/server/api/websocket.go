package api

import (
	"log"
	"net/http"
	"sync"

	"github.com/gorilla/websocket"
)

// WSEvent merepresentasikan pesan standar yang dikirim melalui WebSocket [ADR-03]
type WSEvent struct {
	Type    string      `json:"type"` // TELEMETRY_STREAM, EMERGENCY_ALERT, GATEWAY_STATUS, ACTUATOR_STATUS
	Payload interface{} `json:"payload"`
}

// WSHub mengatur koneksi klien aktif dan mendistribusikan pesan secara thread-safe
type WSHub struct {
	mu        sync.Mutex
	clients   map[*websocket.Conn]bool
	Broadcast chan WSEvent
}

func NewWSHub() *WSHub {
	return &WSHub{
		clients:   make(map[*websocket.Conn]bool),
		Broadcast: make(chan WSEvent, 256), // Buffered channel
	}
}

// Run menjalankan hub untuk memproses broadcast ke semua klien aktif
func (h *WSHub) Run() {
	for event := range h.Broadcast {
		h.mu.Lock()
		for client := range h.clients {
			if err := client.WriteJSON(event); err != nil {
				log.Printf("WS Hub: Gagal mengirim pesan ke klien, menutup koneksi: %v", err)
				client.Close()
				delete(h.clients, client)
			}
		}
		h.mu.Unlock()
	}
}

// Register mendaftarkan koneksi klien baru ke dalam Hub
func (h *WSHub) Register(client *websocket.Conn) {
	h.mu.Lock()
	h.clients[client] = true
	h.mu.Unlock()
	log.Printf("WS Hub: Klien baru terdaftar. Total: %d", len(h.clients))
}

// Unregister menghapus koneksi klien dari Hub
func (h *WSHub) Unregister(client *websocket.Conn) {
	h.mu.Lock()
	if _, ok := h.clients[client]; ok {
		delete(h.clients, client)
		client.Close()
		log.Printf("WS Hub: Klien dihapus. Total: %d", len(h.clients))
	}
	h.mu.Unlock()
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Open Intranet: izinkan semua origin
	},
}

// ServeWS menangani request HTTP dan melakukan upgrade ke protokol WebSocket
func (h *WSHub) ServeWS(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("WS Hub: Gagal upgrade koneksi: %v", err)
		return
	}

	h.Register(conn)

	// Goroutine pendengar untuk membersihkan koneksi jika klien terputus
	go func() {
		defer h.Unregister(conn)
		for {
			if _, _, err := conn.ReadMessage(); err != nil {
				break
			}
		}
	}()
}
