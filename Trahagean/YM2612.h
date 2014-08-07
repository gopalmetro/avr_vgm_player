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


class YM2612 {
    public:
    enum part_e {
        PART1,
        PART2,
        PART_COUNT
    };
    enum channel_e {
        CHAN1,
        CHAN2,
        CHAN3,
        CHAN4,
        CHAN5,
        CHAN6,
        CHAN_COUNT
    };
    enum channelReg_e {
        CHAN_REG1,
        CHAN_REG2,
        CHAN_REG_LENGTH
    };
    enum slot_e { // sequence documented this way
        SLOT1,
        SLOT3,
        SLOT2,
        SLOT4,
        SLOT_COUNT
    };
    enum slotReg_e {
        SLOT_REG1,
        SLOT_REG2,
        SLOT_REG3,
        SLOT_REG4,
        SLOT_REG5,
        SLOT_REG6,
        SLOT_REG7,
        SLOT_REG_LENGTH
    };
    enum reg_e {
        REG_COUNT = 256
    };
    //stateful YM2612 registers
    private:
    union State {
        struct Struc {
            byte dummy;
            struct {
                struct {
                    byte slotReg[SLOT_REG_LENGTH];
                } slotMem[SLOT_COUNT];
                byte channelReg[CHAN_REG_LENGTH];
            } channelMem[CHAN_COUNT];
            struct {
                byte reg20;
                byte reg22;
                byte reg27;
                byte reg2C;
            } globalMem;
        };
        byte flat[sizeof(Struc)];
        Struc struc;
    };
    State state;
    static const PROGMEM State regLookup;
    static const PROGMEM byte stateLookup[PART_COUNT][REG_COUNT];
    
    inline byte toFlat(byte *p) { //assumes pointer is in state.flat
        return static_cast<byte>(p - state.flat);
    }
    public:
    class Field {
        friend class YM2612;
        struct BasicField {
            byte width;
            byte shift;
            BasicField(byte width, byte shift) 
                : width(width), shift(shift) { };
        };
        
        struct IndexField : BasicField {
            byte index;
            IndexField(byte width, byte shift, byte index)
                : BasicField(width, shift), index(index) { };
        };
        
        struct SlotField : IndexField {
            SlotField(byte width, byte shift, slotReg_e index)
                : IndexField(width, shift, index) { };
        };
        
        struct ChannelField : IndexField {
            ChannelField(byte width, byte shift, channelReg_e index)
                : IndexField(width, shift, index) { };
        };
        
        struct GlobalField20 : BasicField {
            GlobalField20 (byte width, byte shift)
                : BasicField(width, shift) { };
        };
        
        struct GlobalField22 : BasicField {
            GlobalField22 (byte width, byte shift)
            : BasicField(width, shift) { };
        };

        struct GlobalField27 : BasicField {
            GlobalField27 (byte width, byte shift)
            : BasicField(width, shift) { };
        };

        struct GlobalField2C : BasicField {
            GlobalField2C (byte width, byte shift)
            : BasicField(width, shift) { };
        };
        
        public:
        static const SlotField     DT;
        static const SlotField     MULTI;
        static const SlotField     TL;
        static const SlotField     KS;
        static const SlotField     AR;
        static const SlotField     AM;
        static const SlotField     DR;
        static const SlotField     SR;
        static const SlotField     SL;
        static const SlotField     RR;
        static const SlotField     SSEG;
        static const ChannelField  FB;
        static const ChannelField  ALGO;
        static const ChannelField  LR;
        static const ChannelField  AMS;
        static const ChannelField  PMS;
        static const GlobalField20 T20H;
        static const GlobalField20 T20L;
        static const GlobalField22 LFOEN;
        static const GlobalField22 LFOFREQ;
        static const GlobalField27 SPECIALEN;
        static const GlobalField27 T27H;
        static const GlobalField27 T27L;
        static const GlobalField2C T2CH;
        static const GlobalField2C T2CL;
    };
    
     
    inline void
    updateField(part_e part, byte reg, byte width, byte shift, byte val) {
        const byte mask = bit(width) - 1;
        setReg(
            part, reg, 
            (getReg(part, reg) & ~(mask << shift)) | ((val & mask) << shift));
    }
    

