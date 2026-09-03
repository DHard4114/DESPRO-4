# 02 KONTROL PENGADAAN DAN BILL OF MATERIALS (BOM)
## Smart-Sanitation eSOS (Emergency Sanitation Operating System)

Status Dokumen: KONTROL PENGADAAN TERKENDALI (CONTROLLED BASELINE)  
Batas Pagu Anggaran (*Budget Ceiling*): Rp2.000.000  
Anggaran Proposal Baseline (RAB): Rp1.901.000  
Estimasi Belanja Aktual Marketplace: Rp1.828.500 (Penghematan: Rp72.500 | Sisa Pagu Kas: Rp171.500)  
Model Pengadaan: **Pengadaan Kolektif Terdistribusi oleh Seluruh 5 Anggota Tim (Joint Procurement by All Members)**  
Koordinator Konsolidasi Finansial & Faktur: Raka Arrayan Muttaqien  
Verifikator Anggaran & Manajer Proyek: Daffa Hardhan  

---

## 1. Tautan Lembar Sebar Live Terpadu (Single Source of Truth)

Pengadaan 15 komponen BoM dieksekusi secara bersama oleh seluruh anggota tim sesuai domain subsistemnya. Seluruh pelacakan 15 item perangkat keras, tautan marketplace, status anggaran otomatis, serta bukti fisik invoice pembelian disatukan secara penuh pada tautan resmi Google Sheets berikut:

> **Tautan Master Live Google Sheets (BoM & WBS Tracker):**  
>  [Live Master BoM & Procurement Tracker — Smart-Sanitation eSOS](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing)

---

## 2. Tabel Master Bill of Materials (BoM) Terpadu

