# Skrip Utilitas & Simulator: Smart-Sanitation eSOS 🛠️

Direktori ini berisi skrip pengujian beban, simulator telemetri perangkat keras (*dummy hardware*), dan perkakas otomasi pengembangan untuk sistem Smart-Sanitation eSOS.

---

## 📋 Daftar Utilitas

| Berkas | Bahasa | Deskripsi & Fungsi |
| :--- | :--- | :--- |
| [`dummy_gateway.go`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/scripts/dummy_gateway.go) | Go | Simulator Gateway LoRa-to-MQTT untuk menginjeksi data telemetri sintesis (level air, amonia, H2S, baterai, status SOS) secara berkala (2 detik) langsung ke broker Mosquitto. |

---

## 🧪 Alur Kerja Simulator Dummy Gateway

```mermaid
sequenceDiagram
    autonumber
    participant SIM as dummy_gateway.go
    participant BRK as Mosquitto Broker (1883)
    participant SRV as Go Server (Backend)
    participant UI as Dashboard Web (Port 8000)

    Note over SIM,BRK: Inisialisasi Koneksi MQTT
    SIM->>BRK: CONNECT (ClientID: EMS_Simulator_Node)
    SIM->>BRK: PUBLISH esos/gateway_01/status ("ONLINE")
    
    loop Setiap 2 Detik
        SIM->>SIM: Hitung Metrik Sintesis (Random Walk)
        SIM->>BRK: PUBLISH esos/node_wc_01/telemetry (JSON Payload)
        BRK->>SRV: Forward Payload ke Subscriber ETL
        SRV->>SRV: Proses Worker Pool & Simpan ke DB
        SRV->>UI: Push Data via Native WebSocket
        Note over UI: Grafik Chart.js bergerak dinamis
    end
```

---

## 🚀 Cara Menjalankan Simulator

1. Pastikan Mosquitto broker dan Go Server backend sudah aktif.
2. Jalankan simulator langsung menggunakan runtime Go:
   ```powershell
   cd src/scripts
   go run dummy_gateway.go
   ```
3. Buka browser ke `http://localhost:8000` untuk memverifikasi penerimaan data dan pergerakan grafik sensor secara interaktif tanpa memerlukan perangkat keras fisik ESP32.