    inline void
    setSlot(channel_e channel, slot_e slot, Field::SlotField field, byte val) {
        byte flat =
            toFlat(&state.struc.channelMem[channel].slotMem[slot]
                .slotReg[field.index]);
        updateField(
            whichPart(flat), whichReg(flat), field.width, field.shift, val);
    }


    inline void
    setChannel(channel_e channel, Field::ChannelField field, byte val) {
        byte flat =
            toFlat(&state.struc.channelMem[channel].channelReg[field.index]);
        updateField(
            whichPart(flat), whichReg(flat), field.width, field.shift, val);
    }


    inline void setGlobal20(Field::GlobalField20 field, byte val) {
        byte flat = toFlat(&state.struc.globalMem.reg20);
        updateField(
            whichPart(flat), whichReg(flat), field.width, field.shift, val);
    }


    inline void setGlobal22(Field::GlobalField22 field, byte val) {
        byte flat = toFlat(&state.struc.globalMem.reg22);
        updateField(
            whichPart(flat), whichReg(flat), field.width, field.shift, val);
    }


    inline void setGlobal27(Field::GlobalField27 field, byte val) {
        byte flat = toFlat(&state.struc.globalMem.reg27);
        updateField(
            whichPart(flat), whichReg(flat), field.width, field.shift, val);
    }


    inline void setGlobal2C(Field::GlobalField2C field, byte val) {
        byte flat = toFlat(&state.struc.globalMem.reg2C);
        updateField(
            whichPart(flat), whichReg(flat), field.width, field.shift, val);
    }


