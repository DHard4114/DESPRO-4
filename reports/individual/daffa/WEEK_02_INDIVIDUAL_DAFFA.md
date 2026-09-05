# TEMPLATE LAPORAN PEKANAN INDIVIDU
**Capstone Project Desain Proyek 2**

### A. Identitas
| Item | Keterangan |
|:---|:---|
| Nama | Daffa Hardhan |
| NPM | 2306161763 |
| Program Studi | Teknik Komputer |
| Kelompok | 4 (Empat) |
| Judul Proyek | Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana |
| Pekan ke- | 2 (Dua) |
| Periode | 28 Agustus 2026 – 04 September 2026 |
| Tanggal | 04 September 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

### B. Target Mingguan
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Inisiasi Repositori Git Proyek | Struktur direktori rapi & GitHub `main` branch aktif | [x] Selesai &nbsp; [ ] Belum |
| 2 | Setup Server Backend Go & DB | Binary server Go (`esos-server.exe`) & SQLite | [x] Selesai &nbsp; [ ] Belum |
| 3 | Perancangan Antarmuka UI Dashboard | Mockup & kode HTML statis Dashboard Real-time | [x] Selesai &nbsp; [ ] Belum |

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Senin, 31 Ags 2026 | Inisialisasi Git, penataan WBS, setup repo GitHub | 3 | URL GitHub |
| Selasa, 01 Sep 2026 | Pemrograman awal Server Go dan konfigurasi SQLite WAL | 4 | `src/server/main.go` |
| Rabu, 02 Sep 2026 | Pemrograman Endpoint API Ingestion & WebSocket | 4 | `src/server/api/` |
| Kamis, 03 Sep 2026 | Penulisan UI frontend Dashboard `index.html` dan Tailwind | 4 | `src/server/static/` |
| Jumat, 04 Sep 2026 | Uji kompilasi menjadi native binary executable | 3 | `src/bin/esos-server.exe` |
| Sabtu, 05 Sep 2026 | Evaluasi capaian pekan 2 dan penyusunan laporan | 2 | Bukti Commit Git |

**Total Jam Kerja Minggu Ini : 20 Jam**

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Telah berhasil menyelesaikan inisialisasi lingkungan pengembangan perangkat lunak (Software) berupa server lokal. Pembuatan server ini memakai bahasa Go agar performa ringan untuk perangkat posko darurat. Repositori kode terpusat di GitHub juga telah 100% rapi dengan folder `src` (source) dan `assets`.

**2. Luaran yang Dihasilkan**
- Binary Server Backend Go (`esos-server.exe`)
- Skema Database SQLite (UUIDv7)
- Kode sumber Dashboard UI statis (`index.html`)
- Repositori GitHub Proyek Kelompok 4 yang Publik

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | - | - | 0% |
| Software | Setup Lingkungan Server & Repo Git | Repo Git Publik, Server Go kompilasi sukses | 100% |
| Mekanik | - | - | 0% |
| Pengujian | Uji kompilasi kode Go | Binary sukses terbentuk nol error | 100% |
| Dokumentasi | Master Logbook & WBS Tracker Sheets | 5 Tabs Spreadsheet siap | 100% |

**Ringkasan Kemajuan**
Kemajuan teknis minggu ini pada ranah *Software* sangat pesat karena inisiatif menggunakan server Go mandiri. Secara keseluruhan, target CPMK Pekan 2 "Persiapan lingkungan pengembangan & pembuatan repositori proyek" telah dicapai dengan sempurna.

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | Kompleksitas kompilasi Go untuk anggota kelompok lain jika harus menginstal SDK Go. | Anggota kesulitan menjalankan server lokal di laptop mereka. | Sedang |

**Analisis Penyebab**
Setiap laptop anggota memiliki *environment* berbeda (Windows/Mac) dan belum tentu memiliki Go compiler, sehingga integrasi testing lokal bisa memakan waktu instalasi di sisi klien.

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Kompleksitas instalasi Go | Mengompilasi program menjadi satu binary mandiri `.exe` sehingga anggota lain cukup klik file tersebut. | Daffa H. | Selesai Pekan 2 |

### H. Dokumentasi Kemajuan
*   **Screenshot Software:** (Bisa dilihat pada riwayat commit di repositori: `https://github.com/DHard4114/DESPRO-4/commits/main`)
*   **Bukti Commit Git:** Commit `fb46784`, `862772d` pada `main` branch.
*   *(Gambar screenshot VS Code terlampir di file dokumen resmi word)*

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Rapat tim tanggal:** 02 September 2026 (Konsolidasi pembelian barang & BoM)
- **Diskusi teknis:** Sinkronisasi struktur repositori dengan Siti (Firmware) dan Ilman (Hardware).
- **Koordinasi dengan dosen:** Evaluasi WBS awal.

**Kontribusi Pribadi**
Menyediakan wadah kolaborasi (*Git repository*), merancang lembar sebar (Spreadsheet) untuk pelacakan 205 jam WBS, dan membangun infrastruktur *backend* penerima data awal sehingga tim fokus pada *hardware*.

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Proses inisialisasi Git dan penyusunan struktur folder berjalan cepat dan disepakati tim.
- Penulisan kode Go server sangat mulus.

**Yang Perlu Diperbaiki**
- Perlu merapikan dokumentasi API secara tertulis agar Siti lebih mudah mengirim payload JSON.

**Pelajaran yang Didapat Minggu Ini**
- Pembagian direktori yang rapi (`src/bin`, `src/config`, `src/data`) sangat vital dalam menghindari konflik *merge* antar cabang fitur GitHub nantinya.

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Membuat API Documentation (Swagger/Markdown) | Dokumen Spesifikasi API | 3 Jam |
| 2 | Menyiapkan dummy data generator untuk testing | Script Python dummy data | 4 Jam |
| 3 | Mengintegrasikan Endpoint dengan format LoRa Siti | Parsing payload JSON | 5 Jam |

### L. Persetujuan
**Mahasiswa**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 04 September 2026

**Ketua Kelompok**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 04 September 2026

**Catatan Pembimbing**
....................................................................................
....................................................................................
