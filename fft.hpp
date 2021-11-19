#ifndef _FFT_HPP
#define _FFT_HPP

//taken from https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display
//thank you, G6EJD!

#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

#define BANDS 25
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

volatile int bands[BANDS] = {0};

void initFFT() {
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
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

void applyBand(int band, int value){
  value /= AMPLITUDE;
  if (value > bands[band]) bands[band] = value;
}

void analyzeAudioWithFFT() {
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only the first SAMPLES/2 are usable.
    // Each array element represents a frequency and its value, is the amplitude. Note the frequencies are not discrete.
    if (vReal[i] > NOIZE_THRESHOLD) { // Add a crude noise filter, 10 x amplitude or more
      int value = ((int)vReal[i]) / AMPLITUDE;
      int band = i * BANDS * 2 / SAMPLES;
      if (value > bands[band]) bands[band] = value;
    }
  }
  //if (millis()%4 == 0) {
    for (int band = 0; band < BANDS; band++) {
      if (bands[band] > 0) bands[band] -= 1 * DECREMENT_SPEED;
      if (bands[band] < 0) bands[band] = 0;
    }
  //}
}

#endif
