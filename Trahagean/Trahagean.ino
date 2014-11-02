/* Dependencies */
#include "Arduino.h"
#include "toggle.h"
#include <avr/delay.h>

// flag for MegaSynth to dump freqs for verification purposes
//#define DUMP_FREQS
// initial tuning (default is 440)
//#define EQUAL_TEMPERAMENT_A4 440.0
#include "MegaSynth.h"

MegaSynth synth;

// SAW WAVE DEMO
volatile byte saw_sample[14] = {
    0x00, 0x09,
    0x12, 0x1B,
    0x24, 0x2D,
    0x36, 0x3F,
    0x48, 0x51,
    0x5A, 0x63,
    0x6C, 0x75
};

// SQUARE WAVE DEMO
volatile byte square_sample[14] = {
    0x00,0x00,
    0x00,0x00,
    0x00,0x00,
    0x00,0x7F,
    0x7F,0x7F,
    0x7F,0x7F,
    0x7F,0x7F
};

volatile byte pos = 0;

ISR(TIMER1_COMPA_vect)
{
//    YM2612::setPcmSample(saw_sample[pos]); //doesn't work??
//    pos = (pos + 1) % 14;
    if (!(pos = (pos + 1) % 7)) { //square wave on D13 (LED)
        PORTB ^= bit(PORTB5);
    }    
}

void setUpSample() {
    YM2612::setPcm(1);
    /*
         Per http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/ 
        "Timer prescaling and CTC"
    */
    // initialize Timer1
    noInterrupts();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    // set compare match register to desired timer count:
    OCR1A = 2596; // per the Timer table for 14 samples at 440Hz (6160Hz Sampling)
    // set WGM12 bit for CTC mode and set CS10 bit for /1 prescaler:
    TCCR1B = bit(WGM12) | bit(CS10);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    interrupts();          // enable global interrupts
}    

//#define USE_QD_PACKETIZER
#define BAUDRATE MIDI_NATIVE_BAUDRATE
//#define BAUDRATE MIDI_SOFTWARE_BAUDRATE


//rate for MIDI bridge software (for historical reasons this number is a multiple of 300)
#define MIDI_SOFTWARE_BAUDRATE 38400
//standard MIDI rate (1MHz/32, easier for embedded systems)
#define MIDI_NATIVE_BAUDRATE 31250


//Pin map for data pins is a bit complicated:
//DATA_BUS_D0 through DATA_BUS_D1 map to PORTD pins 6-7 -- Uno digital pins 6-7
//DATA_BUS_D2 through DATA_BUS_D7 map to PORTB pins 0-5 -- Uno digital pins 8-13
#define DATA_BUS_PORTD_MASK B11000000
#define DATA_BUS_PORTD_BITBANG(b) ((b) << 6)

#define DATA_BUS_PORTB_MASK B00111111
#define DATA_BUS_PORTB_BITBANG(b) ((b) >> 2)

#if !defined(DATA_BUS_PORTB_MASK) \
    && !defined(DATA_BUS_PORTC_MASK) \
    && !defined(DATA_BUS_PORTD_MASK)
#error "define at least one DATA_BUS_PORT[BCD]_MASK"
#endif
#if (defined(DATA_BUS_PORTB_MASK) && !defined(DATA_BUS_PORTB_BITBANG))
#error "DATA_BUS_PORTB_MASK defined without DATA_BUS_PORTB_BITBANG(b)"
#endif
#if (defined(DATA_BUS_PORTC_MASK) && !defined(DATA_BUS_PORTC_BITBANG))
#error "DATA_BUS_PORTC_MASK defined without DATA_BUS_PORTC_BITBANG(b)"
#endif
#if (defined(DATA_BUS_PORTD_MASK) && !defined(DATA_BUS_PORTD_BITBANG))
#error "DATA_BUS_PORTD_MASK defined without DATA_BUS_PORTD_BITBANG(b)"
#endif

