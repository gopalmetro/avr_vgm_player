#ifndef YM2612_ADDR_H__
#define YM2612_ADDR_H__

// NOTE when using these definitions,
// remember channels and slots start at 0
// unless abstracted or specified otherwise

/* Helpers for C macros that make the code slightly more readable */
#define END_REG(block) \
    (YM2612_##block##_BASE_REG \
        + YM2612_##block##_LENGTH * YM2612_##block##_REG_LENGTH - 1)

#define REG_RANGE(name, reg) \
    (YM2612_##name##_BASE_REG <= (reg) && (reg) <= YM2612_##name##_END_REG)

#define YM2612_CHAN_PART_COUNT (YM2612_CHAN_COUNT / YM2612_PART_COUNT)

#define YM2612_CHAN_PART_PAD 1
#define YM2612_CHAN_PART_LENGTH \
    (YM2612_CHAN_PART_COUNT + YM2612_CHAN_PART_PAD)



/*** STATELESS REGISTER CONSTANTS ***/
/* GLOBAL RESGISTER CONSTANTS */
// parameters for DACSAMPLE: DAC SAMPLE
#define YM2612_DACSAMPLE_BASE_REG 0x2A
#define YM2612_DACSAMPLE_WIDTH 8
#define YM2612_DACSAMPLE_SHIFT 0
// parameters for DACEN: DAC enable
#define YM2612_DACEN_BASE_REG 0x2B
#define YM2612_DACEN_WIDTH 1
#define YM2612_DACEN_SHIFT 7

/* CHAN REGISTER CONSTANTS */


//include guard
#endif
