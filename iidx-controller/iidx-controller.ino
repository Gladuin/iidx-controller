#include "src/HID/LUFAConfig.h"
#include <LUFA.h>

#include "src/HID/IIDXHID.h"
#include "src/IO/Buttons.h"
#include "src/IO/Encoder.h"
#include "src/IO/LEDs.h"

#include "config.h"

void setup() {
    initialise_buttons();
    initialise_encoder();
    initialise_leds();

    // Start LUFA USB stuff
    delay(500);
    setup_hardware();
    GlobalInterruptEnable();
}

void loop() {
    if ((millis() - get_last_led_update()) > 3000) {
        write_leds(get_button_state(), false);
    }

    HID_task();
    USB_USBTask();
}
