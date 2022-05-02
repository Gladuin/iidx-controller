#include <stdint.h>

#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#include "Buttons.h"
#include "../../config.h"

Bounce buttons[sizeof(button_pins)];
int16_t button_status;

void initialise_buttons() {
    for (int i = 0; i < sizeof(button_pins); i++) {
        buttons[i] = Bounce();
        buttons[i].attach(button_pins[i], INPUT_PULLUP);
        buttons[i].interval(MS_DEBOUNCE);
    }
}

int16_t get_button_state() {
    for (int i = 0; i < sizeof(button_pins); i++) {
        buttons[i].update();

        switch (buttons[i].read()) {
            case LOW:
                button_status |= (int16_t)1 << i;
                break;

            case HIGH:
                button_status &= ~((int16_t)1 << i);
                break;
        }
    }

    return button_status;
}