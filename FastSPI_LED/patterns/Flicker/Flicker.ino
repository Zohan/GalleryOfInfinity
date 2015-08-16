#include "FastSPI_LED.h"

// How many leds in your strip?
#define NUM_LEDS               44
#define PATTERN_LENGTH         6
#define NUM_STATES             (PATTERN_LENGTH + 2)

struct CRGB { unsigned char g; unsigned char r; unsigned char b; };

// a pointer for array of LEDs
struct CRGB *leds;

int g_loop_state;

unsigned long previousFlickerMillis = 0;
unsigned long flickerInterval       = 200;      // ms
unsigned long previousColorChangeMillis = 0;
unsigned long colorChangeInterval   = 10000;

unsigned int baseColor = 60;
//byte currentColor = 60;
byte nextColor = 60;
unsigned int transitionAmount = 2;

byte brightness;
int colorState = 0;
struct CRGB currentColor;

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

  FastSPI_LED.show();
  
  g_loop_state = 0;
  brightness = random(20, 40);
  
  currentColor.r = 255;
  currentColor.g = 0;
  currentColor.b = 0;
}

void loop() {  
  struct CRGB colorBuffer;
  int i;
  
  updateGlobalVariables();
  //setColor(currentColor, &colorBuffer);
  
  for (i = 0; i < NUM_LEDS; i++) {
    leds[i].r = (currentColor.r * brightness) >> 8;
    leds[i].g = (currentColor.g * brightness) >> 8;
    leds[i].b = (currentColor.b * brightness) >> 8;
  }
  
  FastSPI_LED.show();
}

void updateGlobalVariables() {
  unsigned long currentMillis = millis();
  
  /*if (currentColor <= nextColor)
    currentColor += transitionAmount;
  else
    currentColor -= transitionAmount;*/
  
  if (/*currentColor < nextColor + 20 &&*/
      currentMillis - previousFlickerMillis > flickerInterval) {
        
    previousFlickerMillis = currentMillis;
    if (brightness > 60)
      brightness -= 10;
    else
      brightness = random(20, 40);
  }
  
  if (currentMillis - previousColorChangeMillis > colorChangeInterval) {
    previousColorChangeMillis = currentMillis;
    //currentColor += 15;
    
    switch (colorState) {
    case 0:
      currentColor.r = 255;
      currentColor.g = 0;
      currentColor.b = 0;
      colorState = 1;
      break;
     
    case 1:
      currentColor.r = 0;
      currentColor.g = 255;
      currentColor.b = 0;
      colorState = 2;
      break;
    
    default:
      currentColor.r = 0;
      currentColor.g = 0;
      currentColor.b = 255;
      colorState = 0;
      break;
    }
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

