#ifndef _BUTTONS_H_
#define _BUTTONS_H_

    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // Function prototypes
    EXTERNC void initialise_buttons();
    EXTERNC uint16_t get_button_state();
    EXTERNC void set_debounce_interval();

    #undef EXTERNC

#endif