#pragma once

#include <Arduino.h>
#include <Client.h>
#include <LittleFS.h>

#include "mqttProtocol.h"
#include "AbdOta.h"
#include "board_network.h"

#if (RUN_MODE == 2)

static AbdOta abdOta;

static String getSafeEspHardwareVersion() {
  String hw = mqttDataValue.getESP_HW_VER();
  hw.trim();
  hw.toLowerCase();

  int vPos = hw.indexOf('v');
  if (vPos >= 0)
    hw = hw.substring(vPos + 1);

  String result;

  for (size_t i = 0; i < hw.length(); i++) {
    char c = hw[i];

    if (c >= '0' && c <= '9')
      result += c;
  }

  return result.isEmpty() ? "0" : result;
}

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
  static int lastPercent = -1;
  int percent = total > 0 ? (int)((100ULL * downloaded) / total) : 0;

  if (lastPercent == percent)
    return;

  lastPercent = percent;

  Serial.printf("[OTA] progress %d%% %u/%u\n", percent, (unsigned int)downloaded, (unsigned int)total);
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

    if (millis() - startMs > 5000UL) {
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
    Serial.print("[TCP TEST] bad HTTP status: ");
    Serial.println(statusLine);
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

static String createEspManifestPath() {
  String path = "/firmware.json?id=";
  path += mqttDataValue.getESP_ID();
  path += "&ver=";
  path += mqttDataValue.getOTA_VER();
  path += "&hw=";
  path += getSafeEspHardwareVersion();
  path += "&key=";
  path += mqttDataValue.getOTA_KEY();
  return path;
}

static bool preOtaTcpHealthCheck() {
  if (!recoverNetwork()) {
    Serial.println("[TCP TEST] network recover fail");
    return false;
  }

  Client& testClient = otaSelectClient();
  String path = createEspManifestPath();

  bool ok1 = testTcpConnectionOnce(testClient, "85.105.100.7", 5050, path, 15000, 10000);

  if (!ok1) {
    Serial.println("[TCP TEST] first test fail");
    return false;
  }

  delay(2000);

  bool ok2 = testTcpConnectionOnce(testClient, "85.105.100.7", 5050, path, 15000, 10000);

  if (!ok2) {
    Serial.println("[TCP TEST] second test fail");
    return false;
  }

  return true;
}

static String extractEspJsonString(const String& json, const String& key) {
  String pattern = "\"" + key + "\"";

  int keyPos = json.indexOf(pattern);
  if (keyPos < 0)
    return "";

  int colonPos = json.indexOf(':', keyPos + pattern.length());
  if (colonPos < 0)
    return "";

  int firstQuote = json.indexOf('"', colonPos + 1);
  if (firstQuote < 0)
    return "";

  int secondQuote = json.indexOf('"', firstQuote + 1);
  if (secondQuote < 0)
    return "";

  return json.substring(firstQuote + 1, secondQuote);
}

static AbdOtaInfo checkEspUpdateWithHardware(Client& client) {
  AbdOtaInfo info;

  info.currentVersion = mqttDataValue.getOTA_VER();

  String path = createEspManifestPath();

  Serial.print("[OTA] manifestPath = ");
  Serial.println(path);

  if (client.connected())
    client.stop();

  if (!client.connect("85.105.100.7", 5050)) {
    Serial.println("[OTA] manifest connect failed");
    return info;
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: 85.105.100.7\r\n" +
               "Connection: close\r\n\r\n");

  uint32_t startedAt = millis();

  while (!client.available()) {
    if (!client.connected() || millis() - startedAt > 10000UL) {
      client.stop();
      Serial.println("[OTA] manifest timeout");
      return info;
    }

    delay(5);
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();

  bool httpOk = statusLine.startsWith("HTTP/1.1 200") || statusLine.startsWith("HTTP/1.0 200");

  if (!httpOk) {
    Serial.print("[OTA] manifest HTTP error: ");
    Serial.println(statusLine);
    client.stop();
    return info;
  }

  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');

    if (line == "\r" || line.length() == 0)
      break;
  }

  String body;
  uint32_t lastDataMs = millis();

  while (client.connected() || client.available()) {
    while (client.available()) {
      body += (char)client.read();
      lastDataMs = millis();
    }

    if (millis() - lastDataMs > 3000UL)
      break;

    delay(1);
  }

  client.stop();

  info.rawJson = body;

  String availableText = extractEspJsonString(body, "available");

  info.serverVersion = extractEspJsonString(body, "version");
  info.firmwareUrl = extractEspJsonString(body, "url");
  info.reportUrl = extractEspJsonString(body, "report_url");
  info.available = availableText == "true" || availableText == "1";

  return info;
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

  Serial.print("mqttDataValue.getESP_HW_VER() = ");
  Serial.println(mqttDataValue.getESP_HW_VER());

  Serial.print("Safe ESP HW = ");
  Serial.println(getSafeEspHardwareVersion());

  otaSetupCommon();

  if (!preOtaTcpHealthCheck()) {
    Serial.println("[OTA] TCP health check fail, OTA iptal");
    return;
  }

  Client& otaClient = otaSelectClient();

  // abdOta.checkUpdate() yerine donanim surumunu da
  // sunucuya gonderen kontrol fonksiyonu kullaniliyor.
  AbdOtaInfo info = checkEspUpdateWithHardware(otaClient);

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

    bool ok = abdOta.doUpdateResume(otaClient, info, []() -> bool {
      return recoverNetwork();
    });

    if (!ok)
      Serial.println("OTA FAIL");

  } else {
    Serial.println("No new firmware");

    if (!info.reportUrl.isEmpty())
      abdOta.reportNoNewFirmware(otaClient, info.reportUrl);
  }
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










