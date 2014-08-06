#ifndef MEGASYNTH_H__
#define MEGASYNTH_H__

#include "YM2612.h"
#include "midiPacketizer.h"


class MegaSynth {
    public:
    enum note_e {
        NOTE_C,
        NOTE_Cs,
        NOTE_Df = NOTE_Cs,
        NOTE_D,
        NOTE_Ds,
        NOTE_Ef = NOTE_Ds,
        NOTE_E,
        NOTE_F,
        NOTE_Fs,
        NOTE_Gf = NOTE_Fs,
        NOTE_G,
        NOTE_Gs,
        NOTE_Af = NOTE_Gs,
        NOTE_A,
        NOTE_As,
        NOTE_Bf = NOTE_As,
        NOTE_B,
        NOTE_COUNT
    };
    static const int8_t YM_BLOCK_OFFSET = -1;
    static const int8_t YM_BLOCK_MAX = 7;

    private:
    YM2612 ym;
    static const PROGMEM word noteFreq72[NOTE_COUNT];

    static inline byte keyToBlock(byte key) {
        byte block = key / NOTE_COUNT;
        //apply the offset if it produces a positive value
        if (block + YM_BLOCK_OFFSET >= 0)
            block += YM_BLOCK_OFFSET;
        if (block > YM_BLOCK_MAX) //if block is too big, cap it
            block = YM_BLOCK_MAX;
        return block;
    }
    static inline word keyToFrequency72(byte key) {
        return pgm_read_word(&noteFreq72[key % 12]);
    }
    public:
    void begin() {
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
        ym.begin();
#ifdef DUMP_FREQS
        for (byte k = 0; k < 128; k++) {
            Serial.print(k);
            Serial.print(", ");
            //this is exactly the same logic in YM2612::frequency72()
            Serial.println(
                (keyToBlock(k) << 11)
                    | (((uint32_t)(keyToFrequency72(k)) << 9) / 15625),
                HEX);
        }
#endif
    }

    void noteOn(byte channel, byte key, byte velocity) {
        if (channel <= 5 || (10 <= channel && channel <= 12 )) {
            ym.frequency72(channel, keyToBlock(key), keyToFrequency72(key));
            if (channel <= 5) {
                ym.level(channel, velocity);
                ym.setOperators(channel, bit(YM2612::SLOT1) | bit(YM2612::SLOT2) | bit(YM2612::SLOT3) | bit(YM2612::SLOT4)); //enable ALL the operators
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


#ifndef EQUAL_TEMPERAMENT_A4
#define EQUAL_TEMPERAMENT_A4 440.0
#endif
#define ROOT12_2 1.0594630943592952645618252949463
// Equal temperament uses the 12th root of two a the half-step scaler.
// There's no pow() in the preprocessor, so multiply or divide N times.
// For example, to get from A4 to G4 scale down A4 twice with division,
// as G4 is two half-steps (one whole-step) below A4:
// G4 = A4 / ROOT12_2 / ROOT12_2
// To scale up, multiply repeatedly the scalar by the desired number of 
// half steps.
#define EQUAL_TEMPERAMENT_C4 \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2 \
        / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_Cs4 \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2 \
        / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_D4  \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2 \
        / ROOT12_2 / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_Ds4 \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2 \
        / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_E4  \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2 \
        / ROOT12_2)
#define EQUAL_TEMPERAMENT_F4  \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_Fs4 \
    (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_G4  (EQUAL_TEMPERAMENT_A4 / ROOT12_2 / ROOT12_2)
#define EQUAL_TEMPERAMENT_Gs4 (EQUAL_TEMPERAMENT_A4 / ROOT12_2)
#define EQUAL_TEMPERAMENT_As4 (EQUAL_TEMPERAMENT_A4 * ROOT12_2)
#define EQUAL_TEMPERAMENT_B4  (EQUAL_TEMPERAMENT_A4 * ROOT12_2 * ROOT12_2)
//scale by 72 to get higher integer precision
const PROGMEM word MegaSynth::noteFreq72[MegaSynth::NOTE_COUNT] {
    (word)(72.0 * EQUAL_TEMPERAMENT_C4),
    (word)(72.0 * EQUAL_TEMPERAMENT_Cs4),
    (word)(72.0 * EQUAL_TEMPERAMENT_D4),
    (word)(72.0 * EQUAL_TEMPERAMENT_Ds4),
    (word)(72.0 * EQUAL_TEMPERAMENT_E4),
    (word)(72.0 * EQUAL_TEMPERAMENT_F4),
    (word)(72.0 * EQUAL_TEMPERAMENT_Fs4),
    (word)(72.0 * EQUAL_TEMPERAMENT_G4),
    (word)(72.0 * EQUAL_TEMPERAMENT_Gs4),
    (word)(72.0 * EQUAL_TEMPERAMENT_A4),
    (word)(72.0 * EQUAL_TEMPERAMENT_As4),
    (word)(72.0 * EQUAL_TEMPERAMENT_B4)
};


//include guard
#endif
