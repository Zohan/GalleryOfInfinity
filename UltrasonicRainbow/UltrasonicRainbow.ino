#include "TimerOne.h"
#include "LPD6803.h"
#include "NewPing.h"

//Example to control LPD6803-based RGB LED Modules in a strand
// Original code by Bliptronics.com Ben Moyes 2009
//Use this as you wish, but please give credit, or at least buy some of my LEDs!

// Code cleaned up and Object-ified by ladyada, should be a bit easier to use

/*****************************************************************************/

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 3;       // 'yellow' wire
int clockPin = 13;      // 'green' wire
// Don't forget to connect 'blue' to ground and 'red' to +5V

// Timer 1 is also used by the strip to send pixel clocks

// Set the first variable to the NUMBER of pixels. 20 = 20 pixels in a row
LPD6803 strip = LPD6803(50, dataPin, clockPin);

#define TRIGGER_PIN  A0  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     A1  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 120 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define ZERO_COUNT_LIMIT 5
unsigned int pingSpeed = 100; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
volatile byte zeroCount=0; //Keeps track of the number of '0's seen. Too many 0s = value of 200.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

volatile unsigned int uS;
volatile unsigned int distance = 120;

void setup() {
  //initTimer2();
Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
  // The Arduino needs to clock out the data to the pixels
  // this happens in interrupt timer 1, we can change how often
  // to call the interrupt. setting CPUmax to 100 will take nearly all all the
  // time to do the pixel updates and a nicer/faster display, 
  // especially with strands of over 100 dots.
  // (Note that the max is 'pessimistic', its probably 10% or 20% less in reality)
  
  strip.setCPUmax(80);  // start with 50% CPU usage. up this if the strand flickers or is slow
  
  // Start up the LED counter
  strip.begin();
  // Update the strip, to start they are all 'off'
  strip.show();
}


void loop() {
  // Some example procedures showing how to display to the pixels
  
  //colorWipe(Color(255, 0, 0), 50);
  //colorWipe(Color(0, 255, 0), 50);
  //colorWipe(Color(0, 0, 255), 50);
  //strip.setPixelColor(0, Color(0, 0, 0 ));
  //strip.setPixelColor(1, Color(63, 0, 0 ));

  //rainbow(50);
  rainbowCycle(50);
}

void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 96 * 3; j++) {     // 3 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 96));
    }
    
    strip.show();   // write all the pixels out
    uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
    delay(25+uS / US_ROUNDTRIP_CM);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 96 * 5; j++) {     // 5 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      int preBrightVal = Wheel( ((i * 96 / strip.numPixels()) + j) % 96);
      byte distanceResult = distance/4;
      int brightnessChange = 31-distanceResult;
      if(distance > MAX_DISTANCE) brightnessChange = 0;
      int color = setBrightness(preBrightVal,brightnessChange);
      
      strip.setPixelColor(i, color);
    }  
    strip.show();   // write all the pixels out
    getDistance();
    delay(25+uS / US_ROUNDTRIP_CM);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint16_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

// Create a 15 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b)
{
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}


// Val from 0-32
unsigned int setBrightness(int color, byte brightness)
{
  byte r,g,b;
  r = color & 0x1F;
  g = (color >>10) & 0x1F;
  b = (color >>5) & 0x1F;
  brightness &= 0x1F;
  //Serial.print(r);
  //Serial.print(" ");
  //Serial.print(g);
  //Serial.print(" ");
  //Serial.println(b);
  r = (r*brightness)>>5;
  g = (g*brightness)>>5;
  b = (b*brightness)>>5;
  return(Color(r,g,b));
}

//Input a value 0 to 127 to get a color value.
//The colours are a transition r - g -b - back to r
unsigned int Wheel(byte WheelPos)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break; 
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break; 
    case 2:
      b=31- WheelPos % 32;  //blue down 
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break; 
  }
  return(Color(r,g,b));
}

void getDistance() {
  if(millis() >= pingTimer) {
    uS = sonar.ping();
    long newDist = uS/US_ROUNDTRIP_CM;
    if (newDist >= MAX_DISTANCE || newDist <= 0){
      if(++zeroCount >= ZERO_COUNT_LIMIT) {
        zeroCount = 0;
        if(distance < MAX_DISTANCE) distance+=8;
        Serial.println(distance);
      }
    } else {
        distance = newDist;
        Serial.println(distance);
    }
  }
}


    
    
