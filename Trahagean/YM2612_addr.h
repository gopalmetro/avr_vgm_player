#ifndef YM2612_ADDR_H__
#define YM2612_ADDR_H__

// NOTE when using these definitions,
// remember channels and slots start at 0
// unless abstracted or specified otherwise


/* Helpers for C macros that make the code slightly more readable */
#define toValue(name) YM2612_##name
#define toBaseReg(name) YM2612_##name##_BASE_REG
#define toIndex(name) YM2612_##name##_INDEX
#define toWidth(name) YM2612_##name##_WIDTH
#define toShift(name) YM2612_##name##_SHIFT
#define toLength(name) YM2612_##name##_LENGTH
#define toSlotBaseReg(index) YM2612_SLOT_BASE_REG_INDEX_##index
#define toChanBaseReg(index) YM2612_CHAN_BASE_REG_INDEX_##index
#define toGlobBaseReg(index) YM2612_GLOB_BASE_REG_INDEX_##index

#define indexToChanReg(channame, index) \
    (toChanBaseReg(index) + toValue(channame) % toLength(CHAN_PART))
#define indexToSlotReg(channame, slotname, index) \
	(toSlotBaseReg(index) \
        + toValue(channame) % toLength(CHAN_PART) \
	    + (toValue(slotname) % toLength(SLOT)) * toLength(SLOT))

/* Constants to give semantic names to commonly used values */
#define YM2612_PART1 0
#define YM2612_PART2 1
#define YM2612_REG_PART_LENGTH 2
#define YM2612_CHAN1 0
#define YM2612_CHAN2 1
#define YM2612_CHAN3 2
#define YM2612_CHAN4 3
#define YM2612_CHAN5 4
#define YM2612_CHAN6 5
#define YM2612_CHAN_LENGTH 6
//NOTE the sequence is well documented this way:
#define YM2612_SLOT1 0
#define YM2612_SLOT3 1
#define YM2612_SLOT2 2
#define YM2612_SLOT4 3
#define YM2612_SLOT_LENGTH 4
#define YM2612_CHAN_PART_LENGTH (YM2612_CHAN_LENGTH / YM2612_REG_PART_LENGTH)

// Part number from YM channel [012345]
// * YM2612_PART1: default part
// * YM2612_PART2: does channel exceed the bound YM2612_CHAN_PART_LENGTH?
// Example toPart(3):
// (YM2612_CHAN_PART_LENGTH <= channel) is (3 <= 3) is true
// (channel < YM2612_CHAN_LENGTH) is (3 < 6) is true
// ((YM2612_CHAN_PART_LENGTH <= channel 
//   && channel < YM2612_CHAN_LENGTH ? YM2612_PART2 : YM2612_PART1)
//	 is ((3 <= 3 && 3 < 6) ? 1 : 0) is (true && true ? 1 : 0)
//   is (true ? 1 : 0) is 1, the result
#define toPart(channame) \
	(toLength(CHAN_PART) <= toValue(channame) \
		&& toValue(channame) < toLength(CHAN) ? toValue(PART2) : toValue(PART1))


// Address for channel register from YM channel [012345]
// * Obtain the base register address (x)
// * Make sure channel is in the range 0-2 (chan)
// * The size of the range is 3 (YM2612_CHAN_PART_LENGTH)
// * Each channel past the first adds 1
// Example toChanReg(ALGORITHM, 4):
// channel is 4 also B00000100
// x is toBaseReg(ALGORITHM) is 0xB0 also 176 also B10110000
// chan is (channel % YM2612_CHAN_PART_LENGTH) is (4 % 3) is 1 also B00000001
// (x + chan) is (0xB0 + 1) is 0xB1 also 177 also B10110001, the result
#define toChanReg(name, channame) \
	(toBaseReg(name) + toValue(channame) % YM2612_CHAN_PART_LENGTH)

