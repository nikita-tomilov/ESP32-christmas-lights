#ifndef _LEDS_HPP
#define _LEDS_HPP

//make sure to use 1.7 or higher on ESP32
//see https://github.com/adafruit/Adafruit_NeoPixel/issues/139
#include <Adafruit_NeoPixel.h>

#define PIN 27
#define NUMPIXELS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setPixelColor(int i, int r, int g, int b) {
  pixels.setPixelColor(i, pixels.Color(r, g, b));
}

void initLeds() {
  pixels.begin();
  setPixelColor(0, 255, 0, 0);
  pixels.show();
}

void drawLeds() {
  pixels.show();
}

#endif
