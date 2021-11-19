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

#include "fft.hpp"
#include "fairy-lights.hpp"

TaskHandle_t ledTaskHandle;
TaskHandle_t srvTaskHandle;

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

  fillString(3, "server ready");
  initFFT();
  initFairyLights();

  //loop() runs on core 1
  xTaskCreatePinnedToCore(ledTask, "led-task", 10000, NULL, 1, &ledTaskHandle, 1);
  xTaskCreatePinnedToCore(srvTask, "audio-task", 10000, NULL, 1, &srvTaskHandle, 0);
  fillString(4, "threads are active");
}

long audioMs = 0;
long audioCount = 0;
void loop() {
  captureAudioData();
  analyzeAudioWithFFT();

  audioCount++;
  if (millis() - audioMs > 1000) {
    audioMs = millis();
    //Serial.print("audio: ");
    //Serial.println(audioCount);
    audioCount = 0;
  }
}

void ledTask(void* pvParameters) {
  long ledMs = 0;
  long ledCount = 0;
  for (;;) {
    updateLights();
    vTaskDelay(1);
    ledCount++;
    if (millis() - ledMs > 1000) {
      ledMs = millis();
      //Serial.print("leds : ");
      //Serial.println(ledCount);
      ledCount = 0;
    }
  }
}

void srvTask(void* pvParameters) {
  long srvMs = 0;
  long srvCount = 0;
  for (;;) {
    server.handleClient();
    srvCount++;
    if (millis() - srvMs > 1000) {
      srvMs = millis();
      //Serial.print("srv  : ");
      //Serial.println(srvCount);
      srvCount = 0;
    }
    delay(2);
  }
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

int limit(int ox, int mx) {
  int x = ox;
  if (x < 0) x = 0;
  if (x > mx) x = mx;
  return x;
}

char srvBuf[256];
void setupServer() {
   server.on(F("/"), []() {
    server.send(200, "text/plain", "hello from esp32!");
  });

  server.on(UriBraces("/users/{}"), []() {
    String user = server.pathArg(0);
    server.send(200, "text/plain", "User: '" + user + "'");
  });

  server.on(UriBraces("/color/{}/{}/{}/{}"), []() {
    String iS = server.pathArg(0);
    String rS = server.pathArg(1);
    String gS = server.pathArg(2);
    String bS = server.pathArg(3);
    int i = limit(iS.toInt(), 4);
    int r = limit(rS.toInt(), 255);
    int g = limit(gS.toInt(), 255);
    int b = limit(bS.toInt(), 255);
   
    sprintf(srvBuf, "color %d val %d %d %d\n", i, r, g, b);
    assignedColors[i][0] = r;
    assignedColors[i][1] = g;
    assignedColors[i][2] = b;
    server.send(200, "text/plain", (const char*)srvBuf);
  });

  server.on(UriBraces("/colorMode/{}"), []() {
    String mS = server.pathArg(0);
    int m = limit(mS.toInt(), 6);
   
    sprintf(srvBuf, "color mode %d\n", m);
    setColorMode(m);
    server.send(200, "text/plain", (const char*)srvBuf);
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
