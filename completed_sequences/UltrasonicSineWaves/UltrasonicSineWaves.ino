/* SineWaves
 Uses three sine waves to create a complex pattern.
 Example Code for use with Modern Device Serial LED strips http://shop.moderndevice.com/products/rgb-led-strips-ucs1903
 This code is in the  public domain. moderndevice.com  Paul Badger 4-2013
 The FastSPI_LED library carries an MIT license. http://code.google.com/p/fastspi/
 
 Hardware Setup:
 2 connectiions to your Arduino: Ground (white wire on orignal stip header - use black if you are soldering on wires) 
 to Arduino Ground and Data (blue wire) to data pin (see "PIN" below).
 2 connections from 12 volt power supply to the strip Ground (black wire) and +12 volts (red wire)
 to the stip.
 */

#include "FastSPI_LED.h"
#include "NewPing.h"

// There are 30 LEDs per meter on the MD strips but 3 LEDs are wired in series, so there are
// 10 LEDs (pixels really) per meter. Set "#define NUM_LEDS 50" to however many meters you have
// times 10. Or count the number of chips on your section of strip.

#define NUM_LEDS 50
uint8_t DELAY_SPEED = 30;
const uint8_t LED_MIDPOINT = NUM_LEDS/2;
const unsigned int COLOR_MAX = NUM_LEDS * 3;      // controls color change speed, change multiplier

// x_COLOR_DEPTH sets the variation in one color, smaller values = less variation, values
// greater than 128 will clip (go to 0 or 256 for longer than "normal"). Experiment!
uint8_t R_COLOR_DEPTH=0;       // valid values btwn 1 and 128   1 = pastel white, 128 is brightly colored 
uint8_t G_COLOR_DEPTH=20;
uint8_t B_COLOR_DEPTH=20;

// in most circumstances you probably want OFFSET to be about the same value as COLOR_DEPTH, 
// but changing the offset a bit can help add or subtract a particular primary from the changing mix.
// Experiment!
uint8_t R_OFFSET = 0;
uint8_t  G_OFFSET = 20;
uint8_t  B_OFFSET = 20;

#define PIN 11    // change to your data pin 

float sinValX, sinValY, sinValZ, X, Y, Z;    // floats for sine waves

// This is correct for the Modern Device RGB LED strips
// Somet chipsets wire must be backwards.
struct CRGB { 
    unsigned char g; 
    unsigned char r; 
    unsigned char b; 
};
// struct CRGB { unsigned char r; unsigned char g; unsigned char b; };

struct CRGB * leds;

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


void setup()
{
    Serial.begin(9600);
    FastSPI_LED.setLeds(NUM_LEDS);
    FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
    FastSPI_LED.setPin(PIN);
    FastSPI_LED.init();
    FastSPI_LED.start();
    leds = (struct CRGB*)FastSPI_LED.getRGBData(); 
    Serial.println("Start"); 
}

void loop() { 

    // increment variables - experiment with values!
    // really small nummbers can be useful for slow-moving effects
    sinValX  += .17;
    sinValY  += .2;
    sinValZ  += .3;
    memset(leds, 0, NUM_LEDS * 3);         // turns off all LEDs
    stripSine();
    getDistance();
    FastSPI_LED.show();
    delay(DELAY_SPEED + uS/US_ROUNDTRIP_CM);     // frame rate delay - OK to change
}

void stripSine() {
  // push data out to LEDs one pixel at a time
  for(int i = 0 ; i < NUM_LEDS; i++ ) {
      // sin() will return a float between -1 and 1
      // The we just have to scale the output to appropriate levels.
      // and add an offset to get rid of the negative numbers.

      X =  (sin(sinValX + (i * .3)) * R_COLOR_DEPTH) + R_OFFSET;
      X = constrain(X, 0, 255);
      Y= (sin(sinValY + (i * .2)) * G_COLOR_DEPTH) + G_OFFSET; 
      Y = constrain(Y, 0, 255);
      Z =  (sin(sinValZ + (i * .1)) * B_COLOR_DEPTH) + B_OFFSET; 
      Z = constrain(Z, 0, 255);

      leds[i].r = X;
      leds[i].g = Y;
      leds[i].b = Z;
  }
}

void getDistance() {
  if(millis() - pingTimer >= pingSpeed) {
    pingTimer = millis();
    uS = sonar.ping();
    long newDist = uS/US_ROUNDTRIP_CM;
    if (newDist >= MAX_DISTANCE || newDist <= 0){
      if(++zeroCount >= ZERO_COUNT_LIMIT) {
        zeroCount = 0;
        if(distance < MAX_DISTANCE) distance+=8;
        if(distance > MAX_DISTANCE) distance = MAX_DISTANCE;
        B_COLOR_DEPTH = 120-distance;
        G_COLOR_DEPTH = 120-distance;
        B_OFFSET = 120-distance;
        G_OFFSET = 120-distance;
        Serial.println(distance);
      }
    } else {
        distance = newDist;
        B_COLOR_DEPTH = 120-distance;
        G_COLOR_DEPTH = 120-distance;
        B_OFFSET = 120-distance;
        G_OFFSET = 120-distance;
        Serial.println(distance);
    }
  }
}






