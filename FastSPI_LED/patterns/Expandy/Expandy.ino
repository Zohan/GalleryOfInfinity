#include "FastSPI_LED.h"

// How many leds in your strip?
#define NUM_LEDS               150
#define PATTERN_LENGTH         6
#define NUM_STATES             (PATTERN_LENGTH + 2)

struct CRGB { unsigned char g; unsigned char r; unsigned char b; };

// a pointer for array of LEDs
struct CRGB *leds;

struct CRGB color1;
struct CRGB color2;

int g_loop_state;

void setup() { 
  delay(300);
  
  FastSPI_LED.setLeds(NUM_LEDS);
  
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_SM16716);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_TM1809);
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);

  FastSPI_LED.setPin(3);
  
  FastSPI_LED.init();
  FastSPI_LED.start();
  leds = (struct CRGB *) FastSPI_LED.getRGBData();
  
  color1.r = 255;
  color1.g = 0;
  color1.b = 0;
  
  color2.r = 0;
  color2.g = 0;
  color2.b = 255;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = color1.r;
    leds[i].g = color1.g;
    leds[i].b = color1.b;
  }
  
  FastSPI_LED.show();
  
  g_loop_state = 0;
}

void loop() {
  int state, index, half_index;

  switch (g_loop_state) {
  case 0:
    for (index = 0; index < NUM_LEDS; index += PATTERN_LENGTH) {
      leds[index].r = color2.r;
      leds[index].g = color2.g;
      leds[index].b = color2.b;
    }
    
    FastSPI_LED.show();
    delay(300);
    
    break;
    
  case 1:
    // phase 1, states 0-4
    for (state = 0; state < (NUM_STATES / 2) - 2; state++) {
      index = state + 1;
      half_index = PATTERN_LENGTH - 1 - state;
      
      while (index < NUM_LEDS) {
        phase1(index, half_index);
        index += PATTERN_LENGTH;
        half_index += PATTERN_LENGTH;
      }
      
      FastSPI_LED.show();
      delay(300);
    }
    
    break;
    
  case 2:
    // phase 2, state 5
    index = ((int) PATTERN_LENGTH / 2);
    
    while (index < NUM_LEDS) {
      leds[index].r = color2.r;
      leds[index].g = color2.g;
      leds[index].b = color2.b;
      
      index += PATTERN_LENGTH;
    }

    FastSPI_LED.show();
    delay(300);
    
    break;
    
  case 3:
    // phase 3, state 6
    index = ((int) PATTERN_LENGTH / 2);
    
    while (index < NUM_LEDS) {
      leds[index].r = color1.r;
      leds[index].g = color1.g;
      leds[index].b = color1.b;
      
      index += PATTERN_LENGTH;
    }
    
    FastSPI_LED.show();
    delay(300);
    
    break;
  
  case 4:
    // phase 4, state 7-11
    for (state = (NUM_STATES / 2) - 2 - 1; state >= 0; state--) {
      index = state + 1;
      half_index = PATTERN_LENGTH - 1 - state;
      
      while (index < NUM_LEDS) {
        phase3(index, half_index);
        index += PATTERN_LENGTH;
        half_index += PATTERN_LENGTH;
      }
      
      FastSPI_LED.show();
      delay(300);
    }
    
    break;
    
  default:
    for (index = 0; index < NUM_LEDS; index += PATTERN_LENGTH) {
      leds[index].r = color1.r;
      leds[index].g = color1.g;
      leds[index].b = color1.b;
    }
    
    FastSPI_LED.show();
    delay(300);
  }

  if (++g_loop_state == 6)
    g_loop_state = 0;
}

void phase1(int index, int half_index) {
  leds[index].r = color2.r;
  leds[index].g = color2.g;
  leds[index].b = color2.b;
  
  if (half_index < NUM_LEDS) {
    leds[half_index].r = color2.r;
    leds[half_index].g = color2.g;
    leds[half_index].b = color2.b;
  }
}

void phase3(int index, int half_index) {
  leds[index].r = color1.r;
  leds[index].g = color1.g;
  leds[index].b = color1.b;
  
  if (half_index < NUM_LEDS) {
    leds[half_index].r = color1.r;
    leds[half_index].g = color1.g;
    leds[half_index].b = color1.b;
  }
}

/*	// First slide the led in one direction
	for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i].r = 0;
                leds[i].g = 0;
                leds[i].b = 255;
		// Show the leds
		FastSPI_LED.show();
                delay(30);
		// now that we've shown the leds, reset the i'th led to black
		leds[i].r = 255;
                leds[i].g = 0;
                leds[i].b = 0;
		// Wait a little bit before we loop around and do it again
	}

	// Now go in the other direction.  
	for(int i = NUM_LEDS-1; i >= 0; i--) {
		// Set the i'th led to red 
		leds[i].r = 0;
                leds[i].g = 0;
                leds[i].b = 255;
		// Show the leds
		FastSPI_LED.show();
                delay(30);
		// now that we've shown the leds, reset the i'th led to black
		leds[i].r = 255;
                leds[i].g = 0;
                leds[i].b = 0;
		// Wait a little bit before we loop around and do it again
		
	}*/