// Address for slot register from YM channel [012345] and slot [0123]
// * Obtain the channel register address (which is also the SLOT1 address) (x)
// * Make sure slot is in the range 0-3 (sl)
// ** The size of the range is 4 (YM2612_SLOT_LENGTH)
// ** A range of 4 can be stored in 2 bits
// * Each slot past the first adds 4 (YM2612_SLOT_LENGTH)
// Example toSlotReg(AR, 3, 2):
// channel is 3 also B00000011
// slot is 2 also B00000010
// x is toSlotReg(AR, 3, 2) is 0x50 also 80 also B01010000
// sl is (slot % YM2612_SLOT_LENGTH) is (2 % 4) is 2 also B00000010
// (YM2612_SLOT_LENGTH * sl) is (4 * 2) is 8 also B00001000
// (x + YM2612_SLOT_LENGTH * sl) is (0x50 + 8) is 0x58 also 88 also B01011000,
//   the result
#define toSlotReg(name, channame, slotname) \
	(toChanReg(name, channame) \
		+ (toValue(slotname) % YM2612_SLOT_LENGTH) * YM2612_SLOT_LENGTH)

// Named value from register byte
// * Obtain the value mask of the named value (x)
// * Shift right the register byte the desired amount
// * Keep only the bits that fit the mask
// Example toNamedValueFromRegByte(FB, 47):
// b (register byte) is 47 also B00101111
// FB shift is 3
// x is toValueMask(FB) is 7 also B00000111
// (b >> shift) is (47 >> 3) is 5 also B00000101
// ((b >> shift) & x) is (5 & x) is (5 & 7) is 5 also B00000101, the result
#define toNamedValueFromRegByte(name, b) \
	(((b) >> toShift(name)) & toValueMask(name))

// Register byte from named value
// * Obtain the value mask of the named value (x)
// * Keep only the bits that fit the mask
// * Shift left into desired position
// Example toRegByteFromNamedValue(KS, 10):
// v (value) is 10 also B00001010
// KS shift is 6
// x is toValueMask(name) is 3 also B00000011
// (v & x) is (10 & 3) which is 2 also B00000010
// ((v & x) << shift) is (2 << 6) is 64 also B01000000, the result
#define toRegByteFromNamedValue(name, v) \
	(((v) & toValueMask(name)) << toShift(name))

// Merge a named value (v) into a register byte (b):
// * Obtain the register byte of the named value (x)
// * Obtain the register mask of the named value (y)
// * Keep only the bits in b that are not part of x:
// ** Obtain the mask for bits not in x (~y)
// ** Apply ~y to b
// * Combine what is left of b with x
// Example mergeRegByteNamedValue(147, AMS, 2):
// b (register byte) is 147 also B10010011 (AMS value is 1 also B0000001 here)
// v (value) is 2 also B00000010
// x is toRegByteFromNamedValue(name, v) is 32 also B00100000
// y is toRegMask(name) is 48 also B00110000
// (~y) is (~48) is 207 also B11001111
// (~y & b) is (207 & 147) is 131 also B10000011
// ((~y & b) | x) is (131 | 32) is 163 also B1010011, the result
#define mergeRegByteNamedValue(b, name, v) \
	((~toRegMask(name) & (b)) \
		| toRegByteFromNamedValue(name, v))

// Value mask from a width:
// * Shift left the number 1 to the desired width
// * Subtract 1 to flip the bits needed for the mask
// Example toValueMask(DR):
// DR width is 5
// (1) is also B00000001 
// (1 << width) is (1 << 5) is 32 also B00100000
// ((1 << width) - 1) is (32 - 1) is 31 also B00011111, the result
#define toValueMask(name) ((1 << toWidth(name)) - 1)

// Register mask from name:
// * Obtain the value mask (x)
// * Shift left to get the register mask
// (If shift is zero, then the value and byte masks are effectively the same)
// Example toRegrMask(DT):
// DT shift is 4
// x is toValueMask(DT) is 7 also B00000111 
// (x << shift) is (7 << 4) is 112 also B01110000, the result
#define toRegMask(name) (toValueMask(name) << toShift(name))


/*** STATEFUL REGISTER CONSTANTS ***/

/* SLOT REGISTER CONSTANTS */

