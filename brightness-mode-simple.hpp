class SingleBrightness : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
};

void SingleBrightness::init() {}
void SingleBrightness::tick() {}
void SingleBrightness::slowAction() {
  for (int i = 0; i < NUMPIXELS; i++) {
    brightness[i] = assignedBrightness;
  }
}
void SingleBrightness::fastAction() {}

String SingleBrightness::name() {
  return "SingleBrightness";
}

//==================================================

class RandomBrightnessBursts : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
  private:
    long _prevTs;
    long _delay;
};

void RandomBrightnessBursts::init() {
  _prevTs = 0;
  _delay = 0;
}
void RandomBrightnessBursts::tick() {}
void RandomBrightnessBursts::slowAction() {}
void RandomBrightnessBursts::fastAction() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (brightness[i] > assignedBrightness / 8) {
      brightness[i] = constrain(brightness[i] - 5, assignedBrightness / 8, 255);
    }
  }
  if (millis() - _prevTs > _delay) {
    for (int i = 0; i < random(1, 4); i++) {
      int idx = random(0, 1000) % NUMPIXELS;
      brightness[idx] = assignedBrightness;
    }
    _prevTs = millis();
    _delay = random(0, slowAnimationDelay);
  }
}

String RandomBrightnessBursts::name() {
  return "RandomBrightnessBursts";
}
