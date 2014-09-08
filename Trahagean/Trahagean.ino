/* YM2612 pinout
            ________
GND --- GND|o       |0M ---- 
D2  ---- D0|        |Vcc --- 
D3  ---- D1|        |A.Vcc - 
D4  ---- D2|        |OUT L - 
D5  ---- D3|        |Out R - 
D6  ---- D4|        |A.GND - GND
D7  ---- D5|        |A1 ---- A0
D12 ---- D6|        |A0 ---- A1
D11 ---- D7|        |RD ---- A2
NC  ---- NC|        |WR ---- A3
A5  ---- IC|        |CS ---- A4
GND --- GND|________|IRQ

*/


/* Dependencies */
#include <avr/sleep.h>
#include "Arduino.h"

// flag for MegaSynth to dump freqs for verification purposes
//#define DUMP_FREQS
// initial tuning (default is 440)
//#define EQUAL_TEMPERAMENT_A4 440.0
#include "MegaSynth.h"

#define USE_QD_PACKETIZER

#define BAUDRATE MIDI_NATIVE_BAUDRATE
//#define BAUDRATE MIDI_SOFTWARE_BAUDRATE


//rate for MIDI bridge software (for historical reasons this number is a multiple of 300)
#define MIDI_SOFTWARE_BAUDRATE 38400
//standard MIDI rate (1MHz/32, easier for embedded systems)
#define MIDI_NATIVE_BAUDRATE 31250


MegaSynth synth;


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
		delay(LEDHighTime);
		digitalWrite(LED_BUILTIN, LOW);
		delay(LEDLowTime);
	}
}


void setup() {
    /* activate MIDI Serial input - Gopal */
    Serial.begin(BAUDRATE);
    synth.begin();
    delay(200);
    blinkTest(3,200,200);
    //blinkTest(3,400,200);
    //blinkTest(3,200,200);
    set_sleep_mode(SLEEP_MODE_IDLE);
}


void loop() {
    sleep_enable();
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

