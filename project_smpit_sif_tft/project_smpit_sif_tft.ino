// ==================================================
// RFID ATTENDANCE + TFT ILI9488 480x320
// ==================================================
//
// LAYAR: TFT 3.5" ILI9488 480x320 (TFT_eSPI)
//   SCK  = 18
//   MISO = 19
//   MOSI = 23
//   CS   = 5
//   DC   = 2
//   RST  = 4
//
// PENTING: TFT memakai port HSPI. Aktifkan #define USE_HSPI_PORT
// di TFT_eSPI/User_Setup.h agar tidak bentrok dengan RC522.
//
// RFID: MFRC522 (bus SPI terpisah, VSPI)
//   SS  = 15
//   RST = 14
//   SCK = 25, MISO = 27, MOSI = 26
//
// BUZZER = 16

#include <TFT_eSPI.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// TFT
TFT_eSPI tft;

// WARNA TEMA
const uint16_t COL_BG     = tft.color565(8, 13, 26);      // biru gelap
const uint16_t COL_HEADER = tft.color565(11, 18, 36);     // header
const uint16_t COL_PANEL  = tft.color565(17, 26, 52);     // panel
const uint16_t COL_RING   = tft.color565(32, 48, 80);     // ring radar
const uint16_t COL_ACCENT = tft.color565(0, 205, 255);    // cyan
const uint16_t COL_GREEN  = tft.color565(0, 215, 90);     // hijau
const uint16_t COL_RED    = tft.color565(255, 70, 70);    // merah
const uint16_t COL_ORANGE = tft.color565(255, 155, 0);    // oranye
const uint16_t COL_YELLOW = tft.color565(255, 215, 40);   // kuning
const uint16_t COL_GRAY   = tft.color565(125, 136, 155);  // abu-abu

// RFID RC522 (bus VSPI terpisah dari TFT/HSPI)
#define SS_PIN 15
#define RST_PIN 14
#define SCK_PIN 25
#define MISO_PIN 27
#define MOSI_PIN 26

MFRC522 mfrc522(SS_PIN, RST_PIN);

// BUZZER
#define BUZZER 16

// WIFI
const char* WIFI_SSID = "Mika";
const char* WIFI_PASSWORD = "ogamhurufbesarsemua";

// GOOGLE APPS SCRIPT
const char* SCRIPT_URL =
"https://script.google.com/macros/s/AKfycbw5VAYZxcCi9k2rtpwnHgiynZD8JU8fqUTizF9D0tlZcpZPtaZh1KHbO9KJ1A7kk2kV/exec?uid=";

// STATUS LAYAR
enum Screen {
  SCR_BOOT,
  SCR_WIFI,
  SCR_READY,
  SCR_IDLE,
  SCR_DETECT,
  SCR_SEND,
  SCR_RESULT
};

Screen screen = SCR_BOOT;

// VARIABEL ANIMASI
unsigned long lastIdleAnim = 0;
int idleSweep = 0;
int idlePulse = 0;
int prevPulseR = 62;
int pulseR = 62;

// TEKS TFT
void drawCentered(
  const String& s,
  int x,
  int y,
  const GFXfont* f,
  uint16_t c
) {
  tft.setFreeFont(f);
  tft.setTextColor(c);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(s.c_str(), x, y);
  tft.setTextDatum(TL_DATUM);
}

// HEADER LAYAR
void drawHeader(
  const char* title,
  uint16_t color
) {
  tft.fillRect(0, 0, 480, 6, color);
  tft.fillRect(0, 6, 480, 50, COL_HEADER);
  drawCentered(title, 240, 32, &FreeSansBold18pt7b, TFT_WHITE);
}

// IKON RFID (KARTU + GELOMBANG)
void drawRFIDIcon(
  int cx,
  int cy,
  uint16_t color
) {
  tft.fillRoundRect(cx - 30, cy - 19, 60, 38, 6, color);
  tft.drawArc(cx + 24, cy, 12, 9, 240, 300, color, COL_BG);
  tft.drawArc(cx + 24, cy, 20, 17, 240, 300, color, COL_BG);
  tft.drawArc(cx + 24, cy, 28, 25, 240, 300, color, COL_BG);

}

