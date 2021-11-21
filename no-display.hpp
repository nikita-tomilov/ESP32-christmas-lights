#ifndef _DISPLAY_HPP
#define _DISPLAY_HPP

void clearScreen() {
  
}

void initScreen() {
  
}

void drawString(int x, int y, const char* str) {
  Serial.println(str);
}

void drawString(int x, int y, String str) {
  drawString(x, y, str.c_str());
}

void fillString(int x, int y, const char* str) {
  drawString(x, y, str);
}

void fillString(int x, int y, String str) {
  fillString(x, y, str.c_str());
}

void drawString(int line, String str) {
  drawString(0, line * 8, str.c_str());
}

void fillString(int line, String str) {
  fillString(0, line * 8, str.c_str());
}

#endif
