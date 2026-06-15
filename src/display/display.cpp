#include "display.h"

static uint16_t _colOff = 82;
static uint16_t _rowOff = 18;

Display::Display() : Adafruit_GFX(76, 284) {}

void Display::cmd(uint8_t c) {
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(c);
  digitalWrite(PIN_CS, HIGH);
}

void Display::data8(uint8_t d) {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(d);
  digitalWrite(PIN_CS, HIGH);
}

void Display::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  cmd(0x2A);
  data8((_colOff + x0) >> 8); data8((_colOff + x0) & 0xFF);
  data8((_colOff + x1) >> 8); data8((_colOff + x1) & 0xFF);
  cmd(0x2B);
  data8((_rowOff + y0) >> 8); data8((_rowOff + y0) & 0xFF);
  data8((_rowOff + y1) >> 8); data8((_rowOff + y1) & 0xFF);
}

void Display::writeColor(uint16_t c, uint32_t count) {
  uint8_t hi = c >> 8;
  uint8_t lo = c & 0xFF;
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  for (uint32_t i = 0; i < count; i++) {
    SPI.transfer(hi);
    SPI.transfer(lo);
  }
  digitalWrite(PIN_CS, HIGH);
}

void Display::begin() {
  pinMode(PIN_BL, OUTPUT);   digitalWrite(PIN_BL, LOW);
  pinMode(PIN_CS, OUTPUT);   digitalWrite(PIN_CS, HIGH);
  pinMode(PIN_DC, OUTPUT);   digitalWrite(PIN_DC, HIGH);
  pinMode(PIN_RST, OUTPUT);

  digitalWrite(PIN_RST, LOW);   delay(10);
  digitalWrite(PIN_RST, HIGH);  delay(100);

  SPI.begin(PIN_SCLK, PIN_MISO, PIN_MOSI, -1);
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  cmd(0x01); delay(150);
  cmd(0x11); delay(150);
  cmd(0x3A); data8(0x55);
  cmd(0xB2); data8(0x0C); data8(0x0C); data8(0x00); data8(0x33); data8(0x33);
  cmd(0xB7); data8(0x35);
  cmd(0xBB); data8(0x19);
  cmd(0xC0); data8(0x2C);
  cmd(0xC2); data8(0x01);
  cmd(0xC3); data8(0x12);
  cmd(0xC4); data8(0x20);
  cmd(0xC6); data8(0x0F);
  cmd(0xD0); data8(0xA4); data8(0xA1);
  cmd(0xE0); data8(0xD0); data8(0x04); data8(0x0D); data8(0x11); data8(0x13); data8(0x2B); data8(0x3F); data8(0x54); data8(0x4C); data8(0x18); data8(0x0D); data8(0x0B); data8(0x1F); data8(0x23);
  cmd(0xE1); data8(0xD0); data8(0x04); data8(0x0C); data8(0x11); data8(0x13); data8(0x2C); data8(0x3F); data8(0x44); data8(0x51); data8(0x2F); data8(0x1F); data8(0x1F); data8(0x20); data8(0x23);
  cmd(0x20);
  cmd(0x13); delay(10);
  cmd(0x29); delay(150);

  SPI.endTransaction();

  setRotation(0);
}

void Display::setRotation(uint8_t r) {
  rotation = r % 4;
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  switch (rotation) {
    case 0:
      cmd(0x36); data8(0x00);
      _colOff = 82; _rowOff = 18;
      _width = 76;  _height = 284;
      break;
    case 1:
      cmd(0x36); data8(0x60);  // MX | MV
      _colOff = 18; _rowOff = 82;
      _width = 284; _height = 76;
      break;
    case 2:
      cmd(0x36); data8(0xC0);  // MX | MY
      _colOff = 0;  _rowOff = 80;
      _width = 76;  _height = 284;
      break;
    case 3:
      cmd(0x36); data8(0xA0);  // MY | MV
      _colOff = 0;  _rowOff = 0;
      _width = 284; _height = 76;
      break;
  }
  SPI.endTransaction();
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || x >= _width || y < 0 || y >= _height) return;
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  setAddrWindow(x, y, x, y);
  cmd(0x2C);
  writeColor(color, 1);
  SPI.endTransaction();
}

void Display::fillScreen(uint16_t color) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  setAddrWindow(0, 0, _width - 1, _height - 1);
  cmd(0x2C);
  writeColor(color, _width * _height);
  SPI.endTransaction();
}

void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > _width)  w = _width - x;
  if (y + h > _height) h = _height - y;
  if (w <= 0 || h <= 0) return;
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  cmd(0x2C);
  writeColor(color, w * h);
  SPI.endTransaction();
}
