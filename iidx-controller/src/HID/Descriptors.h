#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

    // Macros
    // Endpoint address of the Generic HID reporting OUT endpoint
    #define GENERIC_OUT_EPADDR        (ENDPOINT_DIR_OUT | 1)

    #define JOYSTICK_IN_EPADDR        (ENDPOINT_DIR_IN | 2)
    
    #define KEYBOARD_IN_EPADDR        (ENDPOINT_DIR_IN | 3)
    
    #define MOUSE_IN_EPADDR           (ENDPOINT_DIR_IN | 4) 
    
    // Size in bytes of the Generic HID reporting endpoint
    #define HID_EPSIZE                8

    enum interface_descriptors_enum {
        INTERFACE_ID_GenericHID = 0,
        INTERFACE_ID_Joystick   = 1,
        INTERFACE_ID_Keyboard   = 2,
        INTERFACE_ID_Mouse      = 3
    };

#endif