#ifndef _LEDS_H_
#define _LEDS_H_

    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif
    
    // Function prototypes
    EXTERNC void initialise_leds();
    EXTERNC void write_leds(uint16_t);
    
    #undef EXTERNC

#endif