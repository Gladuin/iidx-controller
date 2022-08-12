#include <stdint.h>

#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#include "Buttons.h"
#include "../Configuration.h"


Bounce buttons[sizeof(button_pins)];
uint16_t button_status;

static configuration_struct *config;


void set_debounce_interval() {
    for (int i = 0; i < sizeof(button_pins); i++) {
        buttons[i].interval(config->debounce_time);
    }
}

void initialise_buttons() {
    get_configuration(&config);

    for (int i = 0; i < sizeof(button_pins); i++) {
        buttons[i] = Bounce();
        buttons[i].attach(button_pins[i], INPUT_PULLUP);
    }

    set_debounce_interval();
}

uint16_t get_button_state() {
    for (int i = 0; i < sizeof(button_pins); i++) {
        buttons[i].update();

        switch (buttons[i].read()) {
            case LOW:
                button_status |= (uint16_t)1 << i;
                break;

            case HIGH:
                button_status &= ~((uint16_t)1 << i);
                break;
        }
    }

    return button_status;
}