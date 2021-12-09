
#include "config.h"

#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>
#include "IIDXHID.h"

IIDXHID_ IIDXHID;


Bounce buttons[NUMBER_OF_BUTTONS];

uint32_t last_report = 0;

int32_t tt_pos;
uint8_t tt_sensitivity = 9;
uint8_t tt_lookup[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

const int encoder_cooldown_const = 100;
int encoder_cooldown = 0;
int tt_delta;

int button_state_array[NUMBER_OF_BUTTONS];

bool hid_reactive_autoswitch = true;
bool hid_lights = true;
bool reactive = false;

volatile int g_encoderValue = 0;
volatile unsigned int g_encoderValueVL = 0;
volatile byte g_encoderStVL;
  
void initEncoder()
{
  pinMode( ENCODER_PIN0, INPUT_PULLUP );
  pinMode( ENCODER_PIN1, INPUT_PULLUP );

  byte eA = !IO_READ( ENCODER_PIN0 );
  byte eB = !IO_READ( ENCODER_PIN1 );
  g_encoderStVL = eB<<1 | eA;
  
  setupTimerInterrupt();
}

void computeEncoder()
{
  byte eA = !IO_READ(ENCODER_PIN0);
  byte eB = !IO_READ(ENCODER_PIN1);

  byte st = ((byte)eB<<1) | (byte)eA;

  
    byte encoderSt = g_encoderStVL;
    if (encoderSt != st)
    {
      bool wentDown = (!encoderSt && (st == 2));
      bool wentUp = (!encoderSt && (st == 1));

      if ( wentDown ) g_encoderValueVL--;
      if ( wentUp ) g_encoderValueVL++;
      
      g_encoderStVL = st;
    }
}

int encoder_delta()
{ 
  unsigned int v = g_encoderValueVL;  // put in a local var to avoid multiple reads of a volatile var
  int d = v - g_encoderValue;
  g_encoderValue = v;
  return d; 
}

void setup() {

  //Buttons setup
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
        buttons[i] = Bounce();
        buttons[i].attach(button_pins[i], INPUT_PULLUP);
        buttons[i].interval(MS_DEBOUNCE);
    }

  //Encoder setup 
    initEncoder();

  //LED Setup
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], HIGH);
    }

    delay(200);

    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        digitalWrite(led_pins[i], LOW);
    }

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
   if (encoder_cooldown==0) 
    {
     tt_delta = encoder_delta();
     encoder_cooldown=encoder_cooldown_const;
    }
   else 
    {
     encoder_cooldown--;
     tt_delta=0;
    }
  
   if (tt_delta >= ADJUSTED_PPR /360 || tt_delta <= -ADJUSTED_PPR / 360){
     tt_pos += tt_delta*tt_lookup[tt_sensitivity];
     //Serial.println(tt_pos);
   }
  
   // Limit the encoder from 0 to ADJUSTED_PPR
    if (tt_pos >= ADJUSTED_PPR) {
        tt_pos = 0;
    } else if (tt_pos < 0) {
        tt_pos = ADJUSTED_PPR-1;
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

void setupTimerInterrupt()
{
  TCCR3A = 0; // set entire TCCR3A register to 0
  TCCR3B = 0; // same for TCCR3B
  TCNT3  = 0; // initialize counter value to 0

  // set compare match register (write to the high bit first) 
  OCR3AH = 0;
  
  // set compare match register for particular frequency increments
//  OCR3AL = 133; // = (16000000) / 64 / 2000  -> 133   This is  clock_frequency / prescaler / desired_frequency  ( 2 KHz, 0.5ms)
//  OCR3AL = 50;  // = (16000000) / 64 / 5000  ->  50   This is  clock_frequency / prescaler / desired_frequency  ( 5 KHz, 0.2ms)
//  OCR3AL = 25;  // = (16000000) / 64 / 10000 ->  25   This is  clock_frequency / prescaler / desired_frequency  (10 kHz, 0.1ms)
  OCR3AL = INTERRUPT_PERIOD; 
  
  // enable timer compare interrupt
  TIMSK3 = (1 << OCIE3A);
  
  // turn on mode 4 (CTC mode) (up to OCR3A)
  TCCR3B |= (1 << WGM32);
  
  // Set CS10 and CS12 bits for 64 prescaler
  TCCR3B |= (1 << CS30) | (1 << CS31);

  // More information at
  // http://medesign.seas.upenn.edu/index.php/Guides/MaEvArM-timers 
  //
}

SIGNAL(TIMER3_COMPA_vect) 
{  
  computeEncoder();
}
