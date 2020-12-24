#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#include "IIDXHID.h"

#define REPORT_DELAY 1000
#define MS_DEBOUNCE 5

IIDXHID_ IIDXHID;

// Pins where the LEDs are connected to
uint8_t led_pins[NUMBER_OF_LEDS] = {
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

// Pins where the buttons are connected to
uint8_t button_pins[NUMBER_OF_BUTTONS] = {
    3,    // button 1
    5,    // button 2
    7,    // button 3
    9,    // button 4
    11,   // button 5
    13,   // button 6
    19,   // button 7
    21,   // misc button 1
    23,   // misc button 2
    15,   // misc button 3
    17    // misc button 4
};

// Pins encoder is connected to
uint8_t encoder_pins[2] = {
    0,    // green wire (a phase)
    1     // white wire (b phase)
};

Bounce buttons[NUMBER_OF_BUTTONS];

uint32_t last_report = 0;

int32_t tt_pos;
uint8_t encoder_curstate;
uint8_t encoder_laststate;

uint8_t tt_sensitivity = 9;
uint8_t tt_lookup[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

bool hid_lights = true;
bool reactive;

void setup() {
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        buttons[i] = Bounce();
        buttons[i].attach(button_pins[i], INPUT_PULLUP);
        buttons[i].interval(MS_DEBOUNCE);
    }

    for (int i = 0; i < sizeof(encoder_pins); i++) {
        pinMode(encoder_pins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(encoder_pins[i]), update_encoder, CHANGE);
    }

    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }

    delay(200);

    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        digitalWrite(led_pins[i], LOW);
    }

    if (digitalRead(button_pins[0]) == LOW) {
        reactive = true;
    }

    if (digitalRead(button_pins[1]) == LOW) {
        hid_lights = false;
    }

    encoder_laststate = digitalRead(encoder_pins[0]);
}

void loop() {
    uint32_t buttons_state = 0;

    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        buttons[i].update();
        int button_value = buttons[i].read();

        // Put button states into the buttons_state variable via bitwise operations
        if (button_value == LOW) {
            buttons_state |= (uint32_t)1 << i;
        } else {
            buttons_state &= ~((uint32_t)1 << i);
        }

        if (reactive) {
            digitalWrite(led_pins[i], !(button_value));
        }
    }

    // Limit the encoder from 0 to ENCODER_PPR
    if (tt_pos >= ENCODER_PPR) {
        tt_pos = 1;
    } else if (tt_pos <= 0) {
        tt_pos = ENCODER_PPR - 1;
    }

    // Send turntable and button state every 1000 microseconds
    if (((micros() - last_report) >= REPORT_DELAY)) {
        IIDXHID.send_state(buttons_state, tt_pos);
        last_report = micros();
    }
}

void update_encoder() {
    encoder_curstate = digitalRead(encoder_pins[0]);

    if (encoder_curstate != encoder_laststate && encoder_curstate == 1) {
        if (digitalRead(encoder_pins[1]) != encoder_curstate) {
            tt_pos += tt_lookup[tt_sensitivity];
        } else {
            tt_pos -= tt_lookup[tt_sensitivity];
        }
    }

    encoder_laststate = encoder_curstate;
}
