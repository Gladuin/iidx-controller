#ifndef _COMMANDS_H_
#define _COMMANDS_H_

    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // Function prototypes
    EXTERNC void process_command(uint16_t);

    #undef EXTERNC

#endif