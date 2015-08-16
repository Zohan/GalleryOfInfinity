#include "FastSPI_LED.h"

// How many leds in your strip?
#define NUM_LEDS 45

struct CRGB {
  unsigned char b;
  unsigned char r;
  unsigned char g;
};
// Define the array of leds
struct CRGB *leds;

void setup() {
  delay(300);
  FastSPI_LED.setLeds(NUM_LEDS);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_SM16716);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_TM1809);
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
  //FastSPI_LED.setPin(3);

  FastSPI_LED.init();
  FastSPI_LED.start();
  leds = (struct CRGB*)FastSPI_LED.getRGBData();
}

void loop() {
  // First slide the led in one direction
  for (int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 255;
    // Show the leds
    FastSPI_LED.show();
    delay(30);
    // now that we've shown the leds, reset the i'th led to black
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 0;
    // Wait a little bit before we loop around and do it again
  }

  // Now go in the other direction.
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    // Set the i'th led to red
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 255;
    // Show the leds
    FastSPI_LED.show();
    delay(30);
    // now that we've shown the leds, reset the i'th led to black
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 255;
    // Wait a little bit before we loop around and do it again

  }
}
