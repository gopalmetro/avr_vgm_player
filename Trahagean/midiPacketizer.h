#ifndef MIDI_PACKETIZER_H__
#define MIDI_PACKETIZER_H__

#include "binary.h"

// enable as few or as many as needed. disable unused commands to save space/cpu or to debug
#define MIDI_NOTEOFF_ENABLE
#define MIDI_NOTEON_ENABLE
//#define MIDI_AFTERTOUCH_ENABLE
#define MIDI_CONTCONTROL_ENABLE
//#define MIDI_PROGRAMCHANGE_ENABLE
//#define MIDI_CHANNELPRESSURE_ENABLE
#define MIDI_PITCHBEND_ENABLE
//#define MIDI_SYSTEM_ENABLE



//standard MIDI rate (1MHz/32, easier for 1980s embedded systems)
#define MIDI_BAUDRATE 31250

/* MIDI constants */
#define MIDI_PACKET_SIZE 3
#define MIDI_STATE_STATUS 0
#define MIDI_STATE_DATA 1

#define MIDI_STATUS_INDEX 0
#define MIDI_KEY_INDEX 1
#define MIDI_CCNUM_INDEX 1
#define MIDI_PROGNUM_INDEX 1
#define MIDI_PRESSURE_INDEX 1
#define MIDI_BENDLOW_INDEX 1
#define MIDI_VELOCITY_INDEX 2
#define MIDI_TOUCH_INDEX 2
#define MIDI_CCVAL_INDEX 2
#define MIDI_BENDHIGH_INDEX 2

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON 0x90
#define MIDI_AFTERTOUCH 0xA0
#define MIDI_CONTCONTROL 0xB0
#define MIDI_PROGRAMCHANGE 0xC0
#define MIDI_CHANNELPRESSURE 0xD0
#define MIDI_PITCHBEND 0xE0
#define MIDI_SYSTEM 0xF0


/* Abstract some MIDI bit banging */
// is "b" a status byte? return true return true if first bit is 1
#define isMidiStatus(midiByte) (bitRead(midiByte, 7) == 1)
// is "b" a data byte? if it is not a status byte
#define isMidiData(midiByte) (!isMidiStatus(midiByte))

// return the most significant four bits, zero out the rest
#define toMidiCommand(status) ((status) & B11110000)
// return the least significant four bits, zero out the rest
#define toMidiTarget(status) ((status) & B00001111)

class MidiPacketizer {
    byte packet[MIDI_PACKET_SIZE];
    byte have;
    byte need;
    byte mode;

    void reset() {
        have = 0;
        mode = MIDI_STATE_STATUS;
    }
    
    
    void store(byte inByte) {
        packet[have] = inByte; // the have length is stale here, conveniently array indexes are smaller than length by 1
        ++have; // bring have up to date
    }

    public:
    MidiPacketizer() {
        reset();
    }


    // inByte is an int because a negative value can be passed in to indicate an error with the read operation
    // Parses a midi stream one byte at a time, assembling a MIDI packet. Returns the packet buffer when complete
    const byte *receive(int inByte) {
        if (inByte < 0) // did the read fail?
            return NULL; // abort!
        if (mode == MIDI_STATE_STATUS && isMidiStatus(inByte)) {
            store(inByte);
            need = have;
            switch (toMidiCommand(inByte)) {
/* SINGLE BYTE COMMANDS */                
#               ifdef MIDI_PROGRAMCHANGE_ENABLE
                case MIDI_PROGRAMCHANGE:
#               endif
#               ifdef MIDI_CHANNELPRESSURE_ENABLE
                case MIDI_CHANNELPRESSURE:
#               endif
#               if defined(MIDI_PROGRAMCHANGE_ENABLE) \
                    or defined(MIDI_CHANNELPRESSURE_ENABLE)
                need += 1;
                break;
#               endif
/* DOUBLE BYTE COMMANDS */
#               ifdef MIDI_NOTEOFF_ENABLE
                case MIDI_NOTEOFF:
#               endif
#               ifdef MIDI_NOTEON_ENABLE
                case MIDI_NOTEON:
#               endif
#               ifdef MIDI_AFTERTOUCH_ENABLE
                case MIDI_AFTERTOUCH:
#               endif
#               ifdef MIDI_CONTCONTROL_ENABLE
                case MIDI_CONTCONTROL:
#               endif
#               ifdef MIDI_PITCHBEND_ENABLE
                case MIDI_PITCHBEND:
#               endif
#               if defined(MIDI_NOTEOFF_ENABLE) \
                    || defined(MIDI_NOTEON_ENABLE) \
                    || defined(MIDI_AFTERTOUCH_ENABLE) \
                    || defined(MIDI_CONTCONTROL_ENABLE) \
                    || defined(MIDI_PITCHBEND_ENABLE)
                need += 2;
                break;
#               endif
/* SYSTEM COMMANDS */
#               ifdef MIDI_SYSTEM_ENABLE
                case MIDI_SYSTEM:
                // http://www.midi.org/techspecs/midimessages.php
                switch (inByte) {
                    case B11110001:
                    case B11110011:
                    need += 1;
                    break;
                    
                    case B11110010:
                    need += 2;
                    break;
                    
                    case B11110000:
                    reset(); // skip over data, if any
                    return NULL; //unsupported!
                    break;

                    default: // the rest of system commands
                    reset(); // they don't have any data
                    return packet;
                    break;
                }
                break;
#               endif
                
                default: // only get here when a command is disabled
                reset(); // skip over data, if any
                return NULL; // ignore packet
                break;
            }
            mode = MIDI_STATE_DATA; // the next bytes should be data
            return NULL; // packet isn't complete, keep reading
        }
        if (mode == MIDI_STATE_DATA && isMidiData(inByte) && need <= MIDI_PACKET_SIZE) {
            store(inByte);
            if (have == need) {
                reset(); // finished reading
                return packet;
            }
            else
                return NULL; // packet isn't complete, keep reading
        }
        //invalid state: unrecognized packet, or stream hiccup. in either event reset
        reset();
        return NULL; // ignore packet
    }
};

//include guard
#endif
    