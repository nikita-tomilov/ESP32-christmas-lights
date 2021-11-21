//NOTE: this is gitignored.
//You have to create your own file
//and write your own creds there
#include "wifi-details.h"

#ifdef ESP32
//for ESP32 I use WeMos LOLIN 32 with screen; leds are attached to pin 27

  #define LED_STRIP_PIN 27
  #include "leds.hpp"
  #include "display.hpp"
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <WebServer.h>
  WebServer server(80);
  #include <uri/UriBraces.h>
  #include "SPIFFS.h"
  #include "FS.h"

  TaskHandle_t ledTaskHandle;
  TaskHandle_t srvTaskHandle;
  #include "fft.hpp"

#elif defined(ESP8266)
//for ESP8266 I use WeMos D1 Mini without screen; leds are attached to D4/GPIO2

  #define LED_STRIP_PIN 2
  #include "leds.hpp"
  #include "no-display.hpp"
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266WebServer.h> 
  ESP8266WebServer server(80);
  #include <FS.h>

#else
#error Please use ESP32 or ESP8266
#endif


#define WIFI_AP_SSID "espXX-lights"
char apSsid[64] = {0};
char mDNSName[64] = {0};

#include "fairy-lights.hpp"


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
  SPIFFS.begin();

  fillString(3, "server ready");
  #ifdef ESP32
    initFFT();
  #endif
  initFairyLights();

  #ifdef ESP32
    //loop() runs on core 1
    xTaskCreatePinnedToCore(ledTask, "led-task", 10000, NULL, 2, &ledTaskHandle, 1);
    xTaskCreatePinnedToCore(srvTask, "audio-task", 10000, NULL, 1, &srvTaskHandle, 0);
  #endif

  fillString(4, "CM: " + getColorModeName());
  fillString(5, "BR: " + getBrightnessModeName());

  Serial.println("r e a d y");
}

#ifdef ESP32
long audioMs = 0;
long audioCount = 0;
void audioLoopAction() {
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
#endif

long ledMs = 0;
long ledCount = 0;
void ledTaskAction() {
  updateLights();
  delay(1);
  ledCount++;
  if (millis() - ledMs > 1000) {
    ledMs = millis();
    //Serial.print("leds : ");
    //Serial.println(ledCount);
    ledCount = 0;
  }
}

long srvMs = 0;
long srvCount = 0;
void srvTaskAction() {
  server.handleClient();
  srvCount++;
  if (millis() - srvMs > 1000) {
    srvMs = millis();
    //Serial.print("srv  : ");
    //Serial.println(srvCount);
    srvCount = 0;
  }
  delay(1);
}

void loop() {
  #ifdef ESP32
    audioLoopAction();
  #elif defined(ESP8266)
    ledTaskAction();
    srvTaskAction();
    MDNS.update();
  #endif
}

void ledTask(void* pvParameters) {
  for (;;) {
    ledTaskAction();
  }
}

void srvTask(void* pvParameters) {
  for (;;) {
    srvTaskAction();
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

String getChipId() {
#ifdef ESP32
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return String(chipId, HEX);
#elif defined(ESP8266)
  return String(ESP.getChipId(), HEX);
#endif
}

bool createAP() {
  String chipId = getChipId();
  sprintf(apSsid, "%s-%s", WIFI_AP_SSID, chipId.c_str());
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
  String chipId = getChipId();
  #ifdef ESP32
    sprintf(mDNSName, "esp32-%s", chipId.c_str());
  #elif defined(ESP8266)
    sprintf(mDNSName, "esp8266-%s", chipId.c_str());
  #endif
  return MDNS.begin((const char*)mDNSName);
}

int cconstrain(int ox, int mx) {
  return constrain(ox, 0, mx);
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    delay(1);
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    delay(1);
    file.close();                                       // Then close the file again
    return true;
  }
  return false;
}

void acceptArg(String argName, String argValue) {
  if (argName.compareTo("brightness") == 0) {
    int value = cconstrain(argValue.toInt(), 255);
    assignedBrightness = value;
  } else if (argName.compareTo("brightnessMode") == 0) {
    int value = cconstrain(argValue.toInt(), BRIGHTNESS_MODE_COUNT - 1);
    setBrightnessMode(value);
  } else if (argName.compareTo("colorMode") == 0) {
    int value = cconstrain(argValue.toInt(), COLOR_MODE_COUNT - 1);
    setColorMode(value);
  } else if (argName.compareTo("slowAnimationDelay") == 0) {
    int value = constrain(argValue.toInt(), 500, 5000);
    slowAnimationDelay = value;
  } else if (argName.compareTo("fastAnimationDelay") == 0) {
    int value = constrain(argValue.toInt(), 1, 100);
    fastAnimationDelay = value;
  }
  fillString(4, "CM: " + getColorModeName());
  fillString(5, "BR: " + getBrightnessModeName());
}

char srvBuf[256];
void setupServer() {
   server.on(F("/"), []() {
    handleFileRead("index.html");
  });

  server.on("/set/data", HTTP_POST, []() {
    int argsc = server.args();
    Serial.print("Got request with args:");
    Serial.println(argsc);
    for (int i = 0; i < argsc; i++) {
      Serial.print(" - ");
      Serial.print(server.argName(i));
      Serial.print(" - ");
      Serial.println(server.arg(i));
      acceptArg(server.argName(i), server.arg(i));
      
    }
    server.send(200, "text/plain", "send data ok\n");
  });

  server.onNotFound([]() {
  if (!handleFileRead(server.uri()))
    server.send(404, "text/plain", "404: Not Found");
  });
  server.begin();
}
