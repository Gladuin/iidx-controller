#include "HID.h"

#define NUMBER_OF_LEDS 11
#define NUMBER_OF_BUTTONS 11
#define ENCODER_PPR 500

#define EPTYPE_DESCRIPTOR_SIZE uint8_t

class IIDXHID_ : public PluggableUSBModule {
    public:
        IIDXHID_(void);
        int send_state(uint32_t button_state, int32_t turntable_state);

    protected:
        EPTYPE_DESCRIPTOR_SIZE epType[1];

        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        uint8_t getShortName(char* name);
        bool setup(USBSetup& setup);
};

extern IIDXHID_ IIDXHID;
