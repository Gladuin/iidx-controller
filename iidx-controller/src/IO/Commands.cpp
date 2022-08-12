#include <stdint.h>
#include <avr/wdt.h>

#include "../Configuration.h"

#include "Commands.h"

void reboot() {
    // https://blog.fsck.com/2014/08/how-to-reboot-an-arduino-leonardo-micro-into-the-bootloader.html
    uint16_t boot_key = 0x7777;
    uint16_t *const boot_key_ptr = (uint16_t *)0x0800;

    *boot_key_ptr = boot_key;

    wdt_enable(WDTO_15MS);
    while (true) {}
}

void process_command(uint16_t data) {
    switch (data & 0xFF) {
        case 0xFF:
            reboot();
            break;
        case 0x01:
        case 0x10:
        case 0x20:
        case 0x30:
        case 0x40:
        case 0x50:
        case 0x60:
            new_configuration(data);
            break;
    }
}