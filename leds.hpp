#ifndef _LEDS_HPP
#define _LEDS_HPP

#include <Adafruit_NeoPixel.h>

#define PIN 14
#define NUMPIXELS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void initLeds() {
  pixels.begin();
}

void setPixelColor(int i, int r, int g, int b) {
  pixels.setPixelColor(i, pixels.Color(r, g, b));
}

void drawLeds() {
  pixels.show();
}

#endif
