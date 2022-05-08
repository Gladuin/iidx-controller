#include <stdint.h>

#include <Arduino.h>

#include "../../config.h"

#include "LEDs.h"

void initialise_leds() {
    for (int i = 0; i < sizeof(led_pins); i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }
    
    delay(200);
    
    for (int i = 0; i < sizeof(led_pins); i++) {
        digitalWrite(led_pins[i], LOW);
    }
}

void write_leds(uint16_t led_status) {
    for (int i = 0; i < sizeof(led_pins); i++) {
        digitalWrite(led_pins[i], ((led_status >> i) & 1));
    }
}