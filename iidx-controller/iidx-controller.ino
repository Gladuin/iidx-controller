#include "src/HID/LUFAConfig.h"
#include <LUFA.h>

#include "src/HID/IIDXHID.h"
#include "src/IO/Buttons.h"
#include "src/IO/Encoder.h"
#include "src/IO/LEDs.h"
#include "src/Configuration.h"


static configuration_struct *config;


void setup() {
    initialise_configuration();
    initialise_buttons();
    initialise_encoder();
    initialise_leds();

    get_configuration(&config);

    delay(500);
    setup_hardware();
    GlobalInterruptEnable();
}

void loop() {
    if (config->led_mode == 0 || config->led_mode == 2) {
        if ((millis() - get_last_led_update()) > 3000 || config->led_mode == 2) {
            write_leds(get_button_state(), false);
        }
    }

    HID_task();
    USB_USBTask();
}
