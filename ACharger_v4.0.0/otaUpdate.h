#pragma once

#include <Arduino.h>
#include <Client.h>
#include <LittleFS.h>

#include "mqttProtocol.h"
#include "AbdOta.h"
#include "board_network.h"
//#include "mspBootloader.h"
#if (RUN_MODE == 2)

static AbdOta abdOta;

static inline Client& otaSelectClient() {
  switch (netType) {
    case NET_TYPE_ALL: {
      switch (comsupportedNetwork) {
        case NET_TYPE_GSM:  { static TinyGsmClient client(modem, 2); return client; }
        case NET_TYPE_WIFI: { static WiFiClient client; return client; }
        case NET_TYPE_ETH:  { static EthernetClient client; return client; }
        default: break;
      }
    }
    case NET_TYPE_GSM:  { static TinyGsmClient client(modem, 2); return client; }
    case NET_TYPE_WIFI: { static WiFiClient client; return client; }
    case NET_TYPE_ETH:  { static EthernetClient client; return client; }
    default: break;
  }

  static WiFiClient fallbackClient;
  return fallbackClient;
}

void update_prgs(size_t downloaded, size_t total) {
  static int last_percent = -1;
  int percent = (total > 0) ? (int)((100ULL * downloaded) / total) : 0;

  if (last_percent == percent)
    return;
   Serial.printf("[OTA] progress %d%% %u/%u\n",
              percent,
              (unsigned int)downloaded,
              (unsigned int)total);
  last_percent = percent;
  write_Screen_DownloadStatus(downloaded, total, percent);
}

void otaSetupCommon() {
  abdOta.setCurrentVersion(mqttDataValue.getOTA_VER());
  abdOta.setManifestUrl("http://85.105.100.7:5050/firmware.json");
  abdOta.setDeviceId(mqttDataValue.getESP_ID());
  abdOta.setApiKey(mqttDataValue.getOTA_KEY());
  abdOta.onProgress(update_prgs);
}

