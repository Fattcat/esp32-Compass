#include <TinyGPS++.h>  // Knižnica pre GPS
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// Inicializácia GPS a TFT
TinyGPSPlus gps;

// Nastavenie pinov pre ST7789
#define TFT_CS     15   // Môže byť vynechané, ak displej nemá CS pin
#define TFT_RST    4
#define TFT_DC     2

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  // Inicializácia sériovej komunikácie a displeja
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX/TX piny pre GPS modul
  
  // Inicializácia TFT displeja
  tft.init(240, 320);  // Uprav podľa rozlíšenia displeja (napr. 240x240)
  tft.invertDisplay(0);
  tft.setRotation(1);  // Nastavenie rotácie displeja
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  // Načítavanie GPS dát
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // Kontrola platnosti GPS signálu
  if (!gps.location.isValid()) {
    // Ak GPS nemá platné dáta, zobraz upozornenie
    tft.fillScreen(ST77XX_BLACK);  // Vymaže displej
    tft.setCursor(30, 110);        // Nastaví pozíciu textu
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.print("Nie je GPS signal!");
  } else {
    // Keď sú GPS dáta platné a zariadenie sa pohybuje
    if (gps.location.isUpdated() && gps.speed.isValid() && gps.course.isValid()) {
      float heading = gps.course.deg();  // Azimut/smer pohybu v stupňoch

      // Aktualizácia displeja
      tft.fillScreen(ST77XX_BLACK);
      
      // Zobrazenie textu na displeji
      tft.setCursor(10, 10);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2);
      tft.print("Kompas");

      // Výpočet pozície šípky na displeji
      int x_center = 120, y_center = 120;  // Stred displeja pre rozlíšenie 240x240
      float arrow_length = 50;
      float rad = heading * PI / 180;  // Prevod uhla na radiány

      // Koncové súradnice šípky smerom na sever
      int x_end = x_center + arrow_length * cos(rad - PI / 2);  // -PI/2 pre otočenie o 90°
      int y_end = y_center + arrow_length * sin(rad - PI / 2);

      // Vykreslenie šípky
      tft.drawLine(x_center, y_center, x_end, y_end, ST77XX_RED);  // Hlavná šípka
      tft.fillTriangle(x_end, y_end,
                       x_end - 5 * cos(rad - PI / 2 + PI / 4), y_end - 5 * sin(rad - PI / 2 + PI / 4),
                       x_end - 5 * cos(rad - PI / 2 - PI / 4), y_end - 5 * sin(rad - PI / 2 - PI / 4),
                       ST77XX_RED);

      // Zobrazenie svetových strán
      tft.setCursor(110, 30);
      tft.print("N");  // Sever
      tft.setCursor(200, 120);
      tft.print("E");  // Východ
      tft.setCursor(120, 200);
      tft.print("S");  // Juh
      tft.setCursor(30, 120);
      tft.print("W");  // Západ

      delay(1000);  // Obnovovacia frekvencia (1 sekunda)
    }
  }
}
