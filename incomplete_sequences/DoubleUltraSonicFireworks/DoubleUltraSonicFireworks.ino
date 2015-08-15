/* Ultrasonic Cylon Shooter
 *  Ultrasonic sensor behaves like a 'switch' with the distance
 *  determining the strength of the 'shot' which is sent
 *  to the midpoint on the strip
 */

#include "FastSPI_LED.h"
#include "NewPing.h"
#include <math.h>

// There are 30 LEDs per meter on the MD strips but 3 LEDs are wired in series, so there are
// 10 LEDs (pixels really) per meter. Set "#define NUM_LEDS 50" to however many meters you have
// times 10. Or count the number of chips on your section of strip.

#define NUM_LEDS 50
#define NUM_LEDS_HALF 25
#define LED_REAR 25
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

const uint8_t gamma[] PROGMEM = { // Gamma correction table for LED brightness
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

char temp[3];    // one pixel buff

int maxPixArray = sizeof(pixelArr) / 3;   // lets sketches know where to wrap around the pixelArray

unsigned long ledRefreshTimer = 0;     // Holds the next refresh time
unsigned int ledRefreshRate = 2; // How frequently are we going to send out a refresh (in milliseconds) to LEDs.

#define TRIGGER_PIN  A0  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     A1  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN2  A2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN2     A3  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define ZERO_COUNT_LIMIT 12
unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
unsigned long pingTimer2;     // Holds the next ping time.
volatile int zeroCount=0; //Keeps track of the number of '0's seen. Too many 0s = value of 200.
uint8_t shotFired, shotPrepared, shot2Fired, shot2Prepared, shotNum, explosionNum;
uint8_t shotDistance = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

volatile unsigned int uS;
volatile unsigned int distance = 50;

#define MAXSTEPS       30 // Process (up to) this many concurrent steps
int
  explosionMag[MAXSTEPS],  // Magnitude of steps
  explosionStepX[MAXSTEPS],    // Position of 'step wave' along strip
  mag[NUM_LEDS]; // Brightness buffer (one side of shoe)

#define MAXSHOTS  30;

int
  shotMag[MAXSTEPS],    // Position of 'step wave' along strip
  shotStepX[MAXSTEPS],    // Position of 'step wave' along strip
  shotDirection[MAXSTEPS]; // If shot came from the back or the front

void setup()
{
  Serial.begin(19200);
    FastSPI_LED.setLeds(NUM_LEDS);
    FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
    FastSPI_LED.setPin(PIN);
    FastSPI_LED.init();
    FastSPI_LED.start();
  Serial.println("Beginning");
  memset(shotDirection, 0, sizeof(shotDirection));    // Clear magnitude buffer
    leds = (struct CRGB*)FastSPI_LED.getRGBData();   // set leds pointing at the buffer
    addExplosion(8);
    shotNum = 0;
    explosionNum = 0;
}

void loop() {   
  getDistance(sonar, 0);
  getDistance(sonar2, 1);
  if(shotFired) {
    shotFired = false;
    addShot(0);
  }
  if(shot2Fired) {
    addShot(1);
    shot2Fired = false;
  }

  memset(mag, 0, sizeof(mag));    // Clear magnitude buffer
  explode();
  moveShots();
  intersectionCheck();
  
  
  if(shotPrepared) {
    leds[0].r = 255-distance*5;
    leds[0].g = 255-distance*5;
    leds[0].b = 255-distance*5;
  } else {
    leds[0].r = 0;
    leds[0].g = 0;
    leds[0].b = 0;
  }
  if(shot2Prepared) {
    leds[NUM_LEDS-1].r = 255-distance*5;
    leds[NUM_LEDS-1].g = 255-distance*5;
    leds[NUM_LEDS-1].b = 255-distance*5;
  } else {
    leds[NUM_LEDS-1].r = 0;
    leds[NUM_LEDS-1].g = 0;
    leds[NUM_LEDS-1].b = 0;
  }
  FastSPI_LED.show();               // turn them back on
  delayMicroseconds(3000);
  
}

void addShot(uint8_t side) {
  shotMag[shotNum] = (255 - shotDistance) * 5; // Step intensity
  switch(side) {
    case 0:
      shotStepX[shotNum] = -40; // Position starts behind heel, moves forward
      break;

    case 1:
      shotStepX[shotNum] = 200; // Position starts behind heel, moves forward
      //shotStepX[shotNum] = -80; // Position starts behind heel, moves forward
      break;
    
  }
  shotDirection[shotNum] = side;
  if(++shotNum >= MAXSTEPS) shotNum = 0; // If many, overwrite oldest
}

void moveShots() {
  uint8_t i, j;
  int mx1, px1, px2, m;
  for(i=0; i<MAXSTEPS; i++) {     // For each step...
    if(shotMag[i] <= 0) continue; // Skip if inactive
    for(j=0; j<NUM_LEDS; j++) { // For each LED...
      mx1 = (j << 2) - shotStepX[i]; // Position of LED along wave
      if((mx1 <= 0) || (mx1 >= 10)) continue; // Out of range
      if(mx1 > 8) { // Rising edge of wave; ramp up fast (4 px)
        m = ((long)shotMag[i] * (long)(40 - mx1)) >> 3;
      } else { // Falling edge of wave; fade slow (16 px)
        m = ((long)shotMag[i] * (long)mx1) >> 4;
      }
      mag[j] += m; // Add magnitude to buffered sum
    }
    if(shotDirection[i] == 1) {
      shotStepX[i]--;
    } else {
      shotStepX[i]++;
    }
    //Serial.println(abs(shotStepX[i]));
    if(shotStepX[i] >= (120 + (NUM_LEDS_HALF << 2)) || shotStepX[i] <= -60) {
      shotMag[i] = 0; // Off end; disable step wave
    } else
      shotMag[i] = ((long)shotMag[i] * 127L) >> 7; // Fade
  }
  // Now the grayscale magnitude buffer is remapped to color for the LEDs.
  // The code below uses a blackbody palette, which fades from white to yellow
  // to red to black.  The goal here was specifically a "walking on fire"
  // aesthetic, so the usual ostentatious rainbow of hues seen in most LED
  // projects is purposefully skipped in favor of a more plain effect.
  uint8_t r, g, b;
  int     level;
  for(i=0; i<=NUM_LEDS; i++) { // For each LED on one side...
    level = mag[i];                // Pixel magnitude (brightness)
    //Serial.println(level);
    if(level < 255) {              // 0-254 = black to red-1
      r = pgm_read_byte(&gamma[level]);
      g = b = 0;
    } else if(level < 510) {       // 255-509 = red to yellow-1
      r = 255;
      g = pgm_read_byte(&gamma[level - 255]);
      b = 0;
    } else if(level < 765) {       // 510-764 = yellow to white-1
      r = g = 255;
      b = pgm_read_byte(&gamma[level - 510]);
    } else {                       // 765+ = white
      r = g = b = 255;
    }
    // Pixels along inside are funny...
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
}

bool intersectionCheck() {
  for(int i=1; i<MAXSTEPS; i++) {
    if(shotStepX[i-1] == shotStepX[i] && 
        shotMag[i-1] > 0 &&
        shotMag[i] > 0) {
      addExplosion(8);
      Serial.println("Explosion");
    }
  }
  return false;
}

void addExplosion(uint8_t LED) {
  explosionMag[explosionNum] = 1000; // Step intensity
  explosionStepX[explosionNum] = -20; // Position starts behind heel, moves forward
  if(++explosionNum >= MAXSTEPS) explosionNum = 0; // If many, overwrite oldest
}

void explode() {
  uint8_t i, j;
  int mx1, px1, px2, m;
 // memset(mag, 0, sizeof(mag));    // Clear magnitude buffer
  for(i=0; i<MAXSTEPS; i++) {     // For each step...
    if(explosionMag[i] <= 0) continue; // Skip if inactive
    for(j=0; j<NUM_LEDS; j++) { // For each LED...
      // Each step has sort of a 'wave' that's part of the animation,
      // moving from heel to toe.  The wave position has sub-pixel
      // resolution (4X), and is up to 80 units (20 pixels) long.
      mx1 = (j << 2) - explosionStepX[i]; // Position of LED along wave
      if((mx1 <= 0) || (mx1 >= 80)) continue; // Out of range
      if(mx1 > 64) { // Rising edge of wave; ramp up fast (4 px)
        m = ((long)explosionMag[i] * (long)(80 - mx1)) >> 4;
      } else { // Falling edge of wave; fade slow (16 px)
        m = ((long)explosionMag[i] * (long)mx1) >> 6;
      }
      mag[j] += m; // Add magnitude to buffered sum
    }
    explosionStepX[i]++; // Update position of step wave
    if(explosionStepX[i] >= (80 + (NUM_LEDS_HALF << 2)))
      explosionMag[i] = 0; // Off end; disable step wave
    else
      explosionMag[i] = ((long)explosionMag[i] * 127L) >> 7; // Fade
  }
  // For a little visual interest, some 'sparkle' is added.
  // The cumulative step magnitude is added to one pixel at random.
  long sum = 0;
  for(i=0; i<MAXSTEPS; i++) sum += explosionMag[i];
  if(sum > 0) {
    i = random(NUM_LEDS_HALF);
    mag[i] += sum / 4;
  }
 
  // Now the grayscale magnitude buffer is remapped to color for the LEDs.
  // The code below uses a blackbody palette, which fades from white to yellow
  // to red to black.  The goal here was specifically a "walking on fire"
  // aesthetic, so the usual ostentatious rainbow of hues seen in most LED
  // projects is purposefully skipped in favor of a more plain effect.
  uint8_t r, g, b;
  int     level;
  for(i=0; i<=NUM_LEDS; i++) { // For each LED on one side...
    level = mag[i];                // Pixel magnitude (brightness)
    //Serial.println(level);
    if(level < 255) {              // 0-254 = black to red-1
      r = pgm_read_byte(&gamma[level]);
      g = b = 0;
    } else if(level < 510) {       // 255-509 = red to yellow-1
      r = 255;
      g = pgm_read_byte(&gamma[level - 255]);
      b = 0;
    } else if(level < 765) {       // 510-764 = yellow to white-1
      r = g = 255;
      b = pgm_read_byte(&gamma[level - 510]);
    } else {                       // 765+ = white
      r = g = b = 255;
    }
    // Set R/G/B color along outside of shoe
    leds[NUM_LEDS-i].r = r;
    leds[NUM_LEDS-i].g = g;
    leds[NUM_LEDS-i].b = b;
    // Pixels along inside are funny...
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
}

void getDistance(NewPing theSonar, uint8_t sonarNum) {
  long pingSelection;
  switch(sonarNum) {
    case 0:
      pingSelection = pingTimer;
      break;
  
    case 1:
      pingSelection = pingTimer2;
      break;
  }
  if(millis() - pingSelection >= pingSpeed) {
    switch(sonarNum) {
      case 0:
        pingTimer = millis();
        break;
    
      case 1:
        pingTimer2 = millis();
        break;
    }
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
      if(--zeroCount <= 0) zeroCount = 0;
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






