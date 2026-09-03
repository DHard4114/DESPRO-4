package api

import (
	"encoding/json"
	"log"
	"net/http"
	"sync"

	"github.com/gorilla/websocket"
	"esos-server/models"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Allow all local connections
	},
}

// Hub manages active WebSocket client connections for real-time telemetry streaming
type Hub struct {
	clients    map[*websocket.Conn]bool
	broadcast  chan models.WebSocketEvent
	register   chan *websocket.Conn
	unregister chan *websocket.Conn
	mu         sync.Mutex
}

// NewHub initializes and runs the WebSocket streaming hub
func NewHub() *Hub {
	h := &Hub{
		clients:    make(map[*websocket.Conn]bool),
		broadcast:  make(chan models.WebSocketEvent, 256),
		register:   make(chan *websocket.Conn),
		unregister: make(chan *websocket.Conn),
	}
	go h.run()
	return h
}

func (h *Hub) run() {
	for {
		select {
		case conn := <-h.register:
			h.mu.Lock()
			h.clients[conn] = true
			h.mu.Unlock()
			log.Printf("[WS HUB] New dashboard client connected (Total: %d)", len(h.clients))

		case conn := <-h.unregister:
			h.mu.Lock()
			if _, ok := h.clients[conn]; ok {
				delete(h.clients, conn)
				conn.Close()
			}
			h.mu.Unlock()
			log.Printf("[WS HUB] Dashboard client disconnected (Remaining: %d)", len(h.clients))

		case event := <-h.broadcast:
			msgBytes, err := json.Marshal(event)
			if err != nil {
				continue
			}

			h.mu.Lock()
			for conn := range h.clients {
				err := conn.WriteMessage(websocket.TextMessage, msgBytes)
				if err != nil {
					conn.Close()
					delete(h.clients, conn)
				}
			}
			h.mu.Unlock()
		}
	}
}

// Broadcast sends event data to all connected WebSocket clients
func (h *Hub) Broadcast(event models.WebSocketEvent) {
	select {
	case h.broadcast <- event:
	default:
		// Drop message if buffer is full to prevent blocking
	}
}

// ServeWS handles incoming WebSocket upgrade requests
func (h *Hub) ServeWS(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("[WS ERROR] Failed to upgrade connection: %v", err)
		return
	}
	h.register <- conn
}
