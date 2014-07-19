/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;
byte commandByte;
byte noteByte;
byte velocityByte;
byte noteOn = 144;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  digitalWrite(led,LOW);
  Serial.begin(38400);
}

void checkMIDI(){
  do{
    if (Serial.available()){
      commandByte = Serial.read();//read first byte
      noteByte = Serial.read();//read next byte
      velocityByte = Serial.read();//read final byte
      if (commandByte == noteOn){//if note on message
        //check if note == 60 and velocity > 0
        if (noteByte == 60 && velocityByte > 0){
          digitalWrite(13,HIGH);//turn on led
        }
      }
    }
  }
  while (Serial.available() > 2);//when at least three bytes available
}


// the loop routine runs over and over again forever:
void loop() {
  
  do{
    if (Serial.available()){
      while(Serial.available()>0) {
      commandByte = Serial.read();//read first byte
      noteByte = Serial.read();//read next byte
      velocityByte = Serial.read();//read final byte
      byte noteOn = 0x90;
      //  check if note on message received (this is working)
            if (commandByte == noteOn){
      
                       //check if note == ?? and velocity > 0 (this is not working.  Is noteByte receiving data from Serial.read???)
                       if (noteByte == 60){
                          digitalWrite(led,HIGH);
                          delay(500); 
                        } else {
                      digitalWrite(led,LOW);
                     }
             }  
      }
    }
  }
  while (Serial.available() > 2);//when at least three bytes available
}
