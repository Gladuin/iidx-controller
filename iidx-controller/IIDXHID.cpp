#include "IIDXHID.h"

#define BUTTON_PADDING (8 - (NUMBER_OF_BUTTONS % 8))
#define LED_PADDING (8 - (NUMBER_OF_LEDS % 8))

#define ENCODERMAX1 ENCODER_PPR & 0x00FF
#define ENCODERMAX2 ENCODER_PPR >> 8

uint8_t usb_data[128];
uint16_t lamp_hid_state = 0;

uint8_t extern tt_sensitivity;
uint8_t extern led_pins[];
bool extern hid_reactive_autoswitch;

static const uint8_t PROGMEM hid_report[] = {
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                      // USAGE (Game Pad)
    0xa1, 0x01,                      // COLLECTION (Application)

    0x85, 0x05,                      //   REPORT_ID (5)

    /* Buttons */
    0x05, 0x09,                      //   USAGE_PAGE (Button)
    0x19, 0x01,                      //   USAGE_MINIMUM (Button 1)
    0x29, NUMBER_OF_BUTTONS,         //   USAGE_MAXIMUM (Button NUMBER_OF_BUTTONS)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                      //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, NUMBER_OF_BUTTONS,         //   REPORT_COUNT (NUMBER_OF_BUTTONS)
    0x55, 0x00,                      //   UNIT_EXPONENT (0)
    0x65, 0x00,                      //   UNIT (None)
    0x81, 0x02,                      //   INPUT (Data,Var,Abs)
    /* Buttons END */

    /* Buttons padding */
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, BUTTON_PADDING,            //   REPORT_COUNT (BUTTON_PADDING)
    0x81, 0x03,                      //   INPUT (Cnst,Var,Abs)
    /* Buttons padding END */

    /* Encoder */
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
    /* Encoder END*/

    #if NO_SENSITIVITY == 0
    /* Turntable sensitivity input */
    0x05, 0x0a,                      //   USAGE_PAGE (Ordinals)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x09,                      //   LOGICAL_MAXIMUM (9)
    0x75, 0x08,                      //   REPORT_SIZE (8)
    0x95, 0x01,                      //   REPORT_COUNT (1)
    0xa1, 0x02,                      //   COLLECTION (Logical)
    0x09, 0x00,                      //     USAGE (Unused)
    0x91, 0x02,                      //     OUTPUT (Data,Var,Abs)
    0xc0,                            //   END_COLLECTION
    /* Turntable sensitivity input END */
    #endif

    0x85, 0x04,                      //   REPORT_ID (4)

    /* LEDs begin */
    0x05, 0x0a,                      //   USAGE_PAGE (Ordinals)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                      //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, NUMBER_OF_LEDS,            //   REPORT_COUNT (NUMBER_OF_LEDS)
    0xa1, 0x02,                      //   COLLECTION (Logical)
    0x09, 0x01,                      //     USAGE (Instance 1)
    0x09, 0x02,                      //     USAGE (Instance 2)
    0x09, 0x03,                      //     USAGE (Instance 3)
    0x09, 0x04,                      //     USAGE (Instance 4)
    0x09, 0x05,                      //     USAGE (Instance 5)
    0x09, 0x06,                      //     USAGE (Instance 6)
    0x09, 0x07,                      //     USAGE (Instance 7)
    0x09, 0x08,                      //     USAGE (Instance 8)
    0x09, 0x09,                      //     USAGE (Instance 9)
    0x09, 0x0a,                      //     USAGE (Instance 10)
    0x09, 0x0b,                      //     USAGE (Instance 11)
    0x91, 0x02,                      //     OUTPUT (Data,Var,Abs)
    0xc0,                            //   END_COLLECTION
    /* LEDs END */

    /* LEDs padding */
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, LED_PADDING,               //   REPORT_COUNT (BUTTON_PADDING)
    0x91, 0x03,                      //   OUTPUT (Cnst,Var,Abs)
    /* LEDs padding END */

    0xc0                             // END_COLLECTION
};

void setup_leds_reportid() {

}

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
            if (setup.wValueH == HID_REPORT_TYPE_OUTPUT) {
                USB_RecvControl(usb_data, setup.wLength);

                if (usb_data[0] == 4) {
                    lamp_hid_state = usb_data[2]<<8 | usb_data[1];
                    lastHidUpdate = millis();
                    /* no need to write lights or update lightmode here (for autoswitch), main loop() will take care of it */
                }
                #if NO_SENSITIVITY == 0
                else if (usb_data[0] == 5) {
                    tt_sensitivity = usb_data[1];

                    // Limit sensitivity to 9
                    if (tt_sensitivity > 9) {
                        tt_sensitivity = 9;
                    }
                }
                #endif

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

unsigned long IIDXHID_::getLastHidUpdate(){
  return lastHidUpdate;
}

void IIDXHID_::write_lights(uint32_t button_state, bool hid, bool reactive) {
  if (!reactive)
  {
    button_state = 0;
  }
  if (hid) 
  {
    button_state |= lamp_hid_state;
  }
  
  for (int i=0; i<NUMBER_OF_LEDS; i++)
  {
    digitalWrite(led_pins[i], ((button_state>>i)&1));
  }
}
  
int IIDXHID_::send_state(uint32_t button_state, int32_t turntable_state) {
    uint8_t data[5];

    data[0] = (uint8_t) 5;
    data[1] = (uint8_t) (button_state & 0xFF);
    data[2] = (uint8_t) (button_state >> 8) & 0xFF;
    data[3] = (uint8_t) (turntable_state & 0xFF);
    data[4] = (uint8_t) (turntable_state >> 8) & 0xFF;

    return USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, 5);
}
