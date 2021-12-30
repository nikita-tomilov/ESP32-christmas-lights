#ifndef _FFT_HPP
#define _FFT_HPP

//taken from https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display
//and later adapted
//thank you, G6EJD!

#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

#define SAMPLES 256              // Must be a power of 2
#define N ((SAMPLES / 2))
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define DECREMENT_SPEED 5

double INPUT_COEF = 1.0; // how much the input signal from ADC is amplified
int LOW_PASS = 800; // cutoff the fft-computed amplitudes below this mark
#define MAX_COEF 1.1 // amplify the maximum amplitude from FFT by this value to make visualizing more appealing
#define NORMALIZE 0  // normalize peaks so that for same volume amp of low and high frequencies shall be equal

#define MIN_POSSIBLE_GAIN 800  // nuff said
#define AUTO_GAIN 1            // shall recompute the gain
int GAIN = MIN_POSSIBLE_GAIN;  // the gain that is used for vizualizing (e.g. absolute max amplitude after FFT)

double vReal[SAMPLES];
double vImag[SAMPLES];


byte bandIndex[SAMPLES / 2];

#define COARSE_BANDS 25
volatile int coarseBands[COARSE_BANDS] = {0};

#define FINE_BANDS 8
volatile int fineBands[FINE_BANDS] = {0};

void setInputGain(double gain) {
  INPUT_COEF = gain;
}

double getInputGain() {
  return INPUT_COEF;
}

void setCutoff(int value) {
  LOW_PASS = value;
}

int getCutoff() {
  return LOW_PASS;
}

long samplingPeriodUs;
void initFFT() {
  samplingPeriodUs = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
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

long prevSampleTakenAt;
void captureAudioData() {
  for (int i = 0; i < SAMPLES; i++) {
    prevSampleTakenAt = micros();
    //https://github.com/espressif/arduino-esp32/issues/102
    vReal[i] = analogRead(ADC_PIN);
    vImag[i] = 0;
    //while ((micros() - prevSampleTakenAt) < samplingPeriodUs) { /* do nothing to wait */ }
  }
}

inline void computeFFT() __attribute__((always_inline));
inline void prepareFFTResults() __attribute__((always_inline));
inline void applyFFTResultsToBands() __attribute__((always_inline));

long lastGainUpdate;
float k = 0.05, maxAmp = 0.0, prevMaxAmp = 0.0;
void analyzeAudioWithFFT() {
  computeFFT();
  prepareFFTResults();
  applyFFTResultsToBands();

 if (AUTO_GAIN) {
    if (millis() - lastGainUpdate > 10) {
      maxAmp = maxAmp * k + prevMaxAmp * (1 - k);
      if (maxAmp > LOW_PASS) {
        GAIN = (float) MAX_COEF * maxAmp;
      } else {
        GAIN = MIN_POSSIBLE_GAIN;
      }
      if (GAIN < MIN_POSSIBLE_GAIN) GAIN = MIN_POSSIBLE_GAIN;
      lastGainUpdate = millis();
    }
  }
}

void computeFFT() {
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
}

void prepareFFTResults() {
  for (int i = 0; i < N; i++){
    if (vReal[i] < LOW_PASS) vReal[i] = 0;
    vReal[i] = vReal[i] * INPUT_COEF;
    if (NORMALIZE) vReal[i] = vReal[i] / ((float)1 + (float)i / N);
  }
}

void applyFFTResultsToBands() {
  prevMaxAmp = maxAmp;
  maxAmp = 0;
  for (int i = 2; i < N; i++) { // Don't use sample 0 and only the first SAMPLES/2 are usable.
    if (vReal[i] > maxAmp) maxAmp = vReal[i];
      int value = (int)vReal[i];

      int coarseBand = i * COARSE_BANDS * 2 / SAMPLES;
      coarseBands[coarseBand] = value;
      
      int fineBand = bandIndex[i];
      fineBands[fineBand] = value;
  }

/*for (int i = 0; i < FINE_BANDS; i++) {
  Serial.print(fineBands[i]);
  Serial.print("   ");
}
Serial.println(GAIN);*/
  
  /*if (millis()%4 == 0) {
    for (int band = 0; band < COARSE_BANDS; band++) {
      if (coarseBands[band] > 0) coarseBands[band] -= 1 * DECREMENT_SPEED;
      if (coarseBands[band] < 0) coarseBands[band] = 0;
      if (band < FINE_BANDS) {
        if (fineBands[band] > 0) fineBands[band] -= 1 * DECREMENT_SPEED;
        if (fineBands[band] < 0) fineBands[band] = 0;
      }
    }
  }*/
}

#endif
