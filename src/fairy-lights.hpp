#ifndef _FAIRY_LIGHTS_HPP
#define _FAIRY_LIGHTS_HPP

#ifndef TTGO_T_DISPLAY
uint8_t brightness[NUMPIXELS];
uint8_t colors[NUMPIXELS][3];
#endif
uint8_t assignedColors[5][3];
uint8_t assignedBrightness = 128;

long slowAnimationDelay = 1000;
long fastAnimationDelay = 10;

class IEffect {
  public:
    virtual void init() = 0;
    virtual void tick() = 0;
    virtual void slowAction() = 0;
    virtual void fastAction() = 0;
    virtual String name() = 0;
};

#include "color-utils.hpp"
#include "color-mode-simple.hpp"
#include "color-mode-rainbow.hpp"
#include "brightness-mode-simple.hpp"

#ifdef ESP32
#include "cobr-mode-fft.hpp"
#include "cobr-mode-gain.hpp"
#endif

SingleColor singleColor;
TwoColors twoColors;
FiveColors fiveColors;
Rainbow rainbow(0.0, false);
Rainbow rainbowReversed(0.0, true);
Rainbow rainbowCW(0.01, false);
Rainbow rainbowCCW(-0.01, false);
SingleBrightness singleBrightness;
RandomBrightnessBursts rndBrBursts;

#ifdef ESP32
  FFTEffect fftCCoarse  (USE_COARSE_BANDS, AFFECT_COLOR,      GO_FROM_BEGINNING);
  FFTEffect fftCFine    (USE_FINE_BANDS,   AFFECT_COLOR,      GO_FROM_BEGINNING);
  FFTEffect fftCCoarseS (USE_COARSE_BANDS, AFFECT_COLOR,      GO_FROM_MIDDLE);
  FFTEffect fftCFineS   (USE_FINE_BANDS,   AFFECT_COLOR,      GO_FROM_MIDDLE);
  FFTEffect fftBrCoarse (USE_COARSE_BANDS, AFFECT_BRIGHTNESS, GO_FROM_BEGINNING);
  FFTEffect fftBrFine   (USE_FINE_BANDS,   AFFECT_BRIGHTNESS, GO_FROM_BEGINNING);
  FFTEffect fftBrCoarseS(USE_COARSE_BANDS, AFFECT_BRIGHTNESS, GO_FROM_MIDDLE);
  FFTEffect fftBrFineS  (USE_FINE_BANDS,   AFFECT_BRIGHTNESS, GO_FROM_MIDDLE);
  GainEffect gainC  (AFFECT_COLOR);
  GainEffect gainBr  (AFFECT_BRIGHTNESS);
  
  #define COLOR_MODE_COUNT 12
  IEffect *const colorModes[COLOR_MODE_COUNT] = {
    &singleColor, &twoColors, &fiveColors, &rainbow, &rainbowReversed, &rainbowCW, &rainbowCCW, &fftCCoarse, &fftCFine, &fftCCoarseS, &fftCFineS, &gainC
  };
  
  #define BRIGHTNESS_MODE_COUNT 7
  IEffect *const brightnessModes[COLOR_MODE_COUNT] = {
    &singleBrightness, &rndBrBursts, &fftBrCoarse, &fftBrFine, &fftBrCoarseS, &fftBrFineS, &gainBr
  };

#elif defined(ESP8266)

#define COLOR_MODE_COUNT 7
  IEffect *const colorModes[COLOR_MODE_COUNT] = {
    &singleColor, &twoColors, &fiveColors, &rainbow, &rainbowReversed, &rainbowCW, &rainbowCCW
  };
  
  #define BRIGHTNESS_MODE_COUNT 2
  IEffect *const brightnessModes[COLOR_MODE_COUNT] = {
    &singleBrightness, &rndBrBursts
  };

#endif

int colorMode = 0;
int brightnessMode = 0;

void setColorMode(int newMode) {
  if ((newMode < 0) || (newMode > COLOR_MODE_COUNT - 1)) return;
  colorMode = newMode;
}

void setBrightnessMode(int newMode) {
  if ((newMode < 0) || (newMode > BRIGHTNESS_MODE_COUNT - 1)) return;
  brightnessMode =  newMode;
}

String getColorModeName(int idx) {
  return colorModes[idx]->name();
}

String getColorModeName() {
  return getColorModeName(colorMode);
}

String getBrightnessModeName(int idx) {
  return brightnessModes[idx]->name();
}

String getBrightnessModeName() {
  return getBrightnessModeName(brightnessMode);
}

long colorsLastSlowTick = millis();
long colorsLastFastTick = millis();
long brLastSlowTick = millis();
long brLastFastTick = millis();

void colorsActions() {
  if (millis() - colorsLastSlowTick > slowAnimationDelay) {
    colorModes[colorMode]->slowAction();
    colorsLastSlowTick = millis();
  }
  if (millis() - colorsLastFastTick > fastAnimationDelay) {
    colorModes[colorMode]->fastAction();
    colorsLastFastTick = millis();
  }
}
void bightnessActions() {
  if (millis() - brLastSlowTick > slowAnimationDelay) {
    brightnessModes[brightnessMode]->slowAction();
    brLastSlowTick = millis();
  }
  if (millis() - brLastFastTick > fastAnimationDelay) {
    brightnessModes[brightnessMode]->fastAction();
    brLastFastTick = millis();
  }
}

void updateLights() {
  colorsActions();
  colorModes[colorMode]->tick();
  bightnessActions();
  brightnessModes[brightnessMode]->tick();
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
  //all colors - zero
  for (int i = 0; i < NUMPIXELS; i++) {
    brightness[i] = 128;
    for (int j = 0; j < NUMPIXELS; j++) {
      colors[i][j] = 0;
    }
  }
  for (int i = 0; i < COLOR_MODE_COUNT; i++) {
    colorModes[i]->init();
  }
  for (int i = 0; i < BRIGHTNESS_MODE_COUNT; i++) {
    brightnessModes[i]->init();
  }
}

#endif
