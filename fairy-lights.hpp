#ifndef _FAIRY_LIGHTS_HPP
#define _FAIRY_LIGHTS_HPP

void updateLights() {
  /*for (int i = 0; i < 8; i++) {
    int br = bands[i];
    setPixelColor(i, br, br, br);
  }*/
  for (int i = 0; i < NUMPIXELS; i++) {
    setPixelColor(i, 25, 25, 25);
  }
  drawLeds();
}

#endif
