class GainEffect : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
    GainEffect(bool shallAffectBrightness) : _shallAffectBrightness(shallAffectBrightness) {};
  private:
    bool _shallAffectBrightness;
    void applyColorValue(int i, float targetValue, uint8_t brightness, bool clampRedGreen);
    int targetValue[NUMPIXELS];
    int targetValueDecrement = 4;
    int maximumValue = 0;
    int maximumValueDecrement = 500;
    int maximumValueUpdates = 0;
    int upTo = 0;
};

void GainEffect::init() {
  for (int i = 0; i < NUMPIXELS; i++) {
    targetValue[i] = 0;
  }
}


void GainEffect::applyColorValue(int i, float targetValue, uint8_t brightness, bool clampRedGreen) {
  float h = targetValue;
  if (clampRedGreen) {
   h = fmap(targetValue, 0, 255, greenHue, redHue);
  }
  float rgb[3];
  hsv2rgb(h, 1.0, 1.0, rgb);
  for (int j = 0; j < 3; j++) {
    colors[i][j] = (int)(rgb[j] * brightness);
  }
}

void GainEffect::tick() {
  if (maximumValue <= GAIN) {
    maximumValue = GAIN;
  }
  if (maximumValue > MIN_POSSIBLE_GAIN) {
    upTo = imap(GAIN, MIN_POSSIBLE_GAIN, maximumValue, 0, NUMPIXELS - 1);
    //Serial.printf("%i out of %i or upTo %i\n", GAIN, maximumValue, upTo);
  } else {
    upTo = 0;
  }
  for (int i = 0; i < upTo; i++) {
    if (_shallAffectBrightness) {
      brightness[i] = assignedBrightness;
    } else {
      float val = (i * 1.0 / upTo * 255);
      applyColorValue(i, val, 255, true);
    }
  }
  for (int i = upTo; i < NUMPIXELS; i++) {
    if (_shallAffectBrightness) {
      brightness[i] = 0;
    } else {
      applyColorValue(i, blueHue, 64, false);
    }
  }
}
void GainEffect::slowAction() {
  if ((maximumValueUpdates < 8) && (upTo < NUMPIXELS / 4)) {
    if (maximumValue > MIN_POSSIBLE_GAIN) {
      maximumValue = max(maximumValue - maximumValueDecrement, MIN_POSSIBLE_GAIN);
      //Serial.println("reduce max to " + String(maximumValue));
    }
  }
  maximumValueUpdates = 0;
}
void GainEffect::fastAction() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (targetValue[i] > 0) {
      targetValue[i] = max(targetValue[i] - targetValueDecrement, 0);
    }
  }
}

String GainEffect::name() {
  return "Gain";
}
