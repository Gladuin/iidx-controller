#include <stdint.h>

#include <Arduino.h>

#include "../Configuration.h"

#include "LEDs.h"

void initialise_leds() {
    for (int i = 0; i < NUM_LEDS; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }

    delay(200);

    for (int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(led_pins[i], LOW);
    }
}

void write_leds(uint16_t led_status, bool flipped) {
    if (flipped) {
        led_status = led_status << (16 - NUM_LEDS);

        // thanks http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
        // flip first half of led_status
        uint8_t a = led_status >> 8;
        a = ((a * 0x0802LU & 0x22110LU) | (a * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;

        // flip second half of led_status
        uint8_t b = led_status & 0x00FF;
        b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;

        // combine part a and b back into a 16-bit number, but flipped, to complete the reversal of led_status
        led_status = (b << 8) | a;
    }

    for (int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(led_pins[i], ((led_status >> i) & 1));
    }
}