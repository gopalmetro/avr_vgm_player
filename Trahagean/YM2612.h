#ifndef YM2612_H__
#define YM2612_H__

#include "Arduino.h"
#include "YM2612_addr.h"

/* Pin map (Arduino UNO compatible) */
#define YM_IC (5) // PC5 (= pin A5 for Arduino UNO)
#define YM_CS (4) // PC4 (= pin A4 for Arduino UNO)
#define YM_WR (3) // PC3 (= pin A3 for Arduino UNO)
#define YM_RD (2) // PC2 (= pin A2 for Arduino UNO)
#define YM_A0 (1) // PC1 (= pin A1 for Arduino UNO)
#define YM_A1 (0) // PC0 (= pin A0 for Arduino UNO)
#define YM_CTRL_DDR DDRC
#define YM_CTRL_PORT PORTC
#define YM_DATA_DDR DDRD
#define YM_DATA_PORT PORTD // Whole PORT D for data bus (= pins D0 to D7 for Arduino UNO)
#define YM_MCLOCK (1) // PB1 = OC1A (= pin D9 for Arduino UNO)
#define YM_MCLOCK_DDR DDRB

//PD0 & PD1 for serial i/o, using PD2-PD7 as YM 0-5
#define DDATA(d) ((d << 2) & B11111100)
// PB2 PB3 as YM 6-7
#define BDATA(d) ((d >> 3) & B00011000)


#define slotMem(name, channame, slotname) \
    (state.channelMem[toValue(channame)].slotReg[toValue(slotname)][toIndex(name)])
#define toUpdatedSlotMem(name, channame, slotname, v) \
    (slotMem(name, channame, slotname) \
        = mergeRegByteNamedValue(slotMem(name, channame, slotname), name, v))
#define setStatefulSlotReg(name, channame, slotname, v) \
    setreg( \
        toSlotReg(name, channame, slotname), \
        toUpdatedSlotMem(name, channame, slotname, v), \
        toPart(channame))

#define chanMem(name, channame) \
    (state.channelMem[toValue(channame)].channelReg[toIndex(name)])
#define toUpdatedChanMem(name, channame, v) \
    (chanMem(name, channame) \
        = mergeRegByteNamedValue(chanMem(name, channame), name, v))
#define setStatefulChanReg(name, channame, v) \
    setreg( \
        toChanReg(name, channame), \
        toUpdatedChanMem(name, channame, v), \
        toPart(channame))


