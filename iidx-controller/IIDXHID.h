#include "HID.h"

#define NUMBER_OF_LEDS 11
#define NUMBER_OF_BUTTONS 11
#define ENCODER_PPR 360

#define INCREMENTS_PER_FULL_TURN 72   //72 for 60Hz IIDX, 144 for 120Hz IIDX, 255 for 1 to 1

#define ADJUSTED_PPR ((int)((float)ENCODER_PPR * ((float)255/(float)INCREMENTS_PER_FULL_TURN)))

#define NO_SENSITIVITY 0

#define KONAMI_SPOOF 1

#define EPTYPE_DESCRIPTOR_SIZE uint8_t
#define STRING_ID_Base 4

class IIDXHID_ : public PluggableUSBModule {
    public:
        IIDXHID_(void);
        void write_lights(uint32_t button_state, bool hid, bool reactive);
        int send_state(uint32_t button_state, int32_t turntable_state);
        unsigned long getLastHidUpdate();

    protected:
        EPTYPE_DESCRIPTOR_SIZE epType[1];
        unsigned long lastHidUpdate = 0;

        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern IIDXHID_ IIDXHID;
