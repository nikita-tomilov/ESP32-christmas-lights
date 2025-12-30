#ifndef _DISPLAY_HPP
#define _DISPLAY_HPP

/*
Remember: if using TTGO T-Display, do the following:
1) Download TFT_eSPI lib using Arduino IDE
2) Open in external editor (e.g. vi) file <arduino path>/libraries/TFT_eSPI/User_Setup_Select.h
3) Comment out line #include <User_Setup.h>
4) Uncomment line #include <User_Setups/Setup25_TTGO_T_Display.h>
Thus, the lib will automagically work correctly with T-Display's built-in screen.

If using any other TFT screen via this library, correct the <User_Setup.h> file accordingly by yourself.
 */

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

void clearScreen() {
  tft.fillScreen(TFT_BLACK);
}

void initScreen() {
  tft.init();
  tft.setRotation(1);
  clearScreen();
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
}

void drawString(int x, int y, const char* str) {
  tft.setCursor(x, y);
  tft.print(str);
}

void drawString(int x, int y, String str) {
  drawString(x, y, str.c_str());
}

void fillString(int x, int y, const char* str) {
  tft.fillRect(x, y, tft.width(), 12, TFT_BLACK);
  drawString(x, y, str);
}

void fillString(int x, int y, String str) {
  fillString(x, y, str.c_str());
}

void drawString(int line, String str) {
  drawString(0, line * 12, str.c_str());
}

void fillString(int line, String str) {
  fillString(0, line * 12, str.c_str());
}

int fimap(float x, float in_min, float in_max, float out_min, float out_max)
{
  int ans = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if (ans > out_max) ans = out_max;
  return ans;
}

uint8_t brightness[NUMPIXELS];
uint8_t colors[NUMPIXELS][3];
void showLedsOnScreen() {
  float w = tft.width() / NUMPIXELS;
  for (int i = 0; i < NUMPIXELS; i++) {
    int red = (int)(brightness[i]) * (int)(colors[i][0]) / 255;
    int green = (int)(brightness[i]) * (int)(colors[i][1]) / 255;
    int blue = (int)(brightness[i]) * (int)(colors[i][2]) / 255;
    red = fimap(red, 0, 255, 0, 31);
    green = fimap(green, 0, 255, 0, 63);
    blue = fimap(blue, 0, 255, 0, 31);
    unsigned int colour = red << 11 | green << 5 | blue;
    tft.drawFastVLine(i, 100, tft.height(), colour);
  }
}

#endif
