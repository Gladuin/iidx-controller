#include <stdint.h> 

#include <avr/pgmspace.h>

#include "LUFAConfig.h"
#include "../Configuration.h"

#include <LUFA/LUFA/Drivers/USB/USB.h>

#include "Descriptors.h"


// Macros
#define BUTTON_PADDING (8 - ((sizeof(button_pins) + 2) % 8))
#define LED_PADDING (8 - (sizeof(led_pins) % 8))

#if (defined(ARCH_HAS_MULTI_ADDRESS_SPACE) && !(defined(USE_FLASH_DESCRIPTORS) || defined(USE_EEPROM_DESCRIPTORS) || defined(USE_RAM_DESCRIPTORS)))
    #define HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES
#endif


// Types
typedef struct {
	USB_Descriptor_Configuration_Header_t Config;
	
	// Joystick HID Interface
	USB_Descriptor_Interface_t            HID1_joystick_interface;
	USB_HID_Descriptor_HID_t              HID1_joystick_hid;
	USB_Descriptor_Endpoint_t             HID1_joystick_in_endpoint;
	USB_Descriptor_Endpoint_t             HID1_joystick_out_endpoint;
	
	// Keyboard HID Interface
	USB_Descriptor_Interface_t            HID2_keyboard_interface;
    USB_HID_Descriptor_HID_t              HID2_keyboard_hid;
    USB_Descriptor_Endpoint_t             HID2_keyboard_in_endpoint;
    
    // Mouse HID Interface
    USB_Descriptor_Interface_t            HID3_mouse_interface;
    USB_HID_Descriptor_HID_t              HID3_mouse_hid;
    USB_Descriptor_Endpoint_t             HID3_mouse_in_endpoint;
	
} USB_descriptor_configuration_struct;


enum string_descriptors_enum {
	STRING_ID_Language     = 0, // Supported Languages string descriptor ID (must be zero)
	STRING_ID_Manufacturer = 1, // Manufacturer string ID
	STRING_ID_Product      = 2, // Product string ID
};

static configuration_struct *config;


USB_Descriptor_HIDReport_Datatype_t joystick_report[] = {
	HID_RI_USAGE_PAGE(8, 1),
    HID_RI_USAGE(8, 4),
    HID_RI_COLLECTION(8, 1),
    
    HID_RI_REPORT_ID(8, 1),
    
    // Buttons
    HID_RI_USAGE_PAGE(8, 9),
    HID_RI_USAGE_MINIMUM(8, 1),
    HID_RI_USAGE_MAXIMUM(8, (sizeof(button_pins) + 2)),
    HID_RI_LOGICAL_MINIMUM(8, 0),
    HID_RI_LOGICAL_MAXIMUM(8, 1),
    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, (sizeof(button_pins) + 2)),
    HID_RI_UNIT_EXPONENT(8, 0),
    HID_RI_UNIT(8, 0),
    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

    // Button padding
    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, BUTTON_PADDING),
    HID_RI_INPUT(8, HID_IOF_CONSTANT | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    
    // Encoder
    HID_RI_USAGE_PAGE(8, 1),
    HID_RI_USAGE(8, 1),
    HID_RI_LOGICAL_MINIMUM(8, 8),
    HID_RI_LOGICAL_MAXIMUM(16, 0),   // gets changed later down in the code
    HID_RI_REPORT_SIZE(8, 16),
    HID_RI_REPORT_COUNT(8, 1),
    HID_RI_COLLECTION(8, 0),
        HID_RI_USAGE(8, 48),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_RI_END_COLLECTION(0),

    HID_RI_REPORT_ID(8, 2),

    // LEDs
    HID_RI_USAGE_PAGE(8, 10),
    HID_RI_LOGICAL_MINIMUM(8, 0),
    HID_RI_LOGICAL_MAXIMUM(8, 1),
    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, sizeof(led_pins)),
    HID_RI_COLLECTION(8, 2),
        HID_RI_USAGE(8, 11),
        HID_RI_USAGE(8, 10),
        HID_RI_USAGE(8, 9),
        HID_RI_USAGE(8, 8),
        HID_RI_USAGE(8, 7),
        HID_RI_USAGE(8, 6),
        HID_RI_USAGE(8, 5),
        HID_RI_USAGE(8, 4),
        HID_RI_USAGE(8, 3),
        HID_RI_USAGE(8, 2),
        HID_RI_USAGE(8, 1),
        HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_RI_END_COLLECTION(0),

    // LEDs padding
    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, LED_PADDING),
    HID_RI_OUTPUT(8, HID_IOF_CONSTANT | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    
    HID_RI_REPORT_ID(8, 3),

    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, 16),
    HID_RI_OUTPUT(8, HID_IOF_CONSTANT),
    
	HID_RI_END_COLLECTION(0)
};

