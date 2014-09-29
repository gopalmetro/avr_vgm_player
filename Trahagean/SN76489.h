#ifndef SN76489_H__
#define SN76489_H__

#include "Arduino.h"
#include <util/delay.h>

extern void dataBusWrite(byte data);

/*PORTC corresponds to analog pins on the Uno */
#define SN76489_WE_PORT PORTC
#define SN76489_WE_DDR  DDRC
#define SN76489_WE_BIT  PORTC3
// Tie CE to WE

class SN76489 {
    public:
    enum channel_e {
        CHAN1,
        CHAN2,
        CHAN3,
        CHAN4
    };
    enum feedback_e {
        PERIODIC_NOISE,
        WHITE_NOISE
    };
    enum rate_e {
        SHIFT_512,
        SHIFT_1024,
        SHIFT_2048,
        SHIFT_CHAN3
    };

    private:
    inline static void write(byte data) {
        dataBusWrite(data);
	    SN76489_WE_PORT &= ~bit(SN76489_WE_BIT); // WE LOW (latch)
	    _delay_us(150);
	    SN76489_WE_PORT |= bit(SN76489_WE_BIT); // WE HIGH
    }


    inline static void setRegDirect(byte reg, byte data) {
        write(0x80 | ((reg & 0x07) << 4) | (data & 0x0F));
    }
    
    
    inline static void setReg(byte reg, byte data) {
        noInterrupts();
        setRegDirect(reg, data);
        interrupts();
    }

    
    inline static byte toRegAttn(channel_e chan) {
        return toRegFreqCtrl(chan) + 1;
    }


    inline static byte toRegFreqCtrl(channel_e chan) {
        return (chan << 1);
    }

    
    inline static byte toAttn(byte velocity) {
        return B1111 - (velocity >> 3);
    }
    
    public:
    static void begin() {
	    /* Pins setup */
	    SN76489_WE_DDR |= bit(SN76489_WE_BIT);
	    SN76489_WE_PORT |= bit(SN76489_WE_BIT); // HIGH by default

        /* shut up all the channels */
	    level(CHAN1, 0);
	    level(CHAN2, 0);
	    level(CHAN3, 0);
	    level(CHAN4, 0);
    }

    //send 10 LSBs of period (measured in 125kHz clock cycles) to channel
    //first send 4 LSBs then send 6 MSBs
    static inline void setPeriod125k(channel_e channel, word period) {
        noInterrupts();
        setRegDirect(toRegFreqCtrl(channel), period & 0x0F); // 4 LSB
        write((period >> 4) & 0x3F); // 6 MSB
        interrupts();
    }


    static inline void setNoise(feedback_e fb, rate_e shift) {
        setReg(toRegFreqCtrl(CHAN4), (fb << 2) | shift);
    }

    static inline void level(channel_e channel, byte val) {
        setReg(toRegAttn(channel), toAttn(val));
    }
};




//include guard
#endif
