#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <SPI.h>

// Pin mapping (D-pin -> GPIO on XIAO ESP32-C3):
//   D3=5, D4=6, D6=21, D7=20, D8=8, D9=9, D10=10
#define PIN_CS   20  // D7
#define PIN_DC   6   // D4
#define PIN_RST  5   // D3
#define PIN_MOSI 10  // D10
#define PIN_SCLK 8   // D8
#define PIN_BL   21  // D6 (active LOW)
#define PIN_MISO 9   // D9 (unused)

// Common 16-bit RGB565 colors
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_PURPLE  0xF81F

class Display : public Adafruit_GFX {
public:
  Display();

  void begin();
  void setRotation(uint8_t r);
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void fillScreen(uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

private:
  void cmd(uint8_t c);
  void data8(uint8_t d);
  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void writeColor(uint16_t c, uint32_t count);
};

#endif