// IKON STATUS
//   0 = sukses (centang hijau)
//   1 = peringatan (segitiga oranye)
//   2 = ditolak (X merah)
// ==================================================

void drawStatusIcon(
  int type
) {
  uint16_t c = (type == 0) ? COL_GREEN :
               (type == 1) ? COL_ORANGE :
                             COL_RED;

  int cx = 240;
  int cy = 128;

  tft.fillSmoothCircle(cx, cy, 62, COL_PANEL, COL_BG);
  tft.drawSmoothCircle(cx, cy, 62, c, COL_PANEL);

  if (type == 0) {

    tft.drawWideLine(cx - 28, cy + 2, cx - 6, cy + 24, 10, c, COL_PANEL);
    tft.drawWideLine(cx - 6, cy + 24, cx + 30, cy - 28, 10, c, COL_PANEL);
  }
  else if (type == 1) {

    tft.fillTriangle(
      cx, cy - 48,
      cx - 34, cy + 30,
      cx + 34, cy + 30,
      c
    );

    tft.fillRect(cx - 7, cy - 30, 14, 38, COL_BG);
    tft.fillRect(cx - 7, cy + 18, 14, 14, COL_BG);
  }
  else {

    tft.drawWideLine(cx - 30, cy - 28, cx + 30, cy + 28, 10, c, COL_PANEL);
    tft.drawWideLine(cx - 30, cy + 28, cx + 30, cy - 28, 10, c, COL_PANEL);
  }
}

// ==================================================
// INDIKATOR WIFI DI HEADER
// ==================================================

void drawWifiIndicator() {
  bool on = (WiFi.status() == WL_CONNECTED);
  uint16_t c = on ? COL_GREEN : COL_RED;

  tft.fillSmoothCircle(380, 135, 4, c, COL_HEADER);
  drawCentered(
    on ? "WiFi OK" : "WiFi OFF",
    430, 135,
    &FreeSansBold9pt7b,
    c
  );
}

// ==================================================
// SPINNER
// ==================================================

void runSpinner(
  int cx,
  int cy,
  int r,
  unsigned long duration
) {
  unsigned long t0 = millis();
  int a = 0;

  while (millis() - t0 < duration) {
    tft.drawArc(cx, cy, r, r - 7, 0, 360, COL_PANEL, COL_PANEL);
    tft.drawArc(cx, cy, r, r - 7, a, a + 90, COL_ACCENT, COL_PANEL);
    a = (a + 12) % 360;
    delay(30);
  }
}

// ==================================================
// LAYAR SPLASH
// ==================================================

void showBoot() {

  tft.fillScreen(COL_BG);

  drawCentered(
    "SMPIT 'ALAMY",
    240, 80,
    &FreeSansBold24pt7b,
    TFT_WHITE
  );

  drawCentered(
    "ATALAS",
    240, 122,
    &FreeSansBold18pt7b,
    COL_ACCENT
  );

  drawRFIDIcon(240, 175, COL_ACCENT);

  int bw = 320;
  int bx = 240 - bw / 2;
  int by = 268;

  tft.drawRoundRect(bx, by, bw, 16, 8, COL_PANEL);

  for (int p = 0; p <= 100; p += 5) {
    tft.fillRoundRect(bx, by, bw * p / 100, 16, 8, COL_ACCENT);
    delay(100);
  }

  delay(2000);

  screen = SCR_BOOT;
}

// ==================================================
// LAYAR MENGHUBUNGKAN WIFI
// ==================================================