    private:
    static inline void write(byte data) {
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


    static inline byte whichState(part_e part, byte reg) {
        return pgm_read_byte(&stateLookup[part][reg]);
    }
    
    
    inline part_e whichPart(byte index) {
        return 91 <= index && index <= 180 ? PART2 : PART1; // range from spreadsheet
        //return static_cast<part_e>(pgm_read_byte(&partLookup.flat[index]));
    }


    inline byte whichReg(byte index) {
        return pgm_read_byte(&regLookup.flat[index]);
    }


    inline void setRegDirect(part_e part, byte reg, byte data) {
        state.flat[whichState(part, reg)] = data;
        if (part == YM2612::PART1) {
            YM_CTRL_PORT &= ~(bit(YM_A1) | bit(YM_A0)); // A0 low (select register), A1 low (part I, global and YM channels 123)
        }
        else {
            YM_CTRL_PORT |= bit(YM_A1); // A1 high (part II, YM channels 456)
            YM_CTRL_PORT &= ~bit(YM_A0); // A0 low (select register)
        }
        write(reg);
        YM_CTRL_PORT |= bit(YM_A0);  // A0 high (write register)
        write(data);
    }


    inline void setReg(part_e part, byte reg, byte data) {
        state.flat[whichState(part, reg)] = data;
        setRegDirect(part, reg, data);
    }


    inline byte getReg(part_e part, byte reg) {
        return state.flat[pgm_read_byte(&stateLookup[part][reg])];
    }
    
    
//TODO set up masks for scaling carrier slot levels on MIDI velocity
/* algorithm : carrier slots
    0-3 : 4
    4   : 2,4
    5,6 : 2,3,4
    7   : 1,2,3,4
*/
    
    public:
    void begin() {
        /* Pins setup */
        YM_CTRL_DDR |= bit(YM_IC) | bit(YM_CS) | bit(YM_WR) | bit(YM_RD) | bit(YM_A0) | bit(YM_A1);
        YM_DATA_DDR |= B11111100;
        YM_MCLOCK_DDR |= bit(YM_MCLOCK) | B00011000;
        YM_CTRL_PORT |= bit(YM_IC) | bit(YM_CS) | bit(YM_WR) | bit(YM_RD); /* IC, CS, WR and RD HIGH by default */
        YM_CTRL_PORT &= ~(bit(YM_A0) | bit(YM_A1)); /* A0 and A1 LOW by default */

        /* F_CPU / 2 clock generation */
        // OC1A is pin 9 on the Uno
        TCCR1A = bit(COM1A0);            /* Toggle OCA1 on compare match */
        TCCR1B = bit(WGM12) | bit(CS10); /* CTC mode with prescaler /1 */
        TCCR1C = 0;                      /* Flag reset */
        TCNT1 = 0;                       /* Counter reset */
        /* if the comparison register is 0, then the condition is always 0 to the initial counter value 0 */
        /* and the timer toggles the bit and never has a chance to increment */
        OCR1A = 0;                       /* Divide base clock by two, by toggling OC1A every F_CPU clock */

        /* Reset YM2612 */
        YM_CTRL_PORT &= ~bit(YM_IC);
        delay(10);
        YM_CTRL_PORT |= bit(YM_IC);
        delay(10);

        /* YM2612 Test code */
        setReg(PART1, 0x22, 0x00); // LFO off
        /* make sure notes are off */
        setOperators(0, 0);
        setOperators(1, 0);
        setOperators(2, 0);
        setOperators(3, 0);
        setOperators(4, 0);
        setOperators(5, 0);
        setReg(PART1, 0x27, 0x00); // timers, special mode off
        setReg(PART1, 0x2B, 0x00); // DAC off
        /* init voices */
        defaultVoice(CHAN1);
        state.struc.channelMem[CHAN2] = state.struc.channelMem[CHAN1]; //2
        channelRegCopy(CHAN2, CHAN1);
        state.struc.channelMem[CHAN3] = state.struc.channelMem[CHAN1]; //3
        channelRegCopy(CHAN3, CHAN1);
        state.struc.channelMem[CHAN4] = state.struc.channelMem[CHAN1]; //4
        channelRegCopy(CHAN4, CHAN1);
        state.struc.channelMem[CHAN5] = state.struc.channelMem[CHAN1]; //5
        channelRegCopy(CHAN5, CHAN1);
        state.struc.channelMem[CHAN6] = state.struc.channelMem[CHAN1]; //6
        channelRegCopy(CHAN6, CHAN1);
        setReg(PART1, 0x90, 0x00); // Proprietary
        setReg(PART1, 0x94, 0x00); // Proprietary
        setReg(PART1, 0x98, 0x00); // Proprietary
        setReg(PART1, 0x9C, 0x00); // Proprietary
    }

    
    void setOperators(byte channel, byte bitfield) {
        if (bitfield)
            digitalWrite(LED_BUILTIN, HIGH); //debug
        else
            digitalWrite(LED_BUILTIN, LOW); //debug
	    setReg(PART1, 0x28, ((bitfield & B1111) << 4) | (channel < 3 ? channel : channel % 3 + 4)); //skip over 011
    }


    void frequency72(byte channel, byte block, word freq) {
	    byte lsbReg =  channel <= 5 ? 0xA0 : 0xAC; //if higher than 5 then use the special mode registers
	    byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	    byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
        // insert block field, adjust for factor of 72 and YM scaling:
        freq = (block << 11) | (((uint32_t)(freq) << 9) / 15625);
	    setReg(static_cast<part_e>(part), lsbReg + chanOffset + 4, freq >> 8); //pitch MSB first, MSB register is LSB+4
	    setReg(static_cast<part_e>(part), lsbReg + chanOffset, freq & 0xFF); //pitch LSB
    }


    void level(byte channel, byte val)
    {
        if (channel < 6) {
            byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	        byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
            setReg(static_cast<part_e>(part), 0x40 + chanOffset, val); // Operator 1
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

    void defaultVoice(channel_e channel) {
        setSlot(channel, SLOT1, Field::DT,     7);
        setSlot(channel, SLOT1, Field::MULTI,  1);
        setSlot(channel, SLOT1, Field::TL,    35);
        setSlot(channel, SLOT1, Field::KS,     1);
        setSlot(channel, SLOT1, Field::AR,    31);
        setSlot(channel, SLOT1, Field::AM,     0);
        setSlot(channel, SLOT1, Field::DR,     5);
        setSlot(channel, SLOT1, Field::SR,     2);
        setSlot(channel, SLOT1, Field::SL,     1);
        setSlot(channel, SLOT1, Field::RR,     1);

        setSlot(channel, SLOT2, Field::DT,     3);
        setSlot(channel, SLOT2, Field::MULTI,  3);
        setSlot(channel, SLOT2, Field::TL,    38);
        setSlot(channel, SLOT2, Field::KS,     1);
        setSlot(channel, SLOT2, Field::AR,    31);
        setSlot(channel, SLOT2, Field::AM,     0);
        setSlot(channel, SLOT2, Field::DR,     5);
        setSlot(channel, SLOT2, Field::SR,     2);
        setSlot(channel, SLOT2, Field::SL,     1);
        setSlot(channel, SLOT2, Field::RR,     1);

        setSlot(channel, SLOT3, Field::DT,     0);
        setSlot(channel, SLOT3, Field::MULTI, 13);
        setSlot(channel, SLOT3, Field::TL,    45);
        setSlot(channel, SLOT3, Field::KS,     2);
        setSlot(channel, SLOT3, Field::AR,    25);
        setSlot(channel, SLOT3, Field::AM,     0);
        setSlot(channel, SLOT3, Field::DR,     5);
        setSlot(channel, SLOT3, Field::SR,     2);
        setSlot(channel, SLOT3, Field::SL,     1);
        setSlot(channel, SLOT3, Field::RR,     1);

        setSlot(channel, SLOT4, Field::DT,     0);
        setSlot(channel, SLOT4, Field::MULTI,  1);
        setSlot(channel, SLOT4, Field::TL,     0);
        setSlot(channel, SLOT4, Field::KS,     2);
        setSlot(channel, SLOT4, Field::AR,    20);
        setSlot(channel, SLOT4, Field::AM,     0);
        setSlot(channel, SLOT4, Field::DR,     7);
        setSlot(channel, SLOT4, Field::SR,     2);
        setSlot(channel, SLOT4, Field::SL,    10);
        setSlot(channel, SLOT4, Field::RR,     6);

        setChannel(channel, Field::FB,          1);
        setChannel(channel, Field::ALGO,        0);
        setChannel(channel, Field::LR,        B11);
        setChannel(channel, Field::AMS,         0);
        setChannel(channel, Field::PMS,         0);
    }
    
    void channelRegCopy(channel_e dest, channel_e src) {
        for (byte s = SLOT1; s < SLOT_COUNT; s++) {
            for (byte r = SLOT_REG1; r < SLOT_REG_LENGTH; r++) {
                byte *v =
                    &state.struc.channelMem[dest].slotMem[s].slotReg[r];
                byte i = toFlat(v);
                setRegDirect(whichPart(i), whichReg(i), *v);
            }
        }            
        for (byte r = CHAN_REG1; r < CHAN_REG_LENGTH; r++) {
            byte *v = &state.struc.channelMem[dest].channelReg[r];
            byte i = toFlat(v);
            setRegDirect(whichPart(i), whichReg(i), *v);
        }
    }
};


// The following tables were copied from the spreadsheet:
const PROGMEM YM2612::State YM2612::regLookup {
    { //flat[]
        0x00,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0x34,0x44,0x54,0x64,0x74,0x84,
        0x94,0x38,0x48,0x58,0x68,0x78,0x88,0x98,0x3C,0x4C,0x5C,0x6C,0x7C,0x8C,
        0x9C,0xB0,0xB4,0x31,0x41,0x51,0x61,0x71,0x81,0x91,0x35,0x45,0x55,0x65,
        0x75,0x85,0x95,0x39,0x49,0x59,0x69,0x79,0x89,0x99,0x3D,0x4D,0x5D,0x6D,
        0x7D,0x8D,0x9D,0xB1,0xB5,0x32,0x42,0x52,0x62,0x72,0x82,0x92,0x36,0x46,
        0x56,0x66,0x76,0x86,0x96,0x3A,0x4A,0x5A,0x6A,0x7A,0x8A,0x9A,0x3E,0x4E,
        0x5E,0x6E,0x7E,0x8E,0x9E,0xB3,0xB6,0x30,0x40,0x50,0x60,0x70,0x80,0x90,
        0x34,0x44,0x54,0x64,0x74,0x84,0x94,0x38,0x48,0x58,0x68,0x78,0x88,0x98,
        0x3C,0x4C,0x5C,0x6C,0x7C,0x8C,0x9C,0xB0,0xB4,0x31,0x41,0x51,0x61,0x71,
        0x81,0x91,0x35,0x45,0x55,0x65,0x75,0x85,0x95,0x39,0x49,0x59,0x69,0x79,
        0x89,0x99,0x3D,0x4D,0x5D,0x6D,0x7D,0x8D,0x9D,0xB1,0xB5,0x32,0x42,0x52,
        0x62,0x72,0x82,0x92,0x36,0x46,0x56,0x66,0x76,0x86,0x96,0x3A,0x4A,0x5A,
        0x6A,0x7A,0x8A,0x9A,0x3E,0x4E,0x5E,0x6E,0x7E,0x8E,0x9E,0xB2,0xB6,0x20,
        0x22,0x27,0x2C
    }
};


const PROGMEM byte YM2612::stateLookup[YM2612::PART_COUNT][YM2612::REG_COUNT] {
    { // YM2612_PART1 parallels State.flat[]
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,181,0,
        182,0,0,0,0,183,0,0,0,0,184,0,0,0,1,31,61,0,8,38,68,0,15,45,75,0,22,
        52,82,0,2,32,62,0,9,39,69,0,16,46,76,0,23,53,83,0,3,33,63,0,10,40,70,
        0,17,47,77,0,24,54,84,0,4,34,64,0,11,41,71,0,18,48,78,0,25,55,85,0,5,
        35,65,0,12,42,72,0,19,49,79,0,26,56,86,0,6,36,66,0,13,43,73,0,20,50,
        80,0,27,57,87,0,7,37,67,0,14,44,74,0,21,51,81,0,28,58,88,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,29,59,0,89,30,60,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    },
    { // YM2612_PART2 parallels State.flat[]
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,91,121,151,0,98,128,158,0,105,135,165,0,112,
        142,172,0,92,122,152,0,99,129,159,0,106,136,166,0,113,143,173,0,93,
        123,153,0,100,130,150,0,107,137,167,0,114,144,174,0,94,124,154,0,101,
        131,161,0,108,138,168,0,115,145,175,0,95,125,155,0,102,132,162,0,109,
        139,169,0,116,146,176,0,96,126,156,0,103,133,163,0,110,140,170,0,117,
        147,177,0,97,127,157,0,104,134,164,0,111,141,171,0,118,148,178,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,149,179,0,120,150,180,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    }
};


const YM2612::Field::SlotField     YM2612::Field::DT         (3, 4, YM2612::SLOT_REG1);
const YM2612::Field::SlotField     YM2612::Field::MULTI      (4, 0, YM2612::SLOT_REG1);
const YM2612::Field::SlotField     YM2612::Field::TL         (7, 0, YM2612::SLOT_REG2);
const YM2612::Field::SlotField     YM2612::Field::KS         (2, 6, YM2612::SLOT_REG3);
const YM2612::Field::SlotField     YM2612::Field::AR         (5, 0, YM2612::SLOT_REG3);
const YM2612::Field::SlotField     YM2612::Field::AM         (1, 7, YM2612::SLOT_REG4);
const YM2612::Field::SlotField     YM2612::Field::DR         (5, 0, YM2612::SLOT_REG4);
const YM2612::Field::SlotField     YM2612::Field::SR         (5, 0, YM2612::SLOT_REG5);
const YM2612::Field::SlotField     YM2612::Field::SL         (4, 4, YM2612::SLOT_REG6);
const YM2612::Field::SlotField     YM2612::Field::RR         (4, 0, YM2612::SLOT_REG6);
const YM2612::Field::SlotField     YM2612::Field::SSEG       (4, 0, YM2612::SLOT_REG7);
const YM2612::Field::ChannelField  YM2612::Field::FB         (3, 3, YM2612::CHAN_REG1);
const YM2612::Field::ChannelField  YM2612::Field::ALGO       (3, 0, YM2612::CHAN_REG1);
const YM2612::Field::ChannelField  YM2612::Field::LR         (2, 6, YM2612::CHAN_REG2);
const YM2612::Field::ChannelField  YM2612::Field::AMS        (2, 4, YM2612::CHAN_REG2);
const YM2612::Field::ChannelField  YM2612::Field::PMS        (3, 0, YM2612::CHAN_REG2);
const YM2612::Field::GlobalField20 YM2612::Field::T20H       (4, 4);
const YM2612::Field::GlobalField20 YM2612::Field::T20L       (4, 0);
const YM2612::Field::GlobalField22 YM2612::Field::LFOEN      (1, 3);
const YM2612::Field::GlobalField22 YM2612::Field::LFOFREQ    (3, 0);
const YM2612::Field::GlobalField27 YM2612::Field::SPECIALEN  (1, 7);
const YM2612::Field::GlobalField27 YM2612::Field::T27H       (1, 6);
const YM2612::Field::GlobalField27 YM2612::Field::T27L       (6, 0);
const YM2612::Field::GlobalField2C YM2612::Field::T2CH       (4, 4);
const YM2612::Field::GlobalField2C YM2612::Field::T2CL       (4, 0);


//include guard
#endif
