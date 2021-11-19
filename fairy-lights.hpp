#ifndef _FAIRY_LIGHTS_HPP
#define _FAIRY_LIGHTS_HPP

void updateLights() {
  for (int i = 0; i < 8; i++) {
    int br = map(bands[i], 0, AMPLITUDE, 0, 255);
    setPixelColor(i, br, br, br);
  }
  setPixelColor(8, 128, 0, 0);
  /*for (int i = 0; i < NUMPIXELS; i++) {
    setPixelColor(i, 25, 25, 25);
  }*/
  drawLeds();
}

#endif
