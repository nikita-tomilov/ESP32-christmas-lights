#include "display.hpp"
#include "leds.hpp"

//NOTE: this is gitignored.
//You have to create your own file
//and write your own creds there
#include "wifi-details.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#define WIFI_AP_SSID "esp32-lights"
char apSsid[64] = {0};
char mDNSName[64] = {0};

#include <WebServer.h>
WebServer server(80);
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>


void setup() {
  Serial.begin(115200);
  Serial.println("booting...");
  
  initScreen();
  fillString(0, "display: OK");
  
  initLeds();
  fillString(1, "leds: OK");

  fillString(2, "wifi: connecting");
  if (tryConnectWifi(15000)) {
    fillString(0, WIFI_SSID);
    fillString(1, WiFi.localIP().toString());
  } else {
    fillString(2, "wifi: configuring AP");
    createAP();
    fillString(0, apSsid);
    fillString(1, WiFi.softAPIP().toString());
  }
  if (mDNS()) {
    fillString(2, mDNSName);
  } else {
    fillString(2, "mDNS n/a");
  }

  setPixelColor(0, 0, 0, 0);
  drawLeds();

  fillString(3, "starting server...");
  setupServer();
  delay(30000);
}

void loop() {
  fillString(4, "r e a d y");
  server.handleClient();
  delay(2);
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

uint32_t getChipId() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}

bool createAP() {
  uint32_t chipId = getChipId();
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

bool mDNS() {
  uint32_t chipId = getChipId();
  sprintf(mDNSName, "esp32-%u", chipId);
  return MDNS.begin((const char*)mDNSName);
}

void setupServer() {
   server.on(F("/"), []() {
    server.send(200, "text/plain", "hello from esp32!");
  });

  server.on(UriBraces("/users/{}"), []() {
    String user = server.pathArg(0);
    server.send(200, "text/plain", "User: '" + user + "'");
  });

  server.on(UriBraces("/color/{}/{}/{}"), []() {
    String rS = server.pathArg(0);
    String gS = server.pathArg(1);
    String bS = server.pathArg(2);
    server.send(200, "text/plain", "RGB: " + rS + " " + gS + " " + bS);
  });
  
  server.on(UriRegex("^\\/users\\/([0-9]+)\\/devices\\/([0-9]+)$"), []() {
    String user = server.pathArg(0);
    String device = server.pathArg(1);
    server.send(200, "text/plain", "User: '" + user + "' and Device: '" + device + "'");
  });
  /*server.onNotFound([]() {
  if (!handleFileRead(server.uri()))
    server.send(404, "text/plain", "404: Not Found");
  });*/
  server.begin();
}
