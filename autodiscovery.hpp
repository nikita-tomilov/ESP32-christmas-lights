#ifndef _AUTODISC_HPP
#define _AUTODISC_HPP

void initAutodiscovery() {
  MDNS.addService("http", "tcp", 80);
}

long lastPacketSent = millis();
#define DELAY_BETWEEN_AUTODISC_PACKETS 2000

#define SWITCHOFF_DELAY (DELAY_BETWEEN_AUTODISC_PACKETS * 20)


#define MAX_DEVICES 10

int discoveredDevices = 0;
String discoveredIPs[MAX_DEVICES];
String discoveredHostnames[MAX_DEVICES];
long discoveredTs[MAX_DEVICES];


int getIndex(String ip) {
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (ip.equals(discoveredIPs[i])) return i;
  }
  return -1;
}

void addFoundDevice(String ip, String host) {
  int idx = -1;
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (discoveredTs[i] == 0) {
      idx = i;
      break;
    }
  }
  if (idx == -1) idx = MAX_DEVICES - 1;
  discoveredTs[idx] = millis();
  discoveredHostnames[idx] = host;
  discoveredIPs[idx] = ip;
  Serial.println("Found new device " + discoveredHostnames[idx]);
}

void removeDisconnectedDevices() {
  long now = millis();
  for (int i = 0; i < MAX_DEVICES; i++) {
    if ((discoveredTs[i] > 0) && (now - discoveredTs[i] > SWITCHOFF_DELAY)) {
      Serial.println("Device " + discoveredHostnames[i] + " disappeared");
      discoveredTs[i] = 0;
      discoveredHostnames[i] = "";
      discoveredIPs[i] = "";
    }
  }
}

void addOrUpdateDevice(String ip, String host) {
  int idx = getIndex(ip);
  if (idx >= 0) {
    discoveredTs[idx] = millis();
    Serial.println("Device " + discoveredHostnames[idx] + " is still active");
  } else {
    addFoundDevice(ip, host);
  }
}

int getActiveDevicesCount() {
  int ans = 0;
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (discoveredTs[i] > 0) ans++;
  }
  return ans;
}

String buildAutodiscoveredString() {
  String ans = "";
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (discoveredTs[i] > 0) ans += discoveredHostnames[i] + ":" + discoveredIPs[i] + ";";
  }
  return ans;
}

void tryUpdateAutodiscovery() {
  int nrOfServices = MDNS.queryService("http", "tcp");
   
  if (nrOfServices == 0) {
    Serial.println("No services were found.");
  } else {
    for (int i = 0; i < nrOfServices; i=i+1) {
      addOrUpdateDevice(MDNS.IP(i).toString(), MDNS.hostname(i));
    }
  }

  removeDisconnectedDevices();
  fillString(7, "Others: " + String(getActiveDevicesCount()));
}


#endif
