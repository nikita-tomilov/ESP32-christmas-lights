#ifndef _DISPLAY_HPP
#define _DISPLAY_HPP

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void clearScreen() {
  display.clearDisplay();
}

void initScreen() {
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  clearScreen();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void drawString(int x, int y, char* str) {
  display.setCursor(x, y);
  display.println(str);
  display.display();
}

#endif
