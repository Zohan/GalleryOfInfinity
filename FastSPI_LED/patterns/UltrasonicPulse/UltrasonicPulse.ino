//
// Sonar Stuff
//

#include "NewPing.h"

#define TRIGGER_PIN  A0  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     A1  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

//
// LED stuff
//

#include "FastSPI_LED.h"

// How many leds in your strip?
#define NUM_LEDS               44
#define PATTERN_LENGTH         6
#define NUM_STATES             (PATTERN_LENGTH + 2)
#define LED_DATA_PIN           11

// color wheel constants
#define RED    85
#define GREEN  0
#define BLUE   170

struct CRGB { unsigned char g; unsigned char r; unsigned char b; };

// a pointer for array of LEDs
struct CRGB *leds;

//
// globals
//

int g_loop_state;

unsigned long previousFlickerMillis = 0;
unsigned long flickerInterval       = 200;      // ms
unsigned long previousColorChangeMillis = 0;
unsigned long colorChangeInterval   = 1000;
unsigned long pulseInterval = 200;
unsigned long previousPulseMillis = 0;

unsigned int baseColor = 60;
//byte currentColor = 60;
byte nextColor = 60;
unsigned int transitionAmount = 2;

byte brightness;
int colorState = 0;
int markLocation = 0;
byte bgColor, markColor;
bool brightnessUp;

void setup() {
  delay(300);
  
  FastSPI_LED.setLeds(NUM_LEDS);
  
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_SM16716);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_TM1809);
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);

  FastSPI_LED.setPin(LED_DATA_PIN);
  
  FastSPI_LED.init();
  FastSPI_LED.start();
  leds = (struct CRGB *) FastSPI_LED.getRGBData();

  FastSPI_LED.show();
  
  g_loop_state = 0;
  brightness = random(20, 40);
  
  // set strip to red
  struct CRGB colorBuffer;
  setColor(&colorBuffer, RED);
  bgColor = RED;
  
  brightnessUp = true;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = (colorBuffer.r * brightness) >> 8;
    leds[i].g = (colorBuffer.g * brightness) >> 8;
    leds[i].b = (colorBuffer.b * brightness) >> 8;
  }
}

void loop() {
  unsigned int sonarReading;
  
  sonarReading = sonar.ping_cm();
  updateGlobalVariables(sonarReading);
  
  drawStrip();
}

void drawStrip() {
  int i;
  struct CRGB colorBuffer;
  
  setColor(&colorBuffer, bgColor);
  
  for (i = 0; i < NUM_LEDS; i++) {
    leds[i].r = (colorBuffer.r * brightness) >> 8;
    leds[i].g = (colorBuffer.g * brightness) >> 8;
    leds[i].b = (colorBuffer.b * brightness) >> 8;
  }
    
  FastSPI_LED.show();
}

void updateGlobalVariables(unsigned int sonarReading) {
  unsigned long currentMillis = millis();
  
  if (sonarReading == NO_ECHO) {
    flicker(currentMillis);
    
  } else if (sonarReading > (MAX_DISTANCE / 2)) {
    flicker(currentMillis);
    
    if (currentMillis - previousColorChangeMillis > colorChangeInterval) {
      previousColorChangeMillis = currentMillis;
      colorChangeInterval = 1000 - sonarReading;
      
      colorChange();
    }
    
  } else {
    pulse(currentMillis);
    
    if (brightness == 0)
      colorChange();
  }
}

void flicker(unsigned long currentMillis) {
  if (currentMillis - previousFlickerMillis > flickerInterval) {
    previousFlickerMillis = currentMillis;
    
    if (brightness > 60)
      brightness -= 10;
    else
      brightness = random(20, 40);
  }
}

void colorChange() {
  if (bgColor == RED)
    bgColor = GREEN;
  else if (bgColor == GREEN)
    bgColor = BLUE;
  else
    bgColor = RED;
}

void pulse(unsigned long currentMillis) {
  if (currentMillis - previousPulseMillis > pulseInterval) {
    previousPulseMillis = currentMillis;
    
    if (brightnessUp) {
      brightness += 10;
      if (brightness > 200) brightnessUp = false;
    } else {
      if (brightness < 10) {
        brightness = 0;
        brightnessUp = true;
      } else {
        brightness -= 10;
      }
    }
  }
}

void setColor(struct CRGB *rtrn, byte code) {
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

