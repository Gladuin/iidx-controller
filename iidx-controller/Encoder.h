#ifndef Encoder_H
#define Encoder_H

class Encoder
{

public:
  Encoder( byte pinA, byte pinB, byte pinP ) : 
    _pinA( pinA ), _pinB( pinB ), _pinP( pinP ),
    _encoderValueVL(0), _encoderTickVL(0), _encoderTick2VL(0),
    _encoderButtonVL(false),
    _encoderValue(0), _encoderTick(0) {}

public:
  int delta();
  int deltaTick();
  int deltaTick2();
  bool button() const { return _encoderButtonVL; }

private:
  void init();
  void compute();

private:
  volatile unsigned int _encoderValueVL = 0;
  volatile unsigned int _encoderTickVL = 0;
  volatile unsigned int _encoderTick2VL = 0;
  volatile bool _encoderButtonVL = false;
  volatile byte _encoderStVL;
  unsigned int _encoderValue = 0;
  unsigned int _encoderTick = 0;
  unsigned int _encoderTick2 = 0;
  byte _pinA, _pinB, _pinP;

public:
  friend class EncoderInterruptClass;
};

class EncoderInterruptClass
{
public:
  EncoderInterruptClass() : _numEncoders(0) {}

public:
   void begin( Encoder **encoderRefs, int numEncoders);
   inline void begin( Encoder *encoder) { begin( &encoder, 1 ); }

private:
   int _numEncoders;
   Encoder **_encoderArray;
   void computeAll();

public:
  friend void computeEncoder();
};

class Debouncer_encoder
{
  private:
    byte _keepValue;
    byte _bounce;
    
  public:
    Debouncer_encoder():_keepValue(0), _bounce(0) {}
    bool isDebounced( byte value, byte debounceCount);
};

#define DEBOUNCE_COUNT 3    // This is the number of consecutive reads that must be the same before accepting a pin change
#define INTERRUPT_PERIOD 25 // This is   cpu_clock_frequency / prescaler / desired_interrupt_frequency
                            // for example, set to 133 for 0.5ms (2kHz)  on a 16MHz processor: (16000000L / 64 / 2000  -> 133
                            // for example, set to 50  for 0.2ms (5kHz)  on a 16MHz processor: (16000000L / 64 / 5000  -> 50
                            // for example, set to 25  for 0.1ms (10kHz) on a 16MHz processor: (16000000L / 64 / 10000 -> 25

extern EncoderInterruptClass EncoderInterrupt;



/* USAGE FOR ONE ENCODER:

// define the input pins
#define pinA 19
#define pinB 20
#define pinP 21

// create an encoder object initialized with their pins
Encoder encoder( pinA, pinB, pinP );

// setup code
void setup() 
{
  // start the encoder time interrupts
  EncoderInterrupt.begin( &encoder );
}

// loop code
void loop()
{
  // read the debounced value of the encoder button
  bool pb = encoder.button();

  // get the encoder variation since our last check, it can be positive or negative, or zero if the encoder didn't move
  // only call this once per loop cicle, or at any time you want to know any incremental change
  int delta = encoder.delta();

  // add the delta value to the variable we are controlling
  myEncoderControlledVariable += delta;

  // do stuff with the updated value

  // that's it
}

*/  


/* USAGE FOR SEVERAL ENCODERS:

// create a number of encoder objects initialized with their pins
Encoder encoder1( 19, 20, 21 );
Encoder encoder2( 4, 5, 6 );

// put their references in an array
Encoder *encoderArray[] = { &encoder1, &encoder2 };

// setup code
void setup() 
{
  // start the encoder time interrupts
  EncoderInterrupt.begin( encoderArray, 2 );
}

// loop code
void loop()
{
  // read the debounced value of the encoder buttons
  bool pb1 = encoder1.button();
  bool pb2 = encoder2.button();

  // get the encoders deltas (can be positive or negative)
  int deltaValue1 = encoder1.delta();
  int deltaValue2 = encoder2.delta();

  // add the deltas value to the variables you are controlling
  myEncoderControlledVariable1 += delta1;
  myEncoderControlledVariable2 += delta2;

  // do stuff with the updated values

  // that's it
}

*/  


#endif
