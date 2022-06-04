#include <stdint.h> 

#include <avr/pgmspace.h>

#include "LUFAConfig.h"
#include "../Configuration.h"

#include <LUFA/LUFA/Drivers/USB/USB.h>

#include "Descriptors.h"


// Macros
#define BUTTON_PADDING (8 - (sizeof(button_pins) % 8))
#define LED_PADDING (8 - (sizeof(led_pins) % 8))

#if (defined(ARCH_HAS_MULTI_ADDRESS_SPACE) && !(defined(USE_FLASH_DESCRIPTORS) || defined(USE_EEPROM_DESCRIPTORS) || defined(USE_RAM_DESCRIPTORS)))
    #define HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES
#endif

// Types
typedef struct {
	USB_Descriptor_Configuration_Header_t Config;

	// Generic HID Interface
	USB_Descriptor_Interface_t            HID_Interface;
	USB_HID_Descriptor_HID_t              HID_GenericHID;
	USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;
	USB_Descriptor_Endpoint_t             HID_ReportOUTEndpoint;
} USB_descriptor_configuration_struct;

enum interface_descriptors_enum {
	INTERFACE_ID_GenericHID = 0, // GenericHID interface descriptor ID
};

enum string_descriptors_enum {
	STRING_ID_Language     = 0, // Supported Languages string descriptor ID (must be zero)
	STRING_ID_Manufacturer = 1, // Manufacturer string ID
	STRING_ID_Product      = 2, // Product string ID
};

static configuration_struct *config;


USB_Descriptor_HIDReport_Datatype_t generic_report[] = {
    HID_RI_USAGE_PAGE(8, 1),
    HID_RI_USAGE(8, 4),
    HID_RI_COLLECTION(8, 1),
    
    HID_RI_REPORT_ID(8, 5),
    
    // Buttons
    HID_RI_USAGE_PAGE(8, 9),
    HID_RI_USAGE_MINIMUM(8, 1),
    HID_RI_USAGE_MAXIMUM(8, sizeof(button_pins)),
    HID_RI_LOGICAL_MINIMUM(8, 0),
    HID_RI_LOGICAL_MAXIMUM(8, 1),
    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, sizeof(button_pins)),
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

    HID_RI_REPORT_ID(8, 4),   

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
	.Config =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_descriptor_configuration_struct),
			.TotalInterfaces        = 1,

			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,

			.ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
		},

	.HID_Interface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = INTERFACE_ID_GenericHID,
			.AlternateSetting       = 0x00,

			.TotalEndpoints         = 2,

			.Class                  = HID_CSCP_HIDClass,
			.SubClass               = HID_CSCP_NonBootSubclass,
			.Protocol               = HID_CSCP_NonBootProtocol,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.HID_GenericHID =
		{
			.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

			.HIDSpec                = VERSION_BCD(1,1,1),
			.CountryCode            = 0x00,
			.TotalReportDescriptors = 1,
			.HIDReportType          = HID_DTYPE_Report,
			.HIDReportLength        = sizeof(generic_report)
		},

	.HID_ReportINEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = GENERIC_IN_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = GENERIC_EPSIZE,
			.PollingIntervalMS      = 0x01
		},

	.HID_ReportOUTEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = GENERIC_OUT_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = GENERIC_EPSIZE,
			.PollingIntervalMS      = 0x01
		}
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
    generic_report[41] = ADJUSTED_PPR & 0xFF;
    generic_report[42] = ADJUSTED_PPR >> 8;
    
    configuration_descriptor.HID_ReportINEndpoint.PollingIntervalMS = config->polling_rate;
    configuration_descriptor.HID_ReportOUTEndpoint.PollingIntervalMS = config->polling_rate;
    
    
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
			switch (descriptor_number)
			{
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
			address = &configuration_descriptor.HID_GenericHID;
			size    = sizeof(USB_HID_Descriptor_HID_t);
			break;
		case HID_DTYPE_Report:
            #if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
			*descriptor_memory_space = MEMSPACE_RAM;
			#endif
        
			address = &generic_report;
			size    = sizeof(generic_report);
			break;
	}

	*descriptor_address = address;
	return size;
}