static bool testTcpConnectionOnce(Client& client, const char* host, uint16_t port, const String& path, uint32_t connectTimeoutMs = 15000, uint32_t readTimeoutMs = 10000) {
  if (client.connected())
    client.stop();

  uint32_t startMs = millis();
  while (!client.connect(host, port)) {
    if (millis() - startMs > connectTimeoutMs) {
      Serial.println("[TCP TEST] connect timeout");
      return false;
    }
    delay(250);
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  startMs = millis();
  while (!client.available()) {
    if (!client.connected()) {
      client.stop();
      Serial.println("[TCP TEST] server closed before response");
      return false;
    }

    if (millis() - startMs > 5000) {
      client.stop();
      Serial.println("[TCP TEST] response timeout");
      return false;
    }
    delay(10);
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();

  if (!(statusLine.startsWith("HTTP/1.1 200") || statusLine.startsWith("HTTP/1.0 200"))) {
    client.stop();
    Serial.println("[TCP TEST] bad http status");
    return false;
  }

  uint32_t lastDataMs = millis();
  size_t rxCount = 0;

  while (client.connected() || client.available()) {
    while (client.available()) {
      client.read();
      rxCount++;
      lastDataMs = millis();
    }

    if (millis() - lastDataMs > readTimeoutMs)
      break;

    delay(2);
  }

  client.stop();
  delay(500);

  if (rxCount == 0) {
    Serial.println("[TCP TEST] no payload");
    return false;
  }

  return true;
}

static bool preOtaTcpHealthCheck() {
  if (!recoverNetwork()) {
    Serial.println("[TCP TEST] network recover fail");
    return false;
  }

  Client& testClient = otaSelectClient();

  String path = "/firmware.json?id=";
  path += mqttDataValue.getESP_ID();
  path += "&ver=";
  path += mqttDataValue.getOTA_VER();
  path += "&key=";
  path += mqttDataValue.getOTA_KEY();

  bool ok1 = testTcpConnectionOnce(
      testClient,
      "85.105.100.7",
      5050,
      path,
      15000,
      10000
  );

  if (!ok1) {
    Serial.println("[TCP TEST] first test fail");
    return false;
  }

  delay(2000);

  bool ok2 = testTcpConnectionOnce(
      testClient,
      "85.105.100.7",
      5050,
      path,
      15000,
      10000
  );

  if (!ok2) {
    Serial.println("[TCP TEST] second test fail");
    return false;
  }

  return true;
}

void startOtaUpdate() {
  mqttDataValue.setOTA_UPDATE_ACTIVE("61");

  bool fsOk = LittleFS.begin(true);
  Serial.print("LittleFS.begin = ");
  Serial.println(fsOk ? "OK" : "FAIL");

  Serial.print("mqttDataValue.getOTA_VER() = ");
  Serial.println(mqttDataValue.getOTA_VER());

  Serial.print("mqttDataValue.getESP_ID() = ");
  Serial.println(mqttDataValue.getESP_ID());

  Serial.print("mqttDataValue.getOTA_KEY() = ");
  Serial.println(mqttDataValue.getOTA_KEY());

  otaSetupCommon();

  if (!preOtaTcpHealthCheck()) {
    Serial.println("[OTA] TCP health check fail, OTA iptal");
    return;
  }

  Client& otaClient = otaSelectClient();
  AbdOtaInfo info = abdOta.checkUpdate(otaClient);

  Serial.println("----- OTA INFO -----");
  Serial.print("available = ");
  Serial.println(info.available ? "true" : "false");
  Serial.print("currentVersion = ");
  Serial.println(info.currentVersion);
  Serial.print("serverVersion = ");
  Serial.println(info.serverVersion);
  Serial.print("firmwareUrl = ");
  Serial.println(info.firmwareUrl);
  Serial.print("reportUrl = ");
  Serial.println(info.reportUrl);
  Serial.println("--------------------");

  if (info.available) {
    Serial.println("New firmware available!");

    if (!recoverNetwork()) {
      Serial.println("[OTA] recover fail before update");
      Serial.println("OTA FAIL");
      return;
    }

    bool ok = abdOta.doUpdateResume(
        otaClient,
        info,
        []() -> bool {
          return recoverNetwork();
        }
    );

    if (!ok) {
      Serial.println("OTA FAIL");
    }

  } else {
    Serial.println("No new firmware");
    abdOta.reportNoNewFirmware(otaClient, info.reportUrl);
  }
}
bool downloadMspBinToLittleFS() {
  const char* host = "85.105.100.7";
  const uint16_t port = 5050;
  const char* path = "/msp.bin";

  if (!recoverNetwork()) {
    Serial.println("[MSP OTA] Network recover FAIL");
    return false;
  }

  if (!LittleFS.begin(true)) {
    Serial.println("[MSP OTA] LittleFS FAIL");
    return false;
  }

  if (LittleFS.exists("/msp.bin")) {
    Serial.println("[MSP OTA] Eski msp.bin siliniyor...");
    LittleFS.remove("/msp.bin");
  }

  Client& client = otaSelectClient();

  if (client.connected())
    client.stop();

  Serial.println("[MSP OTA] TCP baglaniyor...");

  uint32_t t0 = millis();
  while (!client.connect(host, port)) {
    if (millis() - t0 > 15000) {
      Serial.println("[MSP OTA] TCP connect timeout");
      return false;
    }
    delay(250);
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  t0 = millis();
  while (!client.available()) {
    if (!client.connected()) {
      client.stop();
      Serial.println("[MSP OTA] server response yok kapandi");
      return false;
    }

    if (millis() - t0 > 10000) {
      client.stop();
      Serial.println("[MSP OTA] response timeout");
      return false;
    }
    delay(10);
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();

  Serial.print("[MSP OTA] HTTP status: ");
  Serial.println(statusLine);

  if (!(statusLine.startsWith("HTTP/1.1 200") ||
        statusLine.startsWith("HTTP/1.0 200"))) {
    client.stop();
    Serial.println("[MSP OTA] bad HTTP status");
    return false;
  }

  int total = -1;

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();

    if (line.length() == 0)
      break;

    String lower = line;
    lower.toLowerCase();

    if (lower.startsWith("content-length:")) {
      String lenStr = line.substring(line.indexOf(':') + 1);
      lenStr.trim();
      total = lenStr.toInt();
    }
  }

  Serial.print("[MSP OTA] Content-Length: ");
  Serial.println(total);

  File file = LittleFS.open("/msp.bin", "w");
  if (!file) {
    client.stop();
    Serial.println("[MSP OTA] /msp.bin acilamadi");
    return false;
  }

  uint8_t buffer[512];
  int downloaded = 0;
  int lastPercent = -1;
  uint32_t lastDataMs = millis();

  while (client.connected() || client.available()) {
    while (client.available()) {
      int n = client.read(buffer, sizeof(buffer));

      if (n > 0) {
        file.write(buffer, n);
        downloaded += n;
        lastDataMs = millis();

        if (total > 0) {
          int percent = (downloaded * 100) / total;

          if (percent != lastPercent) {
            Serial.printf("[MSP OTA] %d%% (%d/%d)\n",
                          percent,
                          downloaded,
                          total);
            lastPercent = percent;
          }
        } else {
          Serial.printf("[MSP OTA] downloaded=%d\n", downloaded);
        }
      }
    }

    if (millis() - lastDataMs > 15000) {
      Serial.println("[MSP OTA] data timeout");
      break;
    }

    delay(2);
  }

  file.close();
  client.stop();

  File verify = LittleFS.open("/msp.bin", "r");
  if (!verify) {
    Serial.println("[MSP OTA] Verify FAIL");
    return false;
  }

  size_t finalSize = verify.size();
  verify.close();

  Serial.print("[MSP OTA] Final size: ");
  Serial.println(finalSize);

  if (total > 0 && finalSize != (size_t)total) {
    Serial.println("[MSP OTA] Size mismatch");
    return false;
  }

  Serial.println("[MSP OTA] Download TAMAMLANDI");
  return true;
}
#endif

/*
 * Proje   : EVSE – OTA Firmware Güncelleme Sistemi
 * Dosya   : otaUpdate.h
 * Amaç    : HTTP üzerinden firmware güncellemesi (resume destekli)
 *
 * Yazar   : Abdulhamit Mercan
 * Tarih   : 2026
 *
 * Özellikler:
 * - Resume destekli OTA (kaldığı yerden devam)
 * - WiFi / GSM / Ethernet destekli client seçimi
 * - TCP health check (OTA öncesi bağlantı testi)
 * - Network koparsa otomatik recover
 *
 * Akış:
 * 1. preOtaTcpHealthCheck()
 *    → sunucuya erişim test edilir
 *
 * 2. abdOta.checkUpdate()
 *    → firmware.json çekilir
 *
 * 3. abdOta.doUpdateResume()
 *    → fw.bin parça parça indirilir
 *
 * 4. Progress callback
 *    → ekrana % bilgisi basılır
 *
 * Network Yönetimi:
 * - otaHasInternetNow(): bağlantı gerçekten var mı kontrol eder
 * - otaRecoverNetworkNow(): koparsa yeniden bağlanır
 *
 * Client Seçimi:
 * - otaSelectClient():
 *      aktif network’e göre doğru Client döner
 *
 * Kritik Noktalar:
 * - OTA sırasında bağlantı koparsa:
 *      → resume byte ile devam edilir
 * - Server tarafı Range desteklemelidir (206 Partial)
 *
 * Notlar:
 * - OTA sadece TCP test başarılıysa başlar
 * - Her indirme öncesi network doğrulanır
 */










