#include "IIDXHID.h"
#include "config.h"

#define BUTTON_PADDING (8 - (NUMBER_OF_BUTTONS % 8))
#define LED_PADDING (8 - (NUMBER_OF_LEDS % 8))

#define ENCODERMAX1 ADJUSTED_PPR & 0x00FF
#define ENCODERMAX2 ADJUSTED_PPR >> 8

#define STRING_ID_Base 4

uint8_t usb_data[128];
uint16_t lamp_hid_state = 0;

uint8_t extern tt_sensitivity;
bool extern hid_reactive_autoswitch;

/* HID string and device descriptor */
#if KONAMI_SPOOF == 1
const DeviceDescriptor PROGMEM USB_DeviceDescriptorIAD = D_DEVICE(0xEF, 0x02, 0x01, 64, 0x1ccf, 0x8048, 0x100, IMANUFACTURER, IPRODUCT, ISERIAL, 1);
const char* const PROGMEM String_Manufacturer = "Konami Amusement";
const char* const PROGMEM String_Product = "beatmania IIDX controller premium model";
#else
const char* const PROGMEM String_Manufacturer = "gladuin";
const char* const PROGMEM String_Product = "IIDX Controller";
#endif

const char* const PROGMEM String_Serial = "IIDX";

const char* const PROGMEM LEDString_00 = "Button 1";
const char* const PROGMEM LEDString_01 = "Button 2";
const char* const PROGMEM LEDString_02 = "Button 3";
const char* const PROGMEM LEDString_03 = "Button 4";
const char* const PROGMEM LEDString_04 = "Button 5";
const char* const PROGMEM LEDString_05 = "Button 6";
const char* const PROGMEM LEDString_06 = "Button 7";
const char* const PROGMEM LEDString_07 = "Misc button 1";
const char* const PROGMEM LEDString_08 = "Misc button 2";
const char* const PROGMEM LEDString_09 = "Misc button 3";
const char* const PROGMEM LEDString_10 = "Misc button 4";
const char* const PROGMEM TTString = "TT Sensitivity";

const char* String_indiv[] = { LEDString_00, LEDString_01, LEDString_02, LEDString_03, LEDString_04, LEDString_05, LEDString_06, LEDString_07, LEDString_08, LEDString_09, LEDString_10, TTString };
uint8_t STRING_ID_Count = 12;

static const uint8_t PROGMEM hid_report[] = {
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                      // USAGE (Joystick)
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
    0x79, 0x0f,                      //     STRING_INDEX (15)
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
    0x89, 0x04,                      //     STRING_MINIMUM (4)
    0x99, 0x0e,                      //     STRING_MAXIMUM (14)
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

static bool SendControl(uint8_t d) {
    return USB_SendControl(0, &d, 1) == 1;
}

static bool USB_SendStringDescriptor(const char *string_P, uint8_t string_len, uint8_t flags) {
    bool pgm = flags & TRANSFER_PGM;
    
    SendControl(2 + string_len * 2);
    SendControl(3);
    
    for (uint8_t i = 0; i < string_len; i++) {
        bool r = SendControl(pgm ? pgm_read_byte(&string_P[i]) : string_P[i]);
        r &= SendControl(0); // high byte
        
        if (!r) {
            return false;
        }
    }
    
    return true;
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
    #if KONAMI_SPOOF == 1  
    if (setup.wValueH == USB_DEVICE_DESCRIPTOR_TYPE) {
        return USB_SendControl(TRANSFER_PGM, (const uint8_t*)&USB_DeviceDescriptorIAD, sizeof(USB_DeviceDescriptorIAD));
    }
    #endif
    if (setup.wValueH == USB_STRING_DESCRIPTOR_TYPE) {
        if (setup.wValueL == IPRODUCT) {
            return USB_SendStringDescriptor(String_Product, strlen(String_Product), 0);
        } else if (setup.wValueL == IMANUFACTURER) {
            return USB_SendStringDescriptor(String_Manufacturer, strlen(String_Manufacturer), 0);
        } else if (setup.wValueL == ISERIAL) {
            return USB_SendStringDescriptor(String_Serial, strlen(String_Serial), 0);
        } else if(setup.wValueL >= STRING_ID_Base && setup.wValueL < (STRING_ID_Base + STRING_ID_Count)) {
            return USB_SendStringDescriptor(String_indiv[setup.wValueL - STRING_ID_Base], strlen(String_indiv[setup.wValueL - STRING_ID_Base]), 0);
        }
    }

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
                    // No need to write lights or update lightmode here (for autoswitch), main loop() will take care of it
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

unsigned long IIDXHID_::getLastHidUpdate() {
    return lastHidUpdate;
}

void IIDXHID_::write_lights(uint32_t button_state, bool hid, bool reactive) {
    if (!reactive) {
        button_state = 0;
    }

    if (hid) {
        button_state |= lamp_hid_state;
    }

    for (int i=0; i<NUMBER_OF_LEDS; i++) {
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
