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

void FFTEffect::init() {}
void FFTEffect::tick() {
  float redHue = 0.0;
  float greenHue = 120.0 / 360.0;
  float maxValueAcrossFineBands = 0;
  if (_shallUseFineBands) {
    for (int i = 0; i < FINE_BANDS; i++) {
      if (fineBands[i] > maxValueAcrossFineBands) {
        maxValueAcrossFineBands = fineBands[i];
      }
    }
  }
  for (int i = 0; i < NUMPIXELS; i++) {
    float value = 0;
    if (_shallUseFineBands) {
      int fineBandIdx = map(i, 0, NUMPIXELS, 0, FINE_BANDS);
      value = fineBands[fineBandIdx];
    } else {
      int coarseBandIdx = i / 2;
      value = coarseBands[coarseBandIdx];
    }
    if (_shallAffectBrightness) {
      value = fmap(value, 0, GAIN, 0, assignedBrightness);
      brightness[i] = value;
    } else {
      float h = fmap(value, 0, GAIN, greenHue, redHue);
      float rgb[3];
      hsv2rgb(h, 1.0, 1.0, rgb);
      for (int j = 0; j < 3; j++) {
        colors[i][j] = (int)(rgb[j] * 255.0);
      }
    }
  }
}
void FFTEffect::slowAction() {}
void FFTEffect::fastAction() {}

String FFTEffect::name() {
  String band = " coarse";
  if (_shallUseFineBands) {
    band = " fine";
  }
  return "FFT" + band;
}
