#ifndef MEGASYNTH_H__
#define MEGASYNTH_H__

/*

YM2612 pin-out
            ________
GND --- GND|o       |0M ---- D3
D4  ---- D0|        |Vcc --- 5V
D5  ---- D1|        |A.Vcc - 5V
D6  ---- D2|        |OUT L - To Mixer Circuit
D7  ---- D3|        |Out R - To Mixer Circuit
D10 ---- D4|        |A.GND - GND
D11 ---- D5|        |A1 ---- A0
D12 ---- D6|        |A0 ---- A1
D13 ---- D7|        |RD ---- PULLUP (1K?)
NC  ---- NC|        |WR ---- A2
A5  ---- IC|        |CS ---- A4
GND --- GND|________|IRQ --- NC


SN76489AN pin-out
            ____
D11 ---- D5|o   |Vcc --- 5V
D12 ---- D6|    |D4 ---- D10
D13 ---- D7|    |CLK --- D9
PLDWN READY|    |D3 ---- D7
 A2 ---- WE|    |D2 ---- D6
 A3 ---- CE|    |D1 ---- D5
  AUDIO OUT|    |D0 ---- D4
 GND -- GND|____|NC

*/


/*
Arduino pins:
(port)#/(analog/digital)##
*: exclusive use pin
### PORT C ###
C0/A0:    YM_A1
C1/A1:    YM_A0
C2/A2:    YM_WR | SN_WE
C3/A3:  * SN_CE
C4/A4:  * YM_CS
C5/A5:    YM_IC
### PORT D ###
D0/D00: * MIDI_IN
D1/D01: * SERIAL_OUT
D2/D02:
D3/D03: * YM_[phi]M
D4/D04:   YM_D0 | SN_D0
D5/D05:   YM_D1 | SN_D1
D6/D06:   YM_D2 | SN_D2
D7/D07:   YM_D3 | SN_D3
### PORT B ###
B0/D08:
B1/D09: * SN_CLK
B2/D10:   YM_D4 | SN_D4
B3/D11:   YM_D5 | SN_D5
B4/D12:   YM_D6 | SN_D6
B5/D13:   YM_D7 | SN_D7 | LED_PIN

NOTE: YM_RD needs a pull-up.

*/