// pardon the do while(0) workaround (it's commonplace with macro "magic")
#define defaultVoice(channame) do { \
    setStatefulSlotReg(   DT, channame, SLOT1,  7); \
    setStatefulSlotReg(MULTI, channame, SLOT1,  1); \
    setStatefulSlotReg(   DT, channame, SLOT2,  3); \
    setStatefulSlotReg(MULTI, channame, SLOT2,  3); \
    setStatefulSlotReg(   DT, channame, SLOT3,  0); \
    setStatefulSlotReg(MULTI, channame, SLOT3, 13); \
    setStatefulSlotReg(   DT, channame, SLOT4,  0); \
    setStatefulSlotReg(MULTI, channame, SLOT4,  1); \
    setStatefulSlotReg(   TL, channame, SLOT1, 35); \
    setStatefulSlotReg(   TL, channame, SLOT2, 38); \
    setStatefulSlotReg(   TL, channame, SLOT3, 45); \
    setStatefulSlotReg(   TL, channame, SLOT4,  0); \
    setStatefulSlotReg(   KS, channame, SLOT1,  1); \
    setStatefulSlotReg(   AR, channame, SLOT1, 31); \
    setStatefulSlotReg(   KS, channame, SLOT2,  1); \
    setStatefulSlotReg(   AR, channame, SLOT2, 31); \
    setStatefulSlotReg(   KS, channame, SLOT3,  2); \
    setStatefulSlotReg(   AR, channame, SLOT3, 25); \
    setStatefulSlotReg(   KS, channame, SLOT4,  2); \
    setStatefulSlotReg(   AR, channame, SLOT4, 20); \
    setStatefulSlotReg(   AM, channame, SLOT1,  0); \
    setStatefulSlotReg(   DR, channame, SLOT1,  5); \
    setStatefulSlotReg(   AM, channame, SLOT2,  0); \
    setStatefulSlotReg(   DR, channame, SLOT2,  5); \
    setStatefulSlotReg(   AM, channame, SLOT3,  0); \
    setStatefulSlotReg(   DR, channame, SLOT3,  5); \
    setStatefulSlotReg(   AM, channame, SLOT4,  0); \
    setStatefulSlotReg(   DR, channame, SLOT4,  7); \
    setStatefulSlotReg(   SR, channame, SLOT1,  2); \
    setStatefulSlotReg(   SR, channame, SLOT2,  2); \
    setStatefulSlotReg(   SR, channame, SLOT3,  2); \
    setStatefulSlotReg(   SR, channame, SLOT4,  2); \
    setStatefulSlotReg(   SL, channame, SLOT1,  1); \
    setStatefulSlotReg(   RR, channame, SLOT1,  1); \
    setStatefulSlotReg(   SL, channame, SLOT2,  1); \
    setStatefulSlotReg(   RR, channame, SLOT2,  1); \
    setStatefulSlotReg(   SL, channame, SLOT3,  1); \
    setStatefulSlotReg(   RR, channame, SLOT3,  1); \
    setStatefulSlotReg(   SL, channame, SLOT4, 10); \
    setStatefulSlotReg(   RR, channame, SLOT4,  6); \
    \
    setStatefulChanReg(       FB, channame,   1); \
    setStatefulChanReg(ALGORITHM, channame,   0); \
    setStatefulChanReg(       LR, channame, B11); \
    setStatefulChanReg(      AMS, channame,   0); \
    setStatefulChanReg(      PMS, channame,   0); \
} while(0)


class YM2612 {
//stateful YM2612 registers
//a bitfield would be lovely here, but the C/C++ specification does not
//guarantee the layout to be the same between different compilers or
//different versions of the same compiler
    struct {
        struct {
            byte slotReg[toLength(SLOT)][toLength(SLOT_REG)];
            byte channelReg[toLength(CHAN_REG)];
        } channelMem[toLength(CHAN)];
        byte globalReg[toLength(GLOB_REG)];
    } state;
//TODO set up masks for scaling carrier slot levels on MIDI velocity
/* algorithm : carrier slots
    0-3 : 4
    4   : 2,4
    5,6 : 2,3,4
    7   : 1,2,3,4
*/


    void write(byte data) {
	    YM_CTRL_PORT &= ~bit(YM_CS); // CS LOW
	    YM_DATA_PORT = DDATA(data) | (YM_DATA_PORT & B00000011);
	    PORTB = BDATA(data) | (PORTB & B11100111);
	    delayMicroseconds(1);
	    YM_CTRL_PORT &= ~bit(YM_WR); // Write data
	    delayMicroseconds(5);
	    YM_CTRL_PORT |= bit(YM_WR);
	    delayMicroseconds(5);
	    YM_CTRL_PORT |= bit(YM_CS); // CS HIGH
    }


    void setreg(byte reg, byte data, byte part) {
	    if (part == YM2612_PART1) {
	        YM_CTRL_PORT &= ~(bit(YM_A1) | bit(YM_A0)); // A0 low (select register), A1 low (part I, global and YM channels 123)
        } else {
            YM_CTRL_PORT |= bit(YM_A1); // A1 high (part II, YM channels 456)
            YM_CTRL_PORT &= ~bit(YM_A0); // A0 low (select register)
        }
	    write(reg);
	    YM_CTRL_PORT |= bit(YM_A0);  // A0 high (write register)
	    write(data);
    }


