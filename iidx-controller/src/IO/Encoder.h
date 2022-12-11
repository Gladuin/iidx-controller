#ifndef _ENCODER_H_
#define _ENCODER_H_

    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // Function prototypes
    EXTERNC void initialise_encoder();
    EXTERNC uint8_t get_digital_encoder_state();
    EXTERNC uint16_t get_encoder_state();
    EXTERNC uint16_t get_encoder_virtual_state();

    #undef EXTERNC

#endif