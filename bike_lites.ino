#include "FastLED.h"
FASTLED_USING_NAMESPACE

#define BUTTON1_PIN 2
#define BUTTON2_PIN 4

#define NUM_LEDS 50
#define NUM_MODES 5
#define NUM_PALETTES 7


#define FRAMES_PER_SECOND  120

#define LED_PIN1    6


#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    50

extern const TProgmemPalette16 WarmPastels_p PROGMEM;

CRGB leds[NUM_LEDS];

#define FRAMES_PER_SECOND  120


uint8_t brightness = 50;

// current color palette
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

// List of patterns to cycle through.  Each is defined as a separate function below.

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON1_PIN, INPUT_PULLUP);  // Enable internal pullup resistor
  pinMode(BUTTON2_PIN, INPUT_PULLUP);  // Enable internal pullup resistor

  FastLED.addLeds<LED_TYPE,LED_PIN1,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);

  // limit my draw to 1W
  FastLED.setMaxPowerInMilliWatts(1000);
  
  currentBlending = LINEARBLEND;
  currentPalette = WarmPastels_p;
  
  Serial.begin(9600);
  Serial.print("hello\n");
}


volatile int buttonState[2];             // the current reading from the input pin
int lastButtonState[2] = {LOW,LOW};   // the previous reading from the input pin
boolean pushed[2];   // True if button is currently pressed down
unsigned long pushed_time[2] = {0,0};
unsigned long lastDebounceTime[2] = {0,0};  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers





uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t mode = 0;
uint8_t palette_index = 0;

typedef void (*SimplePatternList[])();
const SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
const CRGBPalette16 gPalettes[] = { WarmPastels_p, RainbowColors_p,  OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, HeatColors_p, PartyColors_p};




void loop() {
  // put your main code here, to run repeatedly:
  button_press();
  if (pushed[0]) {
    if ((millis() - pushed_time[0]) > 1000) {
        if (brightness > 0) {brightness -= 1;}
        FastLED.setBrightness(brightness);
        delay(20);
      }
  }
  if (pushed[1]) {
  if ((millis() - pushed_time[1]) > 1000) {
        if (brightness < 255) {brightness += 1;}
        FastLED.setBrightness(brightness);
        delay(20);
      }
  }


// Call the current pattern function once, updating the 'leds' array
  gPatterns[mode]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}


// ---------- methods ------------

void button_press() {
  int button[2];
  button[0] = digitalRead(BUTTON1_PIN);
  button[1] = digitalRead(BUTTON2_PIN);

for (int i=0; i<2; ++i) {
  if((long)(millis() - lastDebounceTime[i]) >= debounceDelay) {
    if (button[i]==0 && !pushed[i]) {
      // debounced state is down, increment mode
//      mode += (i==0? -1 : 1);
      if (i==0) {
        mode += 1;
        if (mode > NUM_MODES) {mode = 0;}
        if (mode < 0) {mode = NUM_MODES;}
        Serial.print("Mode: ");
        Serial.print(mode);
        Serial.print("\n");
      }
      if (i==1) {
        palette_index += 1;
        if (palette_index > NUM_PALETTES) {palette_index = 0;}
        if (palette_index < 0) {palette_index = NUM_PALETTES;}
        currentPalette = gPalettes[palette_index];
        Serial.print("palette: ");
        Serial.print(palette_index);
        Serial.print("\n"); 
        
      }
      
      pushed[i] = true;
      pushed_time[i] = millis();
    }
    if (button[i]==1) {
      // debounced state is up
      pushed[i] = false;
    }
    lastDebounceTime[i] = millis();
  }
}
}


// --------- Patterns -------------

void rainbow() 
{
  // FastLED's built-in rainbow generator
//  fill_rainbow( leds, NUM_LEDS, gHue, 7);
//  fill_rainbow (struct CRGB *pFirstLED, int numToFill, uint8_t initialhue, uint8_t deltahue=5)
  fill_palette(leds, NUM_LEDS, gHue, int(255/NUM_LEDS), currentPalette, 255, currentBlending); 
//  fill_palette (CRGB *L, uint16_t N, uint8_t startIndex, uint8_t incIndex, const PALETTE &pal, uint8_t brightness, TBlendType blendType)

}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += ColorFromPalette(currentPalette, gHue + random8(64), 255, currentBlending); //CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += ColorFromPalette(currentPalette, gHue, 192, currentBlending); //CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
//  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(currentPalette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= ColorFromPalette(currentPalette, dothue, 255, currentBlending); //CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// ------------ Custom color palettes -------------------
const TProgmemPalette16 WarmPastels_p PROGMEM = {
//    CRGB(251, 189, 127),
//    CRGB(219, 167, 121),
//    CRGB(149, 179, 218),
//    CRGB(254, 228, 229),
//    CRGB(169, 219, 254),
//
//    CRGB(251, 189, 127),
//    CRGB(219, 167, 121),
//    CRGB(149, 179, 218),
//    CRGB(254, 228, 229),
//    CRGB(169, 219, 254),
//    
//    CRGB(251, 189, 127),
//    CRGB(219, 167, 121),
//    CRGB(149, 179, 218),
//    CRGB(254, 228, 229),
//    CRGB(169, 219, 254),
//    CRGB::White

    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
