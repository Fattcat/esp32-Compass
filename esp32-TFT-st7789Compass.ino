#include <TinyGPS++.h>  // Knižnica pre GPS
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// Inicializácia GPS a TFT
TinyGPSPlus gps;

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         15
  #define TFT_RST        4
  #define TFT_DC         2
  #define TFT_MOSI       23
  #define TFT_SCK        18
  #define TFT_MISO       19

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  #define TFT_CS         15
  #define TFT_RST        4
  #define TFT_DC         2
  #define TFT_MOSI       23
  #define TFT_SCK        18
  #define TFT_MISO       19
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

bool hasGPSFix = false;  // Sleduje stav GPS signálu
unsigned long lastDotTime = 0;
int dotCount = 0;  // Počet bodiek v animácii

void setup() {
  // Inicializácia sériovej komunikácie a displeja
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX/TX piny pre GPS modul
  
  // Inicializácia TFT displeja
  tft.init(240, 320);  // Nastavenie rozlíšenia displeja 240x320
  tft.setRotation(1);  // Nastavenie rotácie displeja
  tft.invertDisplay(0); // Vypnutie invertovania displeja (zmeniť na 1, ak farby zostávajú invertné)
  tft.fillScreen(ST77XX_BLACK);

  // Kontrola pripojenia GPS modulu
  Serial.print("Kontrola pripojenia GPS modulu...");

  bool gpsFound = false;
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {  // Čaká 3 sekundy na odpoveď GPS modulu
    if (Serial2.available() > 0) {
      gpsFound = true;
      break;
    }
  }

  if (gpsFound) {
    Serial.println("GPS modul nájdený úspešne.");
    tft.setCursor(20, 150);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(2);
    tft.print("GPS modul najdeny uspesne");
    delay(2000);  // Zobrazí správu na 2 sekundy
    tft.fillScreen(ST77XX_BLACK);  // Vymaže obrazovku
  } else {
    Serial.println("GPS modul nenájdený!");
    tft.setCursor(30, 150);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.print("GPS modul nenajdeny!");
    while (true);  // Zastaví program, ak GPS modul nie je pripojený
  }
}

void displayConnectingToSatellite() {
  // Zobrazenie správy iba pri prvom spustení
  tft.setCursor(30, 150);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.print("Pripajam sa k satelitu");

  // Animácia bodiek
  if (millis() - lastDotTime > 1000) {  // Každú sekundu pridá bodku
    dotCount = (dotCount + 1) % 4;  // Opakuje sa od 0 do 3
    lastDotTime = millis();

    // Vymaže predchádzajúce bodky iba v oblasti animácie
    tft.fillRect(150, 180, 60, 30, ST77XX_BLACK);

    // Zobrazenie bodiek
    for (int i = 0; i < dotCount; i++) {
      tft.setCursor(150 + (i * 10), 180);
      tft.print(".");
    }
  }
}

void displayNoSignal() {
  // Funkcia na zobrazenie chybového hlásenia pri strate signálu
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 150);  // Nastavenie stredu obrazovky pre text
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.print("Nie je GPS signal!");
}

void loop() {
  // Načítavanie GPS dát
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  if (!gps.location.isValid()) {
    // Ak GPS nemá platné dáta
    if (!hasGPSFix) {
      displayConnectingToSatellite();  // Zobrazuje animáciu pripájania
    }
  } else {
    // GPS má platné údaje, nastavíme fix
    if (!hasGPSFix) {
      hasGPSFix = true;
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(10, 10);
      tft.setTextColor(ST77XX_GREEN);
      tft.setTextSize(2);
      tft.print("Pocet satelitov: ");
      tft.print(gps.satellites.value());  // Zobrazí počet satelitov
      delay(2000);  // Zobrazí správu na 2 sekundy
    }

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
      int x_center = 120, y_center = 160;  // Stred displeja pre rozlíšenie 240x320
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
      tft.setCursor(200, 160);
      tft.print("E");  // Východ
      tft.setCursor(120, 290);
      tft.print("S");  // Juh
      tft.setCursor(30, 160);
      tft.print("W");  // Západ

      delay(1000);  // Obnovovacia frekvencia (1 sekunda)
    }
  }
}
