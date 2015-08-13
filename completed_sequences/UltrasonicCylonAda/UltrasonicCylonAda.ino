#include "TimerOne.h"
#include "LPD6803.h"

// How many leds in your strip?
#define NUM_LEDS 50

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 3;       // 'yellow' wire
int clockPin = 13;      // 'green' wire
// Don't forget to connect 'blue' to ground and 'red' to +5V

// Timer 1 is also used by the strip to send pixel clocks

// Set the first variable to the NUMBER of pixels. 20 = 20 pixels in a row
LPD6803 strip = LPD6803(NUM_LEDS, dataPin, clockPin);

#define echoPin A1 // Echo Pin = Analog Pin 1
#define trigPin A0 // Trigger Pin = Analog Pin 0
#define ZERO_COUNT_LIMIT 2

volatile long duration; // Duration used to calculate distance
volatile long HR_dist=240; // Calculated Distance
volatile byte zeroCount=0; //Keeps track of the number of '0's seen. Too many 0s = value of 200.
volatile byte timer2Count;

const int minimumRange=1; //Minimum Sonar range
const int maximumRange=120; //Maximum Sonar Range. Maximum Range is 200, but we don't want more than 5 feet of behavior.

void setup() { 
  //Setup the trigger and Echo pins of the HC-SR04 sensor
  Serial.begin(9600);
  //Serial.println("Starting Cylon");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  initTimer2();
  cli();
  strip.setCPUmax(95);  // start with 50% CPU usage. up this if the strand flickers or is slow
  // Start up the LED counter
  strip.begin();
  //Serial.print("hello");
  sei();
}

void loop() { 
  //Serial.print("hello");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) { 
    // Set the i'th led to red 
    int8_t stripValue = maximumRange - HR_dist/2;
    strip.setPixelColor(i, Color(stripValue, 0, 0));
    strip.show();
    // now that we've shown the leds, reset the i'th led to black
    //strip.setPixelColor(i, Color(0, 0, 0));
    // Wait a little bit before we loop around and do it again
    //FastLED.setBrightness( 255-(HR_dist/2)*3 );
    delay(30+HR_dist);
    strip.setPixelColor(i, Color(0, 0, 0));
    
  }

  // Now go in the other direction.  
  for(int i = NUM_LEDS-1; i >= 0; i--) {
    // Set the i'th led to red 
    int stripValue = maximumRange - (HR_dist/2);
    strip.setPixelColor(i, Color(stripValue, 0, 0));
    strip.show();
    // now that we've shown the leds, reset the i'th led to black
    //strip.setPixelColor(i, Color(0, 0, 0));
    // Wait a little bit before we loop around and do it again
    //FastLED.setBrightness( 255-(HR_dist/2)*3 );
    delay(30+HR_dist);
    strip.setPixelColor(i, Color(0, 0, 0));
  }
}

// Create a 15 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b)
{
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)r & 0x1F)<<5 | (unsigned int)b & 0x1F);
}


/*--------------------getDistance() FUNCTION ---------------*/
void getDistance(){ 
 
 /* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */ 
 digitalWrite(trigPin, LOW); 
 //delayMicroseconds(2); 
 
 digitalWrite(trigPin, HIGH);
 //delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 long newDist = duration/58.2;
 /*Send the reading from the ultrasonic sensor to the computer */
  if (newDist >= maximumRange || newDist <= minimumRange){
    if(++zeroCount >= ZERO_COUNT_LIMIT) {
       zeroCount = 0;
       HR_dist+=60;
     }
     /* Send a 0 to computer and Turn LED ON to indicate "out of range" */
     //digitalWrite(LEDPin, HIGH); 
   } else {
     HR_dist = newDist;
     Serial.println(HR_dist);
     /* Send the distance to the computer using Serial protocol, and
     turn LED OFF to indicate successful reading. */
     //digitalWrite(LEDPin, LOW);
  }
}

// Timer section

void initTimer2() 
{
  // I like to disable global interrupts while initializing counter registers.
  // That may not be necessary, but...
  cli();
  int prescale  = 1024;
  int ocr2aval  = 160;

  // The following are scaled for convenient printing
  //
  // Interrupt interval in microseconds
  float iinterval = prescale * (ocr2aval+1) / (F_CPU / 1.0e6);

  // Period in microseconds
  float period    = 2.0 * iinterval;

  // Frequency in Hz
  float freq      = 1.0e6 / period;
  
  // Set Timer 2 CTC mode and prescale by 8
  //
  // WGM22:0 = 010: CTC Mode
  // WGM2 bits 1 and 0 are in TCCR2A,
  // WGM2 bit 2 is in TCCR2B
  //
  TCCR2A = (1 << WGM21);
  
  // Set Timer 2  No prescaling
  //
  // CS22:0 = 010: prescale = 8
  // CS2 bits 2:0 are all in TCCR2B
  TCCR2B = (1 << CS20)|(1 << CS21)|(1 << CS22);
  
  // Enable Compare-match register A interrupt for timer2
  TIMSK2 = (1 << OCIE2A);
  
  // This value determines the interrupt interval
  OCR2A = ocr2aval;
  sei();
}

ISR(TIMER2_COMPA_vect)
{
   timer2Count++;
   if(timer2Count >= 20) {
     timer2Count = 0;
     getDistance();
     if(HR_dist > 240) HR_dist = 240;
   }
}