const USB_Descriptor_HIDReport_Datatype_t PROGMEM keyboard_report[] = {
    HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
    HID_RI_USAGE(8, 0x06), /* Keyboard */
    HID_RI_COLLECTION(8, 0x01), /* Application */
        HID_RI_USAGE_PAGE(8, 0x07), /* Key Codes */
        HID_RI_USAGE_MINIMUM(8, 0xE0), /* Keyboard Left Control */
        HID_RI_USAGE_MAXIMUM(8, 0xE7), /* Keyboard Right GUI */
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0x01),
        HID_RI_REPORT_SIZE(8, 0x01),
        HID_RI_REPORT_COUNT(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
        HID_RI_REPORT_COUNT(8, 0x01),
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_CONSTANT),
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0x65),
        HID_RI_USAGE_PAGE(8, 0x07), /* Keyboard */
        HID_RI_USAGE_MINIMUM(8, 0x00), /* Reserved (no event indicated) */
        HID_RI_USAGE_MAXIMUM(8, 0x65), /* Keyboard Application */
        HID_RI_REPORT_COUNT(8, sizeof(button_pins) + 2),   // amount of keys
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
    HID_RI_END_COLLECTION(0),
};

const USB_Descriptor_HIDReport_Datatype_t PROGMEM mouse_report[] = {
    HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
    HID_RI_USAGE(8, 0x02), /* Mouse */
    HID_RI_COLLECTION(8, 0x01), /* Application */
        HID_RI_USAGE(8, 0x01), /* Pointer */
        HID_RI_COLLECTION(8, 0x00), /* Physical */
            HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
            HID_RI_USAGE(8, 0x30), /* Usage X */
            HID_RI_USAGE(8, 0x31), /* Usage Y */
            HID_RI_LOGICAL_MINIMUM(8, -1),
            HID_RI_LOGICAL_MAXIMUM(8, 1),
            HID_RI_PHYSICAL_MINIMUM(8, -1),
            HID_RI_PHYSICAL_MAXIMUM(8, 1),
            HID_RI_REPORT_COUNT(8, 0x02),
            HID_RI_REPORT_SIZE(8, 0x08),
            HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),
        HID_RI_END_COLLECTION(0),
    HID_RI_END_COLLECTION(0),
};



const USB_Descriptor_Device_t PROGMEM device_descriptor = {
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(1,1,0),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x0001,
	.ProductID              = 0x0001,
	.ReleaseNumber          = VERSION_BCD(0,0,1),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = NO_DESCRIPTOR,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

USB_descriptor_configuration_struct configuration_descriptor = {
	.Config = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

		.TotalConfigurationSize = sizeof(USB_descriptor_configuration_struct),
		.TotalInterfaces        = 3,

		.ConfigurationNumber    = 1,
		.ConfigurationStrIndex  = NO_DESCRIPTOR,

		.ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

		.MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
	},
    
    
    .HID1_joystick_interface = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

		.InterfaceNumber        = INTERFACE_ID_Joystick,
		.AlternateSetting       = 0x00,

		.TotalEndpoints         = 2,

		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,

		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID1_joystick_hid = {
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

		.HIDSpec                = VERSION_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(joystick_report)
	},

	.HID1_joystick_in_endpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = JOYSTICK_IN_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = HID_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
    
    .HID1_joystick_out_endpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = JOYSTICK_OUT_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = HID_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
    
    
    .HID2_keyboard_interface = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

		.InterfaceNumber        = INTERFACE_ID_Keyboard,
		.AlternateSetting       = 0x00,

		.TotalEndpoints         = 1,

		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,

		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID2_keyboard_hid = {
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

		.HIDSpec                = VERSION_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(keyboard_report)
	},

	.HID2_keyboard_in_endpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = KEYBOARD_IN_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = HID_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
    
    
    .HID3_mouse_interface = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

		.InterfaceNumber        = INTERFACE_ID_Mouse,
		.AlternateSetting       = 0x00,

		.TotalEndpoints         = 1,

		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,

		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID3_mouse_hid = {
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

		.HIDSpec                = VERSION_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(mouse_report)
	},

	.HID3_mouse_in_endpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = MOUSE_IN_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = HID_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
};

