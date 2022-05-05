#include <stdint.h>

#include "IIDXHID.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "LUFAConfig.h"

#include <LUFA/LUFA/Drivers/USB/USB.h>
#include <LUFA/LUFA/Platform/Platform.h>

#include "../IO/Buttons.h"
#include "Descriptors.h"


typedef struct {
    uint8_t report_id;
    uint16_t button_status;
    uint16_t turntable_position;
} input_data_struct;

typedef struct {
    uint8_t report_id;
    uint16_t data;
} output_data_struct;


void setup_hardware(void) {
	// Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disable clock division
	clock_prescale_set(clock_div_1);

	// Hardware Initialization
	USB_Init();
}

void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// Setup HID Report Endpoints
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
}

void EVENT_USB_Device_ControlRequest(void) {
	// Handle HID Class specific requests
	switch (USB_ControlRequest.bRequest) {
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
				input_data_struct input_struct;
				create_hid_report(&input_struct);

				Endpoint_ClearSETUP();

				// Write the report data to the control endpoint
				Endpoint_Write_Control_Stream_LE(&input_struct, sizeof(input_struct));
				Endpoint_ClearOUT();
			}

			break;

		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
				output_data_struct output_struct;

				Endpoint_ClearSETUP();

				// Read the report data from the control endpoint
				Endpoint_Read_Control_Stream_LE(&output_struct, sizeof(output_struct));
				Endpoint_ClearIN();

				process_hid_report(&output_struct);
			}

			break;
	}
}

void process_hid_report(output_data_struct* output_struct) {
    if (output_struct->report_id == 3) {
        process_command(output_struct->data);
    }
}

void create_hid_report(input_data_struct* input_struct) {
    input_struct->report_id = 5;
    input_struct->button_status = get_button_state();
    input_struct->turntable_position = 0;
}

void HID_task(void) {
	// Device must be connected and configured for the task to run
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);

	// Check to see if a packet has been sent from the host
	if (Endpoint_IsOUTReceived()) {
		// Check to see if the packet contains data
		if (Endpoint_IsReadWriteAllowed()) {
			// Create a temporary buffer to hold the read in report from the host
			output_data_struct output_struct;

			// Read Generic Report Data
			Endpoint_Read_Stream_LE(&output_struct, sizeof(output_struct), NULL);

			// Process Generic Report Data
			process_hid_report(&output_struct);
		}

		// Finalize the stream transfer to send the last packet
		Endpoint_ClearOUT();
	}

	Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);

	// Check to see if the host is ready to accept another packet
	if (Endpoint_IsINReady())  {
		// Create a temporary buffer to hold the report to send to the host
		input_data_struct input_struct;

		// Create Generic Report Data
		create_hid_report(&input_struct);

		// Write Generic Report Data
		Endpoint_Write_Stream_LE(&input_struct, sizeof(input_struct), NULL);

		// Finalize the stream transfer to send the last packet
		Endpoint_ClearIN();
	}
}