void dataBusWrite(byte data) {
#ifdef DATA_BUS_PORTB_MASK
    PORTB = (PORTB & ~(DATA_BUS_PORTB_MASK)) | (DATA_BUS_PORTB_BITBANG(data) & DATA_BUS_PORTB_MASK);
#endif
#ifdef DATA_BUS_PORTC_MASK
    PORTC = (PORTC & ~(DATA_BUS_PORTC_MASK)) | (DATA_BUS_PORTC_BITBANG(data) & DATA_BUS_PORTC_MASK);
#endif
#ifdef DATA_BUS_PORTD_MASK
    PORTD = (PORTD & ~(DATA_BUS_PORTD_MASK)) | (DATA_BUS_PORTD_BITBANG(data) & DATA_BUS_PORTD_MASK);
#endif
}

#ifdef USE_QD_PACKETIZER
// Quick and dirty packetizer
// It doesn't use any MACROS and can be plopped in anywhere
// Technical limitations:
// * Packets smaller than size 3 will get hung until the next command comes in
// * Only the first two data bytes of a 0xF0 command will get through
// * No sanity checks for wrong sized packets (reason behind first limitation)
void qdMidiPacketizer(void (*midiPacketParser)(const byte *), int inByte){
    static byte packet[3];
    static byte count = 0; // no bytes read yet
    if (inByte < 0) {  // is inByte an error?
        return; //abort!
    }
    if (bitRead(inByte, 7) == 1) { // MSB set = status byte, new packet start
        if (count > 0) { // is there an existing packet?
            midiPacketParser(packet); //PARSE THAT PACKET
        }
        count = 1; // inByte is new packet's first byte
        packet[0] = inByte; // new status byte
    }
    else if (count >= 1) { // has a packet been started?
        // append data byte to packet
        // adding inByte immediately increases count by 1
        // but first use stale value since it's already array index: count - 1
        packet[count] = inByte;
        ++count; //increment after using the stale value
        if (count == 3) { // process a full packet immediately
            midiPacketParser(packet); //PARSE THAT PACKET
            count = 0; // reset packet
        }
    }
}
#else
MidiPacketizer packetizer;
#endif

void blinkTest(byte numBlinks = 1, word LEDHighTime = 50, word LEDLowTime = 50) {
	for (int i = 0; i < numBlinks; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
        for (word i = LEDHighTime; i > 0; i--) {
            _delay_ms(1);
        }        
		digitalWrite(LED_BUILTIN, LOW);
        for (word i = LEDLowTime; i > 0; i--) {
            _delay_ms(1);
        }
	}
}


void setup() {
    /* activate MIDI Serial input - Gopal */
    Serial.begin(BAUDRATE);
        
    //SN Clock on Uno's D5
    toggle_OC0B(4000000.0); // 4MHz
    pinMode(5, OUTPUT);

    //YM Clock on Uno's D3
    toggle_OC2B(8000000.0); // 8MHz
    pinMode(3, OUTPUT);

#ifdef DATA_BUS_PORTB_MASK
    DDRB |= DATA_BUS_PORTB_MASK;
#endif
#ifdef DATA_BUS_PORTC_MASK
    DDRC |= DATA_BUS_PORTC_MASK;
#endif
#ifdef DATA_BUS_PORTD_MASK
    DDRD |= DATA_BUS_PORTD_MASK;
#endif

    pinMode(LED_BUILTIN, OUTPUT);
    
    synth.begin();
    _delay_ms(200);
    setUpSample();
    blinkTest(3,200,200);
    //blinkTest(3,400,200);
    //blinkTest(3,200,200);
}


void loop() {
    // nothing! -- see serialEvent() instead
    // do not sleep here because it adds latency
}


#ifdef USE_QD_PACKETIZER
// ideally this would use a base class/interface instead of function pointer
void qdHelper(const byte *packet) {
    //put your packet parsers here:
    synth.parseMidiPacket(packet);
}


void serialEvent() { // Serial port triggers this when there is data waiting
    qdMidiPacketizer(qdHelper, Serial.read());
}
#else
void serialEvent() { // Serial port triggers this when there is data waiting
    const byte *packet = NULL;
    packet = packetizer.receive(Serial.read());
    //put your packet parsers here:
    synth.parseMidiPacket(packet);
}
#endif