void showWiFiScreen() {

  tft.fillScreen(COL_BG);
  drawHeader("MENGHUBUNGKAN WIFI", COL_YELLOW);

  drawCentered(
    "RFID ATTENDANCE",
    240, 110,
    &FreeSansBold18pt7b,
    TFT_WHITE
  );

  drawCentered(
    "Menghubungkan ke jaringan...",
    240, 150,
    &FreeSans12pt7b,
    COL_GRAY
  );

  tft.drawArc(240, 230, 42, 35, 0, 360, COL_PANEL, COL_PANEL);

  screen = SCR_WIFI;
}

// ==================================================
// LAYAR WIFI CONNECTED
// ==================================================

void showReady() {

  tft.fillScreen(COL_BG);
  drawHeader("WIFI CONNECTED", COL_GREEN);

  drawCentered(
    "WIFI TERHUBUNG",
    240, 110,
    &FreeSansBold18pt7b,
    TFT_WHITE
  );

  drawCentered(
    "IP ADDRESS",
    240, 175,
    &FreeSansBold12pt7b,
    COL_GRAY
  );

  drawCentered(
    WiFi.localIP().toString(),
    240, 215,
    &FreeSansBold24pt7b,
    COL_GREEN
  );

  drawCentered(
    "MENYIAPKAN SISTEM...",
    240, 270,
    &FreeSans12pt7b,
    COL_GRAY
  );

  screen = SCR_READY;

  delay(2000);
}

// ==================================================
// LAYAR SIAP / MENUNGGU KARTU
// ==================================================

void showIdle() {

  tft.fillScreen(COL_BG);
  drawHeader("RFID ATTENDANCE", COL_ACCENT);
  drawWifiIndicator();

  int cx = 240;
  int cy = 140;

  tft.drawSmoothCircle(cx, cy, 56, COL_RING, COL_BG);
  tft.drawArc(cx, cy, 80, 72, 0, 360, COL_RING, COL_RING);

  drawRFIDIcon(cx, cy, COL_ACCENT);

  idleSweep = 0;
  idlePulse = 0;
  pulseR = 62;
  prevPulseR = 62;
  lastIdleAnim = millis();

  drawCentered(
    "TEMPELKAN KARTU RFID",
    240, 250,
    &FreeSansBold18pt7b,
    TFT_WHITE
  );

  drawCentered(
    "Silakan tempelkan kartu di atas reader",
    240, 288,
    &FreeSans12pt7b,
    COL_GRAY
  );

  screen = SCR_IDLE;
}

// ==================================================
// ANIMASI LAYAR SIAP (RADAR)
// ==================================================

void updateIdleAnimation() {

  unsigned long now = millis();

  if (now - lastIdleAnim < 33) {
    return;
  }

  lastIdleAnim = now;

  int cx = 240;
  int cy = 140;

  // hapus pulse lama
  tft.drawSmoothCircle(cx, cy, prevPulseR, COL_RING, COL_RING);

  // hapus sweep lama, gambar ulang ring penuh
  tft.drawArc(cx, cy, 80, 72, 0, 360, COL_RING, COL_RING);

  // gambar sweep baru
  tft.drawArc(
    cx, cy, 80, 72,
    idleSweep, idleSweep + 70,
    COL_ACCENT,
    COL_RING
  );

  idleSweep = (idleSweep + 8) % 360;

  // pulse
  prevPulseR = pulseR;
  pulseR = 62 + (int)(8.0 * sin(idlePulse * 0.05));
  idlePulse++;

  tft.drawSmoothCircle(cx, cy, pulseR, COL_ACCENT, COL_RING);

  // gambar ikon agar selalu di atas
  drawRFIDIcon(cx, cy, COL_ACCENT);
}

// ==================================================
// LAYAR KARTU TERDETEKSI
// ==================================================

void showCardDetected(
  const String& uid
) {

  tft.fillScreen(COL_BG);
  drawHeader("KARTU TERDETEKSI", COL_ACCENT);

  drawRFIDIcon(240, 120, COL_ACCENT);

  drawCentered(
    "UID KARTU",
    240, 190,
    &FreeSansBold12pt7b,
    COL_GRAY
  );

  drawCentered(
    uid,
    240, 225,
    &FreeSansBold24pt7b,
    TFT_WHITE
  );

  drawCentered(
    "MEMBACA DATA...",
    240, 272,
    &FreeSans12pt7b,
    COL_GRAY
  );

  screen = SCR_DETECT;
}

