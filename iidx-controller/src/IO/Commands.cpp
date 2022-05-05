#include <stdint.h>
#include <avr/wdt.h>

#include "Commands.h"

void process_command(uint16_t data) {
    if ((data & 0xFF) == 0xFF) {
        wdt_enable(WDTO_15MS);
        while (true) {}
    }
}