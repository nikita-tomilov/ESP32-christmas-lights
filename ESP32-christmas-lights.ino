#include "display.hpp"
#include "leds.hpp"

//NOTE: this is gitignored.
//You have to create your own file
//and write your own creds there
#include "wifi-details.h"
#include <WiFi.h>
#define WIFI_AP_SSID "esp32-lights"
char apSsid[64] = {0};

int i = 0;

bool tryConnectWifi(int);
bool createAP();

void setup() {
  Serial.begin(115200);
  Serial.println("booting...");
  
  initScreen();
  drawString(0, 0, "display: OK");
  
  initLeds();
  drawString(0, 8, "leds: OK");

  drawString(0, 16, "wifi: connecting");
  if (tryConnectWifi(15000)) {
    fillString(0, 16, WIFI_SSID);
    drawString(0, 24, WiFi.localIP().toString());
  } else {
    fillString(0, 16, "wifi: configuring AP");
    createAP();
    fillString(0, 16, apSsid);
    drawString(0, 24, WiFi.softAPIP().toString());
  }

  setPixelColor(0, 0, 0, 0);
  drawLeds();
  delay(30000);
}

void loop() {
  char str[12];
  sprintf(str, "%d", i);
  clearScreen();
  drawString(i, i, str);
  delay(1000);
  i++;
}

bool tryConnectWifi(int timeoutMs) {
  Serial.println("trying to connect to:");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_OFF);
  Serial.println("wifi offed");
  WiFi.mode(WIFI_STA);
  Serial.println("wifi mode sta");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int br = 30;
  int delta = 10;
  long now = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    br += delta;
    if ((br > 200) || (br < 20)) delta = -1 * delta;
    setPixelColor(0, br, 0, 0);
    drawLeds();
    if (millis() - now > timeoutMs) {
      return false;
    }
  }
  return true;
}

bool createAP() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  sprintf(apSsid, "%s-%u", WIFI_AP_SSID, chipId);
  Serial.println("trying ap:");
  Serial.println(apSsid);
  WiFi.mode(WIFI_OFF);
  Serial.println("wifi offed");
  WiFi.mode(WIFI_AP);
  Serial.println("wifi mode ap");
  WiFi.softAP((const char*)apSsid, (const char*)apSsid);
  Serial.println("wifi ap ok");
  return true;
}
