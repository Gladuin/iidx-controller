#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

    // Includes
    #include "../config.h"

    // Structs
    typedef struct {
        uint8_t controller_mode;            // 0 = joystick, 1 = keyboard
        uint8_t led_mode;                   // 0 = reactive + hid, 1 = hid, 2 = reactive, 3 = off
        uint8_t tt_mode;                    // 0 = analog joystick, 1 = analog mouse, 2 = digital joystick, 3 = digital button, 4 = digital keyboard
        uint8_t increments_per_full_turn;   // 72 for 60Hz IIDX, 144 for 120Hz IIDX, 255 for 1 to 1
        uint8_t debounce_time;              // 1 - 255, in ms
        uint8_t polling_rate;               // 1 - 255, in ms
    } configuration_struct;

    // Macros
    #define PPR_SCALE (255.0f / (float)ENCODER_PPR)
    #define INCREMENT_SCALE ((float)config->increments_per_full_turn / (float)ENCODER_PPR)
    #define NUM_BUTTONS (sizeof(button_pins) / sizeof(uint8_t))
    #define NUM_LEDS (sizeof(led_pins) / sizeof(uint8_t))

    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // Function prototypes
    EXTERNC void initialise_configuration();
    EXTERNC void get_configuration(configuration_struct**);
    EXTERNC void new_configuration(uint16_t);

    #undef EXTERNC

#endif