#include "HID.h"

#define NUMBER_OF_LEDS 11
#define NUMBER_OF_BUTTONS 11
#define ENCODER_PPR 360

//#define INCREMENTS_PER_FULL_TURN 72   //72 for 60Hz IIDX, 144 for 120Hz IIDX

#define TT_INCREMENTS_MULTIPLIER 3.56   //3.56 for 60Hz IIDX (72 per full turn) (255/72), 
                                        //1.77 for 120Hz IIDX (144 per full turn) (255/144),
                                        //1 for 1 full turn IRL to be 1 full turn in software

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
