#define USE_FINE_BANDS true
#define USE_COARSE_BANDS false
#define AFFECT_BRIGHTNESS true
#define AFFECT_COLOR false
#define GO_FROM_MIDDLE true
#define GO_FROM_BEGINNING false

class FFTEffect : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
    FFTEffect(bool shallUseFineBands, bool shallAffectBrightness, bool shallGoFromMiddle) : _shallUseFineBands(shallUseFineBands), _shallAffectBrightness(shallAffectBrightness), _shallGoFromMiddle(shallGoFromMiddle) {};
  private:
    bool _shallUseFineBands;
    bool _shallAffectBrightness;
    bool _shallGoFromMiddle;
    void applyValue(int i, int targetValue);
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
float redHue = 0.0;
float greenHue = 120.0 / 360.0;

void FFTEffect::applyValue(int i, int targetValue) {
  if (_shallAffectBrightness) {
    brightness[i] = targetValue;
  } else {
    float h = fmap(targetValue, 0, 255, greenHue, redHue);
    float rgb[3];
    hsv2rgb(h, 1.0, 1.0, rgb);
    for (int j = 0; j < 3; j++) {
      colors[i][j] = (int)(rgb[j] * 255.0);
    }
  }
}

void FFTEffect::tick() {
  int n = NUMPIXELS;
  if (_shallGoFromMiddle) {
    n = NUMPIXELS / 2;
  }
  for (int i = 0; i < n; i++) {
    int value = 0;
    if (_shallUseFineBands) {
      int fineBandIdx = map(i, 0, n, 0, FINE_BANDS);
      value = fineBands[fineBandIdx];
    } else {
      int coarseBandIdx = map(i, 0, n, 0, COARSE_BANDS);
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

    if (_shallUseFineBands) {
      applyValue(n - i, targetValue[i]);
      applyValue(NUMPIXELS - (n - i), targetValue[i]);
    } else {
      applyValue(i, targetValue[i]);
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
  if (_shallGoFromMiddle) {
    band += " sym";
  }
  return "FFT" + band;
}
