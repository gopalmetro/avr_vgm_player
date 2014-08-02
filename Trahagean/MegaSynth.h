#ifndef MEGASYNTH_H__
#define MEGASYNTH_H__

#include "YM2612.h"
#include "tables.h"
#include "midiPacketizer.h"


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


    void parseMidiPacket(const byte *packet) {
        if (packet == NULL) // does the packet exist?
            return; // abort!
        switch (toMidiCommand(packet[MIDI_STATUS_INDEX])) {
            case MIDI_NOTEOFF:
            noteOff(toMidiTarget(packet[MIDI_STATUS_INDEX]));
            break;

            case MIDI_NOTEON:
            noteOn(
                toMidiTarget(packet[MIDI_STATUS_INDEX]),
                packet[MIDI_KEY_INDEX], packet[MIDI_VELOCITY_INDEX]);
            break;
            
            default:
            break;
        }        
    }
};




//include guard
#endif
