#ifndef _FFT_HPP
#define _FFT_HPP

//taken from https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display
//thank you, G6EJD!

#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

#define SAMPLES 256             // Must be a power of 2
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE 75
#define NOIZE_THRESHOLD ((AMPLITUDE * 3))
#define DECREMENT_SPEED 5

unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
int dominant_value;

byte bandIndex[SAMPLES / 2];

#define COARSE_BANDS 25
volatile int coarseBands[COARSE_BANDS] = {0};

#define FINE_BANDS 8
volatile int fineBands[FINE_BANDS] = {0};

void initFFT() {
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  for (int i = 0; i < (SAMPLES / 2); i++) {
    if (i<=2 )            bandIndex[i] = 0; // 125Hz
    if (i == 3 )          bandIndex[i] = 1; // 250Hz
    if (i == 4 )          bandIndex[i] = 2; // 500Hz
    if (i >4   && i<=6 )  bandIndex[i] = 3; // 1000Hz
    if (i >6  && i<=10 )  bandIndex[i] = 4; // 2000Hz
    if (i >10  && i<=18 ) bandIndex[i] = 5; // 4000Hz
    if (i >18  && i<=72 ) bandIndex[i] = 6; // 8000Hz
    if (i >72           ) bandIndex[i] = 7; // 16000Hz
  }
}

void captureAudioData() {
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    //https://github.com/espressif/arduino-esp32/issues/102
    vReal[i] = analogRead(33);
    vImag[i] = 0;
    //while ((micros() - newTime) < sampling_period_us) { /* do nothing to wait */ }
  }
}

void analyzeAudioWithFFT() {
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only the first SAMPLES/2 are usable.
    // Each array element represents a frequency and its value, is the amplitude. Note the frequencies are not discrete.
    if (vReal[i] > NOIZE_THRESHOLD) { // Add a crude noise filter, 10 x amplitude or more
      int value = (int)vReal[i];
      value /= AMPLITUDE;

      int coarseBand = i * COARSE_BANDS * 2 / SAMPLES;
      if (value > coarseBands[coarseBand]) coarseBands[coarseBand] = value;
      
      int fineBand = bandIndex[i];
      if (value > fineBands[fineBand]) fineBands[fineBand] = value;
    }
  }
  //if (millis()%4 == 0) {
    for (int band = 0; band < COARSE_BANDS; band++) {
      if (coarseBands[band] > 0) coarseBands[band] -= 1 * DECREMENT_SPEED;
      if (coarseBands[band] < 0) coarseBands[band] = 0;
      if (band < FINE_BANDS) {
        if (fineBands[band] > 0) fineBands[band] -= 1 * DECREMENT_SPEED;
        if (fineBands[band] < 0) fineBands[band] = 0;
      }
    }
  //}
}

#endif
