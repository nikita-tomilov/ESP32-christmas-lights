#include "display.hpp"

int i = 0;

void setup() {
  Serial.begin(115200);
  initScreen();
}

void loop() {
  char str[12];
  sprintf(str, "%d", i);
  clearScreen();
  drawString(i, i, str);
  delay(1000);
  i++;
}
