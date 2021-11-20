class Rainbow : public IEffect {
  public:
    void init();
    void tick();
    void slowAction();
    void fastAction();
    String name();
    Rainbow(float delta, bool reverseIdx) : _delta(delta), _reverseIdx(reverseIdx) {};
  private:
    float _delta;
    bool _reverseIdx;
    float _rainbowStartingPoint;
};

void Rainbow::init() {}
void Rainbow::tick() {}
void Rainbow::slowAction() {}
void Rainbow::fastAction() {
  _rainbowStartingPoint += _delta;
  _rainbowStartingPoint = cycleHue(_rainbowStartingPoint);

  for (int i = 0; i < NUMPIXELS; i++) {
    float h = fmap(i, 0, NUMPIXELS, 0.0, 1.0);
    if (_delta != 0) {
      h += _rainbowStartingPoint;
    }
    h = cycleHue(h);
    float rgb[3];
    hsv2rgb(h, 1.0, 1.0, rgb);
    for (int j = 0; j < 3; j++) {
      int idx = i;
      if (_reverseIdx) {
        idx = NUMPIXELS - idx;
      }
      colors[idx][j] = (int)(rgb[j] * 255.0);
    }
  }
}

String Rainbow::name() {
  String movement = "";
  if (_delta < 0) movement = " CCW";
  if (_delta > 0) movement = " CW";
  String reversed = "";
  if (_reverseIdx) reversed = " R";
  return "Rainbow" + movement + reversed;
}
