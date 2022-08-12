#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

    // Macros
    // Endpoint address of the Generic HID reporting OUT endpoint
    #define JOYSTICK_IN_EPADDR        (ENDPOINT_DIR_IN | 1)
    #define JOYSTICK_OUT_EPADDR       (ENDPOINT_DIR_OUT | 2)

    #define KEYBOARD_IN_EPADDR        (ENDPOINT_DIR_IN | 3)

    #define MOUSE_IN_EPADDR           (ENDPOINT_DIR_IN | 4)

    // Size in bytes of the Generic HID reporting endpoint
    #define HID_EPSIZE                8

    enum interface_descriptors_enum {
        INTERFACE_ID_Joystick   = 0,
        INTERFACE_ID_Keyboard   = 1,
        INTERFACE_ID_Mouse      = 2
    };

#endif