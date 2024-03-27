#include <Arduino.h>
// --------------------------------
// -- VU Meter - Scott's version --
// --------------------------------
#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <JC_Button.h>

# define LEFT_OUT_PIN 6             // Left channel data out pin to LEDs [6]
# define RIGHT_OUT_PIN 5            // Right channel data out pin to LEDs [5]
# define LEFT_IN_PIN A7             // Left aux in signal [A5 Reserved for SCL]
# define RIGHT_IN_PIN A6            // Right aux in signal and MIC [A4 Reserved for SCA]
# define BRIGHTNESS_PIN A2          // Brightness potentiometer in [A2]
# define SENSITIVITY_PIN A1         // Sensitivity potentiometer in [A1]
# define BTN_PIN 3                  // Push button on this pin [3]
# define DEBOUNCE_MS 20             // Number of ms to debounce the button [20]
# define LONG_PRESS 500             // Number of ms to hold the button to count as long press [500]
# define N_PIXELS 34                // Number of pixels in each string [18]
# define MAX_MILLIAMPS 500          // Maximum current to draw [500]
# define COLOR_ORDER GRB            // Colour order of LED strip [GRB]
# define LED_TYPE WS2812B           // LED string type [WS2812B]
# define DC_OFFSET 0                // DC offset in aux signal [0]
# define NOISE 0                    // Noise/hum/interference in aux signal [0]
# define SAMPLES 64                 // Length of buffer for dynamic level adjustment [64]
# define SAMPLES2 64                // Length of buffer for dynamic level adjustment  [64]
# define TOP (N_PIXELS + 2)         // Allow dot to go slightly off scale [(N_PIXELS + 2)]
# define PEAK_FALL 20               // Rate of peak falling dot [20]
# define N_PIXELS_HALF (N_PIXELS / 2)
# define PATTERN_TIME 10            // Seconds to show eaach pattern on auto [10]
# define STEREO true                // If true, L&R channels are independent. If false, both L&R outputs display same data from L audio channel [true]

uint8_t volCountLeft = 0;           // Frame counter for storing past volume data
uint16_t volLeft[SAMPLES];               // Collection of prior volume samples
uint16_t lvlLeft = 0;                    // Current "dampened" audio level
uint16_t minLvlAvgLeft = 0;              // For dynamic adjustment of graph low & high
uint16_t maxLvlAvgLeft = 512;

uint8_t volCountRight = 0;          // Frame counter for storing past volume data
uint16_t volRight[SAMPLES];              // Collection of prior volume samples
uint16_t lvlRight = 0;                   // Current "dampened" audio level
uint16_t minLvlAvgRight = 0;             // For dynamic adjustment of graph low & high
uint16_t maxLvlAvgRight = 512;

byte dotCountLeft  = 0;      // Frame counter for delaying dot-falling speed
byte dotCountRight = 0;      // Frame counter for delaying dot-falling speed

CRGB ledsLeft[N_PIXELS];
CRGB ledsRight[N_PIXELS];

uint8_t myhue = 0;

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;

const uint8_t FADE_RATE = 2; // How long should the trails be. Very low value = longer trails.

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_PIXELS, RIGHT_OUT_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(N_PIXELS, LEFT_OUT_PIN, NEO_GRB + NEO_KHZ800);

//Colors
uint32_t red = strip.Color(255, 0, 0);
uint32_t orange = strip.Color(255, 127, 0);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t purple = strip.Color(75, 0, 130);
uint32_t white = strip.Color(125, 125, 125);

int vol[SAMPLES];       // Collection of prior volume samples

// --------------------
// --- Button Stuff ---
// --------------------
uint8_t state = 0;
int buttonPushCounter = 0;
bool autoChangeVisuals = false;
Button modeBtn(BTN_PIN, DEBOUNCE_MS);

void incrementButtonPushCounter() {
  buttonPushCounter = (buttonPushCounter + 1) %19;
  EEPROM.write(1, buttonPushCounter);
}

int sensitivity = 0;

