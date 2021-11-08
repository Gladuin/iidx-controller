#include <Arduino.h>
#include "Encoder.h"


EncoderInterruptClass EncoderInterrupt;
void computeEncoder();

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
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

#elif  defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
void setupTimerInterrupt()
{
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  
  // set compare match register for particular frequency increments
//  OCR2A = 133; // = (16000000) / 64 / 2000  -> 133   This is  clock_frequency / prescaler / desired_frequency  ( 2 KHz, 0.5ms)
//  OCR2A = 50;  // = (16000000) / 64 / 5000  ->  50   This is  clock_frequency / prescaler / desired_frequency  ( 5 KHz, 0.2ms)
//  OCR2A = 25;  // = (16000000) / 64 / 10000 ->  25   This is  clock_frequency / prescaler / desired_frequency  (10 kHz, 0.1ms)
  OCR2A = INTERRUPT_PERIOD;
  
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  
  // turn on mode 2 (CTC mode) (up to OCR2A)
  TCCR2A |= (1 << WGM21);
  
  // Set CS22 bit for 64 prescaler
  TCCR2B |= (1 << CS22);
}

SIGNAL(TIMER2_COMPA_vect)
{
  computeEncoder();
}

/*  Alternative using timer 1 (Unusable pin 10, 11 for PWM)
void setupTimerInterrupt()
{
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
 
  OCR1AH = 0;
  // set compare match register for particular frequency increments
//  OCR1AL = 133; // = (16000000) / 64 / 2000  -> 133   This is  clock_frequency / prescaler / desired_frequency  ( 2 KHz, 0.5ms)
//  OCR1AL = 50;  // = (16000000) / 64 / 5000  ->  50   This is  clock_frequency / prescaler / desired_frequency  ( 5 KHz, 0.2ms)
//  OCR1AL = 25;  // = (16000000) / 64 / 10000 ->  25   This is  clock_frequency / prescaler / desired_frequency  (10 kHz, 0.1ms)
  OCR1AL = INTERRUPT_PERIOD;
 
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
 
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
 
  // Set CS10 and CS12 bits for 64 prescaler
  TCCR1B |= (1 << CS10) | (1 << CS11);
}

SIGNAL(TIMER1_COMPA_vect)
{
  computeEncoder();
}
*/
#else
#error "Unsupported or unknown board"
#endif




/*  Alternative using timer 0 that is used for millis()
void setupTimerInterrupt()
{

// Timer0 is already used for millis() - we'll just interrupt somewhere
// in the middle and call the "Compare A" function below

// per defecte el temporitzador de millis() esta configurat per fer overflow (de 256) cada milisegon (de fet cada 1.024 ms)
// ho configurem per tenir interrupcions al comenzament i a la meitat, es a dir obtenim una interrupcio mes o menys cada 0.5 ms

// by default the millis() timer is configured to overflow (from 256) every millisecond ( actually every 1.024 ms)
// we configure it to have interruptps at the beginning and half the period, so we effectivelly get an interrupt every 0.5 ms
  OCR0A = 0x01;      // call at the beginiing
  TIMSK0 |= _BV(OCIE0A);

  OCR0B = 0x80;     // call at half the period
  TIMSK0 |= _BV(OCIE0B);
}

SIGNAL(TIMER0_COMPA_vect) 
{  
  computeEncoder();
}
*/

void Encoder::init()
{
  pinMode( _pinA, INPUT_PULLUP );
  pinMode( _pinB, INPUT_PULLUP );
  pinMode( _pinP, INPUT_PULLUP );

  byte eA = !digitalRead( _pinA );
  byte eB = !digitalRead( _pinB );
  _encoderStVL = eB<<1 | eA;
}

void Encoder::compute()
{
  static Debouncer_encoder stDebouncer, pDebouncer;

  byte eA = !digitalRead( _pinA );
  byte eB = !digitalRead( _pinB );
  bool pb = !digitalRead( _pinP ); 

  byte st = ((byte)eB<<1) | (byte)eA;

  if ( stDebouncer.isDebounced( st, DEBOUNCE_COUNT ) )
  {
    byte encoderSt = _encoderStVL;
    if (encoderSt != st)
    {
      bool wentDown = (!encoderSt && (st == 2));
      bool wentUp = (!encoderSt && (st == 1));

      if ( wentDown ) _encoderValueVL--;
      if ( wentUp ) _encoderValueVL++;

      /* if ( st == 0 || st == 3 )
      {
        if ( wentDown ) _encoderTickVL--;
        else _encoderTickVL++;
      }*/
      _encoderTickVL = _encoderValueVL/2 ;

      /*if ( st == 0 )
      {
        if ( wentDown ) _encoderTick2VL--;
        else _encoderTick2VL++;
      }*/
      _encoderTick2VL = _encoderTickVL/2;
      
      _encoderStVL = st;
    }
  }

  if ( pDebouncer.isDebounced( pb, DEBOUNCE_COUNT ) && _encoderButtonVL != pb )
  {
    _encoderButtonVL = pb ;
  }
}

int Encoder::delta()
{ 
   unsigned int v = _encoderValueVL;  // put in a local var to avoid multiple reads of a volatile var
   int d = v - _encoderValue;
  _encoderValue = v;
  return d; 
}

int Encoder::deltaTick()
{
   unsigned int v = _encoderTickVL;  // put in a local var to avoid multiple reads of a volatile var
   int d = v - _encoderTick;
   _encoderTick = v;
   return d; 
}

int Encoder::deltaTick2()
{
   unsigned int v = _encoderTick2VL;  // put in a local var to avoid multiple reads of a volatile var
   int d = v - _encoderTick2;
   _encoderTick2 = v;
   return d;
}

void EncoderInterruptClass::begin( Encoder **encoderRefs, int numEncoders)
{
   _encoderArray = encoderRefs;
   _numEncoders = numEncoders;
   for ( int i=0 ; i<_numEncoders ; i++ ) _encoderArray[i]->init();

   setupTimerInterrupt();
}

void EncoderInterruptClass::computeAll()
{
  for ( int i=0 ; i<_numEncoders ; i++ )
  {
    _encoderArray[i]->compute();
  }
}

void computeEncoder()
{
  EncoderInterrupt.computeAll();
}

bool Debouncer_encoder::isDebounced( byte value, byte debounceCount)
{
  if ( _keepValue != value ) 
  {
      _bounce = debounceCount;  
      _keepValue = value;
  }
  else if ( _bounce > 0 ) _bounce-- ;
  
  return _bounce == 0;
}
