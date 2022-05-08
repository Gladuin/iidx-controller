#ifndef _CONFIG_H_
#define _CONFIG_H_

/* PINOUT */
// Pins where the LEDs are connected to
static const uint8_t led_pins[] = {
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
static const uint8_t button_pins[] = {
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

// Pins where the encoder is connected to
static const uint8_t encoder_pin0 = 0;  // green wire (a phase)
static const uint8_t encoder_pin1 = 1;  // white wire (b phase)

/* OPTIONS */
// Your encoder pulses per rotation
#define ENCODER_PPR 600

// The manufacturer name of this controller (leave the L in front of the ")
#define MF_NAME L"username"

// Set the ability to spoof the beatmania IIDX controller premium model (enable this if you plan on playing infinitas)
#define KONAMI_SPOOF 0

// Number of increments before a full turn is reached
// Set to 72 for 60Hz IIDX, 144 for 120Hz IIDX, 255 for 1 to 1
#define INCREMENTS_PER_FULL_TURN 72

// Set the ability to set sensitivity via spice
#define NO_SENSITIVITY 1

/* OPTIONS YOU PROBABLY DON'T HAVE TO TOUCH */
// Adjusted PPR macro for use with other stuff
#define ADJUSTED_PPR ((int)((float)ENCODER_PPR * ((float)255 / (float)INCREMENTS_PER_FULL_TURN)))

// Button debouncing time, make this higher if you experience bouncing in your buttons
#define MS_DEBOUNCE 5

// Period of time between polling events on the endpoints (in miliseconds)
#define POLLING_INTERVAL 1

// This is (cpu_clock_frequency / prescaler / desired_interrupt_frequency)
// for example, set to 133 for 0.5ms (2kHz)  on a 16MHz processor: (16000000L / 64 / 2000  -> 133)
// for example, set to 50  for 0.2ms (5kHz)  on a 16MHz processor: (16000000L / 64 / 5000  -> 50)
// for example, set to 25  for 0.1ms (10kHz) on a 16MHz processor: (16000000L / 64 / 10000 -> 25)
#define INTERRUPT_PERIOD 25

#endif
