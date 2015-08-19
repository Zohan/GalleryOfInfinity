#include "FastSPI_LED.h"

// How many leds in your strip?
#define NUM_LEDS               44
#define LED_DATA_PIN           11

// color wheel constants
#define RED    85
#define GREEN  0
#define BLUE   170

struct CRGB { unsigned char g; unsigned char r; unsigned char b; };

// a pointer for array of LEDs
struct CRGB *leds;

#define READ_INTERVAL 50  // ms
#define VELOSTAT_IN   A0

unsigned long previousUpdate = 0;
int stripLocation = 0;

void setup() {
  struct CRGB colorBuffer;
  int i;
  
  Serial.begin(9600);
  
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
  
  // set the entire strip to red initially
  setColor(&colorBuffer, RED);
  
  for (i = 0; i < NUM_LEDS; i++) {
    leds[i].r = colorBuffer.r;
    leds[i].g = colorBuffer.g;
    leds[i].b = colorBuffer.b;
  }
  
  FastSPI_LED.show();
}

void loop() {
  unsigned long currentTime = millis();
  struct CRGB colorBuffer;
  int veloValue;
  
  if (currentTime - previousUpdate > READ_INTERVAL) {    
    previousUpdate = currentTime;
    
    veloValue = analogRead(VELOSTAT_IN);    
    dumpVeloReading(veloValue);
    
    setColor(&colorBuffer, (byte) veloValue);
    leds[stripLocation].r = colorBuffer.r;
    leds[stripLocation].g = colorBuffer.g;
    leds[stripLocation].b = colorBuffer.b;
    
    if (++stripLocation == NUM_LEDS) stripLocation = 0;
    
    FastSPI_LED.show();
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

void dumpVeloReading(int veloValue) {
  Serial.print("Velo: ");
  Serial.println(veloValue);
}
