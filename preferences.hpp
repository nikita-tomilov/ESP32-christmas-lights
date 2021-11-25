
void prefsPutInt(char* key, int value) {
  //Serial.println("Write value " + String(key) + " : " + String(value));
  char buf[64];
  sprintf(buf, "/%s", key);
  File f = SPIFFS.open(buf, "w");
  if (!f) {
    Serial.println("error opening file " + String(key));
    return;
  }
  f.write((uint8_t*)(&value), sizeof(int));
  f.close();
}

int prefsGetInt(char* key, int defaultValue) {
  char buf[64];
  sprintf(buf, "/%s", key);
  File f = SPIFFS.open(buf, "r");
  if (!f) {
    //Serial.println("Read value " + String(key) + " answered default " + String(defaultValue));
    return defaultValue;
  }
  int value;
  int b = f.read((uint8_t*)(&value), sizeof(int));
  if (b != sizeof(int)) {
    f.close();
    //Serial.println("Read value " + String(key) + " answered default " + String(defaultValue));
    return defaultValue;
  }
  f.close();
  //Serial.println("Read value " + String(key) + " answered " + String(value));
  return value;
}

void prefsPutFloat(char* key, float value) {
  prefsPutInt(key, value * 10000);
}

float prefsGetFloat(char* key, float defaultValue) {
  int defVal = defaultValue * 10000;
  int val = prefsGetInt(key, defVal);
  return (float)(defVal) / 10000;
}

void prefsPutColor(int idx, uint8_t r, uint8_t g, uint8_t b) {
  char buf[64];
  sprintf(buf, "/color%d", idx);
  File f = SPIFFS.open(buf, "w");
  if (!f) {
    Serial.println("error opening color file " + String(idx));
    return;
  }
  uint8_t rgb[3];
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
  f.write(rgb, 3);
  f.close();
}

void prefsGetColor(int idx, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t dr, uint8_t dg, uint8_t db) {
  char buf[64];
  sprintf(buf, "/color%d", idx);
  File f = SPIFFS.open(buf, "r");
  if (!f) {
    Serial.println("error opening color file " + String(idx));
    *r = dr;
    *g = dg;
    *b = db;
    return;
  }
  uint8_t rgb[3];

  int bb = f.read(rgb, 3);
  if (bb != 3) {
    f.close();
    Serial.println("error opening color file " + String(idx));
    *r = dr;
    *g = dg;
    *b = db;
    return;
  }
  f.close();
  *r = rgb[0];
  *g = rgb[1];
  *b = rgb[2];
  //Serial.println("Read value " + String(key) + " answered " + String(value));
  return;
}
