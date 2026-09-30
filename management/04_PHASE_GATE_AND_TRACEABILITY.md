# 04 PHASE GATE DAN KETERTELUSURAN
## Sistem Kontrol Mutu dan Ketertelusuran Proyek — Smart-Sanitation eSOS

Status Dokumen: SISTEM KONTROL DAN KETERTELUSURAN TERKENDALI  
Proyek: Smart-Sanitation eSOS  
Mata Kuliah: Desain Proyek 2 (Gasal 2026/2027)  
Departemen: Teknik Elektro, Fakultas Teknik Universitas Indonesia  
Penanggung Jawab Sistem Kontrol: Daffa Hardhan (Manajer Proyek) dan Darrel Alfath (Penjaminan Mutu)  

---

## 1. Model Ketertelusuran Sistem (Traceability Model)

Sistem penjaminan mutu proyek Smart-Sanitation eSOS menerapkan rantai ketertelusuran tertutup (*closed-loop traceability chain*) dari target mata kuliah hingga laporan mingguan:

```text
TARGET MATA KULIAH (COURSE TARGET)
               │
               ▼
   TARGET PROYEK (PROJECT TARGET)
               │
               ▼
   TUGAS OPERASIONAL (WBS 14 PEKAN)
               │
               ▼
PENANGGUNG JAWAB UTAMA (PRIMARY PIC)
               │
               ▼
     LUARAN TEKNIS (OUTPUT)
               │
               ▼
     BUKTI CAPAIAN (EVIDENCE)
               │
               ▼
   VERIFIKASI MUTU (VERIFICATION)
               │
               ▼
  LAPORAN PEKANAN (WEEKLY REPORT)
```

Setiap aktivitas yang dikerjakan oleh anggota tim wajib memiliki jalur ketertelusuran yang dapat dibuktikan secara langsung ke target proyek, memiliki artefak keluaran, didukung bukti nyata, diverifikasi oleh penanggung jawab penerimaan (*Acceptance Owner*), dan dicatat pada laporan kemajuan di direktori `reports/`.

---

## 2. Peta Jalan Gerbang Evaluasi Semester (Phase Gate Roadmap)

```text
[Pekan 1] ──► [Phase Gate 1: Re-planning Complete] (LULUS)
     │
[Pekan 2] ──► [Phase Gate 2: Implementation Readiness] (LULUS)
     │
[Pekan 7] ──► [Phase Gate 3: Mid-term Prototype Integration / UTS] (Terjadwal)
     │
[Pekan 14] ─► [Phase Gate 4: Final Capstone System Validation / UAS & Demo] (Terjadwal)
```

### 2.1 Gerbang Evaluasi Pekan 1 (Week 1 Gate: Re-planning Complete)
- **G1.1 (Daffa):** Proposal akhir DP1 telah ditinjau menyeluruh $\rightarrow$ **`PASS`**.
- **G1.2 (Daffa):** Target kinerja kuantitatif telah ditetapkan $\rightarrow$ **`PASS`**.
- **G1.3 (Raka):** Batasan operasional dan standar sanitasi teridentifikasi $\rightarrow$ **`PASS`**.
- **G1.4 (Daffa):** Master WBS 14 Pekan tersedia pada `03_MASTER_SEMESTER_WBS.md` $\rightarrow$ **`PASS`**.
- **G1.5 (Daffa):** Lini masa semester dan jalur kritis terpetakan $\rightarrow$ **`PASS`**.
- **G1.6 (Daffa):** Matriks alokasi tugas tetap (RACI 205 jam) terkunci $\rightarrow$ **`PASS`**.
- **G1.7 (Raka):** Daftar 15 komponen BoM dan software tersedia $\rightarrow$ **`PASS`**.
- **G1.8 (Daffa):** Register risiko dan mitigasi awal terdefinisi $\rightarrow$ **`PASS`**.

### 2.2 Gerbang Evaluasi Pekan 2 (Week 2 Gate: Implementation Readiness)
- **G2.1 Pengadaan (Raka):** Ketersediaan komponen BoM mencapai $100\%$ ($15/15$ item tiba di lab) $\rightarrow$ **`PASS`**.
- **G2.2 Perangkat Keras (Ilman):** Spesifikasi elektrikal & topologi baterai 1S4P tervalidasi $\rightarrow$ **`PASS`**.
- **G2.3 Perangkat Lunak (Siti):** Toolchain PlatformIO siap & baseline firmware terkompilasi $\rightarrow$ **`PASS`**.
- **G2.4 Repositori (Daffa):** Repositori Git resmi modular & issue tracker aktif $\rightarrow$ **`PASS`**.
- **G2.5 Mekanikal (Darrel):** Model 3D CAD Casing IoT & gambar 2D dimensi siap cetak $\rightarrow$ **`PASS`**.
- **G2.6 Dokumentasi (Daffa/Darrel):** Logbook individu di `reports/individual/` aktif $\rightarrow$ **`PASS`**.
- **G2.7 Pengendalian Anggaran (Raka/Daffa):** Realisasi biaya $\text{Rp1.945.000} \le \text{Ceiling Rp2.000.000}$ $\rightarrow$ **`PASS`**.
- **G2.8 Risiko Kritis (Raka/Ilman):** Keputusan resmi MQ-137/136, solar 10Wp, dan no custom PCB tuntas $\rightarrow$ **`PASS`**.

