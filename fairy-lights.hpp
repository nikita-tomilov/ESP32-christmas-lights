#ifndef _FAIRY_LIGHTS_HPP
#define _FAIRY_LIGHTS_HPP

uint8_t brightness[NUMPIXELS];
uint8_t colors[NUMPIXELS][3];
uint8_t assignedColors[5][3];
uint8_t assignedBrightness = 128;


enum ColorMode {
  ONE_COLOR, TWO_COLORS, FIVE_COLORS, RAINBOW, RAINBOW_MOVING_CW, RAINBOW_MOVING_CCW, FFT_COLOR_FROM_BANDS
};
#define COLOR_MODE_COUNT 7

enum BrignthessMode {
  ONE_BRIGHTNESS, PWM_UP_DOWN, WAVE, FFT_SPECTRUM, FFT_BANDS
};
#define BRIGHTNESS_MODE_COUNT 5

ColorMode colorMode = ONE_COLOR;
BrignthessMode brightnessMode = ONE_BRIGHTNESS;

void setColorMode(int newMode) {
  if ((newMode < 0) || (newMode > 6)) return;
  colorMode = (ColorMode)(newMode);
}

void setBrightnessMode(int newMode) {
  if ((newMode < 0) || (newMode > 4)) return;
  brightnessMode = (BrignthessMode)(newMode);
}


int twoColorsTickValue = 1;
int fiveColorsFlag = 0;
int rainbowMovingFlag = 0;
float rainbowStartingPoint = 0.0;
float rainbowDelta = 0;
float* hsv2rgb(float h, float s, float b, float* rgb);
float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float cycleHue(float h) {
  if (h > 1.0) {
    h -= 1.0;
  } else if (h < 0.0) {
    h += 1.0;
  }
  return h;
}
void updateColors() {
  int prevIdxForFiveColors = 1;
  for (int i = 0; i < NUMPIXELS; i++) {
    switch(colorMode) {
      case ONE_COLOR: {
        for (int j = 0; j < 3; j++) {
          colors[i][j] = assignedColors[0][j];
        }
      } break;
      case TWO_COLORS: {
        int idx = 0;
        if (i % 2 == 0) {
          idx = 2 - twoColorsTickValue;
        } else {
          idx = 1 + twoColorsTickValue;
        }
        for (int j = 0; j < 3; j++) {
          colors[i][j] = assignedColors[idx][j];
        }
      } break;
      case FIVE_COLORS: {
        if (fiveColorsFlag) {
          int idx = random(1000) % 5;
          if (idx == prevIdxForFiveColors) {
            idx = 4 - idx;
          }
          for (int j = 0; j < 3; j++) {
            colors[i][j] = assignedColors[idx][j];
          }
          prevIdxForFiveColors = idx;
        }
      } break;
      case RAINBOW:
      case RAINBOW_MOVING_CW:
      case RAINBOW_MOVING_CCW: {
        float delta = rainbowStartingPoint;
        if (colorMode == RAINBOW) {
          delta = 0.0;
        }
        float h = fmap(i, 0, NUMPIXELS, 0.0, 1.0) + delta;
        h = cycleHue(h);
        float rgb[3];
        hsv2rgb(h, 1.0, 1.0, rgb);
        for (int j = 0; j < 3; j++) {
          colors[i][j] = (int)(rgb[j] * 255.0);
        }
      } break;
      default:
        break;
    }
  }
  if (fiveColorsFlag) {
    fiveColorsFlag = 0;
  }
  if (rainbowMovingFlag) {
    if (colorMode == RAINBOW_MOVING_CW) rainbowDelta = 0.001;
    if (colorMode == RAINBOW_MOVING_CCW) rainbowDelta = -0.001;
    rainbowStartingPoint += rainbowDelta;
    rainbowStartingPoint = cycleHue(rainbowStartingPoint);
    rainbowMovingFlag = 0;
  }
}

void updateBrightness() {
  for (int i = 0; i < NUMPIXELS; i++) {
    switch(brightnessMode) {
      case ONE_BRIGHTNESS:
         brightness[i] = assignedBrightness;
        break;
      default:
        break;
    }
  }
}

long slowAnimationDelay = 1000;
long fastAnimationDelay = 10;

long colorsLastSlowTick = millis();
long colorsLastFastTick = millis();
void colorsTick() {
  if (millis() - colorsLastSlowTick > slowAnimationDelay) {
    twoColorsTickValue = 1 - twoColorsTickValue;
    fiveColorsFlag = 1;
    colorsLastSlowTick = millis();
  }
  if (millis() - colorsLastFastTick > fastAnimationDelay) {
    rainbowMovingFlag = 1;
  }
}


void updateLights() {
  colorsTick();
  updateColors();
  updateBrightness();
  for (int i = 0; i < NUMPIXELS; i++) {
    //int br = map(bands[i / 2], 0, AMPLITUDE, 0, 255);
    //setPixelColor(i, br, br, br);
    int r = (int)(brightness[i]) * (int)(colors[i][0]) / 255;
    int g = (int)(brightness[i]) * (int)(colors[i][1]) / 255;
    int b = (int)(brightness[i]) * (int)(colors[i][2]) / 255;
    setPixelColor(i, r, g, b);
  }
  drawLeds();
}

void initFairyLights() {
  //first color shall be white
  assignedColors[0][0] = 255;
  assignedColors[0][1] = 255;
  assignedColors[0][2] = 255;
  //second and third - red/green - christmas
  assignedColors[1][0] = 255;
  assignedColors[1][1] = 0;
  assignedColors[1][2] = 0;
  assignedColors[2][0] = 0;
  assignedColors[2][1] = 255;
  assignedColors[2][2] = 0;
  //4th and 5th - yellow/blue
  assignedColors[3][0] = 255;
  assignedColors[3][1] = 128; //yes it has to be less green for yellow colour
  assignedColors[3][2] = 0;
  assignedColors[4][0] = 0;
  assignedColors[4][1] = 0;
  assignedColors[4][2] = 64;
  //all colors - zero
  for (int i = 0; i < NUMPIXELS; i++) {
    brightness[i] = 128;
    for (int j = 0; j < NUMPIXELS; j++) {
      colors[i][j] = 0;
    }
  }
}

float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float c_abs(float x) { return x < 0 ? -1 * x : x; }

float c_constrain(float x, float a, float b) {
  if (x < a) return a;
  if (x > b) return b;
  return x;
}

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, c_constrain(c_abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, c_constrain(c_abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, c_constrain(c_abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

#endif
