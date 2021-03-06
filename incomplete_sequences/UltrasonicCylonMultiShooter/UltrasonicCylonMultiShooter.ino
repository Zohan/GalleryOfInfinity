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
#define ZERO_COUNT_LIMIT 10
unsigned int pingSpeed = 100; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
unsigned long pingTimer2;     // Holds the next ping time.
volatile byte zeroCount=0; //Keeps track of the number of '0's seen. Too many 0s = value of 200.
uint8_t shotNum = 0;
uint8_t shotFired = false;
uint8_t shotPrepared = false;
uint8_t shotDistance = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

volatile unsigned int uS;
volatile unsigned int distance = 50;

#define MAXSTEPS       3 // Process (up to) this many concurrent steps
int
  stepMag[MAXSTEPS],  // Magnitude of steps
  stepX[MAXSTEPS],    // Position of 'step wave' along strip
  mag[NUM_LEDS], // Brightness buffer (one side of shoe)
  stepFiltered,       // Current filtered pressure reading
  stepCount,          // Number of 'frames' current step has lasted
  stepMin;            // Minimum reading during current step

void setup()
{
  Serial.begin(9600);
    FastSPI_LED.setLeds(NUM_LEDS);
    FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
    FastSPI_LED.setPin(PIN);
    FastSPI_LED.init();
    FastSPI_LED.start();
  Serial.println("Beginning");
    leds = (struct CRGB*)FastSPI_LED.getRGBData();   // set leds pointing at the buffer
}

void loop() { 
  uint8_t i, j;
  
  getDistance();
  //if(shotFired) {
    int mx1, px1, px2, m;
    memset(mag, 0, sizeof(mag));    // Clear magnitude buffer
    for(i=0; i<MAXSTEPS; i++) {     // For each step...
      if(stepMag[i] <= 0) continue; // Skip if inactive
      for(j=0; j<NUM_LEDS; j++) { // For each LED...
        // Each step has sort of a 'wave' that's part of the animation,
        // moving from heel to toe.  The wave position has sub-pixel
        // resolution (4X), and is up to 80 units (20 pixels) long.
        mx1 = (j << 2) - stepX[i]; // Position of LED along wave
        if((mx1 <= 0) || (mx1 >= 80)) continue; // Out of range
        if(mx1 > 64) { // Rising edge of wave; ramp up fast (4 px)
          m = ((long)stepMag[i] * (long)(80 - mx1)) >> 4;
        } else { // Falling edge of wave; fade slow (16 px)
          m = ((long)stepMag[i] * (long)mx1) >> 6;
        }
        mag[j] += m; // Add magnitude to buffered sum
      }
      stepX[i]++; // Update position of step wave
      if(stepX[i] >= (80 + (NUM_LEDS << 2)))
        stepMag[i] = 0; // Off end; disable step wave
      else
        stepMag[i] = ((long)stepMag[i] * 127L) >> 7; // Fade
    }
    // For a little visual interest, some 'sparkle' is added.
    // The cumulative step magnitude is added to one pixel at random.
    long sum = 0;
    for(i=0; i<MAXSTEPS; i++) sum += stepMag[i];
    if(sum > 0) {
      i = random(NUM_LEDS);
      mag[i] += sum / 4;
    }
   
    // Now the grayscale magnitude buffer is remapped to color for the LEDs.
    // The code below uses a blackbody palette, which fades from white to yellow
    // to red to black.  The goal here was specifically a "walking on fire"
    // aesthetic, so the usual ostentatious rainbow of hues seen in most LED
    // projects is purposefully skipped in favor of a more plain effect.
    uint8_t r, g, b;
    int     level;
    for(i=0; i<NUM_LEDS; i++) { // For each LED on one side...
      level = mag[i];                // Pixel magnitude (brightness)
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
      leds[i].r = r;
      leds[i].g = g;
      leds[i].b = b;
      // Pixels along inside are funny...
      //j = dup[i];
      //if(j < 255) strip.setPixelColor(j, r, g, b);
    }
  //}
  FastSPI_LED.show();               // turn them back on
  //shotFired = false;
}

void getDistance() {
  if(millis() >= pingTimer) {
    uS = sonar.ping();
    long newDist = uS/US_ROUNDTRIP_CM;
    if (newDist >= MAX_DISTANCE || newDist <= 0){
      if(++zeroCount >= ZERO_COUNT_LIMIT) {
        zeroCount = 0;
        if(shotPrepared) {
          Serial.println("Shot Fired");
          shotFired = true;
          shotPrepared = false;
          shotDistance = distance;
          stepMag[shotNum] = (255 - shotDistance) * 5; // Step intensity
          stepX[shotNum]   = -80; // Position starts behind heel, moves forward
          if(++shotNum >= MAXSTEPS) shotNum = 0; // If many, overwrite oldest
        }
      }
    } else {
        distance = newDist;
        //if(distance > 
        shotPrepared = true;
        Serial.print("Shots Prepared: ");
        Serial.println(distance);
        leds[0].r = 255-distance*5;
        leds[0].g = 255-distance*5;
        leds[0].b = 255-distance*5;
        FastSPI_LED.show();               // turn them back on
    }
  }
}






