#ifndef CONF2_H
#define CONF2_H

#include <FS.h>
#include <SPIFFS.h>

struct Config {
  const char* backendUrl;
  const char* chargeBoxId;
  const char* chargePointVendor;
  int model;
  const char* qrCode;
  int runMode;
  int maxPower;
};

Config config;


bool setupSPIFFS() {
  if (!SPIFFS.begin(true)) {
    return false;
  }
  return true;
}

bool loadConfig() {
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    return false;
  }

  size_t size = file.size();
  if (size == 0) {
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size + 1]);
  file.readBytes(buf.get(), size);
  buf[size] = '\0';
  file.close();

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, buf.get());

  if (error) {
    return false;
  }

  config.backendUrl        = doc["url"] | "";
  config.chargeBoxId       = doc["id"] | "";
  config.chargePointVendor = doc["vendor"] | "";
  config.model             = doc["model"] | 0;
  config.qrCode            = doc["qr_code"] | "";
  config.runMode           = doc["runmode"] | 0;
  config.maxPower          = doc["max_power"] | 0;

  return true;
}


void printConfig() {
  /*Serial.println("=== Yüklenen Konfigürasyon ===");
  Serial.println("Backend URL     : " + config.backendUrl);
  Serial.println("Charge Box ID   : " + config.chargeBoxId);
  Serial.println("Vendor          : " + config.chargePointVendor);
  Serial.print("Model (int)     : "); Serial.println(config.model);
  Serial.println("QR Code         : " + config.qrCode);
  Serial.print("Run Mode        : "); Serial.println(config.runMode);
  Serial.print("Max Power (kW)  : "); Serial.println(config.maxPower);
  Serial.println("==============================");*/
}



void configurationInit() {
  if (!setupSPIFFS()) return;

  if (!loadConfig()) {
    // konfigürasyon yapılamadı, mevcut koşullar devam ediyor
  }

}

#endif