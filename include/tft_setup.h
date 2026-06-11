// TFT_eSPI configuration for ST7789 76x284 display on Seeed XIAO ESP32-C3
//
// Pin mapping (XIAO D-pin -> GPIO):
//   D0=2, D1=3, D2=4, D3=5, D4=6, D5=7, D6=21, D7=20, D8=8, D9=9, D10=10
//
// Calibrated CGRAM offsets (set at runtime via tft.colstart / tft.rowstart):
//   colstart = 82   rowstart = 18

#define ST7789_DRIVER
#define TFT_CS      20     // D7
#define TFT_DC      6      // D4
#define TFT_RST     5      // D3
#define TFT_MOSI    10     // D10
#define TFT_SCLK    8      // D8
#define TFT_BL      21     // D6
#define TFT_MISO    9      // D9 (unused, satisfies SPI)

#define TFT_BACKLIGHT_ON 0  // Active LOW
#define TFT_WIDTH    76
#define TFT_HEIGHT   284
#define SPI_FREQUENCY   27000000
#define SUPPORT_TRANSACTIONS

#define USER_SETUP_LOADED
