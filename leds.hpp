#ifndef _LEDS_HPP
#define _LEDS_HPP

#include <FastLED.h>

#define NUMPIXELS 50
CRGB leds[NUMPIXELS];

void setPixelColor(int i, int r, int g, int b) {
  leds[i] = CRGB(r, g, b);
}

void drawLeds() {
  FastLED.show();
}

void initLeds() {
  FastLED.addLeds<NEOPIXEL, LED_STRIP_PIN>(leds, NUMPIXELS);
  setPixelColor(0, 255, 0, 0);
  drawLeds();
}

#endif
