// ESP32 + ST7789 240x320 TFT + HMC5883L magnetometer (kompas)
// Zobrazenie kompasu so sipkou a pozadim

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_HMC5883_U.h>
#include <SPI.h>

// TFT displej - piny (upravit podla zapojenia)
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Inicializacia displeja
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  
  // Inicializacia magnetometra
  if (!mag.begin()) {
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(20, 150);
    tft.print("HMC5883L chyba!");
    while (1);
  }

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(30, 150);
  tft.print("Kompas inicializovany");
  delay(2000);
}

void drawCompass(float heading) {
  tft.fillScreen(ST77XX_BLACK);

  int cx = 120; // stred X
  int cy = 160; // stred Y
  int r = 100;  // polomer kruhu

  // Kruhove pozadie
  tft.drawCircle(cx, cy, r, ST77XX_WHITE);
  tft.drawCircle(cx, cy, r - 1, ST77XX_WHITE);
  tft.drawCircle(cx, cy, r - 2, ST77XX_WHITE);

  // Svetove strany
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(cx - 5, cy - r - 20);
  tft.print("N");
  tft.setCursor(cx + r + 5, cy - 5);
  tft.print("E");
  tft.setCursor(cx - 5, cy + r + 5);
  tft.print("S");
  tft.setCursor(cx - r - 15, cy - 5);
  tft.print("W");

  // Prevod heading na radiany (0 = sever)
  float rad = (heading - 90) * DEG_TO_RAD;

  int len = 80;
  int x_end = cx + len * cos(rad);
  int y_end = cy + len * sin(rad);

  // Siipka - cervena, hrubsia
  for (int i = -1; i <= 1; i++) {
    tft.drawLine(cx + i, cy, x_end + i, y_end, ST77XX_RED);
    tft.drawLine(cx, cy + i, x_end, y_end + i, ST77XX_RED);
  }

  // Spicka sipky (trojuholnik)
  int arrowSize = 10;
  float angle1 = rad + PI / 8;
  float angle2 = rad - PI / 8;

  int x1 = x_end;
  int y1 = y_end;
  int x2 = x_end - arrowSize * cos(angle1);
  int y2 = y_end - arrowSize * sin(angle1);
  int x3 = x_end - arrowSize * cos(angle2);
  int y3 = y_end - arrowSize * sin(angle2);

  tft.fillTriangle(x1, y1, x2, y2, x3, y3, ST77XX_RED);
}

void loop() {
  sensors_event_t event;
  mag.getEvent(&event);

  float heading = atan2(event.magnetic.y, event.magnetic.x);
  if (heading < 0) heading += 2 * PI;
  heading = heading * 180 / PI;

  drawCompass(heading);
  delay(500); // aktualizacia kazdych 0.5s
}
