byte statusByte;
byte dataByte1;
byte dataByte2;

void setup(){
  Serial.begin(38400);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);
}

void MIDIinput() {
  do {
      /*delay(1000);  //checking to see if the 'do' loop works (it does!)
      digitalWrite(13,HIGH);
      delay(500);
      digitalWrite(13,LOW);*/
    
    if (Serial.available()) {
      statusByte = Serial.read(); // get first byte of MIDI data from computer
      switch (statusByte) {
        case 0x90: //NoteOn
          dataByte1 = Serial.read(); //note value
          dataByte2 = Serial.read(); //note velocity
            /*digitalWrite(13,HIGH); //turn on led //// checking for Note On (Successful!!!)
            delay(5);
            digitalWrite(13,LOW); //turn led off*/
          break;
        case 0x80: //NoteOff
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
         dataByte1 = Serial.read(); // CC number
         dataByte2 = Serial.read(); // CC value
            digitalWrite(13,HIGH); //turn on led  //// checking for CC (Successful!!!)
            delay(100);
            digitalWrite(13,LOW); //turn led off   
         break;
        }
      }
  }
  while (Serial.available() >= 2);//when at least three bytes available
}


void loop() {
  MIDIinput();
}
