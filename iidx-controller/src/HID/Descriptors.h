#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

    // Macros
    // Endpoint address of the Generic HID reporting IN endpoint
    #define GENERIC_IN_EPADDR         (ENDPOINT_DIR_IN  | 1)
    
    // Endpoint address of the Generic HID reporting OUT endpoint
    #define GENERIC_OUT_EPADDR        (ENDPOINT_DIR_OUT | 2)
    
    // Size in bytes of the Generic HID reporting endpoint
    #define GENERIC_EPSIZE            8

#endif
