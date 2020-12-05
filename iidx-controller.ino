#include "IIDXHID.h"
IIDXHID_ IIDXHID;

// Pins LEDs are connected to.
uint8_t leds[NUMBER_OF_LEDS] = {
    2,    // button 1 led
    4,    // button 2 led
    6,    // button 3 led
    8,    // button 4 led
    10,   // button 5 led
    12,   // button 6 led
    18,   // button 7 led
    20,   // misc button 1 led
    22,   // misc button 2 led
    14,   // misc button 3 led
    16    // misc button 4 led
};

void setup() {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        pinMode(leds[i], OUTPUT);
        digitalWrite(leds[i], HIGH);
    }

    delay(200);

    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        digitalWrite(leds[i], LOW);
    }
}

void loop() {
}
