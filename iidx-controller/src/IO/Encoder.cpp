#include <stdint.h>

#include <Arduino.h>

#include "../digitalWriteFast.h"
#include "../Configuration.h"

#include "Encoder.h"


#define AMOUNT 300

// Set to 133 for 0.5ms (2kHz)   on a 16MHz processor: (16000000L / 64 / 2000  -> 133)
// Set to 50  for 0.2ms (5kHz)   on a 16MHz processor: (16000000L / 64 / 5000  -> 50)
// Set to 25  for 0.1ms (10kHz)  on a 16MHz processor: (16000000L / 64 / 10000 -> 25)
// Set to 12  for 0.05ms (20kHz) on a 16MHz processor: (16000000L / 64 / 20833 -> 12)
#if ENCODER_PPR < 360
    #define INTERRUPT_PERIOD 25
#else // High PPR encoders have the possibility to switch faster than the timer
    #define INTERRUPT_PERIOD 12
#endif

#define LIMIT(min,val,max) if (val >= max) { val -= max; } else if (val < min) { val += max; }

volatile byte encoder_state_volatile;
volatile int16_t encoder_value_volatile = 0;    // Count from 0 to (ENCODER_PPR - 1)
volatile int16_t encoder_virtual_volatile = 0;  // Count from 0 to 255
volatile int16_t encoder_virtual_raw_volatile = 0;
volatile int16_t encoder_virtual_old_volatile = 0;

int digital_rotation = 0;

static configuration_struct *config;


void setup_timer_interrupt() {
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

void initialise_encoder() {
    get_configuration(&config);

    pinMode(encoder_pin0, INPUT_PULLUP);
    pinMode(encoder_pin1, INPUT_PULLUP);

    byte encoder_pin_a = !digitalReadFast(encoder_pin0);
    byte encoder_pin_b = !digitalReadFast(encoder_pin1);
    encoder_state_volatile = encoder_pin_b << 1 | encoder_pin_a;

    setup_timer_interrupt();
}

uint8_t get_digital_encoder_state() {
    // thank you CrazyRedMachine for the inspiration / half stolen code for this digital tt implementation

    if (digital_rotation > 0) {
        // going up
        digital_rotation--;
        return 1;
    } else if (digital_rotation < 0) {
        // going down
        digital_rotation++;
        return 2;
    }
}

uint16_t get_encoder_state() {
    LIMIT(0, encoder_value_volatile, ENCODER_PPR)

    return (uint16_t)((float)encoder_value_volatile * PPR_SCALE);
}

uint16_t get_encoder_virtual_state() {
    encoder_virtual_raw_volatile = (int16_t)((float)encoder_value_volatile * INCREMENT_SCALE);
    if (encoder_virtual_raw_volatile < encoder_virtual_old_volatile) {
        encoder_virtual_volatile--;
    } else if (encoder_virtual_raw_volatile > encoder_virtual_old_volatile) {
        encoder_virtual_volatile++;
    }
    LIMIT(0, encoder_value_volatile, ENCODER_PPR)
    encoder_virtual_old_volatile = encoder_virtual_raw_volatile;
    LIMIT(0, encoder_virtual_raw_volatile, 255)
    LIMIT(0, encoder_virtual_volatile, 255)
    return encoder_virtual_volatile;
}

void compute_encoder() {
    byte encoder_pin_a = !digitalReadFast(encoder_pin0);
    byte encoder_pin_b = !digitalReadFast(encoder_pin1);

    byte state = ((byte)encoder_pin_b << 1) | (byte)encoder_pin_a;

    byte encoder_state = encoder_state_volatile;

    if (encoder_state != state) {
        bool went_down = (!encoder_state && (state == 2));
        bool went_up = (!encoder_state && (state == 1));

        if (went_down) {
            digital_rotation = AMOUNT;
            encoder_value_volatile--;
        }

        if (went_up) {
            digital_rotation = -1 * AMOUNT;
            encoder_value_volatile++;
        }

        encoder_state_volatile = state;
    }
}

SIGNAL (TIMER3_COMPA_vect) {
    compute_encoder();
}