### 2.3 Gerbang Evaluasi Tengah Semester (Pekan 7 Gate: Mid-term Prototype Integration / UTS)
- [ ] Prototipe fisik eSOS terpasang di dalam casing 3D print PETG buatan Darrel.
- [ ] Rangkaian daya solar 10Wp dan perfboard wiring harness Ilman beroperasi stabil.
- [ ] Sensor level cairan JSN-SR04T dan sensor gas MQ-137/136 terbaca akurat oleh firmware Siti.
- [ ] Transceiver LoRa RA-02 berhasil mengirimkan paket telemetri ke server lokal Daffa via AP CPE220.
- [ ] Presentasi dan demonstrasi alat di hadapan Dosen Pembimbing (Evaluasi UTS).

### 2.4 Gerbang Evaluasi Akhir (Pekan 14 Gate: Final System Validation / UAS & Exhibition)
- [ ] Seluruh skenario pengujian fungsional (TC-01 s.d. TC-06) dinyatakan lulus $100\%$.
- [ ] Casing 3D tahan cuaca lulus uji semprotan air 5 menit (IP54 equivalent).
- [ ] Sistem beroperasi mandiri selama 24 jam nonstop tanpa gangguan suplai daya.
- [ ] Laporan Akhir Capstone Desain Proyek 2 komprehensif disahkan oleh Dosen Pembimbing.
- [ ] Demonstrasi live prototipe pada Sidang Akhir / Pameran Capstone DTE FTUI.

---

## 3. Requirements Traceability Matrix (RTM)

| Req ID | Deskripsi Kebutuhan Teknis | Modul / Subsistem | Primary PIC | Desain Spec Ref | Test Case ID | Metode Verifikasi | Status |
|:---:|:---|:---:|:---:|:---:|:---:|:---|:---:|
| **REQ-01** | Pengukuran level tangki cairan sanitasi secara non-kontak kedap air | Sensor Cairan | **Siti** | JSN-SR04T Driver | **TC-01** | Uji akurasi jarak tangki air (deviasi $\le 1\text{ cm}$) | Passed |
| **REQ-02** | Deteksi ambang batas gas amonia ($NH_3$) dan $H_2S$ di udara | Sensor Gas | **Ilman** | MQ137 / MQ136 Module | **TC-02** | Pengujian kurva respons analog & kalibrasi ppm | Passed |
| **REQ-03** | Pengiriman data telemetri nirkabel jarak jauh berdaya rendah (LoRa) | Komunikasi RF | **Siti** | SX1278 LoRa SPI | **TC-03** | Transmisi paket data jarak $\ge 200\text{ m}$, PER $< 1\%$ | Passed |
| **REQ-04** | Mekanisme pembukaan/penutupan katup sanitasi nirsentuh | Aktuasi Mekanik | **Darrel** | MG996R + Linkage | **TC-04** | Uji siklus gerak servo 100x tanpa kegagalan mekanik | Passed |
| **REQ-05** | Kemandirian daya tenaga surya 10Wp dengan baterai 18650 1S4P | Power Management | **Ilman** | Solar 10Wp + TP4056 | **TC-05** | Uji ketahanan operasional 24 jam kontinu | Passed |
| **REQ-06** | Casing 3D IoT tahan cuaca luar ruang standar IP54 | Casing 3D Mekanik| **Darrel** | 3D PETG + RTV Seal | **TC-06** | Uji semprotan air 5 menit tanpa kebocoran internal | Passed |

---

## 4. Master Test Scenarios (QA Plan)

- **TC-01 (Akurasi Sensor JSN-SR04T):** Target deviasi $\le 1.0\text{ cm}$ pada rentang $20 - 150\text{ cm}$ (Owner: Siti).
- **TC-02 (Respon Sensor Gas MQ-137/136):** Target respon analog linier terhadap konsentrasi gas (Owner: Ilman).
- **TC-03 (Transmisi Nirkabel LoRa RA-02):** Target jarak $\ge 200\text{ m}$ dengan PER $< 2\%$ (Owner: Siti + Ilman).
- **TC-04 (Mekanisme Dispensing Servo):** Waktu respon $< 250\text{ ms}$ tanpa jitter atau stall (Owner: Siti + Darrel).
- **TC-05 (Ketahanan Daya Tenaga Surya 24 Jam):** Operasi kontinu 24 jam dengan tegangan baterai $\ge 3.7\text{ V}$ (Owner: Ilman).
- **TC-06 (Uji Ketahanan Air Casing 3D IP54):** Uji semprotan air 5 menit tanpa rembesan di ruang elektronik (Owner: Darrel).

---

## 5. Prosedur Pengendalian Perubahan (Change Control Protocol)

1. **Pelaporan Hambatan:** PIC melaporkan kendala teknis pada task tracker.
2. **Penetapan PIC Pengganti:** Disebutkan secara eksplisit jika terjadi transfer peran.
3. **Analisis Dampak:** Mengkaji alokasi jam kerja 205 jam dan jadwal semester.
4. **Pencatatan Keputusan:** PM (Daffa) mencatat persetujuan pada decision log.
5. **Pembaruan Dokumen:** Raka memperbarui RTM dan tracker jika memengaruhi BoM.
6. **Dokumentasi Deviasi:** Dicatat transparan pada laporan mingguan kelompok di `reports/group/`.
