#ifndef MEGASYNTH_H__
#define MEGASYNTH_H__

#include "YM2612.h"
#include "tables.h"
#include "midiParser.h"


//rate for MIDI bridge software (for historical reasons this number is a multiple of 300)
#define MIDI_SOFTWARE_BAUDRATE 38400
//standard MIDI rate (1MHz/32, easier for embedded systems)
#define MIDI_NATIVE_BAUDRATE 31250


class MegaSynth {
    YM2612 ym;
    
    public:
    void begin() {
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
        ym.begin();
    }


    void noteOn(byte channel, byte key, byte velocity) {
        if (channel <= 5 || (10 <= channel && channel <= 12 )) {
            ym.frequency(channel, pgm_read_word(&PITCHTABLE[key])); //use pitch table in tables.h to get the appropriate pitch word
            if (channel <= 5) {
                ym.level(channel, velocity);
                ym.setOperators(channel, bit(YM2612_SLOT1) | bit(YM2612_SLOT2) | bit(YM2612_SLOT3) | bit(YM2612_SLOT4)); //enable ALL the operators
            }            

        }
    }


    void noteOff(byte channel) {
        if (channel <= 5)
            ym.setOperators(channel, 0); //disable ALL the operators
    }


    void parsePacket(const byte *packet) {
        byte command = toMidiCommand(packet[MIDI_STATUS_INDEX]);
        byte channel = toMidiTarget(packet[MIDI_STATUS_INDEX]);
        if (command == MIDI_NOTEOFF)
            noteOff(channel);
        else if (command == MIDI_NOTEON)
            noteOn(channel, packet[MIDI_KEY_INDEX], packet[MIDI_VELOCITY_INDEX]);
    }
};




//include guard
#endif
