# esp32-Compass
TFT Display code for COMPASS
<div align="center" height: 200px; width: 50px;>
  <img src="https://komarev.com/ghpvc/?username=Fattcat&style=flat-square" alt="Visitor Badge">
</div>
# Connection
## GPS neo6m - esp32
- VCC -> 3.3V
- GND -> GND
- TX -> RXD
- RX -> TXD

## HMC5883L - esp32
- VCC - 3.3V
- GNC - GND
- SDA	to GPIO 21
- SCL to GPIO 22 

## TFT st7789 -> esp32
- VCC -> 3.3V
- GND -> GND
- TFT_CS  15
- RST -> 4
- DC -> 2
- MOSI -> 23
- MiSO -> 19
- SCK -> 18
- LED -> 3.3V
