#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================================================
// OLED
// ==================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ==================================================
// RFID RC522
// ==================================================

#define SS_PIN 5
#define RST_PIN 4

#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23

MFRC522 mfrc522(
  SS_PIN,
  RST_PIN
);

// ==================================================
// BUZZER
// ==================================================

#define BUZZER 15

// ==================================================
// WIFI
// ==================================================

const char* WIFI_SSID = "Medjay";
const char* WIFI_PASSWORD = "devilkiller";

// ==================================================
// GOOGLE APPS SCRIPT
// ==================================================

const char* SCRIPT_URL =
"https://script.google.com/macros/s/AKfycbw5VAYZxcCi9k2rtpwnHgiynZD8JU8fqUTizF9D0tlZcpZPtaZh1KHbO9KJ1A7kk2kV/exec?uid=";


// ==================================================
// OLED DISPLAY
// ==================================================

void showMessage(
  String line1,
  String line2 = "",
  String line3 = "",
  String line4 = ""
) {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(line1);

  if (line2 != "") {
    display.println();
    display.println(line2);
  }

  if (line3 != "") {
    display.println();
    display.println(line3);
  }

  if (line4 != "") {
    display.println();
    display.println(line4);
  }

  display.display();
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

  showMessage(
    "RFID ATTENDANCE",
    "Menghubungkan WiFi"
  );

  Serial.println();
  Serial.println("Menghubungkan WiFi...");

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  int percobaan = 0;

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

    percobaan++;

    if (percobaan >= 40) {

      Serial.println();
      Serial.println("WiFi gagal.");

      WiFi.disconnect();

      delay(1000);

      WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
      );

      percobaan = 0;
    }
  }

  Serial.println();
  Serial.println("WIFI CONNECTED");

  Serial.print("IP ESP32: ");
  Serial.println(
    WiFi.localIP()
  );

  showMessage(
    "WIFI CONNECTED",
    WiFi.localIP().toString()
  );

  delay(2000);
}


// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" RFID ATTENDANCE");
  Serial.println("==============================");

  // -----------------------------
  // BUZZER
  // -----------------------------

  pinMode(
    BUZZER,
    OUTPUT
  );

  digitalWrite(
    BUZZER,
    LOW
  );

  // -----------------------------
  // OLED
  // -----------------------------

  Wire.begin(
    21,
    22
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println(
      "OLED ERROR"
    );

    while (true) {
      delay(1000);
    }
  }

  Serial.println(
    "OLED OK"
  );

  // -----------------------------
  // WIFI
  // -----------------------------

  connectWiFi();

  // -----------------------------
  // SPI RFID
  // -----------------------------

  SPI.begin(
    SCK_PIN,
    MISO_PIN,
    MOSI_PIN,
    SS_PIN
  );

  // -----------------------------
  // RC522
  // -----------------------------

  mfrc522.PCD_Init();

  delay(500);

  Serial.println(
    "RC522 OK"
  );

  // -----------------------------
  // SIAP
  // -----------------------------

  showMessage(
    "RFID ATTENDANCE",
    "TEMPELKAN",
    "KARTU RFID"
  );

  Serial.println();
  Serial.println(
    "SISTEM SIAP"
  );

  Serial.println(
    "Tempelkan kartu RFID..."
  );

  Serial.println(
    "=============================="
  );
}


// ==================================================
// LOOP
// ==================================================