// Global
#include "common.h"
// Effects library
//#include "colorWipe.h"
#include "rainbow.h"
#include "fire.h"
#include "twinkle.h"
#include "balls.h"
#include "juggle.h"
#include "sinelon.h"
#include "fireblu.h"
#include "ripple.h"
// VU
#include "vu4.h"
#include "vu5.h"
#include "vu6.h"
#include "vu7.h"
#include "vu8.h"
#include "vu9.h"
#include "vu10.h"
#include "vu11.h"
/*
void vu4(bool is_centered, uint8_t channel);
void vu5(bool is_centered, uint8_t channel);
void vu6(bool is_centered, uint8_t channel);
void vu7(bool show_background);
void vu8();
void vu9();
void vu10();
void balls();
void fire();
void juggle();
void ripple(boolean show_background);
void sinelon();
void rainbow(uint8_t rate);
*/

void setup() {
  delay(1000); // power-up safety delay

  analogReference(INTERNAL);

  pinMode(LEFT_IN_PIN, INPUT);
  pinMode(RIGHT_IN_PIN, INPUT);
  pinMode(BTN_PIN, INPUT);
  pinMode(BRIGHTNESS_PIN, INPUT);
  pinMode(SENSITIVITY_PIN, INPUT);
  pinMode(LEFT_OUT_PIN, OUTPUT);
  pinMode(RIGHT_OUT_PIN, OUTPUT);

  FastLED.addLeds < LED_TYPE, LEFT_OUT_PIN, COLOR_ORDER > (ledsLeft, N_PIXELS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds < LED_TYPE, RIGHT_OUT_PIN, COLOR_ORDER > (ledsRight, N_PIXELS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);

  modeBtn.begin();
  Serial.begin(9600);

  buttonPushCounter = 0;
  buttonPushCounter = (int)EEPROM.read(1); // load previous setting
  Serial.print("Starting pattern ");
  Serial.println(buttonPushCounter);
}

void loop() {

  // Read button
  modeBtn.read(); 
  switch (state) {
    case 0:                
      if (modeBtn.wasReleased()) {
        Serial.print("Short press, pattern ");
        Serial.println(buttonPushCounter);
        incrementButtonPushCounter();
        autoChangeVisuals = false;
      }
      else if (modeBtn.pressedFor(LONG_PRESS))
        state = 1;
      break;
    
    case 1:
      if (modeBtn.wasReleased()) {
        state = 0;
        Serial.print("Long press, auto, pattern ");
        Serial.println(buttonPushCounter);
        autoChangeVisuals = true;
      }
      break;
  }
  
  // Switch pattern if on auto
  if(autoChangeVisuals){
    EVERY_N_SECONDS(PATTERN_TIME) { 
      incrementButtonPushCounter();
      Serial.print("Auto, pattern ");
      Serial.println(buttonPushCounter); 
    }
  }

  // Run selected pattern
  switch (buttonPushCounter) {
    case 0:
      vu4(false, 0);
      if (STEREO) vu4(false, 1);
      else copyLeftToRight();
      break;
  
    case 1:
      vu4(true, 0);
      if (STEREO) vu4(true, 1);
      else copyLeftToRight();
      break;
  
    case 2:
      vu5(false, 0);
      if (STEREO) vu5(false, 1);
      else copyLeftToRight();
      break;
  
    case 3:
      vu5(true, 0);
      if (STEREO) vu5(true, 1);
      else copyLeftToRight();
      break;
  
    case 4:
      vu6(false, 0);
      if (STEREO) vu6(false, 1);
      else copyLeftToRight();
      break;
      
    case 5:
      vu7(true);
      copyLeftToRight();
      break;
  
    case 6:
      vu8();
      copyLeftToRight();
      break;
  
    case 7:
      vu9();
      break;
  
    case 8:
      vu10();
      break;
    
    case 9:
      vu7(false);
      copyLeftToRight();
      break;

    case 10:
      vu11();
      break;  
  
    case 11:
      sinelon();
      break;
  
    case 12:
      balls();
      break;
  
    case 13:
      juggle();
      break;
  
    case 14:
      fire();
      break;
  
    case 15:
      ripple(true);
      break;
  
    case 16:
      rainbow(10);
      break;

    case 17:
      fireblu();
      break;

     case 18:
      twinkle();
      break; 
  }
  sensitivity = analogRead(SENSITIVITY_PIN);
  FastLED.setBrightness(map(analogRead(BRIGHTNESS_PIN),0,1023,0,255));
  
}


