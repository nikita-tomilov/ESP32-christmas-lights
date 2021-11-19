#ifndef _FFT_HPP
#define _FFT_HPP

//taken from https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display
//thank you, G6EJD!

#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

#define SAMPLES 1024             // Must be a power of 2
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE 300
#define NOIZE_THRESHOLD ((AMPLITUDE * 10))
#define DECREMENT_SPEED 3

unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
int dominant_value;

volatile int bands[8] = {0};

void initFFT() {
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void captureAudioData() {
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    //https://github.com/espressif/arduino-esp32/issues/102
    vReal[i] = analogRead(33);
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* do nothing to wait */ }
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
      if (i<=2 )             applyBand(0,(int)vReal[i]); // 125Hz
      if (i >2   && i<=4 )   applyBand(1,(int)vReal[i]); // 250Hz
      if (i >4   && i<=7 )   applyBand(2,(int)vReal[i]); // 500Hz
      if (i >7   && i<=15 )  applyBand(3,(int)vReal[i]); // 1000Hz
      if (i >15  && i<=40 )  applyBand(4,(int)vReal[i]); // 2000Hz
      if (i >40  && i<=70 )  applyBand(5,(int)vReal[i]); // 4000Hz
      if (i >70  && i<=288 ) applyBand(6,(int)vReal[i]); // 8000Hz
      if (i >288           ) applyBand(7,(int)vReal[i]); // 16000Hz
      //Serial.println(i);
    }
  }
  //if (millis()%4 == 0) {
    for (byte band = 0; band <= 7; band++) {
      if (bands[band] > 0) bands[band] -= 1 * DECREMENT_SPEED;
      if (bands[band] < 0) bands[band] = 0;
    }
  //}
}

#endif
