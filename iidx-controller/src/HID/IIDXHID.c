#include <stdint.h>

#include "IIDXHID.h"

#include <Arduino.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "LUFAConfig.h"

#include <LUFA/LUFA/Drivers/USB/USB.h>
#include <LUFA/LUFA/Platform/Platform.h>

#include "../IO/Buttons.h"
#include "../IO/Encoder.h" 
#include "../IO/LEDs.h"
#include "../Configuration.h"
#include "Descriptors.h"


static bool using_report_protocol = true;
static uint16_t idle_count = 500;
static uint16_t idle_ms_remaining = 0;

unsigned long last_led_update;
uint16_t previous_turntable_position;

static configuration_struct *config;

typedef struct {
    uint8_t report_id;
    uint16_t button_status;
    uint16_t turntable_position;
} input_data_struct_joystick;

typedef struct {
    uint8_t report_id;
    uint16_t data;
} output_data_struct_joystick;

typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[sizeof(button_pins) + 2];
} input_data_struct_keyboard;

typedef struct {
    int16_t x;
    int16_t y;
} input_data_struct_mouse;


unsigned long get_last_led_update() {
    return last_led_update;
}

void setup_hardware(void) {
    get_configuration(&config);
    
	// Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disable clock division
	clock_prescale_set(clock_div_1);

	// Hardware Initialization
	USB_Init();
}

void EVENT_USB_Device_Connect(void) {
	using_report_protocol = true;
}

void EVENT_USB_Device_ConfigurationChanged(void) {
	// Setup HID Report Endpoints
	Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(MOUSE_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
    
    USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void) {
	// Handle HID Class specific requests
	switch (USB_ControlRequest.bRequest) {
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
				Endpoint_ClearSETUP();
                
                switch (USB_ControlRequest.wIndex) {
                    case INTERFACE_ID_Joystick: ;
                        input_data_struct_joystick input_data_joystick;
                        create_joystick_report(&input_data_joystick);
                        Endpoint_Write_Control_Stream_LE(&input_data_joystick, sizeof(input_data_struct_joystick));
                        break;
                    case INTERFACE_ID_Keyboard: ;
                        input_data_struct_keyboard input_data_keyboard;
                        create_joystick_report(&input_data_keyboard);
                        Endpoint_Write_Control_Stream_LE(&input_data_keyboard, sizeof(input_data_struct_keyboard));
                        break;
                    case INTERFACE_ID_Mouse: ;
                        input_data_struct_mouse input_data_mouse;
                        create_mouse_report(&input_data_mouse);
                        Endpoint_Write_Control_Stream_LE(&input_data_mouse, sizeof(input_data_struct_mouse));
                        break;
                }
                
				// Write the report data to the control endpoint
				
				Endpoint_ClearOUT();
			}

			break;

		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
				output_data_struct_joystick output_struct;

				Endpoint_ClearSETUP();

				// Read the report data from the control endpoint
				Endpoint_Read_Control_Stream_LE(&output_struct, sizeof(output_data_struct_joystick));
				Endpoint_ClearIN();

				process_joystick_report(&output_struct);
			}

			break;
            
        case HID_REQ_GetProtocol:
            if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
                Endpoint_ClearSETUP();
                
                Endpoint_Write_8(using_report_protocol);
                
                Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
            }
            
            break;
            
        case HID_REQ_SetProtocol:
            if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
                Endpoint_ClearSETUP();
				Endpoint_ClearStatusStage();
                
                using_report_protocol = (USB_ControlRequest.wValue != 0);
            }
            
            break;
            
        case HID_REQ_SetIdle:
            if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
                Endpoint_ClearSETUP();
				Endpoint_ClearStatusStage();

                idle_count = ((USB_ControlRequest.wValue & 0xFF00) >> 6);
            }
            
            break;
            
        case HID_REQ_GetIdle:
            if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
                Endpoint_ClearSETUP();
                
                Endpoint_Write_8(idle_count >> 2);
                
                Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
            }
            
            break;
	}
}

void EVENT_USB_Device_StartOfFrame(void) {
	if (idle_ms_remaining)
	  idle_ms_remaining--;
}

void process_joystick_report(output_data_struct_joystick* output_struct) {
    switch (output_struct->report_id) {
        case 2:
            if (config->led_mode == 0 || config->led_mode == 1) {
                last_led_update = millis();
                write_leds(output_struct->data, true);
            }
            break;
        case 3:
            process_command(output_struct->data);
            break;
    }
}

