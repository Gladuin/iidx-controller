#ifndef _IIDXHID_H_
#define _IIDXHID_H_

    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // Function prototypes
    EXTERNC unsigned long get_last_led_update();
    EXTERNC void setup_hardware(void);
    EXTERNC void HID_task(void);

#endif