void loop() {

  // ==================================================
  // CEK WIFI
  // ==================================================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println(
      "WiFi terputus."
    );

    connectWiFi();
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

    if (
      mfrc522.uid.uidByte[i] < 0x10
    ) {

      uid += "0";
    }

    uid += String(
      mfrc522.uid.uidByte[i],
      HEX
    );
  }

  uid.toUpperCase();


  // ==================================================
  // SERIAL
  // ==================================================

  Serial.println();
  Serial.println(
    "=============================="
  );

  Serial.print(
    "UID : "
  );

  Serial.println(uid);

  Serial.println(
    "=============================="
  );


  // ==================================================
  // OLED
  // ==================================================

  showMessage(
    "KARTU TERDETEKSI",
    uid
  );

  beepSuccess();

  delay(1000);


  // ==================================================
  // MENGIRIM DATA
  // ==================================================

  showMessage(
    "MENGIRIM DATA",
    "ABSENSI MASUK",
    uid
  );

  Serial.println(
    "Mengirim data absensi..."
  );


  // ==================================================
  // HTTPS
  // ==================================================

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient https;

  // PENTING:
  // Google Apps Script melakukan redirect.
  // ESP32 harus mengikuti redirect.

  https.setFollowRedirects(
    HTTPC_STRICT_FOLLOW_REDIRECTS
  );


  // ==================================================
  // URL
  // ==================================================

  String url =
    String(SCRIPT_URL) + uid;

  Serial.print(
    "URL : "
  );

  Serial.println(url);


  // ==================================================
  // BEGIN HTTPS
  // ==================================================

  if (!https.begin(
        client,
        url
      )) {

    Serial.println(
      "HTTPS BEGIN GAGAL"
    );

    showMessage(
      "KONEKSI GAGAL",
      "SERVER"
    );

    beepDenied();

    https.end();

    delay(3000);

    showMessage(
      "TEMPELKAN",
      "KARTU RFID"
    );

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return;
  }


  // ==================================================
  // GET
  // ==================================================

  int httpCode =
    https.GET();

  Serial.print(
    "HTTP CODE : "
  );

  Serial.println(
    httpCode
  );


  // ==================================================
  // RESPONSE
  // ==================================================

  if (httpCode > 0) {

    String response =
      https.getString();

    response.trim();


    Serial.println();
    Serial.println(
      "RESPON GOOGLE APPS SCRIPT:"
    );

    Serial.println(
      response
    );

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


    int p1 =
      response.indexOf('|');


    if (p1 >= 0) {

      // STATUS

      status =
        response.substring(
          0,
          p1
        );


      String sisa =
        response.substring(
          p1 + 1
        );


      int p2 =
        sisa.indexOf('|');


      if (p2 >= 0) {

        // NAMA

        nama =
          sisa.substring(
            0,
            p2
          );


        // KELAS

        kelas =
          sisa.substring(
            p2 + 1
          );

      }
      else {

        nama = sisa;
      }


      status.trim();
      nama.trim();
      kelas.trim();

    }
    else {

      status =
        response;

      status.trim();
    }


    // ==================================================
    // SERIAL HASIL
    // ==================================================

    Serial.print(
      "STATUS : "
    );

    Serial.println(
      status
    );

    Serial.print(
      "NAMA   : "
    );

    Serial.println(
      nama
    );

    Serial.print(
      "KELAS  : "
    );

    Serial.println(
      kelas
    );


    // ==================================================
    // TEPAT
    // ==================================================

    if (status == "TEPAT") {

      Serial.println(
        "ABSENSI BERHASIL"
      );

      showMessage(
        "ABSENSI BERHASIL",
        nama,
        kelas
      );

      beepSuccess();
    }


    // ==================================================
    // TELAT
    // ==================================================

    else if (
      status == "TELAT"
    ) {

      Serial.println(
        "ABSENSI TELAT"
      );

      Serial.println(
        "IQOB"
      );


      showMessage(
        "ABSENSI TELAT",
        nama,
        kelas
      );

      beepDouble();

      delay(2000);


      showMessage(
        "ABSENSI TELAT",
        nama,
        "IQOB"
      );
    }


    // ==================================================
    // SUDAH ABSEN
    // ==================================================

    else if (
      status == "SUDAH_ABSEN"
    ) {

      Serial.println(
        "ABSENSI DITOLAK"
      );

      Serial.println(
        "SUDAH ABSEN"
      );


      showMessage(
        "ABSENSI DITOLAK",
        nama,
        kelas
      );

      beepDenied();

      delay(2000);


      showMessage(
        "ABSENSI DITOLAK",
        "SUDAH ABSEN"
      );
    }


    // ==================================================
    // UID TIDAK ADA
    // ==================================================

    else if (
      status == "DENIED"
    ) {

      Serial.println(
        "UID TIDAK ADA"
      );


      showMessage(
        "AKSES DITOLAK",
        "UID TIDAK ADA"
      );

      beepDenied();
    }


    // ==================================================
    // SHEET ERROR
    // ==================================================

    else if (
      status == "SHEET_ERROR"
    ) {

      Serial.println(
        "SHEET ERROR"
      );


      showMessage(
        "ERROR DATABASE",
        "CEK DATABASE",
        "DAN ABSENSI"
      );

      beepDenied();
    }


    // ==================================================
    // ERROR LAIN
    // ==================================================

    else {

      Serial.println(
        "RESPON TIDAK DIKENALI"
      );


      showMessage(
        "SERVER ERROR",
        status
      );

      beepDenied();
    }
  }


  // ==================================================
  // HTTP ERROR
  // ==================================================

  else {

    Serial.print(
      "HTTP ERROR : "
    );

    Serial.println(
      httpCode
    );


    showMessage(
      "HTTP ERROR",
      String(httpCode)
    );

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


  showMessage(
    "TEMPELKAN",
    "KARTU RFID"
  );


  // ==================================================
  // HENTIKAN KARTU
  // ==================================================

  mfrc522.PICC_HaltA();

  mfrc522.PCD_StopCrypto1();

  delay(500);
}
