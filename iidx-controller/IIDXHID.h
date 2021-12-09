#include "HID.h"

#define ADJUSTED_PPR ((int)((float)ENCODER_PPR * ((float)255 / (float)INCREMENTS_PER_FULL_TURN)))

class IIDXHID_ : public PluggableUSBModule {
    public:
        IIDXHID_(void);
        void write_lights(uint32_t button_state, bool hid, bool reactive);
        int send_state(uint32_t button_state, int32_t turntable_state);
        unsigned long getLastHidUpdate();

    protected:
        uint8_t epType[1];
        unsigned long lastHidUpdate = 0;

        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern IIDXHID_ IIDXHID;