// ==================================================
// LAYAR MENGIRIM DATA
// ==================================================

void showSending(
  const String& uid
) {

  tft.fillScreen(COL_BG);
  drawHeader("MENGIRIM DATA", COL_YELLOW);

  drawCentered(
    "ABSENSI MASUK",
    240, 110,
    &FreeSansBold18pt7b,
    TFT_WHITE
  );

  drawCentered(
    uid,
    240, 148,
    &FreeSansBold12pt7b,
    COL_GRAY
  );

  tft.drawArc(240, 230, 42, 35, 0, 360, COL_PANEL, COL_PANEL);

  screen = SCR_SEND;
}

// ==================================================
// LAYAR HASIL
// ==================================================

void showResult(
  int iconType,
  const String& title,
  const String& line1,
  const String& line2,
  const String& line3,
  uint16_t accent
) {

  tft.fillScreen(COL_BG);
  drawHeader(title.c_str(), accent);
  drawStatusIcon(iconType);

  if (line1.length() > 0) {
    drawCentered(line1, 240, 214, &FreeSansBold24pt7b, TFT_WHITE);
  }

  if (line2.length() > 0) {
    drawCentered(line2, 240, 254, &FreeSansBold18pt7b, accent);
  }

  if (line3.length() > 0) {
    drawCentered(line3, 240, 294, &FreeSans12pt7b, COL_GRAY);
  }

  screen = SCR_RESULT;
}

// ==================================================
// HASIL: TEPAT
// ==================================================

void showResultTepat(
  const String& nama,
  const String& kelas
) {
  showResult(
    0,
    "ABSENSI BERHASIL",
    nama,
    kelas,
    "SELAMAT BELAJAR",
    COL_GREEN
  );
}

// ==================================================
// HASIL: TELAT
// ==================================================

void showResultTelat(
  const String& nama,
  const String& kelas
) {
  showResult(
    1,
    "ABSENSI TELAT",
    nama,
    kelas,
    "IQOB",
    COL_ORANGE
  );

  delay(2000);

  tft.fillScreen(COL_BG);
  drawHeader("ABSENSI TELAT", COL_ORANGE);

  drawCentered(
    "IQOB",
    240, 130,
    &FreeSansBold24pt7b,
    COL_ORANGE
  );

  drawCentered(
    "Mohon datang tepat waktu",
    240, 190,
    &FreeSans12pt7b,
    COL_GRAY
  );

  drawCentered(
    "Absensi tercatat sebagai TELAT",
    240, 225,
    &FreeSans12pt7b,
    COL_GRAY
  );
}

// ==================================================
// HASIL: SUDAH ABSEN
// ==================================================

void showResultSudahAbsen(
  const String& nama,
  const String& kelas
) {
  showResult(
    2,
    "ABSENSI DITOLAK",
    nama,
    kelas,
    "SUDAH ABSEN",
    COL_RED
  );
}

// ==================================================
// HASIL: AKSES DITOLAK
// ==================================================

void showResultDenied() {
  showResult(
    2,
    "AKSES DITOLAK",
    "UID TIDAK ADA",
    "KARTU TIDAK TERDAFTAR",
    "",
    COL_RED
  );
}

// ==================================================
// HASIL: SHEET ERROR
// ==================================================

void showResultSheetError() {
  showResult(
    2,
    "ERROR DATABASE",
    "CEK DATABASE",
    "DAN ABSENSI",
    "",
    COL_ORANGE
  );
}

// ==================================================
// HASIL: SERVER ERROR
// ==================================================

void showResultServerError(
  const String& status
) {
  showResult(
    2,
    "SERVER ERROR",
    status,
    "",
    "",
    COL_RED
  );
}

