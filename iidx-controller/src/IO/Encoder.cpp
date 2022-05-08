#include <stdint.h>

#include <Arduino.h>

#include "../digitalWriteFast.h"
#include "../../config.h"

#include "Encoder.h"

volatile byte encoder_state_volatile;
volatile int16_t encoder_value_volatile = 0;

void setup_timer_interrupt() {
    TCCR3A = 0; // Set entire TCCR3A register to 0
    TCCR3B = 0; // Same for TCCR3B
    TCNT3  = 0; // Initialize counter value to 0

    // Set compare match register (write to the high bit first)
    OCR3AH = 0;

    // Set compare match register for particular frequency increments
    //  OCR3AL = 133; // = (16000000) / 64 / 2000  -> 133   This is  clock_frequency / prescaler / desired_frequency  ( 2 KHz, 0.5ms)
    //  OCR3AL = 50;  // = (16000000) / 64 / 5000  ->  50   This is  clock_frequency / prescaler / desired_frequency  ( 5 KHz, 0.2ms)
    //  OCR3AL = 25;  // = (16000000) / 64 / 10000 ->  25   This is  clock_frequency / prescaler / desired_frequency  (10 kHz, 0.1ms)
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
    pinMode(encoder_pin0, INPUT_PULLUP);
    pinMode(encoder_pin1, INPUT_PULLUP);

    byte encoder_pin_a = !digitalReadFast(encoder_pin0);
    byte encoder_pin_b = !digitalReadFast(encoder_pin1);
    encoder_state_volatile = encoder_pin_b << 1 | encoder_pin_a;

    setup_timer_interrupt();
}

uint16_t get_encoder_state() {
   
    if (encoder_value_volatile >= ADJUSTED_PPR) {
        encoder_value_volatile = 0;
    } else if (encoder_value_volatile < 0) {
        encoder_value_volatile = ADJUSTED_PPR - 1;
    }

    return encoder_value_volatile;
}

void compute_encoder() {
    byte encoder_pin_a = !digitalReadFast(encoder_pin0);
    byte encoder_pin_b = !digitalReadFast(encoder_pin1);

    byte state = ((byte)encoder_pin_b << 1) | (byte)encoder_pin_a;

    byte encoder_state = encoder_state_volatile;

    if (encoder_state != state) {
        bool went_down = (!encoder_state && (state == 2));
        bool went_up = (!encoder_state && (state == 1));

        if (went_down) encoder_value_volatile--;
        if (went_up) encoder_value_volatile++;

        encoder_state_volatile = state;
    }
}

SIGNAL (TIMER3_COMPA_vect) {
    compute_encoder();
}