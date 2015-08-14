/* Ultrasonic Cylon Shooter
 *  Ultrasonic sensor behaves like a 'switch' with the distance
 *  determining the strength of the 'shot' which is sent
 *  to the midpoint on the strip
 */

#include "FastSPI_LED.h"
#include "NewPing.h"

// There are 30 LEDs per meter on the MD strips but 3 LEDs are wired in series, so there are
// 10 LEDs (pixels really) per meter. Set "#define NUM_LEDS 50" to however many meters you have
// times 10. Or count the number of chips on your section of strip.

#define NUM_LEDS 50
#define DELAY_SPEED 2  // larger numbers are slower - in mS change to suit

// The folowing struct is definitely correct for the Modern Device strips
// There must be some chipsets with blue and green swapped out there
struct CRGB { 
    unsigned char g; 
    unsigned char r; 
    unsigned char b; 
};

struct CRGB *leds; //pointer to the beginning of the struct

#define PIN 11    // change to your data pin

// This is an array of rgb pixels to display. Feel free to add, delete, 
// or change this anyway you wish but be sure to add or delete values in r,g,b triplets.
// One triplet represents a pixel to display
unsigned char pixelArr[]={ 
    40,40,40};

char temp[3];    // one pixel buff

int maxPixArray = sizeof(pixelArr) / 3;   // lets sketches know where to wrap around the pixelArray

unsigned long ledRefreshTimer = 0;     // Holds the next refresh time
unsigned int ledRefreshRate = 2; // How frequently are we going to send out a refresh (in milliseconds) to LEDs.

#define TRIGGER_PIN  A0  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     A1  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN2  A2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN2     A3  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define ZERO_COUNT_LIMIT 10
unsigned int pingSpeed = 100; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
volatile byte zeroCount=0; //Keeps track of the number of '0's seen. Too many 0s = value of 200.
uint8_t shotFired, shotPrepared, shot2Fired, shot2Prepared;
uint8_t shotDistance = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

volatile unsigned int uS;
volatile unsigned int distance = 50;

void setup()
{
  Serial.begin(9600);
    FastSPI_LED.setLeds(NUM_LEDS);
    FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
    FastSPI_LED.setPin(PIN);
    FastSPI_LED.init();
    FastSPI_LED.start();

    leds = (struct CRGB*)FastSPI_LED.getRGBData();   // set leds pointing at the buffer
}

void loop() { 
    //index through the Pixel Array of colors  new pixel color each time
  getDistance(sonar, 0);
  getDistance(sonar2, 1);
  if(shotFired) {
    for(int i = 0 ; i < maxPixArray ; i++ ) { 
      temp[0] = pixelArr[i * 3];            // r
      temp[1] = pixelArr[(i * 3) + 1];      // g
      temp[2] = pixelArr[(i * 3) + 2];      // b 
      memset(leds, 0, NUM_LEDS * 3);        // turns off all LEDs - try commenting this out
      for (int j = 0; j < NUM_LEDS; j++ ) {
          // Serial.println(j);
          memset(leds, 0, NUM_LEDS * 3);  // turns off all LEDs - try commenting this out
          leds[j].r = shotDistance;
          leds[j].g = shotDistance;
          leds[j].b = shotDistance;
          FastSPI_LED.show();               // turn them back on
          getDistance(sonar, 0);
          getDistance(sonar2, 1);
          if(shotPrepared) {
            leds[0].r = 255-distance*5;
            leds[0].g = 255-distance*5;
            leds[0].b = 255-distance*5;
            FastSPI_LED.show();               // turn them back on
          }
          if(shot2Prepared) {
            leds[NUM_LEDS-1].r = 255-distance*5;
            leds[NUM_LEDS-1].g = 255-distance*5;
            leds[NUM_LEDS-1].b = 255-distance*5;
            FastSPI_LED.show();               // turn them back on
          }
          delay(DELAY_SPEED+shotDistance);
      }
      memset(leds, 0, NUM_LEDS * 3);    // turns off all LEDs
      FastSPI_LED.show();              // display the beauty
      /*for (int j = NUM_LEDS - 1; j >= 0 ; j-- ) {
          // Serial.println(j);
          memset(leds, 0, NUM_LEDS * 3);  // turns off all LEDs - try commenting this out
          leds[j].r = temp[0];
          leds[j].g = temp[1];
          leds[j].b = temp[2];
          FastSPI_LED.show();              // display the beauty
          getDistance();
          delay(DELAY_SPEED);
      }*/
    }
  }
  shotFired = false;
}

void getDistance(NewPing theSonar, uint8_t sonarNum) {
  if(millis() >= pingTimer) {
    uS = theSonar.ping();
    long newDist = uS/US_ROUNDTRIP_CM;
    if (newDist >= MAX_DISTANCE || newDist <= 0){
      if(++zeroCount >= ZERO_COUNT_LIMIT) {
        zeroCount = 0;
        if(shotPrepared) {
          Serial.println("Shot Fired");
          shotFired = true;
          shotPrepared = false;
          shotDistance = distance;
        }
        if(shot2Prepared) {
          Serial.println("Shot 2 Fired");
          shot2Fired = true;
          shot2Prepared = false;
          shotDistance = distance;
        }
      }
    } else {
        distance = newDist;
        //if(distance > 
        switch(sonarNum) {
          case 0:
            shotPrepared = true;
            Serial.print("Shot Prepared: ");
            Serial.println(distance);
            break;

          case 1:
            Serial.print("Shot 2 Prepared: ");
            Serial.println(distance);
            shot2Prepared = true;
            break;

        }
        
    }
  }
}