const USB_Descriptor_String_t PROGMEM language_string = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM manufacturer_string = USB_STRING_DESCRIPTOR(MF_NAME);
const USB_Descriptor_String_t PROGMEM product_string = USB_STRING_DESCRIPTOR(L"IIDX Controller");

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const descriptor_address
                                    #if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
                                    , uint8_t* const descriptor_memory_space
                                    #endif
                                    )
{
    get_configuration(&config);
    
	const uint8_t  descriptor_type   = (wValue >> 8);
	const uint8_t  descriptor_number = (wValue & 0xFF);

	const void* address = NULL;
	uint16_t    size    = NO_DESCRIPTOR;

    // adjust logical maximum of encoder
    joystick_report[41] = ADJUSTED_PPR & 0xFF;
    joystick_report[42] = ADJUSTED_PPR >> 8;
    
    configuration_descriptor.HID1_joystick_in_endpoint.PollingIntervalMS    = config->polling_rate;
    configuration_descriptor.HID1_joystick_out_endpoint.PollingIntervalMS   = config->polling_rate;
    configuration_descriptor.HID2_keyboard_in_endpoint.PollingIntervalMS    = config->polling_rate;
    configuration_descriptor.HID3_mouse_in_endpoint.PollingIntervalMS       = config->polling_rate;
    
    
    #if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
	*descriptor_memory_space = MEMSPACE_FLASH;
	#endif

	switch (descriptor_type)
	{
		case DTYPE_Device:
			address = &device_descriptor;
			size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
            #if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
			*descriptor_memory_space = MEMSPACE_RAM;
			#endif
			address = &configuration_descriptor;
			size    = sizeof(USB_descriptor_configuration_struct);
			break;
		case DTYPE_String:
			switch (descriptor_number) {
				case STRING_ID_Language:
					address = &language_string;
					size    = pgm_read_byte(&language_string.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					address = &manufacturer_string;
					size    = pgm_read_byte(&manufacturer_string.Header.Size);
					break;
				case STRING_ID_Product:
					address = &product_string;
					size    = pgm_read_byte(&product_string.Header.Size);
					break;
			}

			break;
		case HID_DTYPE_HID:
            #if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
            *descriptor_memory_space = MEMSPACE_RAM;
            #endif 
        
            switch(wIndex) {               
                case INTERFACE_ID_Joystick:
                    address = &configuration_descriptor.HID1_joystick_hid;
                    break;
                case INTERFACE_ID_Keyboard:
                    address = &configuration_descriptor.HID2_keyboard_hid;
                    break;
                case INTERFACE_ID_Mouse:
                    address = &configuration_descriptor.HID3_mouse_hid;
                    break;
            }
            
            size = sizeof(USB_HID_Descriptor_HID_t);
            
			break;
		case HID_DTYPE_Report:
            switch(wIndex) {
                case INTERFACE_ID_Joystick:
                    #if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
                    *descriptor_memory_space = MEMSPACE_RAM;
                    #endif
                    address = &joystick_report;
                    size    = sizeof(joystick_report);
                    break;
                case INTERFACE_ID_Keyboard:
                    address = &keyboard_report;
                    size    = sizeof(keyboard_report);
                    break;
                case INTERFACE_ID_Mouse:
                    address = &mouse_report;
                    size    = sizeof(mouse_report);
                    break;
            }
            
			break;
	}

	*descriptor_address = address;
	return size;
}