| No | Nama Komponen Perangkat Keras | Spesifikasi Teknis Minimum | Qty | Satuan | Harga Target RAB (Rp) | Total Target RAB (Rp) | Harga Aktual Toko (Rp) | Total Aktual Toko (Rp) | Selisih (Rp) | Status Anggaran | Toko Rekomendasi (Vendor) | Tautan Marketplace | Invoice / Bukti Fisik | Prioritas | PIC Pembelian & Verifikasi |
|:---:|:---|:---|:---:|:---:|---:|---:|---:|---:|---:|:---:|:---|:---:|:---:|:---:|:---|
| 1 | Access Point Outdoor TP-Link CPE220 | 2.4GHz 300Mbps High-Power 23dBm 12dBi Directional | 1 | Unit | Rp600.000 | Rp600.000 | Rp585.000 | Rp585.000 | Rp15.000 | `HEMAT` | TP-Link Official / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=tp-link+cpe220+outdoor) | `EV-W2-PROC-001` | HIGH | **Daffa Hardhan** |
| 2 | Panel Surya Polikristalin 10 Wp | 10 Wp 18V Output Vmp 17.5V Voc 21.5V Waterproof | 1 | Unit | Rp130.000 | Rp130.000 | Rp125.000 | Rp125.000 | Rp5.000 | `HEMAT` | Solar Solution ID / Shopee | [Buka Toko](https://shopee.co.id/search?keyword=solar+panel+10wp) | `EV-W2-PROC-001` | HIGH | **M. Ilman Zuhriy** |
| 3 | Baterai Li-ion 18650 3.7V (Pack 4 pcs) | Sony VTC6 / Panasonic 3000mAh 3.7V Original Grade A | 4 | Pcs | Rp45.000 | Rp180.000 | Rp42.500 | Rp170.000 | Rp10.000 | `HEMAT` | Battery Center / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=baterai+18650+3000mah+original) | `EV-W2-PROC-001` | HIGH | **M. Ilman Zuhriy** |
| 4 | Modul Charger BMS Baterai TP4056 | 5V 1A Micro USB / Type-C with Overdischarge DW01 | 2 | Unit | Rp6.000 | Rp12.000 | Rp5.000 | Rp10.000 | Rp2.000 | `HEMAT` | E-Store Robotic / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=tp4056+type+c+protection) | `EV-W2-PROC-001` | MEDIUM | **M. Ilman Zuhriy** |
| 5 | Mikrokontroler ESP32 DevKitC V4 | 38-Pin CP2102 Dual Core Wi-Fi + BLE 4MB Flash | 1 | Unit | Rp75.000 | Rp75.000 | Rp72.000 | Rp72.000 | Rp3.000 | `HEMAT` | Digiware Store / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=esp32+devkitc+v4+38+pin) | `EV-W2-PROC-001` | HIGH | **Siti Amalia N.** |
| 6 | Sensor Gas Amonia MQ-137 | Deteksi NH3 Range 5 - 500 ppm Analog + Digital | 1 | Unit | Rp160.000 | Rp160.000 | Rp155.000 | Rp155.000 | Rp5.000 | `HEMAT` | Sensor Shop ID / Shopee | [Buka Toko](https://shopee.co.id/search?keyword=sensor+mq137+amonia) | `EV-W2-PROC-001` | HIGH | **M. Ilman Zuhriy** |
| 7 | Sensor Gas Hidrogen Sulfida MQ-136 | Deteksi H2S Range 1 - 200 ppm Analog + Digital | 1 | Unit | Rp175.000 | Rp175.000 | Rp168.000 | Rp168.000 | Rp7.000 | `HEMAT` | Sensor Shop ID / Shopee | [Buka Toko](https://shopee.co.id/search?keyword=sensor+mq136+h2s) | `EV-W2-PROC-001` | HIGH | **M. Ilman Zuhriy** |
| 8 | Sensor Jarak Ultrasonik JSN-SR04T | Probe Kedap Air (Waterproof) Blind Zone 20cm | 1 | Unit | Rp85.000 | Rp85.000 | Rp82.000 | Rp82.000 | Rp3.000 | `HEMAT` | RoboTech Official / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=jsn-sr04t+waterproof) | `EV-W2-PROC-001` | HIGH | **Siti Amalia N.** |
| 9 | Motor Servo Metal Gear MG996R | Torsi 11 kg.cm Metal Gear 180 Degree 4.8V-7.2V | 1 | Unit | Rp55.000 | Rp55.000 | Rp52.000 | Rp52.000 | Rp3.000 | `HEMAT` | Hobby King Store / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=servo+mg996r+metal+gear) | `EV-W2-PROC-001` | MEDIUM | **Darrel Alfath** |
| 10 | Modul Transceiver RF LoRa RA-02 433MHz | Semtech SX1278 SPI Interface 433MHz + Antena Spring | 2 | Unit | Rp70.000 | Rp140.000 | Rp68.000 | Rp136.000 | Rp4.000 | `HEMAT` | Radio Wireless ID / Shopee | [Buka Toko](https://shopee.co.id/search?keyword=lora+ra02+433mhz+sx1278) | `EV-W2-PROC-001` | HIGH | **Siti Amalia N.** |
| 11 | Step-Down Buck Converter DC-DC LM2596 | Input 4.5-40V Output 1.25-35V 3A Adjustable | 2 | Unit | Rp12.000 | Rp24.000 | Rp10.500 | Rp21.000 | Rp3.000 | `HEMAT` | Komponen Elektronika / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=lm2596+step+down) | `EV-W2-PROC-001` | MEDIUM | **M. Ilman Zuhriy** |
| 12 | Filamen 3D Printing PETG 1kg | 1.75mm Weatherproof Heat & UV Resistant (eSUN/Sunlu) | 1 | Roll | Rp160.000 | Rp160.000 | Rp155.000 | Rp155.000 | Rp5.000 | `HEMAT` | 3D Filament Store / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=filament+petg+1.75mm+1kg) | `EV-W2-PROC-001` | HIGH | **Darrel Alfath** |
| 13 | Gland Kabel Waterproof PG7 & Baut M3/M4 | Set PG7 Nylon Waterproof IP68 + Baut Mur Stainless | 1 | Set | Rp35.000 | Rp35.000 | Rp32.000 | Rp32.000 | Rp3.000 | `HEMAT` | Baut Mur Teknik / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=cable+gland+pg7+waterproof) | `EV-W2-PROC-001` | MEDIUM | **Darrel Alfath** |
| 14 | Perfboard PCB Titik & Kabel Silicone 22AWG | Single Side Perfboard FR4 + Kabel Awg22 Flexible 5m | 1 | Set | Rp45.000 | Rp45.000 | Rp42.000 | Rp42.000 | Rp3.000 | `HEMAT` | Elektronika Mandiri / Shopee | [Buka Toko](https://shopee.co.id/search?keyword=perfboard+fr4+kabel+silicone+22awg) | `EV-W2-PROC-001` | HIGH | **M. Ilman Zuhriy** |
| 15 | Tombol Darurat SOS Push Button 16mm | Stainless Steel Waterproof Momentary LED Ring 5V | 1 | Unit | Rp25.000 | Rp25.000 | Rp23.500 | Rp23.500 | Rp1.500 | `HEMAT` | Saklar Industri / Tokopedia | [Buka Toko](https://www.tokopedia.com/search?q=push+button+16mm+waterproof+momentary) | `EV-W2-PROC-001` | HIGH | **Darrel Alfath** |
| **TOTAL** | **TOTAL AKUMULASI PENGADAAN (15 ITEM)** | | | | | **Rp1.901.000** | | **Rp1.828.500** | **Rp72.500** | **`HEMAT`** | | | | |
| **PAGU** | **PAGU MAKSIMUM PROPOSAL (RAB CEILING)** | | | | | **Rp2.000.000** | | | | **`HEMAT`** | | | | |
| **SISA** | **SISA ANGGARAN KAS TERHADAP PAGU Rp 2.000.000** | | | | | **Rp171.500** | | | | **`HEMAT`** | | | | |

---

## 3. Gerbang Teknis Kritis Sebelum Pembelian (Critical Technical Gates)

1. **`GATE-PROC-01` (Sensor Gas MQ-137):**  
   Verifikasi identitas modul fisik, tipe sensing element ($SnO_2$), arus pemanas ($\approx 150\text{ mA}$), dan ketersediaan datasheet resmi pabrikan Hanwei.
2. **`GATE-PROC-02` (Sensor Gas MQ-136):**  
   Verifikasi rentang deteksi $H_2S$, resistansi sensor $R_s$, dan metode pemanasan awal (*pre-heating* 24 jam).
3. **`GATE-PROC-03` (Kapasitas Daya Tenaga Surya 10 Wp):**  
   Rekonsiliasi konsumsi beban operasional membuktikan pembangkitan solar 10 Wp memiliki margin surplus $42.5\%$ terhadap beban kontinu 24 jam.
4. **`GATE-PROC-04` (Topologi Baterai 18650 & TP4056):**  
   Konfigurasi 1S4P (3.7V nominal, 12000 mAh) dengan dual charger TP4056 dan proteksi ganda DW01A tervalidasi aman.
5. **`GATE-PROC-05` (Verifikasi Kapasitas Baterai 18650):**  
   Uji kapasitas debit riil membuktikan setiap sel memiliki kapasitas $\ge 2950\text{ mAh}$ tanpa risiko penurunan tegangan berlebih.

---

## 4. Alur Kerja Pengadaan dan Standar Bukti Minimum

$$\text{Pemeriksaan Spesifikasi} \rightarrow \text{Verifikasi Datasheet} \rightarrow \text{Komparasi Vendor} \rightarrow \text{Persetujuan Gate} \rightarrow \text{Pemesanan} \rightarrow \text{Faktur & Resi} \rightarrow \text{Fisik Tiba} \rightarrow \text{Registrasi Aset} \rightarrow \text{Uji Acceptance}$$

Arsip bukti pengadaan wajib tersimpan pada repositori:
- **Dokumen Faktur & Resi:** Terkumpul pada bundel PDF `assets/procurement/receipts_bundle.pdf` (`EV-W2-PROC-001`).
- **Foto Fisik Komponen:** Tersimpan pada direktori `assets/procurement/asset_photos/` (`EV-W2-PROC-002`).
- **Pelacakan Live:** Terkoneksi ke [Google Sheets BoM Tracker](https://docs.google.com/spreadsheets/d/1zRfozwUUJNMKAodWKQSK0rwU4v89uHyB7RLKbxfwlRk/edit?usp=sharing).
