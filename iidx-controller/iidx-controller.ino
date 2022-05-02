#include "src/HID/LUFAConfig.h"
#include <LUFA.h>

#include "src/HID/IIDXHID.h"
#include "src/IO/Buttons.h"

#include "config.h"

void setup() {
    initialise_buttons();

    // Start LUFA USB stuff
    delay(500);
    setup_hardware();
    GlobalInterruptEnable();
}

void loop() {
    HID_task();
    USB_USBTask();
}
