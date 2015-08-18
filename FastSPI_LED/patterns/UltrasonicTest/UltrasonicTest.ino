#include "NewPing.h"
#include "FastSPI_LED.h"

// How many leds in your strip?
#define NUM_LEDS               44
#define LED_DATA_PIN           11

struct CRGB { unsigned char g; unsigned char r; unsigned char b; };

// a pointer for array of LEDs
struct CRGB *leds;
struct CRGB currentColor;

unsigned long colorUpdateInterval = 500;
unsigned long previousUpdate = 0;

#define TRIGGER_PIN  A0  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     A1  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
  int i;
  
  FastSPI_LED.setLeds(NUM_LEDS);
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
  FastSPI_LED.setPin(LED_DATA_PIN);

  FastSPI_LED.init();
  FastSPI_LED.start();
  leds = (struct CRGB *) FastSPI_LED.getRGBData();

  FastSPI_LED.show();
  
  currentColor.r = 255;
  currentColor.g = 0;
  currentColor.b = 0;
  
  for (i = 0; i < NUM_LEDS; i++) {
    leds[i].r = currentColor.r;
    leds[i].g = currentColor.g;
    leds[i].b = currentColor.b;
  }
  
  FastSPI_LED.show();
}

void loop() {
  unsigned long now = millis();
  unsigned int sonarReading;
  byte cmDist;
  int i;
  
  if (now - previousUpdate >= colorUpdateInterval) {
    sonarReading = sonar.ping();
    
    if (sonarReading == NO_ECHO) {
      currentColor.r = 255;
      currentColor.g = 255;
      currentColor.b = 255;
    } else {
      cmDist = (byte) (sonarReading / US_ROUNDTRIP_CM);
      setColor(cmDist << 4, &currentColor);
    }
    
    for (i = 0; i < NUM_LEDS; i++) {
      leds[i].r = currentColor.r;
      leds[i].g = currentColor.g;
      leds[i].b = currentColor.b;
    }
    
    FastSPI_LED.show();
    previousUpdate = now;
  }
}

void setColor(byte code, struct CRGB *rtrn) {
  if (code < 85) {
    rtrn->r = code * 3;
    rtrn->g = 255 - (code*3);
    rtrn->b = 0;
  } else if (code < 170) {
    code -= 85;
    
    rtrn->r = 255 - (code*3);
    rtrn->g = 0;
    rtrn->b = code * 3;
  } else {
    code -= 170;
    
    rtrn->r = 0;
    rtrn->g = code * 3;
    rtrn->b = 255 - (code*3);
  }
}
