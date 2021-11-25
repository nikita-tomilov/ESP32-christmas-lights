#ifdef ESP32
//for ESP32 I use WeMos LOLIN 32 with screen; leds are attached to pin 27

  #define LED_STRIP_PIN 27
  #include "leds.hpp"
  #include "display.hpp"
  #include <WiFi.h>
  #include <WiFiMulti.h>
  #include <ESPmDNS.h>
  #include <WebServer.h>
  WebServer server(80);
  WiFiMulti wifiMulti;
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
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #include <ESP8266mDNS.h>
  #include <ESP8266WebServer.h> 
  ESP8266WebServer server(80);
  #include <FS.h>

#else
#error Please use ESP32 or ESP8266
#endif


#define WIFI_AP_SSID "esp-lights"
char apSsid[64] = {0};
char mDNSName[64] = {0};

#include "autodiscovery.hpp"
#include "fairy-lights.hpp"
#include "preferences.hpp"

void setup() {
  Serial.begin(115200);
  Serial.println("booting...");
  
  initScreen();
  fillString(0, "display: OK");
  
  initLeds();
  fillString(1, "leds: OK");

  fillString(2, "wifi: connecting");
  if (tryConnectWifi(15000)) {
    fillString(0, WiFi.SSID().c_str());
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

  restorePreferences();

  #ifdef ESP32
    //loop() runs on core 1
    xTaskCreatePinnedToCore(ledTask, "led-task", 10000, NULL, 3, &ledTaskHandle, 1);
    xTaskCreatePinnedToCore(srvTask, "audio-task", 10000, NULL, 1, &srvTaskHandle, 0);
  #endif

  fillString(4, "CM: " + getColorModeName());
  fillString(5, "BR: " + getBrightnessModeName());
  initAutodiscovery();
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
  tryUpdateAutodiscovery();
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
  setPixelColor(0, 255, 0, 0);
  Serial.println("trying to connect to wifi:");
  WiFi.mode(WIFI_OFF);
  Serial.println("wifi offed");
  WiFi.mode(WIFI_STA);
  Serial.println("wifi mode sta");

  //NOTE: this is gitignored.
  //You have to create your own file
  //and write your own creds there
  //e.g. wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
  #include "wifi-details.h"

  if (wifiMulti.run(timeoutMs) == WL_CONNECTED) {
    Serial.print("WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("WiFi not connected!");
    return false;
  }
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
  setPixelColor(0, 0, 0, 255);
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

void restorePreferences() {
  assignedBrightness = prefsGetInt("brightness",  128);
  setBrightnessMode(prefsGetInt("brightnessMode",  0));
  setColorMode(prefsGetInt("colorMode",  0));
  slowAnimationDelay = prefsGetInt("slowAnimationDelay",  1000);
  fastAnimationDelay = prefsGetInt("fastAnimationDelay",  10);
#ifdef ESP32
  setInputGain(prefsGetFloat("inputGain",  1.0));
  setCutoff(prefsGetInt("cutoff",  800));
#endif
  restoreColor(0, 255, 255, 255);
  restoreColor(1, 255, 0, 0);
  restoreColor(2, 0, 255, 0);
  restoreColor(3, 255, 255, 0);
  restoreColor(4, 0, 0, 255);
  Serial.println("settings restored");
}

float cconstrain(float x, float a, float b) {
  if (x < a) return a;
  if (x > b) return b;
  return x;
}

void restoreColor(int idx, int dr, int dg, int db) {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  prefsGetColor(idx, &r, &g, &b, dr, dg, db);
  assignedColors[idx][0] = r;
  assignedColors[idx][1] = g;
  assignedColors[idx][2] = b;
}

void setColor(int idx, String value) {
  long long number = strtoll( value.c_str(), NULL, 16);
  long long r = number >> 16;
  long long g = number >> 8 & 0xFF;
  long long b = number & 0xFF;
  assignedColors[idx][0] = r;
  assignedColors[idx][1] = g;
  assignedColors[idx][2] = b;
  prefsPutColor(idx, r, g, b);
}

//TODO: color
void acceptArg(String argName, String argValue) {
  if (argName.compareTo("brightness") == 0) {
    int value = cconstrain(argValue.toInt(), 255);
    assignedBrightness = value;
    prefsPutInt("brightness",  value);
  } else if (argName.compareTo("brightnessMode") == 0) {
    int value = cconstrain(argValue.toInt(), BRIGHTNESS_MODE_COUNT - 1);
    setBrightnessMode(value);
    prefsPutInt("brightnessMode",  value);
  } else if (argName.compareTo("colorMode") == 0) {
    int value = cconstrain(argValue.toInt(), COLOR_MODE_COUNT - 1);
    setColorMode(value);
    prefsPutInt("colorMode",  value);
  } else if (argName.indexOf("color") >= 0) {
    char num = argName[5];
    int idx = num - '0';
    setColor(idx, argValue);
  } else if (argName.compareTo("slowAnimationDelay") == 0) {
    int value = constrain(argValue.toInt(), 500, 5000);
    slowAnimationDelay = value;
    prefsPutInt("slowAnimationDelay",  value);
  } else if (argName.compareTo("fastAnimationDelay") == 0) {
    int value = constrain(argValue.toInt(), 1, 100);
    fastAnimationDelay = value;
    prefsPutInt("fastAnimationDelay",  value);
  } 
#ifdef ESP32
  else if (argName.compareTo("inputGain") == 0) {
    float value = cconstrain(argValue.toFloat(), 0, 2); //todo: float
    setInputGain(value);
    prefsPutFloat("inputGain",  value);
  } else if (argName.compareTo("cutoff") == 0) {
    int value = constrain(argValue.toInt(), 500, 4000);
    setCutoff(value);
    prefsPutInt("cutoff",  value);
  }
#endif
  fillString(4, "CM: " + getColorModeName());
  fillString(5, "BR: " + getBrightnessModeName());
}

void sendStatus() {
  String status = "fft=";
  #ifdef ESP32
    status += "true";
    status += "\n&hw=ESP32-" + getChipId();
  #elif defined(ESP8266)
    status += "false";
    status += "\n&hw=ESP8266-" + getChipId();
  #endif  
  status += "\n&ip=" + WiFi.localIP().toString();
  status += "\n&autodiscovery=" + buildAutodiscoveredString();
  status += "\n&colorModes=";
  for (int i = 0; i < COLOR_MODE_COUNT; i++) {
    status += String(i) + ":" + getColorModeName(i) + ";";
  }
  status += "\n&brightnessModes=";
  for (int i = 0; i < BRIGHTNESS_MODE_COUNT; i++) {
    status += String(i) + ":" + getBrightnessModeName(i) + ";";
  }
  status += "\n&brightness=" + String(assignedBrightness);
  status += "\n&brightnessMode=" + String(brightnessMode);
  status += "\n&colorMode=" + String(colorMode);
  char buf[10] = {0};
  for (int i = 0; i < 5; i++) {
    sprintf(buf, "%02X%02X%02X", assignedColors[i][0], assignedColors[i][1], assignedColors[i][2]);
    status += "\n&color" + String(i) + "=" + String(buf);
  }
  status += "\n&slowAnimationDelay=" + String(slowAnimationDelay);
  status += "\n&fastAnimationDelay=" + String(fastAnimationDelay);
#ifdef ESP32
  status += "\n&inputGain=" + String(getInputGain());
  status += "\n&cutoff=" + String(getCutoff());
#endif
  status += "\n";
  server.send(200, "text/plain", status);
}

char srvBuf[256];
void setupServer() {
   server.on(F("/"), []() {
    handleFileRead("/");
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

  server.on("/get/data", HTTP_GET, []() {
    sendStatus();
  });

  server.onNotFound([]() {
  if (!handleFileRead(server.uri()))
    server.send(404, "text/plain", "404: Not Found");
  });
  server.begin();
}
