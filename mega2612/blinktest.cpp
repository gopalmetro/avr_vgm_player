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