#include "YM2612.h"
#include "SN76489.h"
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
    private:
    YM2612 ym;
    SN76489 sn;
    static const PROGMEM word noteFreq72[NOTE_COUNT];

    static inline int8_t keyToBlock(byte key) {
        // obtain octave by applying / NOTE_COUNT to key
        // then apply the offset (-1)
        return key / NOTE_COUNT - 1;
    }
    
    
    static inline word keyToFrequency72(byte key) {
        //look up the freq72 for the key, disregarding octave by
        // applying % NOTE_COUNT
        return pgm_read_word(&noteFreq72[key % NOTE_COUNT]);
    }
    
    
    static inline word keyToPeriod125k(byte key) {
        //look up the freq72 for the key, disregarding octave by
        // applying % NOTE_COUNT
        word freq72 = pgm_read_word(&noteFreq72[key % NOTE_COUNT]);
        int8_t octave = key / NOTE_COUNT - 5;
        // noteFreq72 starts at key 60 or octave 5, corrections follow:
        // multiply by 2 for each octave above 5
        // divide by 2 for each octave below 5
        // finally, add half of freq72 to numerator to reduce roundoff errors
        // 125000 is based on 4MHz clock
        long period = ((
                (octave < 0) ? ((125000 * 72) << -octave)
                : (octave > 0) ? ((125000 * 72) >> octave)
                : (125000 * 72)
            ) + (freq72 >> 1)) / freq72;
        while (period > 1023) // period does not fit in 10bits
            period >>= 1; // halve period until it does
        return period;
    }


    inline byte doGlobalCc(byte num, byte val) {
        switch (num) {
            // global CCs
            // DO NOT FORGET: break
            case  1: ym.setGlobal22(YM2612::Field::  LFOFREQ, val); break;
            case 74: ym.setGlobal22(YM2612::Field::    LFOEN, val); break;
            case 80: ym.setGlobal27(YM2612::Field::SPECIALEN, val); break;
            case 92: ym.setGlobal27(YM2612::Field::     T27L, val); break;
            case 93: ym.setGlobal27(YM2612::Field::     T27H, val); break;
            case 94: ym.setGlobal20(YM2612::Field::     T20L, val); break;
            case 95: ym.setGlobal20(YM2612::Field::     T20H, val); break;
            case 96: ym.setGlobal2C(YM2612::Field::     T2CL, val); break;
            case 97: ym.setGlobal2C(YM2612::Field::     T2CH, val); break;

            // valid but unimplemented global CCs:
            case 85: //pitch transposition (YM & SN)
            case 84: //octave division
            case 83: //PAL/NTSC flag - likely never supported
            break;

            default:
            return 0; //did not handle CC
            break;
        }
        return 1; //handled CC
    }
    inline byte doYmCc(byte channel, byte num, byte val) {
        if (channel > YM2612::CHAN_COUNT) //sanity check 
            return 0; //did not handle CC
        //now that channel is sane, cast it to the channel enum:
        YM2612::channel_e c = static_cast<YM2612::channel_e>(channel);
        switch (num) {
            // channel CCs
// SHORTCUT:            
#define YM_CHANNEL_CC(cc, field) \
            case cc: \
            ym.setChannel(c, YM2612::Field:: field, val); \
            break

            YM_CHANNEL_CC(14, ALGO);
            YM_CHANNEL_CC(15, FB);
            YM_CHANNEL_CC(77, LR);
            YM_CHANNEL_CC(76, AMS);
            YM_CHANNEL_CC(75, PMS);
#undef YM_CHANNEL_CC

            // valid but unimplemented channel CCs
            case  6: //instrument store
            case  9: //instrument recall
            case 81: //pitch bend sensitivity (SN only?)
            break;

            // slotCCs
// SHORTCUT:            
#define YM_SLOT_CC(cc, slot, field) \
            case cc: \
            ym.setSlot(c, YM2612:: slot, YM2612::Field:: field, val); \
            break
            
            YM_SLOT_CC(90, SLOT1, SSEG);
            YM_SLOT_CC(91, SLOT2, SSEG);
            YM_SLOT_CC(92, SLOT3, SSEG);
            YM_SLOT_CC(93, SLOT4, SSEG);

            YM_SLOT_CC(16, SLOT1, TL);
            YM_SLOT_CC(17, SLOT2, TL);
            YM_SLOT_CC(18, SLOT3, TL);
            YM_SLOT_CC(19, SLOT4, TL);

            YM_SLOT_CC(20, SLOT1, MULTI);
            YM_SLOT_CC(21, SLOT2, MULTI);
            YM_SLOT_CC(22, SLOT3, MULTI);
            YM_SLOT_CC(23, SLOT4, MULTI);

            YM_SLOT_CC(24, SLOT1, DT);
            YM_SLOT_CC(25, SLOT2, DT);
            YM_SLOT_CC(26, SLOT3, DT);
            YM_SLOT_CC(27, SLOT4, DT);

            YM_SLOT_CC(39, SLOT1, KS);
            YM_SLOT_CC(40, SLOT2, KS);
            YM_SLOT_CC(41, SLOT3, KS);
            YM_SLOT_CC(42, SLOT4, KS);

            YM_SLOT_CC(43, SLOT1, AR);
            YM_SLOT_CC(44, SLOT2, AR);
            YM_SLOT_CC(45, SLOT3, AR);
            YM_SLOT_CC(46, SLOT4, AR);


            YM_SLOT_CC(47, SLOT1, DR);
            YM_SLOT_CC(48, SLOT2, DR);
            YM_SLOT_CC(49, SLOT3, DR);
            YM_SLOT_CC(50, SLOT4, DR);

            YM_SLOT_CC(51, SLOT1, SR);
            YM_SLOT_CC(52, SLOT2, SR);
            YM_SLOT_CC(53, SLOT3, SR);
            YM_SLOT_CC(54, SLOT4, SR);

            YM_SLOT_CC(55, SLOT1, SL);
            YM_SLOT_CC(56, SLOT2, SL);
            YM_SLOT_CC(57, SLOT3, SL);
            YM_SLOT_CC(58, SLOT4, SL);

            YM_SLOT_CC(59, SLOT1, RR);
            YM_SLOT_CC(60, SLOT2, RR);
            YM_SLOT_CC(61, SLOT3, RR);
            YM_SLOT_CC(62, SLOT4, RR);

            YM_SLOT_CC(70, SLOT1, AM);
            YM_SLOT_CC(71, SLOT2, AM);
            YM_SLOT_CC(72, SLOT3, AM);
            YM_SLOT_CC(73, SLOT4, AM);
#undef YM_SLOT_CC
            
            default:
            return 0; //did not handle CC
            break;
        }
        return 1; //handled CC        
    }

    public:
    void begin() {
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
        
        ym.begin();
        sn.begin();
    }

    void noteOn(byte channel, byte key, byte velocity) {
        if (channel <= 5 || (10 <= channel && channel <= 12 )) {
            ym.frequency72(channel, keyToBlock(key), keyToFrequency72(key));
            if (channel <= 5) {
                ym.level(channel, velocity);
                //kill existing notes -- is this what we want?
                ym.setOperators(channel, 0);
                ym.setOperators(channel, bit(YM2612::SLOT1) | bit(YM2612::SLOT2) | bit(YM2612::SLOT3) | bit(YM2612::SLOT4)); //enable ALL the operators
            }            
        } else if (6 <= channel && channel <= 9) {
            if (channel == 9) {
                key %= NOTE_COUNT;
                enum SN76489::feedback_e fb;
                enum SN76489::rate_e shift;
                if (key == NOTE_C
                    || key == NOTE_Cs
                    || key == NOTE_D
                    || key == NOTE_Ds
                    || key == NOTE_E
                    || key == NOTE_A
                    || key == NOTE_As) {
                    fb = SN76489::PERIODIC_NOISE;
                }
                else {
                    fb = SN76489::WHITE_NOISE;
                }
                if (key == NOTE_C
                    || key == NOTE_Cs
                    || key == NOTE_F) {
                    shift = SN76489::SHIFT_512;
                }
                else if (key == NOTE_D
                    || key == NOTE_Ds
                    || key == NOTE_Fs) {
                    shift = SN76489::SHIFT_1024;
                }
                else if (key == NOTE_E
                    || key == NOTE_G
                    || key == NOTE_Gs) {
                    shift = SN76489::SHIFT_2048;
                }
                else {
                    shift = SN76489::SHIFT_CHAN3;
                }
                sn.setNoise(fb, shift);
            } else {                
                sn.setPeriod125k(static_cast<SN76489::channel_e>(channel - 6), keyToPeriod125k(key));
            }                
            sn.level(static_cast<SN76489::channel_e>(channel - 6), velocity);
        }
    }


    void noteOff(byte channel) {
        if (channel <= 5) {
            ym.setOperators(channel, 0); //disable ALL the operators
        } else if (6 <= channel && channel <= 9) {
            sn.level(static_cast<SN76489::channel_e>(channel - 6), 0);
        }
    }
    
    
    void continuousController(byte channel, byte ccnum, byte ccval) {
        byte done = 0;
        if (!done) {
            done = doGlobalCc(ccnum, ccval);
        }
        if (!done) {
            done = doYmCc(channel, ccnum, ccval);
        }
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
                packet[MIDI_KEY_INDEX],
                packet[MIDI_VELOCITY_INDEX]);
            break;
            
            case MIDI_CONTCONTROL:
            continuousController(
                toMidiTarget(packet[MIDI_STATUS_INDEX]),
                packet[MIDI_CCNUM_INDEX],
                packet[MIDI_CCVAL_INDEX]);
            break;
            
            default:
            break;
        }        
    }
};


#ifndef EQUAL_TEMPERAMENT_A4
#define EQUAL_TEMPERAMENT_A4 440.0
#endif
// A precise calculation for the 12th root of two:
#define ROOT12_2 1.0594630943592952645618252949463
// Equal temperament uses the 12th root of two as the half-step scaler.
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
const PROGMEM word MegaSynth::noteFreq72[MegaSynth::NOTE_COUNT] = 
{
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
