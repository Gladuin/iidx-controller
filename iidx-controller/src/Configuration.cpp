#include <stdint.h>

#include <Arduino.h>
#include <EEPROM.h>

#include "HID/LUFAConfig.h"

#include <LUFA/LUFA/Drivers/USB/USB.h>

#include "IO/Buttons.h"
#include "Configuration.h"


static configuration_struct config;
configuration_struct new_config;

bool temporary_config = true;


void initialise_configuration() {
    if (EEPROM.read(0) == 0xCB) {
        EEPROM.get(1, config);
    } else {
        config = {
            .controller_mode = 0,
            .led_mode = 0,
            .tt_mode = 0,
            .increments_per_full_turn = 72,
            .debounce_time = 5,
            .polling_rate = 1
        };
    }
}

void get_configuration(configuration_struct** config_ptr) {
    *config_ptr = &config;
}

void new_configuration(uint16_t data) {
    switch (data & 0xFF) {
        case 0x01:
            switch (data >> 8) {
                case 0x00:
                    temporary_config = false;
                    break;
                case 0x01:
                    temporary_config = true;
                    break;
                case 0xFF:
                    if (temporary_config == false) {
                        EEPROM.update(0, 0xCB);
                        EEPROM.put(1, new_config);
                    }

                    USB_Disable();

                    config = new_config;
                    delay(2000);

                    set_debounce_interval();
                    USB_Init();
                    break;
            }
            break;
        case 0x10:
            new_config.controller_mode = (data >> 8);
            break;
        case 0x20:
            new_config.led_mode = (data >> 8);
            break;
        case 0x30:
            new_config.tt_mode = (data >> 8);
            break;
        case 0x40:
            new_config.increments_per_full_turn = (data >> 8);
            break;
        case 0x50:
            new_config.debounce_time = (data >> 8);
            break;
        case 0x60:
            new_config.polling_rate = (data >> 8);
            break;
    }
}