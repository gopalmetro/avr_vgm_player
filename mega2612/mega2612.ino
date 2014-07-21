
// FOR DETAILED INFORMATION ON THE MIDI PROTOCOL VISIT THE FOLLOWING LINKS:
// http://www.midi.org/techspecs/midimessages.php
// http://ccrma.stanford.edu/~craig/articles/linuxmidi/misc/essenmidi.html  

//NOTES ABOUT THE MIDI DATA TRANSMISSION FORMAT
// Data bytes range in decimal value from 0 to 127 (Binary 00000000 to 011111111; Hex 0x00 to 0x7F)
// Command bytes rang in decimal value from 128 to 255 (Binary 10000000 to 11111111; Hex 0x80 to 0xFF)
// Most MIDI messages are three Bytes in length, with the first Byte always being a command message
// (The "Channel Pressure" message and "Program Change" message 
//  are only two bytes in length (not relevant to the current program;
//  The Sysex messages are only one byte in length (not relevant to the current program)
//
// Standard MIDI Messages are formatted as follows:
// The first byte is the Command or Status byte.
// - The four Most Significant Bits (MSBs) are the MIDI Command 
//   the hex values are 0x8m to 0xFm, where m = the LSB
//   -- the binary values are 1000mmmm to 1111mmmm
// - the four Least Significant Bits (LSBs) are the MIDI Channels 1-16 (or 0-15 in code)
//   the hex values are 0xn0 to 0xnF, where n = the MSB
//   -- the binary values are nnnn0000 to nnnn1111
// An example message is 0x9m, where the hex value "9" in the MSB position = MIDI NoteOn
// The second half of the message is 0xn0, where the LSB hex value of "0" = MIDI Channel 1
// The full message is 0x90, representing NoteOn on MIDI Channel One
// -- binary B10010000
// Another example is 0x95, representing NoteOn on MIDI Channel Six
// -- binary B10010101
// Another example is 0x80, represengin MIDI NoteOff on MIDI Channel One
// -- binary B10000000
// 
// The second byte is either the Key/Note value, the CC number, 
// the instrument number, the channel pressure value or 
// the 7-bit LSBs (Least Significant Bits) for Pitch Bend
// - As noted above, as a Data Byte, this byte is actually only 7 bits
//   ranging in value from 0x00 to 0x7F or B00000000 to B01111111
//
// The third byte is either the velocity value, the touch value (aftertouch), 
// the CC value, of the 7-biyt MSBs (Most Significant Bits) for Pitch Bend.
// - As noted above, as a Data Byte, this byte is actually only 7 bits
//   ranging in value from 0x00 to 0x7F or B00000000 to B01111111
//
// These three-byte MIDI messages are the ones the current iteration of this program will use

// SPLITTING UP THE COMMAND/STATUS byte INTO TWO FOUR BIT VALUES
// The Command/Status byte is split into two distinct messages
// We first capture a full byte in the 'statusbyte' variable
// Then, using some bitmath, we "AND" out the Most Significan Bits and assign
// the remaining Least Significant Bits to the "MIDIChannel" variable
// ---NEED TO THINK ABOUT HOW TO HANDLE THE FOUR MSBs.  HOW CAN WE APPEND THEM TO THE SPLIT OFF
// MIDI MESSAGES IN A WAY THAT MAKES SENSE?----
// Now, we are able to isolate the MIDI Channel independently from the actual MIDI command 
// (e.g. NoteON, NoteOFF, CC, etc.)
// this gives us far more programmatical control and allows us to apply universal command functions 
// to each MIDI channel without having to use gigantic if/then or switch/case codeblocks

// GETTING INTO THE ACTUAL DATA!
// 'statusbyte' records the initial MIDI Command/Status/MIDI Channel byte
byte statusByte;

// 'StatusCommand' variable will hold the most significant bits of the initial 
// MIDI Status Command message, defining the actual MIDI command 
// When combined with the uint4_t 'MIDIChannel', the 'StatusCommand' half 
// of the full byte-size message will be: B100000000-B11110000 or 0x8m-0xFm
byte statusCommand = ((statusByte & B11110000) >> 4) & B00000111;

// The four bit MIDIChannel variable will hold the least significant bits of the initial 
// MIDI Status Command message, defining the MIDI Channel 
// When combined with the uint4_t 'statusCommand', the 'MIDIChanel' half 
// of the full byte-size message will be: B10000000-B10001111 or 0xn0-0xnF
// essentially representing MIDI channels 1 through 16 (or 0-15, in code)
byte MIDIChannel = statusByte & B00001111;

// The 'databyte1' variable is actually the second data byte
// It will store the Note value,  CC Number, Instrument Number, etc.
byte databyte1; 

// Likewise, the 'databyte2' variable is actually the third data byte
// 
byte databyte2; 

// ORIGINAL METHOD (hope to replace this)
// byte statusbyte;
// byte databyte1;
// byte databyte2;

// BLINKTEST - THE VERY USEFUL TOOL FOR TESTING CODE BLOCKS
// If you want to test to see if a switch case or if statement is active, 
// add 'blinkTest([numBlinks], [LEDHighTime], [LEDLowTime])' to the code block
// -blinkTest() fires the LED attached to Pin 13
// -numBlinks is the number of times the LED blinks
// -LEDHighTime is the duration in microseconds that the LED is lit; 1000 = 1 second
// -LEDLowTime is the duration in microseconds that the LED is off; 1000 = 1 second
// Put the function "blinkTest([duration LED is lit], [number of blinks])" (without the quotes!) 
// anywhere in your code to get feedback on whether or not that code is being executed
void blinkTest(int numBlinks = 1, int LEDHighTime = 50, int LEDLowTime = 50) {
  for (int i = 0; i < numBlinks; i++) {
    digitalWrite(13,HIGH);
    delay(LEDHighTime);
    digitalWrite(13,LOW);
    delay(LEDLowTime);
  }
}

void setup(){
  Serial.begin(38400);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  
  //Blink SOS on the LED, just to get the Swashbuckling off to a good start!
  blinkTest(3,200,200);
  blinkTest(3,400,200);
  blinkTest(3,200,200);
}
  
void MIDIinput() {
  do {    
    if (Serial.available()) {
      statusByte = Serial.read(); // get first byte of MIDI data from computer
      switch (statusByte) {
        case 0x90: //NoteOn
          //blinkTest(1,50,1);
          databyte1 = Serial.read(); //note value
          databyte2 = Serial.read(); //note velocity
          break;
        case 0x80: //NoteOff
         // blinkTest(3,100,10);
         databyte1 = Serial.read();  //note value
         databyte2 = Serial.read();   //note velocity
          break;
        case 0xA0: //Aftertouch
         databyte1 = Serial.read(); //note value
         databyte2 = Serial.read(); //note aftertouch value
         break;
        case 0xB0: //Continuous Controller (CC)
         // blinkTest(1, 100, 100);
         databyte1 = Serial.read(); // CC number
         databyte2 = Serial.read(); // CC value
         break;
        }
      }
  }
  while (Serial.available() >= 2);//when at least three bytes available
}

void commandMessage() {
  switch (statusCommand)
    case 0x09:
    //do stuff
    break;
}

void manageChannels() {
  // MIDI Channels 1, 2 and 3 are identical
  switch (MIDIChannel)
    case 0x00:
    case 0x01:
    case 0x02:
    if blinkTest(3,50,1)
    break;
}


void loop() {
  MIDIinput();
}