#define YM2612_SLOT_BASE_REG_INDEX_0    0x30
// parameters for DT: detune B01110000
#define YM2612_DT_BASE_REG	toSlotBaseReg(0)
#define YM2612_DT_INDEX	0
#define YM2612_DT_WIDTH	3
#define YM2612_DT_SHIFT	4
// parameters for MULTI: multiple B00001111
#define YM2612_MULTI_BASE_REG	toSlotBaseReg(0)
#define YM2612_MULTI_INDEX	0
#define YM2612_MULTI_WIDTH	4
#define YM2612_MULTI_SHIFT	0

#define YM2612_SLOT_BASE_REG_INDEX_1    0x40
// parameters for TL: total level B01111111
#define YM2612_TL_BASE_REG	toSlotBaseReg(1)
#define YM2612_TL_INDEX	1
#define YM2612_TL_WIDTH	7
#define YM2612_TL_SHIFT	0

#define YM2612_SLOT_BASE_REG_INDEX_2    0x50
// parameters for KS: key scale B11000000
#define YM2612_KS_BASE_REG  toSlotBaseReg(2)
#define YM2612_KS_INDEX	2
#define YM2612_KS_WIDTH	2
#define YM2612_KS_SHIFT	6
// parameters for AR: attack rate B00011111
#define YM2612_AR_BASE_REG	toSlotBaseReg(2)
#define YM2612_AR_INDEX	2
#define YM2612_AR_WIDTH	5
#define YM2612_AR_SHIFT	0

#define YM2612_SLOT_BASE_REG_INDEX_3    0x60
// parameters for AM: amplitude modulation enable B10000000
#define YM2612_AM_BASE_REG	toSlotBaseReg(3)
#define YM2612_AM_INDEX	3
#define YM2612_AM_WIDTH	1
#define YM2612_AM_SHIFT	7
// parameters for DR: decay rate B00011111
#define YM2612_DR_BASE_REG	toSlotBaseReg(3)
#define YM2612_DR_INDEX	3
#define YM2612_DR_WIDTH	5
#define YM2612_DR_SHIFT	0

#define YM2612_SLOT_BASE_REG_INDEX_4    0x70
// parameters for SR: sustain rate B00011111
#define YM2612_SR_BASE_REG	toSlotBaseReg(4)
#define YM2612_SR_INDEX	4
#define YM2612_SR_WIDTH	5
#define YM2612_SR_SHIFT	0

#define YM2612_SLOT_BASE_REG_INDEX_5    0x80
// parameters for SL: sustain level B11110000
#define YM2612_SL_BASE_REG	toSlotBaseReg(5)
#define YM2612_SL_INDEX	5
#define YM2612_SL_WIDTH	4
#define YM2612_SL_SHIFT	4
// parameters for RR: release rate B00001111
#define YM2612_RR_BASE_REG	toSlotBaseReg(5)
#define YM2612_RR_INDEX	5
#define YM2612_RR_WIDTH	4
#define YM2612_RR_SHIFT	0

#define YM2612_SLOT_BASE_REG_INDEX_6    0x90
// parameters for SSGEG: envelope control B00001111
#define YM2612_SSGEG_BASE_REG	toSlotBaseReg(6)
#define YM2612_SSGEG_INDEX	6
#define YM2612_SSGEG_WIDTH	4
#define YM2612_SSGEG_SHIFT	0
// the last index is 6, so length is +1
#define YM2612_SLOT_REG_LENGTH 7

/* CHANNEL REGISTER CONSTANTS */
#define YM2612_CHAN_BASE_REG_INDEX_0    0xB0
// parameters for ALGORITHM: operator connections B00000111
#define YM2612_ALGORITHM_BASE_REG	toChanBaseReg(0)
#define YM2612_ALGORITHM_INDEX	0
#define YM2612_ALGORITHM_WIDTH	3
#define YM2612_ALGORITHM_SHIFT	0
// parameters for FB: slot feedback B00111000
#define YM2612_FB_BASE_REG	toChanBaseReg(0)
#define YM2612_FB_INDEX	0
#define YM2612_FB_WIDTH	3
#define YM2612_FB_SHIFT	3

