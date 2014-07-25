#ifndef _TABLES
#define _TABLES

/*
//Voice frequency tuning word vs. MIDI note [0-127]
const uint16_t PROGMEM PITCHS[]={
0x001A,0x001C,0x001E,0x001F,0x0021,0x0023,0x0025,0x0028,0x002A,0x002D,0x002F,0x0032,0x0035,0x0038,0x003C,0x003F,
0x0043,0x0047,0x004B,0x0050,0x0055,0x005A,0x005F,0x0065,0x006B,0x0071,0x0078,0x007F,0x0087,0x008F,0x0097,0x00A0,
0x00AA,0x00B4,0x00BE,0x00CA,0x00D6,0x00E3,0x00F0,0x00FE,0x010E,0x011E,0x012F,0x0141,0x0154,0x0168,0x017D,0x0194,
0x01AC,0x01C6,0x01E1,0x01FD,0x021C,0x023C,0x025E,0x0282,0x02A8,0x02D0,0x02FB,0x0329,0x0359,0x038C,0x03C2,0x03FB,
0x0438,0x0478,0x04BC,0x0504,0x0550,0x05A1,0x05F7,0x0652,0x06B2,0x0718,0x0784,0x07F6,0x0870,0x08F0,0x0978,0x0A08,
0x0AA1,0x0B43,0x0BEF,0x0CA4,0x0D65,0x0E31,0x0F09,0x0FED,0x10E0,0x11E1,0x12F1,0x1411,0x1543,0x1687,0x17DE,0x1949,
0x1ACA,0x1C62,0x1E12,0x1FDB,0x21C0,0x23C2,0x25E3,0x2823,0x2A86,0x2D0E,0x2FBC,0x3292,0x3594,0x38C4,0x3C24,0x3FB7,
0x4381,0x4785,0x4BC6,0x5047,0x550D,0x5A1C,0x5F78,0x6525,0x6B29,0x7188,0x7848,0x7F6F,0x8703,0x8F0A,0x978C,0xA08F,
};
*/

/*
//Voice frequency tuning word vs. MIDI note [0-127] DESCENDING
const uint16_t PROGMEM PITCHTABLE[]={
0x4B9C,0x4B68,0x4B37,0x4B09,0x4ADE,0x4AB4,0x4A8D,0x4A69,0x448D,0x444B,0x440E,0x43D3,0x4368,0x4368,0x4337,0x4309,
0x42DE,0x42B4,0x428D,0x4269,0x3C8D,0x3C4B,0x3C0E,0x3BD3,0x3B9C,0x3B68,0x3B37,0x3B09,0x3ADE,0x3AB4,0x3A8D,0x3A69,
0x348D,0x344B,0x340E,0x33D3,0x339C,0x3368,0x3337,0x3309,0x32DE,0x32B4,0x328D,0x3269,0x2C8D,0x2C4B,0x2C0E,0x2BD3,
0x2B9C,0x2B68,0x2B37,0x2B09,0x2ADE,0x2AB4,0x2A8D,0x2A69,0x248D,0x244B,0x240E,0x23D3,0x239C,0x2368,0x2337,0x2309,
0x22DE,0x22B4,0x228D,0x2269,0x1C8D,0x1C4B,0x1C0E,0x1BD3,0x1B9C,0x1B68,0x1B37,0x1B09,0x1ADE,0x1AB4,0x1A8D,0x1A69,
0x148D,0x144B,0x140E,0x13D3,0x139C,0x1368,0x1337,0x1309,0x12DE,0x12B4,0x128D,0x1269,0x0C8D,0x0C4B,0x0C0E,0x0BD3,
0x0B9C,0x0B68,0x0B37,0x0B09,0x0ADE,0x0AB4,0x0A8D,0x0A69,0x048D,0x044B,0x040E,0x03D3,0x039C,0x0368,0x0337,0x0309,
0x02DE,0x02B4,0x028D,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269
};
*/

//Voice frequency tuning word vs. MIDI note [0-127] ASCENDING
const uint16_t PROGMEM PITCHS[]={
0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x0269,0x028D,0x02B4,0x02DE,
0x0309,0x0337,0x0368,0x039C,0x03D3,0x040E,0x044B,0x048D,0x0A69,0x0A8D,0x0AB4,0x0ADE,0x0B09,0x0B37,0x0B68,0x0B9C,
0x0BD3,0x0C0E,0x0C4B,0x0C8D,0x1269,0x128D,0x12B4,0x12DE,0x1309,0x1337,0x1368,0x139C,0x13D3,0x140E,0x144B,0x148D,
0x1A69,0x1A8D,0x1AB4,0x1ADE,0x1B09,0x1B37,0x1B68,0x1B9C,0x1BD3,0x1C0E,0x1C4B,0x1C8D,0x2269,0x228D,0x22B4,0x22DE,
0x2309,0x2337,0x2368,0x239C,0x23D3,0x240E,0x244B,0x248D,0x2A69,0x2A8D,0x2AB4,0x2ADE,0x2B09,0x2B37,0x2B68,0x2B9C,
0x2BD3,0x2C0E,0x2C4B,0x2C8D,0x3269,0x328D,0x32B4,0x32DE,0x3309,0x3337,0x3368,0x339C,0x33D3,0x340E,0x344B,0x348D,
0x3A69,0x3A8D,0x3AB4,0x3ADE,0x3B09,0x3B37,0x3B68,0x3B9C,0x3BD3,0x3C0E,0x3C4B,0x3C8D,0x4269,0x428D,0x42B4,0x42DE,
0x4309,0x4337,0x4368,0x4368,0x43D3,0x440E,0x444B,0x448D,0x4A69,0x4A8D,0x4AB4,0x4ADE,0x4B09,0x4B37,0x4B68,0x4B9C
};

#endif
