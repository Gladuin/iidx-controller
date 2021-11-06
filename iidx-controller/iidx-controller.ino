
#include "config.h"

#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>
#include "Encoder.h"
#include "IIDXHID.h"

#define REPORT_DELAY 1000
#define MS_DEBOUNCE 5

IIDXHID_ IIDXHID;

Bounce buttons[NUMBER_OF_BUTTONS];
Encoder encoder(encoder_pins[0], encoder_pins[1], 99);

uint32_t last_report = 0;

int32_t tt_pos;
//uint8_t encoder_curstate;
//uint8_t encoder_laststate;

//uint8_t tt_lookup[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
int button_state_array[NUMBER_OF_BUTTONS];

bool hid_reactive_autoswitch = true;
bool hid_lights = true;
bool reactive = false;

void setup() {

  //Buttons setup
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        buttons[i] = Bounce();
        buttons[i].attach(button_pins[i], INPUT_PULLUP);
        buttons[i].interval(MS_DEBOUNCE);
    }

  //Encoder setup 
    EncoderInterrupt.begin( &encoder );

  //LED Setup
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }

    delay(200);

    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        digitalWrite(led_pins[i], LOW);
    }

  //Encoder last state
    //encoder_laststate = digitalRead(encoder_pins[0]);
}

void loop() {
    uint32_t buttons_state = 0;

    // Mixed mode will behave sometimes like HID, sometimes like reactive
    if (hid_reactive_autoswitch){
        if ((millis() - IIDXHID.getLastHidUpdate()) > 3000) {
            reactive = true;
            hid_lights = false;
        } else {
            reactive = false;
            hid_lights = true;
        }
    }

  //write button status into array
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        buttons[i].update();
        button_state_array[i] = buttons[i].read();
    }
  
  //read from array and convert to bitfield
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        int button_value = button_state_array[i];

        // Put button states into the buttons_state variable via bitwise operations
        if (button_value == LOW) {
            buttons_state |= (uint32_t)1 << i;
        } else {
            buttons_state &= ~((uint32_t)1 << i);
        }
    }

    IIDXHID.write_lights(buttons_state, hid_lights, reactive);


  //Encoder update
   if (encoder.delta() >= tt_sensitivity || encoder.delta() <= -tt_sensitivity ){
     tt_pos += encoder.delta();
   }
  
  // Limit the encoder from 0 to ENCODER_PPR
    if (tt_pos >= ENCODER_PPR) {
        tt_pos = 1;
    } else if (tt_pos <= 0) {
        tt_pos = ENCODER_PPR - 1;
    }

  // Send turntable and button state every 1000 microseconds
    if (((micros() - last_report) >= REPORT_DELAY)) {
        IIDXHID.send_state(buttons_state, tt_pos);
        last_report = micros();
    }

  // MANUAL LIGHTMODE UPDATE
    static bool modeChanged = false;
    if (buttons_state & ((uint32_t)1 << (NUMBER_OF_BUTTONS - 1))) {
        if ((buttons_state & 1) && (modeChanged == false)) {
            modeChanged = true;
            if (hid_reactive_autoswitch) {
                hid_reactive_autoswitch = false;
                hid_lights = false;
                reactive = true;  // 1 x x -> 0 0 1
            } else if (reactive && hid_lights) {
                hid_reactive_autoswitch = true;  // 0 1 1 -> 1 x x
            } else if (reactive) {
                reactive = false;
                hid_lights = true; // 0 0 1 -> 0 1 0
            } else {
                reactive = true; // 0 1 0 -> 0 1 1
            }
        } else if (!(buttons_state&1)) {
            modeChanged = false;
        }
    }
}

/*void update_encoder() {
    encoder_curstate = digitalRead(encoder_pins[0]);

    if (encoder_curstate != encoder_laststate && encoder_curstate == 1) {
        if (digitalRead(encoder_pins[1]) != encoder_curstate) {
            tt_pos += tt_lookup[tt_sensitivity];
        } else {
            tt_pos -= tt_lookup[tt_sensitivity];
        }
    }

    encoder_laststate = encoder_curstate;
}*/
