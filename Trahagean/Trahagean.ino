/* Dependencies */
#include <avr/sleep.h>
#include "Arduino.h"
#include "MegaSynth.h"

#define BAUDRATE MIDI_NATIVE_BAUDRATE
//#define BAUDRATE MIDI_SOFTWARE_BAUDRATE

MegaSynth synth;
MidiParser parser;


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


void serialEvent() { // Serial port triggers this when there is data waiting
    const byte *packet = NULL;
    packet = parser.parseByte(Serial.read());
    if (packet != NULL) {
        synth.parsePacket(packet);
    }
}
