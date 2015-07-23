#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(300, PIN, NEO_GRB + NEO_KHZ800);

/* Define pins for HC-SR04 ultrasonic sensor */
 
#define echoPin A1 // Echo Pin = Analog Pin 0
#define trigPin A0 // Trigger Pin = Analog Pin 1
 
#define LEDPin 13 // Onboard LED
 
volatile long duration; // Duration used to calculate distance
volatile long HR_dist=0; // Calculated Distance
 
const int minimumRange=1; //Minimum Sonar range
const int maximumRange=120; //Maximum Sonar Range. Maximum Range is 200, but we don't want more than 5 feet of behavior.

/*--------------------SETUP()------------------------*/
void setup() {
  //Begin Serial communication using a 9600 baud rate
  Serial.begin (19200);
   
  //Setup the trigger and Echo pins of the HC-SR04 sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = 1000;            // compare match register 16MHz/256/1000 = 62.5Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (0 << CS10);    // 256 prescaler
  TCCR1B |= (0 << CS11);    // 256 prescaler
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
   
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}
 
/*----------------------LOOP()--------------------------*/
void loop() {
 //getDistance();
 theaterChaseRainbow(50);
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      if(HR_dist == 0 || HR_dist > maximumRange) strip.setBrightness(0);
      else { 
        strip.setBrightness(255-(HR_dist/2)*3);
        wait = HR_dist/2;
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
 
/*--------------------getDistance() FUNCTION ---------------*/
void getDistance(){ 
 
 /* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */ 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 
 
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 HR_dist = duration/58.2;
 
 /*Send the reading from the ultrasonic sensor to the computer */
 if (HR_dist >= maximumRange || HR_dist <= minimumRange){
   HR_dist = 200;
 /* Send a 0 to computer and Turn LED ON to indicate "out of range" */
 Serial.println("0");
 //digitalWrite(LEDPin, HIGH); 
 } else {
 /* Send the distance to the computer using Serial protocol, and
 turn LED OFF to indicate successful reading. */
 Serial.println(HR_dist);
 //digitalWrite(LEDPin, LOW);
 }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  getDistance();
}
