#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ===================
// TFT ST7789 nastavenia
#define TFT_CS    15
#define TFT_RST   4
#define TFT_DC    2
#define TFT_SCLK  18
#define TFT_MOSI  23
// TFT LED pin pripoj na pin esp32 "3.3V"
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ===================
// GPS – NEO-6M na Serial2
#define GPS_RX 16  // ESP32 RX (pripoj na TX z GPS)
#define GPS_TX 17  // ESP32 TX (pripoj na RX z GPS)

TinyGPSPlus gps;
bool hasGPSFix = false;
unsigned long lastUpdate = 0;
unsigned long lastDotTime = 0;
int dotCount = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // Inicializácia displeja
  tft.init(240, 320);
  tft.setRotation(1);
  tft.invertDisplay(false);
  tft.fillScreen(ST77XX_BLACK);

  // Test pripojenia GPS modulu
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(20, 140);
  tft.print("Cakam na GPS...");

  unsigned long timeout = millis() + 5000;
  while (millis() < timeout) {
    if (Serial2.available()) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextColor(ST77XX_GREEN);
      tft.setCursor(20, 140);
      tft.print("GPS modul OK");
      delay(1500);
      tft.fillScreen(ST77XX_BLACK);
      return;
    }
  }

  tft.fillScreen(ST77XX_RED);
  tft.setCursor(20, 140);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS modul NENAJDENY");
  while (true);  // Stop, chyba GPS
}

void drawWaitingAnimation() {
  tft.setCursor(30, 150);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.print("Pripajam sa k satelitu");

  if (millis() - lastDotTime > 800) {
    dotCount = (dotCount + 1) % 4;
    lastDotTime = millis();
    tft.fillRect(150, 180, 60, 30, ST77XX_BLACK);
    for (int i = 0; i < dotCount; i++) {
      tft.setCursor(150 + (i * 10), 180);
      tft.print(".");
    }
  }
}

void drawCompass(float heading, int satellites) {
  tft.fillScreen(ST77XX_BLACK);

  // Názov
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("Kompas");

  // Počet satelitov
  tft.setCursor(10, 40);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Satelity: ");
  tft.print(satellites);

  // Kompas šípka
  int cx = 120, cy = 160;
  float rad = (heading - 90) * PI / 180.0;
  int len = 50;
  int x1 = cx + len * cos(rad);
  int y1 = cy + len * sin(rad);

  tft.drawLine(cx, cy, x1, y1, ST77XX_RED);
  tft.fillTriangle(x1, y1,
                   x1 - 5 * cos(rad + PI / 6), y1 - 5 * sin(rad + PI / 6),
                   x1 - 5 * cos(rad - PI / 6), y1 - 5 * sin(rad - PI / 6),
                   ST77XX_RED);

  // Svetové strany
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(110, 30); tft.print("N");
  tft.setCursor(200, 160); tft.print("E");
  tft.setCursor(120, 290); tft.print("S");
  tft.setCursor(30, 160); tft.print("W");
}

void drawNoSignal() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 150);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.print("Nie je GPS signal!");
}

void loop() {
  while (Serial2.available()) {
    gps.encode(Serial2.read());
  }

  // Časový limit na aktualizáciu
  if (millis() - lastUpdate < 1000) return;
  lastUpdate = millis();

  if (!gps.location.isValid() || !gps.satellites.isValid()) {
    if (hasGPSFix) {
      hasGPSFix = false;
      drawNoSignal();
    } else {
      drawWaitingAnimation();
    }
    return;
  }

  // Ak sa získal fix po prvýkrát
  if (!hasGPSFix) {
    hasGPSFix = true;
    tft.fillScreen(ST77XX_BLACK);
  }

  // Ak sú údaje aktualizované
  if (gps.course.isValid()) {
    float heading = gps.course.deg();
    int sats = gps.satellites.value();
    drawCompass(heading, sats);
  }
}
