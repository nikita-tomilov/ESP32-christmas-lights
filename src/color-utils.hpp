float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float cycleHue(float h) {
  if (h > 1.0) {
    h -= 1.0;
  } else if (h < 0.0) {
    h += 1.0;
  }
  return h;
}


float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float c_abs(float x) { return x < 0 ? -1 * x : x; }

float c_constrain(float x, float a, float b) {
  if (x < a) return a;
  if (x > b) return b;
  return x;
}

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, c_constrain(c_abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, c_constrain(c_abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, c_constrain(c_abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}
