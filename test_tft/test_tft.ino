#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup()
{
    tft.init();

    tft.setRotation(1);

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextSize(2);

    tft.drawString("ESP32 + ILI9488",20,20);

    tft.drawLine(20,60,300,60,TFT_RED);

    tft.drawRect(20,80,120,80,TFT_GREEN);

    tft.fillCircle(250,120,40,TFT_BLUE);

    tft.drawRoundRect(150,220,120,80,10,TFT_YELLOW);

    tft.fillRoundRect(30,220,80,80,15,TFT_MAGENTA);

    tft.drawFastHLine(0,300,480,TFT_CYAN);

    tft.drawFastVLine(160,0,320,TFT_WHITE);

    tft.setCursor(20,340);

    tft.print("Resolution:");

    tft.setCursor(20,370);

    tft.print("320 x 480");
}

void loop()
{

}