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

#include "FS.h"
#include "SPIFFS.h"

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
  initFFT();
  initFairyLights();

  //loop() runs on core 1
  xTaskCreatePinnedToCore(ledTask, "led-task", 10000, NULL, 1, &ledTaskHandle, 1);
  xTaskCreatePinnedToCore(srvTask, "audio-task", 10000, NULL, 1, &srvTaskHandle, 0);
  fillString(4, "threads are active");

  Serial.println("r e a d y");
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
    delay(1);
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
    delay(1);
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
  /*Serial.println("\tFile Not Found;\nAvailable files:");
  String str = "";
  File dir = SPIFFS.open("/");
  File file = dir.openNextFile();
  while(file){
    str += file.path();
    str += " / ";
    str += file.size();
    str += "\r\n";
    delay(1);
  }
  Serial.print(str);
  Serial.println("Available files end\n\n");*/
  return false;
}

void acceptArg(String argName, String argValue) {
  if (argName.compareTo("brightness") == 0) {
    int value = limit(argValue.toInt(), 255);
    assignedBrightness = value;
  } else if (argName.compareTo("brightnessMode") == 0) {
    int value = limit(argValue.toInt(), BRIGHTNESS_MODE_COUNT - 1);
    setBrightnessMode(value);
  } else if (argName.compareTo("colorMode") == 0) {
    int value = limit(argValue.toInt(), COLOR_MODE_COUNT - 1);
    setColorMode(value);
  }
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
