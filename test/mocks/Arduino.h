#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <cstdint>

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define LOW 0
#define HIGH 1

#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define D8 8
#define D9 9
#define D10 10

static const int MAX_PINS = 16;

extern int mock_pin_values[MAX_PINS];
extern int mock_pin_modes[MAX_PINS];

void pinMode(int pin, int mode);
int digitalRead(int pin);
void digitalWrite(int pin, int value);
void mock_set_pin(int pin, int value);
void mock_reset_pins();

struct MockSerial {
    void begin(int) {}
    void print(int) {}
    void print(char) {}
    void println(int) {}
};

extern MockSerial Serial;

#endif
