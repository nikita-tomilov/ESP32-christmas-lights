class SingleColor : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
};

void SingleColor::init() {}
void SingleColor::tick() {}
void SingleColor::slowAction() {
  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < 3; j++) {
      colors[i][j] = assignedColors[0][j];
    }
  }
}
void SingleColor::fastAction() {}

String SingleColor::name() {
  return "SingleColor";
}

//==================================================

class TwoColors : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
  private:
    int twoColorsTickValue;
};

void TwoColors::init() {
  twoColorsTickValue = 0;
}
void TwoColors::tick() {}
void TwoColors::slowAction() {
  for (int i = 0; i < NUMPIXELS; i++) {
    int idx = 0;
    if (i % 2 == 0) {
      idx = 2 - twoColorsTickValue;
    } else {
      idx = 1 + twoColorsTickValue;
    }
    for (int j = 0; j < 3; j++) {
      colors[i][j] = assignedColors[idx][j];
    }
  }
  twoColorsTickValue = 1 - twoColorsTickValue;
}
void TwoColors::fastAction() {}

String TwoColors::name() {
  return "TwoColors";
}

//==================================================

class FiveColors : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
};

void FiveColors::init() {}
void FiveColors::tick() {}
void FiveColors::slowAction() {
  int prevIdx = 0;
  for (int i = 0; i < NUMPIXELS; i++) {
    int idx = random(1000) % 5;
    if (idx == prevIdx) {
      idx = 4 - idx;
    }
    for (int j = 0; j < 3; j++) {
      colors[i][j] = assignedColors[idx][j];
    }
    prevIdx = idx;
  }
}
void FiveColors::fastAction() {}

String FiveColors::name() {
  return "FiveColors";
}
