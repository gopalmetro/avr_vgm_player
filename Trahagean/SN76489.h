#ifndef SN76489_H__
#define SN76489_H__

#include "Arduino.h"
#include <util/delay.h>

/* Pin map (Arduino UNO compatible) */

#define SN76489_CE_PORT PORTC
#define SN76489_CE_DDR  DDRC
#define SN76489_CE_BIT  3
#define SN76489_WE_PORT PORTC
#define SN76489_WE_DDR  DDRC
#define SN76489_WE_BIT  2

//Pin map for data pins splits nibbles across two ports:
//SN76489_D0 through SN76489_D3 map to PORTD pins 4-7 -- Uno digital pins 4-7
//SN76489_D4 through SN76489_D7 map to PORTB pins 2-5 -- Uno digital pins 10-13

#define SN76489_DATA_PORTD_MASK B11110000
#define SN76489_DATA_PORTD_BITBANG(b) ((b) << 4)

#define SN76489_DATA_PORTB_MASK B00111100
#define SN76489_DATA_PORTB_BITBANG(b) ((b) >> 2)

// Complain if no DATA masks are defined and if there's no BITBANG for a mask
#if !defined(SN76489_DATA_PORTB_MASK)
    && !defined(SN76489_DATA_PORTC_MASK)
    && !defined(SN76489_DATA_PORTD_MASK)
#error "define at least one SN76489_DATA_PORT[BCD]_MASK"
#endif
#if defined(SN76489_DATA_PORTB_MASK) && !defined(SN76489_DATA_PORTB_BITBANG)
#error "SN76489_DATA_PORTB_MASK defined without SN76489_DATA_PORTB_BITBANG(b)"
#endif
#if defined(SN76489_DATA_PORTC_MASK) && !defined(SN76489_DATA_PORTC_BITBANG)
#error "SN76489_DATA_PORTC_MASK defined without SN76489_DATA_PORTC_BITBANG(b)"
#endif
#if defined(SN76489_DATA_PORTD_MASK) && !defined(SN76489_DATA_PORTD_BITBANG)
#error "SN76489_DATA_PORTD_MASK defined without SN76489_DATA_PORTD_BITBANG(b)"
#endif

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
    #ifdef SN76489_DATA_PORTB_MASK
        PORTB = (PORTB & ~(SN76489_DATA_PORTB_MASK))
            | (SN76489_DATA_PORTB_BITBANG(data) & SN76489_DATA_PORTB_MASK);
    #endif
    #ifdef SN76489_DATA_PORTC_MASK
        PORTC = (PORTC & ~(SN76489_DATA_PORTC_MASK))
            | (SN76489_DATA_PORTC_BITBANG(data) & SN76489_DATA_PORTC_MASK);
    #endif
    #ifdef SN76489_DATA_PORTD_MASK
        PORTD = (PORTD & ~(SN76489_DATA_PORTD_MASK))
            | (SN76489_DATA_PORTD_BITBANG(data) & SN76489_DATA_PORTD_MASK);
    #endif
	    SN76489_WE_PORT &= ~bit(SN76489_WE_BIT); // WE LOW (latch)
	    SN76489_CE_PORT &= ~bit(SN76489_CE_BIT); // CS LOW
	    _delay_us(150);
	    SN76489_CE_PORT |= bit(SN76489_CE_BIT); // CS HIGH
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
	    SN76489_CE_DDR |= bit(SN76489_CE_BIT);

	    #ifdef SN76489_DATA_PORTB_MASK
	    DDRB |= SN76489_DATA_PORTB_MASK;
	    #endif
	    #ifdef SN76489_DATA_PORTC_MASK
	    DDRC |= SN76489_DATA_PORTC_MASK;
	    #endif
	    #ifdef SN76489_DATA_PORTD_MASK
	    DDRD |= SN76489_DATA_PORTD_MASK;
	    #endif
	    SN76489_WE_PORT |= bit(SN76489_WE_BIT); // HIGH by default
	    SN76489_CE_PORT |= bit(SN76489_CE_BIT); // HIGH by default

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
