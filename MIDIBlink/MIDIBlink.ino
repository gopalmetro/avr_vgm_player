/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  Serial.begin(38400);
}


// the loop routine runs over and over again forever:
void loop() {
  byte commandByte;
  byte noteByte;
  byte velocityByte;
  byte noteOn = 0x90;
  
  do{
    if (Serial.available()){

      commandByte = Serial.read();//read first byte
      noteByte = Serial.read();//read next byte
      velocityByte = Serial.read();//read final byte
      byte noteOn = 0x90;
      if (commandByte == noteOn){
                  digitalWrite(led,HIGH);
                    delay(5); 
                  } else {
                digitalWrite(led,LOW);
//  if note on message check if note == ?? and velocity > 0
                 if (noteByte == 69){  }
      }
    }
  }
  while (Serial.available() > 2);//when at least three bytes available
}