// ==================================================
// HASIL: HTTP ERROR
// ==================================================

void showResultHttpError(
  int httpCode
) {
  showResult(
    2,
    "HTTP ERROR",
    String(httpCode),
    "",
    "",
    COL_RED
  );
}

// ==================================================
// HASIL: KONEKSI GAGAL
// ==================================================

void showResultConnFail() {
  showResult(
    2,
    "KONEKSI GAGAL",
    "SERVER",
    "",
    "",
    COL_RED
  );
}

// ==================================================
// BUZZER BERHASIL
// ==================================================

void beepSuccess() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

// ==================================================
// BUZZER DITOLAK
// ==================================================

void beepDenied() {
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
}

// ==================================================
// BUZZER TELAT
// ==================================================

void beepDouble() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);

  delay(100);

  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

// ==================================================
// WIFI
// ==================================================

void connectWiFi() {

  showWiFiScreen();

  Serial.println();
  Serial.println("Menghubungkan WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long animT = millis();
  int a = 0;
  int percobaan = 0;

  while (WiFi.status() != WL_CONNECTED) {

    // animasi spinner
    if (millis() - animT >= 40) {
      animT = millis();
      tft.drawArc(240, 230, 42, 35, 0, 360, COL_PANEL, COL_PANEL);
      tft.drawArc(240, 230, 42, 35, a, a + 90, COL_ACCENT, COL_PANEL);
      a = (a + 12) % 360;
    }

    delay(5);

    percobaan++;

    if (percobaan >= 4000) {

      Serial.println();
      Serial.println("WiFi gagal.");

      WiFi.disconnect();

      delay(1000);

      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

      percobaan = 0;
    }
  }

  Serial.println();
  Serial.println("WIFI CONNECTED");

  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  showReady();
}

// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" RFID ATTENDANCE (TFT)");
  Serial.println("==============================");

  // -----------------------------
  // BUZZER
  // -----------------------------

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // -----------------------------
  // TFT
  // -----------------------------

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_BG);

  Serial.println("TFT OK");

  showBoot();

  // -----------------------------
  // WIFI
  // -----------------------------

  connectWiFi();

  // -----------------------------
  // SPI RFID
  // -----------------------------

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);

  // -----------------------------
  // RC522
  // -----------------------------

  mfrc522.PCD_Init();

  delay(500);

  Serial.println("RC522 OK");

  // -----------------------------
  // SIAP
  // -----------------------------

  showIdle();

  Serial.println();
  Serial.println("SISTEM SIAP");
  Serial.println("Tempelkan kartu RFID...");
  Serial.println("==============================");
}

// ==================================================
// LOOP
// ==================================================

