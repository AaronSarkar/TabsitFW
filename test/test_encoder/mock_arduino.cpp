#include "Arduino.h"

int mock_pin_values[MAX_PINS] = {};
int mock_pin_modes[MAX_PINS] = {};
MockSerial Serial;

void pinMode(int pin, int mode) {
    if (pin >= 0 && pin < MAX_PINS)
        mock_pin_modes[pin] = mode;
}

int digitalRead(int pin) {
    if (pin >= 0 && pin < MAX_PINS)
        return mock_pin_values[pin];
    return 0;
}

void digitalWrite(int pin, int value) {
    if (pin >= 0 && pin < MAX_PINS)
        mock_pin_values[pin] = value;
}

void mock_set_pin(int pin, int value) {
    if (pin >= 0 && pin < MAX_PINS)
        mock_pin_values[pin] = value;
}

void mock_reset_pins() {
    for (int i = 0; i < MAX_PINS; i++) {
        mock_pin_values[i] = 0;
        mock_pin_modes[i] = 0;
    }
}
