
//NOTES ABOUT THE MIDI DATA TRANSMISSION FORMAT
// The "Channel Pressure" message is only two Bytes in length (not relevant to the current program)
// The Sysex messages or only one Byte in length (not relevant to the current program)
// All other MIDI messages are three Bytes in length
// The first Byte is the Status or Command byte.
// - The four most significant bits are the MIDI Command
// - the four least significant bits are the MIDI Channel
// 
// The second Byte is either the Key/Note value, the CC number, 
// the instrument number, the channel pressure value or the 7-bit LSB for Pitch Bend
//
// The third Byte is either the velocity value, the touch value (aftertouch), 
// the CC value, of the MSB for Pitch Bend.
//
// These three Byte messages are the ones the current iteration of this program will use

// SPLITTING UP THE COMMAND/STATUS BYTE INTO TWO FOUR BIT VALUES
// We first capture a full Byte in the 'statusByte' variable
// Then, using some bitmath, we "AND" out the Most Significan Bits and assign
// the remaining Least Significant Bits to the "MIDIChannel" variable
// ---NEED TO THINK ABOUT HOW TO HANDLE THE FOUR MSBs.  HOW CAN WE APPEND THEM TO THE SPLIT OFF
// MIDI MESSAGES IN A WAY THAT MAKES SENSE?----
// Now, we are able to isolate the MIDI Channel independently from the actual MIDI command 
// (e.g. NoteON, NoteOFF, CC, etc.)
// this gives us far more programmatical control and allows us to apply universal command functions 
// to each MIDI channel without having to use gigantic if/then or switch/case codeblocks

// GETTING INTO THE ACTUAL DATA!
// 'statusByte' records the initial MIDI Command/Status/MIDI Channel Byte
byte statusByte;

// 'StatusCommand' variable will hold the most significant bits of the initial 
// MIDI Status Command message, defining the actual MIDI command 
// When combined with the uint4_t 'MIDIChannel', the 'StatusCommand' half 
// of the full Byte-size message will be: B100000000-B11110000 or 0x8m-0xFm
byte statusCommand = statusByte & B11110000;

// The four bit MIDIChannel variable will hold the least significant bits of the initial 
// MIDI Status Command message, defining the MIDI Channel 
// When combined with the uint4_t 'statusCommand', the 'MIDIChanel' half 
// of the full Byte-size message will be: B10000000-B10001111 or 0xn0-0xnF
// essentially representing MIDI channels 1 through 16 (or 0-15, in code)
byte MIDIChanel = statusByte & B00001111;

// The 'dataByte1' variable is actually the second data byte
// It will store the Note value,  CC Number, Instrument Number, etc.
byte dataByte1; 

// Likewise, the 'dataByte2' variable is actually the third data byte
// 
byte dataByte2; 

// ORIGINAL METHOD (hope to replace this)
// byte statusByte;
// byte dataByte1;
// byte dataByte2;


// -blinkTest() fires the Pin 13 LED
// -LEDTime is the duration in microseconds taht the LED is lit; 1000 = 1 second
// -numBlinks is the number of times the LED blinks
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
  delay(500);
  blinkTest(3,300,200);
  blinkTest(3,600,200);
  blinkTest(3,300,200);
}
  
void MIDIinput() {
  do {    
    if (Serial.available()) {
      statusByte = Serial.read(); // get first byte of MIDI data from computer
      switch (statusByte) {
        case 0x90: //NoteOn
          blinkTest();
          dataByte1 = Serial.read(); //note value
          dataByte2 = Serial.read(); //note velocity
          break;
        case 0x80: //NoteOff
         blinkTest(30,2);
         dataByte1 = Serial.read();  //note value
         dataByte2 = Serial.read();   //note velocity
            /*digitalWrite(13,HIGH); //turn on led  //// checking for Note Off (Successful!!!)
            delay(100);
            digitalWrite(13,LOW); //turn led off*/ 
          break;
        case 0xA0: //Aftertouch
         dataByte1 = Serial.read(); //note value
         dataByte2 = Serial.read(); //note aftertouch value
         break;
        case 0xB0: //Continuous Controller (CC)
         blinkTest(100,3);
         dataByte1 = Serial.read(); // CC number
         dataByte2 = Serial.read(); // CC value
         break;
        }
      }
  }
  while (Serial.available() >= 2);//when at least three bytes available
}


void loop() {
  MIDIinput();
}
