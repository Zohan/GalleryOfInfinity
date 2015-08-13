#ifdef __AVR__
  #include <avr/power.h>
#endif
#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 40

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 3
#define CLOCK_PIN 13

#define echoPin A1 // Echo Pin = Analog Pin 0
#define trigPin A0 // Trigger Pin = Analog Pin 1

volatile long duration; // Duration used to calculate distance
volatile long HR_dist=0; // Calculated Distance
 
const int minimumRange=1; //Minimum Sonar range
const int maximumRange=120; //Maximum Sonar Range. Maximum Range is 200, but we don't want more than 5 feet of behavior.

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() { 
  //Setup the trigger and Echo pins of the HC-SR04 sensor
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = 1000;            // compare match register 16MHz/256/1000 = 62.5Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (0 << CS10);    // 256 prescaler
  TCCR1B |= (0 << CS11);    // 256 prescaler
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
	FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
}

void loop() { 
	// First slide the led in one direction
	for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i] = CRGB::Red;
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		leds[i] = CRGB::Black;
		// Wait a little bit before we loop around and do it again
    FastLED.setBrightness( 255-(HR_dist/2)*3 );
		delay(HR_dist);
	}

	// Now go in the other direction.  
	for(int i = NUM_LEDS-1; i >= 0; i--) {
		// Set the i'th led to red 
		leds[i] = CRGB::Red;
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		leds[i] = CRGB::Black;
		// Wait a little bit before we loop around and do it again
    FastLED.setBrightness( 255-(HR_dist/2)*3 );
		delay(HR_dist);
	}
}

/*--------------------getDistance() FUNCTION ---------------*/
void getDistance(){ 
 
 /* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */ 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 
 
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 HR_dist = duration/58.2;
 
 /*Send the reading from the ultrasonic sensor to the computer */
 if (HR_dist >= maximumRange || HR_dist <= minimumRange){
   HR_dist = 200;
 /* Send a 0 to computer and Turn LED ON to indicate "out of range" */
 Serial.println("0");
 //digitalWrite(LEDPin, HIGH); 
 } else {
 /* Send the distance to the computer using Serial protocol, and
 turn LED OFF to indicate successful reading. */
 Serial.println(HR_dist);
 //digitalWrite(LEDPin, LOW);
 }
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  getDistance();
}
