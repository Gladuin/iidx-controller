#include "IIDXHID.h"

#define BUTTON_PADDING (8 - (NUMBER_OF_BUTTONS % 8))

#define ENCODERMAX1 ENCODER_PPR & 0x00FF
#define ENCODERMAX2 ENCODER_PPR >> 8

uint8_t led_value[NUMBER_OF_LEDS];
uint8_t extern led_pins[];
bool extern hid_lights;

static const uint8_t PROGMEM hid_report[] = {
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                      // USAGE (Game Pad)
    0xa1, 0x01,                      // COLLECTION (Application)
    0x05, 0x0a,                      //   USAGE_PAGE (Ordinals)
    0x19, 0x01,                      //   USAGE_MINIMUM (Instance 1)
    0x29, NUMBER_OF_LEDS,            //   USAGE_MAXIMUM (Instance NUMBER_OF_LEDS)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                      //   REPORT_SIZE (8)
    0x95, NUMBER_OF_LEDS,            //   REPORT_COUNT (NUMBER_OF_LEDS)
    0x91, 0x02,                      //   OUTPUT (Data,Var,Abs)
    0x05, 0x09,                      //   USAGE_PAGE (Button)
    0x19, 0x01,                      //   USAGE_MINIMUM (Button 1)
    0x29, NUMBER_OF_BUTTONS,         //   USAGE_MAXIMUM (Button 11)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                      //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, NUMBER_OF_BUTTONS,         //   REPORT_COUNT (11)
    0x55, 0x00,                      //   UNIT_EXPONENT (0)
    0x65, 0x00,                      //   UNIT (None)
    0x81, 0x02,                      //   INPUT (Data,Var,Abs)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, BUTTON_PADDING,            //   REPORT_COUNT (5)
    0x81, 0x03,                      //   INPUT (Cnst,Var,Abs)
    0x05, 0x01,                      //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                      //   USAGE (Pointer)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x26, ENCODERMAX1, ENCODERMAX2,  //   LOGICAL_MAXIMUM (ENCODER_PPR)
    0x75, 0x10,                      //   REPORT_SIZE (16)
    0x95, 0x01,                      //   REPORT_COUNT (1)
    0xa1, 0x00,                      //   COLLECTION (Physical)
    0x09, 0x30,                      //     USAGE (X)
    0x81, 0x02,                      //     INPUT (Data,Var,Abs)
    0xc0,                            //   END_COLLECTION
    0xc0                             // END_COLLECTION
};

IIDXHID_::IIDXHID_(void) : PluggableUSBModule(1, 1, epType) {
    epType[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);
}

int IIDXHID_::getInterface(byte* interface_count) {
    *interface_count += 1;
    HIDDescriptor hid_interface = {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(sizeof(hid_report)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hid_interface, sizeof(hid_interface));
}

int IIDXHID_::getDescriptor(USBSetup& setup) {
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
        return 0;
    }

    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
        return 0;
    }

    if (setup.wIndex != pluggedInterface) {
        return 0;
    }

    return USB_SendControl(TRANSFER_PGM, hid_report, sizeof(hid_report));
}

bool IIDXHID_::setup(USBSetup& setup) {
    if (pluggedInterface != setup.wIndex) {
        return false;
    }

    uint8_t request = setup.bRequest;
    uint8_t request_type = setup.bmRequestType;

    if (request_type == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
        return true;
    }

    if (request_type == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (request == HID_SET_REPORT) {
            if (setup.wValueH == HID_REPORT_TYPE_OUTPUT && setup.wLength == NUMBER_OF_LEDS) {
                USB_RecvControl(led_value, NUMBER_OF_LEDS);

                if (hid_lights) {
                    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
                        bool on = led_value[i] > 128;
                        digitalWrite(led_pins[i], on);
                    }
                }

                return true;
            }
        }
    }

    return false;
}

uint8_t IIDXHID_::getShortName(char *name) {
    name[0] = 'I';
    name[1] = 'I';
    name[2] = 'D';
    name[3] = 'X';
    return 4;
}

int IIDXHID_::send_state(uint32_t button_state, int32_t turntable_state) {
    uint8_t data[4];

    data[0] = (uint8_t) (button_state & 0xFF);
    data[1] = (uint8_t) (button_state >> 8) & 0xFF;
    data[2] = (uint8_t) (turntable_state & 0xFF);
    data[3] = (uint8_t) (turntable_state >> 8) & 0xFF;

    return USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, 4);
}