#define YM2612_CHAN_BASE_REG_INDEX_1    0xB4
// parameters for PMS: phase modulation sensitivity B00000111
#define YM2612_PMS_BASE_REG	toChanBaseReg(1)
#define YM2612_PMS_INDEX	1
#define YM2612_PMS_WIDTH	3
#define YM2612_PMS_SHIFT	0
// parameters for AMS: amplitude modulation sensitivity B00110000
#define YM2612_AMS_BASE_REG	toChanBaseReg(1)
#define YM2612_AMS_INDEX	1
#define YM2612_AMS_WIDTH	2
#define YM2612_AMS_SHIFT	4
// parameters for LR: left/right enable B11000000
#define YM2612_LR_BASE_REG	toChanBaseReg(1)
#define YM2612_LR_INDEX	1
#define YM2612_LR_WIDTH	2
#define YM2612_LR_SHIFT	6
// the last index is 1, so count is +1
#define YM2612_CHAN_REG_LENGTH 2

/* GLOBAL REGISTER CONSTANTS */
#define YM2612_GLOB_BASE_REG_INDEX_0    0x20
// parameters for T20L: Test register 0x20 low nibble B00001111
#define YM2612_T20L_BASE_REG	toChanBaseReg(0)
#define YM2612_T20L_INDEX	0
#define YM2612_T20L_WIDTH	4
#define YM2612_T20L_SHIFT	0
// parameters for T20H: Test register 0x20 high nibble B11110000
#define YM2612_T20H_BASE_REG	toChanBaseReg(0)
#define YM2612_T20H_INDEX	0
#define YM2612_T20H_WIDTH	4
#define YM2612_T20H_SHIFT	4

#define YM2612_GLOB_BASE_REG_INDEX_1    0x22
// parameters for LFOFREQ: LFO frequency B00000111
#define YM2612_LFOFREQ_BASE_REG	toChanBaseReg(1)
#define YM2612_LFOFREQ_INDEX	1
#define YM2612_LFOFREQ_WIDTH	3
#define YM2612_LFOFREQ_SHIFT	0
// parameters for LFOEN: LFO enable B00001000
#define YM2612_LFOEN_BASE_REG	toChanBaseReg(1)
#define YM2612_LFOEN_INDEX	1
#define YM2612_LFOEN_WIDTH	1
#define YM2612_LFOEN_SHIFT	3

#define YM2612_GLOB_BASE_REG_INDEX_2    0x27
// parameters for T27L: Test register 0x27 lowest six bits B00111111
#define YM2612_T27L_BASE_REG	toChanBaseReg(2)
#define YM2612_T27L_INDEX	2
#define YM2612_T27L_WIDTH	6
#define YM2612_T27L_SHIFT	0
// parameters for T27H: Test register 0x27 high bit B01000000
#define YM2612_T27H_BASE_REG	toChanBaseReg(2)
#define YM2612_T27H_INDEX	2
#define YM2612_T27H_WIDTH	1
#define YM2612_T27H_SHIFT	6
// parameters for SPECIALMODE: Special YM2612_CHAN3 mode enable B10000000
#define YM2612_SPECIALEN_BASE_REG	toChanBaseReg(2)
#define YM2612_SPECIALEN_INDEX	2
#define YM2612_SPECIALEN_WIDTH	1
#define YM2612_SPECIALEN_SHIFT	7

#define YM2612_GLOB_BASE_REG_INDEX_3    0x2C
// parameters for T2CH: Test register 0x2C high nibble B11110000
#define YM2612_T2CH_BASE_REG	toChanBaseReg(3)
#define YM2612_T2CH_INDEX	3
#define YM2612_T2CH_WIDTH	4
#define YM2612_T2CH_SHIFT	4
// parameters for T2CL: Test register 0x2C low nibble B00001111
#define YM2612_T2CL_BASE_REG	toChanBaseReg(3)
#define YM2612_T2CL_INDEX	3
#define YM2612_T2CL_WIDTH	4
#define YM2612_T2CL_SHIFT	0
// the last index is 3, so length is +1
#define YM2612_GLOB_REG_LENGTH 4

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
