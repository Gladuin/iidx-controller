#include <stdint.h>
#include <avr/wdt.h>

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
    if ((data & 0xFF) == 0xFF) {
        reboot();
    }
}