void create_joystick_report(input_data_struct_joystick* input_data) {
    memset(input_data, 0, sizeof(input_data_struct_joystick));
    
    input_data->report_id = 1;
    
    if (config->controller_mode == 0) {
        input_data->button_status = get_button_state();

        if (config->tt_mode == 0) {
            input_data->turntable_position = get_encoder_state();
        } else if (config->tt_mode == 1) {
            switch (get_digital_encoder_state()) {
                case 1:
                    input_data->button_status = input_data->button_status | 0b0000100000000000;
                    break;
                case 2:
                    input_data->button_status = input_data->button_status | 0b0001000000000000;
                    break;
            }
        }
    }
}

void create_keyboard_report(input_data_struct_keyboard* input_data) {
    memset(input_data, 0, sizeof(input_data_struct_keyboard));
    
    if (config->controller_mode == 1) {
        uint16_t button_status = get_button_state();
        
        for (int i = 0; i < sizeof(button_pins); i++) {
            if (button_status & ((uint16_t)1 << i)) input_data->keycode[i] = 0x04 + i;
        }
        
        if (config->tt_mode == 1) {
            switch (get_digital_encoder_state()) {
                case 1:
                    input_data->keycode[sizeof(button_pins)] = (0x03 + sizeof(button_pins)) + 1;
                    break;
                case 2:
                    input_data->keycode[sizeof(button_pins) + 1] = (0x03 + sizeof(button_pins)) + 2;
                    break;
            }
        }
    }
}

void create_mouse_report(input_data_struct_mouse* input_data) {
    memset(input_data, 0, sizeof(input_data_struct_mouse));
    
    if (config->controller_mode == 1 && config->tt_mode == 0) {  
        uint16_t turntable_position = get_encoder_state();
        
        input_data->y = turntable_position - previous_turntable_position;
        
        previous_turntable_position = turntable_position;
    }
}

void HID_task(void) {
	// Device must be connected and configured for the task to run
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

    static input_data_struct_joystick  previous_joystick_data;
    input_data_struct_joystick                joystick_data;
    
    static input_data_struct_keyboard  previous_keyboard_data;
    input_data_struct_keyboard                keyboard_data;
    
    static input_data_struct_mouse     previous_mouse_data;
    input_data_struct_mouse                   mouse_data;
    
    bool send_joy_report = false;
    bool send_kb_report  = false;
    bool send_m_report   = false;
    
    create_joystick_report(&joystick_data);
    create_keyboard_report(&keyboard_data);
    create_mouse_report(&mouse_data);
    
    if (idle_count && (!(idle_ms_remaining))) {
        idle_ms_remaining = idle_count;
        
        send_joy_report = true;
        send_kb_report  = true;
        send_m_report   = true;
    } else {
        send_joy_report = (memcmp(&previous_joystick_data, &joystick_data, sizeof(input_data_struct_joystick)) != 0);
        send_kb_report  = (memcmp(&previous_keyboard_data, &keyboard_data, sizeof(input_data_struct_keyboard)) != 0);
        send_m_report   = (memcmp(&previous_mouse_data, &mouse_data, sizeof(input_data_struct_mouse)) != 0);
    }
    
    if (mouse_data.y != 0) {
        send_m_report = true;
    }
    

	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);

	if (Endpoint_IsReadWriteAllowed() && send_joy_report) {
		previous_joystick_data = joystick_data;

		Endpoint_Write_Stream_LE(&joystick_data, sizeof(input_data_struct_joystick), NULL);

		Endpoint_ClearIN();
	}
    
    Endpoint_SelectEndpoint(KEYBOARD_IN_EPADDR);

	if (Endpoint_IsReadWriteAllowed() && send_kb_report) {
		previous_keyboard_data = keyboard_data;

		Endpoint_Write_Stream_LE(&keyboard_data, sizeof(input_data_struct_keyboard), NULL);

		Endpoint_ClearIN();
	}
    
	Endpoint_SelectEndpoint(MOUSE_IN_EPADDR);

	if (Endpoint_IsReadWriteAllowed() && send_m_report) {
		previous_mouse_data = mouse_data;

		Endpoint_Write_Stream_LE(&mouse_data, sizeof(input_data_struct_mouse), NULL);

		Endpoint_ClearIN();
	}
    
    
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

	// Check to see if a packet has been sent from the host
	if (Endpoint_IsOUTReceived()) {
		// Check to see if the packet contains data
		if (Endpoint_IsReadWriteAllowed()) {
			// Create a temporary buffer to hold the read in report from the host
			output_data_struct_joystick output_struct;

			// Read Generic Report Data
			Endpoint_Read_Stream_LE(&output_struct, sizeof(output_data_struct_joystick), NULL);

			// Process Generic Report Data
			process_joystick_report(&output_struct);
		}

		// Finalize the stream transfer to send the last packet
		Endpoint_ClearOUT();
	}
}
