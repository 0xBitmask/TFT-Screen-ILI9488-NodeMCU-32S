// konfigurasi pin dan driver di library TFT_eSPI
// sesuaikan driver dengan board, kalau disi menggunakan:
// ILI9488, gunakan itu dengan menghapus komentar

#include <TFT_eSPI.h>

TFT_eSPI tft;

int x = 20;

void setup()
{
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

void loop()
{
    tft.fillCircle(x,120,20,TFT_BLACK);
    tft.fillRect(x,200,40,40,TFT_BLACK);

    x++;

    if(x>460)
        x=20;

    tft.fillCircle(x,120,20,TFT_RED);
    tft.fillRect(x,200,40,40,TFT_VIOLET);

    delay(10);
}