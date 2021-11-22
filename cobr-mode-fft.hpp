#define USE_FINE_BANDS true
#define USE_COARSE_BANDS false
#define AFFECT_BRIGHTNESS true
#define AFFECT_COLOR false

class FFTEffect : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
    FFTEffect(bool shallUseFineBands, bool shallAffectBrightness) : _shallUseFineBands(shallUseFineBands), _shallAffectBrightness(shallAffectBrightness) {};
  private:
    bool _shallUseFineBands;
    bool _shallAffectBrightness;
};

int targetValue[NUMPIXELS];
int targetValueDecrement = 4;
void FFTEffect::init() {
  for (int i = 0; i < NUMPIXELS; i++) {
    targetValue[i] = 0;
  }
}

long imap(long x, long in_min, long in_max, long out_min, long out_max) {
  long ans = out_min + (x - in_min) * 1.0 / (in_max - in_min) * (out_max - out_min);
  if (ans > out_max) ans = out_max;
  return ans;
}

void FFTEffect::tick() {
  float redHue = 0.0;
  float greenHue = 120.0 / 360.0;
  for (int i = 0; i < NUMPIXELS; i++) {
    int value = 0;
    if (_shallUseFineBands) {
      int fineBandIdx = map(i, 0, NUMPIXELS, 0, FINE_BANDS);
      value = fineBands[fineBandIdx];
    } else {
      int coarseBandIdx = i / 2;
      value = coarseBands[coarseBandIdx];
    }

    if (_shallAffectBrightness) {
      value = imap(value, 0, GAIN, 0, assignedBrightness); //TODO: investigate why value is more than GAIN
    } else {
      value = imap(value, 0, GAIN, 0, 255);
    }
    if (value > targetValue[i]) {
      targetValue[i] = value;
    }
    
    if (_shallAffectBrightness) {
      brightness[i] = targetValue[i];
    } else {
      float h = fmap(targetValue[i], 0, 255, greenHue, redHue);
      float rgb[3];
      hsv2rgb(h, 1.0, 1.0, rgb);
      for (int j = 0; j < 3; j++) {
        colors[i][j] = (int)(rgb[j] * 255.0);
      }
    }
  }
}
void FFTEffect::slowAction() {}
void FFTEffect::fastAction() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (targetValue[i] > 0) {
      targetValue[i] -= targetValueDecrement;
      if (targetValue[i] < 0) targetValue[i] = 0;
    }
  }
}

String FFTEffect::name() {
  String band = " coarse";
  if (_shallUseFineBands) {
    band = " fine";
  }
  return "FFT" + band;
}
