# RFID Attendance + TFT ILI9488 (SMPIT SIF)

Sistem absensi RFID (MFRC522) dengan tampilan TFT 3.5" ILI9488 480x320.
Data absensi dikirim ke Google Sheets via Google Apps Script.
Dikembangkan dari `project_smpit_sif` (OLED → TFT).

## Pin Mapping

| ESP32 | SCK | MISO | MOSI | CS | DC | RST |
|-------|-----|------|------|-----|-----|-----|
| TFT ILI9488 | 18 | 19 | 23 | 5 | 2 | 4 |
| MFRC522 | 25 | 27 | 26 | 15 | – | 14 |
| Buzzer | – | – | – | – | – | 16 |

TFT dan RFID sekarang memakai **bus SPI yang terpisah**:

- TFT memakai port **HSPI** (pin tetap 18/19/23/5/2/4).
- MFRC522 memakai port **VSPI** (SCK 25, MISO 27, MOSI 26) dengan SS 15 dan RST 14.

> **Penting:** aktifkan `#define USE_HSPI_PORT` di `TFT_eSPI/User_Setup.h`
> (sudah otomatis jika file User_Setup ikut tersinkronisasi dari setup ini).
> Tanpa ini, TFT dan RC522 berebut port SPI yang sama sehingga RFID tidak terbaca.

## Library yang Dibutuhkan

Install via Library Manager Arduino IDE:

- `TFT_eSPI` (Bodmer)
- `MFRC522`
- `WiFi` (bawaan ESP32)
- `HTTPClient` (bawaan ESP32)
- `WiFiClientSecure` (bawaan ESP32)

### Konfigurasi TFT_eSPI

Pastikan di `TFT_eSPI/User_Setup.h` (atau `User_Setup_Select.h`):

```c
#define ILI9488_DRIVER
#define USE_HSPI_PORT   // wajib: TFT di port HSPI, RC522 di VSPI

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

#define SPI_FREQUENCY 40000000

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF   // wajib, untuk FreeFont yang dipakai UI
#define SMOOTH_FONT
```

## Cara Pakai

1. Sambungkan hardware sesuai tabel pin mapping di atas. Perhatikan bahwa
   RC522 sekarang memakai pin SCK=25, MISO=27, MOSI=26 (bukan lagi 18/19/23),
   dan pastikan `#define USE_HSPI_PORT` aktif di `User_Setup.h`.
2. Edit kredensial WiFi di `project_smpit_sif_tft.ino`:

   ```cpp
   const char* WIFI_SSID = "Medjay";
   const char* WIFI_PASSWORD = "devilkiller";
   ```

3. Sesuaikan `SCRIPT_URL` dengan URL Apps Script absensi Anda.
4. Upload `project_smpit_sif_tft.ino` ke ESP32 via Arduino IDE
   (board: ESP32 Dev Module).

## Fitur Tampilan

- **Splash** — logo SMPIT AL IFADAH + progress bar
- **Menghubungkan WiFi** — spinner berputar
- **WiFi Connected** — menampilkan IP ESP32
- **Menunggu kartu** — animasi radar (sweep + pulse) + ikon RFID,
  indikator status WiFi di header
- **Kartu terdeteksi** — menampilkan UID kartu
- **Mengirim data** — spinner selama proses kirim
- **Hasil absensi** dengan ikon dan warna berbeda:
  - `TEPAT` → centang hijau, "ABSENSI BERHASIL" + nama + kelas
  - `TELAT` → segitiga oranye, "ABSENSI TELAT" + IQOB
  - `SUDAH_ABSEN` → X merah, "ABSENSI DITOLAK"
  - `DENIED` → X merah, "AKSES DITOLAK / UID TIDAK ADA"
  - `SHEET_ERROR` / `HTTP ERROR` / `KONEKSI GAGAL` → layar error

## Format Respon Google Apps Script

| Respon | Arti |
|---|---|
| `TEPAT\|NAMA\|KELAS` | Absen masuk berhasil |
| `TELAT\|NAMA\|KELAS` | Absen tercatat telat (IQOB) |
| `SUDAH_ABSEN\|NAMA\|KELAS` | Kartu sudah absen hari ini |
| `DENIED` | UID tidak terdaftar |
| `SHEET_ERROR` | Masalah di database/sheet |