    public:
    void begin() {
        /* Pins setup */
        YM_CTRL_DDR |= bit(YM_IC) | bit(YM_CS) | bit(YM_WR) | bit(YM_RD) | bit(YM_A0) | bit(YM_A1);
        YM_DATA_DDR |= B11111100;
        YM_MCLOCK_DDR |= bit(YM_MCLOCK) | B00011000;
        YM_CTRL_PORT |= bit(YM_IC) | bit(YM_CS) | bit(YM_WR) | bit(YM_RD); /* IC, CS, WR and RD HIGH by default */
        YM_CTRL_PORT &= ~(bit(YM_A0) | bit(YM_A1)); /* A0 and A1 LOW by default */

        /* F_CPU / 2 clock generation */
        // OCA1 is pin 9 on the Uno
        TCCR1A = bit(COM1A0);            /* Toggle OCA1 on compare match */
        TCCR1B = bit(WGM12) | bit(CS10); /* CTC mode with prescaler /1 */
        TCCR1C = 0;                      /* Flag reset */
        TCNT1 = 0;                       /* Counter reset */
        /* if the comparison register is 0, then the condition is always 0 to the initial counter value 0 */
        /* and the timer toggles the bit and never has a chance to increment */
        OCR1A = 0;                       /* Divide base clock by two, by toggling OCA1 every F_CPU clock */

        /* Reset YM2612 */
        YM_CTRL_PORT &= ~bit(YM_IC);
        delay(10);
        YM_CTRL_PORT |= bit(YM_IC);
        delay(10);

        /* YM2612 Test code */
        setreg(0x22, 0x00, 0); // LFO off
        /* make sure notes are off */
        setOperators(0, 0);
        setOperators(1, 0);
        setOperators(2, 0);
        setOperators(3, 0);
        setOperators(4, 0);
        setOperators(5, 0);
        setreg(0x27, 0x00, 0); // timers, special mode off
        setreg(0x2B, 0x00, 0); // DAC off
        /* init voices */
        defaultVoice(CHAN1);
        defaultVoice(CHAN2);
        defaultVoice(CHAN3);
        defaultVoice(CHAN4);
        defaultVoice(CHAN5);
        defaultVoice(CHAN6);
        setreg(0x90, 0x00, 0); // Proprietary
        setreg(0x94, 0x00, 0); // Proprietary
        setreg(0x98, 0x00, 0); // Proprietary
        setreg(0x9C, 0x00, 0); // Proprietary
    }

    
    void setOperators(byte channel, byte bitfield) {
        if (bitfield)
            digitalWrite(13, HIGH); //debug
        else
            digitalWrite(13, LOW); //debug
	    setreg(0x28, ((bitfield & B1111) << 4) | (channel < 3 ? channel : channel % 3 + 4), 0); //skip over 011
    }


    void frequency(byte channel, word pitch) {
	    byte lsbReg =  channel <= 5 ? 0xA0 : 0xAC; //if higher than 5 then use the special mode registers
	    byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	    byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
	    setreg(lsbReg + chanOffset + 4, pitch >> 8, part); //pitch MSB first, MSB register is LSB+4
	    setreg(lsbReg + chanOffset, pitch & 0xFF, part); //pitch LSB
    }


    void level(byte channel, byte val)
    {
        if (channel < 6) {
            byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	        byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
            setreg(0x40 + chanOffset, val, part); // Operator 1
/* TODO FIXME
            if (slotOperators[channel] & YM2612_OP1)
	            setreg(0x40 + chanOffset, val, part); // Operator 1
            if (slotOperators[channel] & YM2612_OP2)
	            setreg(0x44 + chanOffset, val, part); // Operator 2
            if (slotOperators[channel] & YM2612_OP3)
	            setreg(0x48 + chanOffset, val, part); // Operator 3
            if (slotOperators[channel] & YM2612_OP4)
	            setreg(0x4C + chanOffset, val, part); // Operator 4
*/
        }
    }
};


//include guard
#endif
