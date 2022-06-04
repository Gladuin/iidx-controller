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
#endif
