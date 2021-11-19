#include "display.hpp"
#include "leds.hpp"

int i = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("booting...");
  
  initScreen();
  drawString(0, 0, "display: OK");
  
  initLeds();
  drawString(0, 8, "leds: OK");

  setPixelColor(0, 32, 0, 0);
  setPixelColor(1, 0, 32, 0);
  setPixelColor(2, 0, 0, 32);
  drawLeds();
  delay(5000);
}

void loop() {
  char str[12];
  sprintf(str, "%d", i);
  clearScreen();
  drawString(i, i, str);
  delay(1000);
  i++;
}
