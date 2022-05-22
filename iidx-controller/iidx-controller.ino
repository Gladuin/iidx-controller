#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#include "IIDXHID.h"
#include "config.h"

IIDXHID_ IIDXHID;

Bounce buttons[NUM_BUTTONS];

int tt_delta;
int32_t tt_pos;
uint8_t tt_sensitivity = 9;
uint8_t tt_lookup[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

const int encoder_cooldown_const = 100;
int encoder_cooldown = 0;

int button_state_array[NUM_BUTTONS];

bool hid_reactive_autoswitch = true;
bool hid_lights = true;
bool reactive = false;

volatile int g_encoderValue = 0;
volatile unsigned int g_encoderValueVL = 0;
volatile byte g_encoderStVL;

uint32_t last_report = 0;

void initEncoder() {
    pinMode(encoder_pin0, INPUT_PULLUP);
    pinMode(encoder_pin1, INPUT_PULLUP);

    computeEncoder();

    setupTimerInterrupt();
}

void computeEncoder() {
    // Reference: https://github.com/PaulStoffregen/Encoder
    uint8_t s = g_encoderStVL & 3;
    if (IO_READ(encoder_pin0)) s |= 4;
    if (IO_READ(encoder_pin1)) s |= 8;
    switch (s) {
        case 0: case 5: case 10: case 15:
            break;
        case 1: case 7: case 8: case 14:
            g_encoderValueVL += 1;
            break;
        case 2: case 4: case 11: case 13:
            g_encoderValueVL -= 1;
            break;
        case 3: case 12:
            g_encoderValueVL += 2;
            break;
        default:
            g_encoderValueVL -= 2;
            break;
    }
    g_encoderStVL = (s >> 2);
}

int encoder_delta() {
    unsigned int v = g_encoderValueVL;  // put in a local var to avoid multiple reads of a volatile var
    int d = v - g_encoderValue;
    g_encoderValue = v;

    return d;
}

void setup() {
    // Buttons setup
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i] = Bounce();
        buttons[i].attach(button_pins[i], INPUT_PULLUP);
        buttons[i].interval(MS_DEBOUNCE);
    }

    // Encoder setup
    initEncoder();

    // LED startup animation and setup
    for (int i = 0; i < NUM_LEDS; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }

    delay(200);

    for (int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(led_pins[i], LOW);
    }
}

void loop() {
    uint32_t buttons_state = 0;

    // Mixed mode will behave sometimes like HID, sometimes like reactive
    if (hid_reactive_autoswitch){
        if ((millis() - IIDXHID.getLastHidUpdate()) > 3000) {
            reactive = true;
            hid_lights = false;
        } else {
            reactive = false;
            hid_lights = true;
        }
    }

    // Write button status into array
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].update();
        button_state_array[i] = buttons[i].read();
    }

    // Read from array and convert to bitfield
    for (int i = 0; i < NUM_BUTTONS; i++) {
        int button_value = button_state_array[i];

        // Put button states into the buttons_state variable via bitwise operations
        if (button_value == LOW) {
            buttons_state |= (uint32_t)1 << i;
        } else {
            buttons_state &= ~((uint32_t)1 << i);
        }
    }

    IIDXHID.write_lights(buttons_state, hid_lights, reactive);

    // Encoder update
    if (encoder_cooldown == 0) {
        tt_delta = encoder_delta();
        encoder_cooldown = encoder_cooldown_const;
    } else {
        encoder_cooldown--;
        tt_delta = 0;
    }

    if (tt_delta >= ADJUSTED_PPR / 360 || tt_delta <= -ADJUSTED_PPR / 360) {
        tt_pos += tt_delta * tt_lookup[tt_sensitivity];
    }

    // Limit the encoder from 0 to ADJUSTED_PPR
    if (tt_pos >= ADJUSTED_PPR) {
        tt_pos = 0;
    } else if (tt_pos < 0) {
        tt_pos = ADJUSTED_PPR-1;
    }

    // Send turntable and button state every REPORT_DELAY microseconds
    if (((micros() - last_report) >= REPORT_DELAY)) {
        IIDXHID.send_state(buttons_state, tt_pos);
        last_report = micros();
    }

    // Manual lightmode update
    static bool modeChanged = false;
    if (buttons_state & ((uint32_t)1 << (sizeof(button_pins) - 1))) {
        if ((buttons_state & 1) && (modeChanged == false)) {
            modeChanged = true;
            if (hid_reactive_autoswitch) {
                hid_reactive_autoswitch = false;
                hid_lights = false;
                reactive = true;  // 1 x x -> 0 0 1
            } else if (reactive && hid_lights) {
                hid_reactive_autoswitch = true;  // 0 1 1 -> 1 x x
            } else if (reactive) {
                reactive = false;
                hid_lights = true; // 0 0 1 -> 0 1 0
            } else {
                reactive = true; // 0 1 0 -> 0 1 1
            }
        } else if (!(buttons_state&1)) {
            modeChanged = false;
        }
    }
}

void setupTimerInterrupt() {
    TCCR3A = 0; // Set entire TCCR3A register to 0
    TCCR3B = 0; // Same for TCCR3B
    TCNT3  = 0; // Initialize counter value to 0

    // Set compare match register (write to the high bit first)
    OCR3AH = 0;

    // Set compare match register for particular frequency increments
    OCR3AL = INTERRUPT_PERIOD;

    // Enable timer compare interrupt
    TIMSK3 = (1 << OCIE3A);

    // Turn on mode 4 (CTC mode) (up to OCR3A)
    TCCR3B |= (1 << WGM32);

    // Set CS10 and CS12 bits for 64 prescaler
    TCCR3B |= (1 << CS30) | (1 << CS31);

    /* More information at
       http://medesign.seas.upenn.edu/index.php/Guides/MaEvArM-timers */
}

SIGNAL (TIMER3_COMPA_vect) {
    computeEncoder();
}