void loop() {

  // ==================================================
  // ANIMASI LAYAR SIAP
  // ==================================================

  if (screen == SCR_IDLE) {
    updateIdleAnimation();
  }

  // ==================================================
  // CEK WIFI
  // ==================================================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi terputus.");

    connectWiFi();

    showIdle();
  }

  // ==================================================
  // CEK KARTU
  // ==================================================

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // ==================================================
  // BACA UID
  // ==================================================

  String uid = "";

  for (
    byte i = 0;
    i < mfrc522.uid.size;
    i++
  ) {

    if (mfrc522.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(mfrc522.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();

  // ==================================================
  // SERIAL
  // ==================================================

  Serial.println();
  Serial.println("==============================");
  Serial.print("UID : ");
  Serial.println(uid);
  Serial.println("==============================");

  // ==================================================
  // TFT: KARTU TERDETEKSI
  // ==================================================

  showCardDetected(uid);

  beepSuccess();

  delay(1200);

  // ==================================================
  // TFT: MENGIRIM DATA
  // ==================================================

  showSending(uid);

  runSpinner(240, 230, 42, 1200);

  Serial.println("Mengirim data absensi...");

  // ==================================================
  // HTTPS
  // ==================================================

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient https;

  // PENTING:
  // Google Apps Script melakukan redirect.
  // ESP32 harus mengikuti redirect.

  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // ==================================================
  // URL
  // ==================================================

  String url = String(SCRIPT_URL) + uid;

  Serial.print("URL : ");
  Serial.println(url);

  // ==================================================
  // BEGIN HTTPS
  // ==================================================

  if (!https.begin(client, url)) {

    Serial.println("HTTPS BEGIN GAGAL");

    showResultConnFail();

    beepDenied();

    https.end();

    delay(3000);

    showIdle();

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return;
  }

  // ==================================================
  // GET
  // ==================================================

  int httpCode = https.GET();

  Serial.print("HTTP CODE : ");
  Serial.println(httpCode);

  // ==================================================
  // RESPONSE
  // ==================================================

  if (httpCode > 0) {

    String response = https.getString();

    response.trim();

    Serial.println();
    Serial.println("RESPON GOOGLE APPS SCRIPT:");
    Serial.println(response);
    Serial.println();

    // ==================================================
    // PARSING
    //
    // TEPAT|BUDI|IX A
    // TELAT|BUDI|IX A
    // SUDAH_ABSEN|BUDI|IX A
    // DENIED
    // ==================================================

    String status = "";
    String nama = "";
    String kelas = "";

    int p1 = response.indexOf('|');

    if (p1 >= 0) {

      // STATUS

      status = response.substring(0, p1);

      String sisa = response.substring(p1 + 1);

      int p2 = sisa.indexOf('|');

      if (p2 >= 0) {

        // NAMA

        nama = sisa.substring(0, p2);

        // KELAS

        kelas = sisa.substring(p2 + 1);
      }
      else {

        nama = sisa;
      }

      status.trim();
      nama.trim();
      kelas.trim();
    }
    else {

      status = response;

      status.trim();
    }

    // ==================================================
    // SERIAL HASIL
    // ==================================================

    Serial.print("STATUS : ");
    Serial.println(status);
    Serial.print("NAMA   : ");
    Serial.println(nama);
    Serial.print("KELAS  : ");
    Serial.println(kelas);

    // ==================================================
    // TEPAT
    // ==================================================

    if (status == "TEPAT") {

      Serial.println("ABSENSI BERHASIL");

      showResultTepat(nama, kelas);

      beepSuccess();
    }

    // ==================================================
    // TELAT
    // ==================================================

    else if (status == "TELAT") {

      Serial.println("ABSENSI TELAT");
      Serial.println("IQOB");

      showResultTelat(nama, kelas);

      beepDouble();

      delay(2000);
    }

    // ==================================================
    // SUDAH ABSEN
    // ==================================================

    else if (status == "SUDAH_ABSEN") {

      Serial.println("ABSENSI DITOLAK");
      Serial.println("SUDAH ABSEN");

      showResultSudahAbsen(nama, kelas);

      beepDenied();

      delay(2000);
    }

    // ==================================================
    // UID TIDAK ADA
    // ==================================================

    else if (status == "DENIED") {

      Serial.println("UID TIDAK ADA");

      showResultDenied();

      beepDenied();
    }

    // ==================================================
    // SHEET ERROR
    // ==================================================

    else if (status == "SHEET_ERROR") {

      Serial.println("SHEET ERROR");

      showResultSheetError();

      beepDenied();
    }

    // ==================================================
    // ERROR LAIN
    // ==================================================

    else {

      Serial.println("RESPON TIDAK DIKENALI");

      showResultServerError(status);

      beepDenied();
    }
  }

  // ==================================================
  // HTTP ERROR
  // ==================================================

  else {

    Serial.print("HTTP ERROR : ");
    Serial.println(httpCode);

    showResultHttpError(httpCode);

    beepDenied();
  }

  // ==================================================
  // TUTUP HTTPS
  // ==================================================

  https.end();

  // ==================================================
  // SELESAI
  // ==================================================

  delay(3000);

  showIdle();

  // ==================================================
  // HENTIKAN KARTU
  // ==================================================

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(500);
}
