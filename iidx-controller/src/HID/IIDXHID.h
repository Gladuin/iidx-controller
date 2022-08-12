#ifndef _IIDXHID_H_
#define _IIDXHID_H_

    // Function prototypes
    #ifdef __cplusplus
    extern "C" {
    #endif
        unsigned long get_last_led_update();
        void setup_hardware(void);
        void HID_task(void);
    #ifdef __cplusplus
    }
    #endif

#endif
