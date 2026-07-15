#ifndef DYNAMIC_LOGO_H
#define DYNAMIC_LOGO_H

#include <Arduino.h>
#include <LittleFS.h>
#include <lvgl.h>
#include <Client.h>

#include "mqttProtocol.h"
#include "board_network.h"

#define LOGO_FILE     "/logo.bin"
#define LOGO_TMP_FILE "/logo.tmp"

#define LOGO_HOST "85.105.100.7"
#define LOGO_PORT 5050

extern const lv_img_dsc_t ui_img_5inclogo2_png;

static bool logoFsReady = false;

static uint8_t * logoRamData = NULL;
static size_t logoRamSize = 0;
static lv_img_dsc_t dynamicLogoDsc;

static bool loadLogoFromFsToRam();

static inline Client& logoSelectClient()
{
    switch (netType) {
        case NET_TYPE_ALL: {
            switch (comsupportedNetwork) {
                case NET_TYPE_GSM:  { static TinyGsmClient client(modem, 3); return client; }
                case NET_TYPE_WIFI: { static WiFiClient client; return client; }
                case NET_TYPE_ETH:  { static EthernetClient client; return client; }
                default: break;
            }
            break;
        }

        case NET_TYPE_GSM:  { static TinyGsmClient client(modem, 3); return client; }
        case NET_TYPE_WIFI: { static WiFiClient client; return client; }
        case NET_TYPE_ETH:  { static EthernetClient client; return client; }

        default: break;
    }

    static WiFiClient fallbackClient;
    return fallbackClient;
}

static bool dynamicLogoFsInit()
{
    if(logoFsReady) return true;

    if(!LittleFS.begin(false)) {
        Serial.println("[LOGO] LittleFS mount fail, format deneniyor");

        if(!LittleFS.begin(true)) {
            Serial.println("[LOGO] LittleFS mount/format fail");
            return false;
        }
    }

    logoFsReady = true;

    Serial.println("[LOGO] LVGL FS OK -> S:");
    return true;
}

static bool copyDefaultLogoToFs()
{
    if(!dynamicLogoFsInit()) return false;

    if(LittleFS.exists(LOGO_FILE)) {
        Serial.println("[LOGO] logo.bin zaten var");
        return true;
    }

    File f = LittleFS.open(LOGO_FILE, "w");

    if(!f) {
        Serial.println("[LOGO] logo.bin acilamadi");
        return false;
    }

    size_t written = f.write(
        (const uint8_t *)ui_img_5inclogo2_png.data,
        ui_img_5inclogo2_png.data_size
    );

    f.close();

    Serial.printf("[LOGO] default logo LittleFS'e yazildi: %u byte\n", written);

    return written == ui_img_5inclogo2_png.data_size;
}

static bool downloadLogoFileRaw(const char * path)
{
    if(!dynamicLogoFsInit()) return false;

    Client& client = logoSelectClient();

    if(client.connected())
        client.stop();

    Serial.print("[LOGO] connect host: ");
    Serial.print(LOGO_HOST);
    Serial.print(":");
    Serial.println(LOGO_PORT);

    if(!client.connect(LOGO_HOST, LOGO_PORT)) {
        Serial.println("[LOGO] TCP connect fail");
        client.stop();
        return false;
    }

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + LOGO_HOST + "\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long waitStart = millis();

    while(!client.available()) {
        if(millis() - waitStart > 15000UL) {
            Serial.println("[LOGO] HTTP response timeout");
            client.stop();
            return false;
        }
        delay(5);
    }

    String statusLine = client.readStringUntil('\n');
    statusLine.trim();

    Serial.print("[LOGO] HTTP status: ");
    Serial.println(statusLine);

    if(!(statusLine.startsWith("HTTP/1.1 200") || statusLine.startsWith("HTTP/1.0 200"))) {
        client.stop();
        return false;
    }

    int contentLength = -1;

    while(client.connected() || client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();

        if(line.length() == 0)
            break;

        if(line.startsWith("Content-Length:")) {
            String lenStr = line.substring(strlen("Content-Length:"));
            lenStr.trim();
            contentLength = lenStr.toInt();
        }
    }

    if(contentLength <= 0) {
        Serial.println("[LOGO] Content-Length invalid");
        client.stop();
        return false;
    }

    Serial.printf("[LOGO] Content-Length=%d\n", contentLength);

    File f = LittleFS.open(LOGO_TMP_FILE, "w");

    if(!f) {
        Serial.println("[LOGO] tmp acilamadi");
        client.stop();
        return false;
    }

    uint8_t buffer[512];
    int downloaded = 0;
    int lastPercent = -1;
    unsigned long lastDataMs = millis();

    while(downloaded < contentLength) {
        int availableLen = client.available();

        if(availableLen > 0) {
            int readLen = client.readBytes(
                buffer,
                min((int)sizeof(buffer), availableLen)
            );

            if(readLen > 0) {
                f.write(buffer, readLen);
                downloaded += readLen;
                lastDataMs = millis();

                int percent = (downloaded * 100) / contentLength;

                if(percent != lastPercent) {
                    Serial.printf("[LOGO] download %d%% | %d/%d\n",
                                  percent,
                                  downloaded,
                                  contentLength);
                    lastPercent = percent;
                }
            }
        }
        else {
            if(!client.connected()) {
                Serial.printf("[LOGO] TCP closed downloaded=%d total=%d\n",
                              downloaded,
                              contentLength);
                break;
            }

            if(millis() - lastDataMs > 20000UL) {
                Serial.printf("[LOGO] download timeout downloaded=%d total=%d\n",
                              downloaded,
                              contentLength);
                break;
            }

            delay(5);
        }
    }

    f.close();
    client.stop();

    if(downloaded != contentLength) {
        Serial.printf("[LOGO] eksik indi downloaded=%d total=%d\n",
                      downloaded,
                      contentLength);

        LittleFS.remove(LOGO_TMP_FILE);
        return false;
    }

    LittleFS.remove(LOGO_FILE);

    if(!LittleFS.rename(LOGO_TMP_FILE, LOGO_FILE)) {
        Serial.println("[LOGO] rename fail");
        LittleFS.remove(LOGO_TMP_FILE);
        return false;
    }

    Serial.printf("[LOGO] yeni logo kaydedildi: %d byte\n", downloaded);

    return true;
}

static bool loadLogoFromFsToRam()
{
    if(!dynamicLogoFsInit()) return false;

    if(!LittleFS.exists(LOGO_FILE)) {
        Serial.println("[LOGO] logo.bin yok");
        return false;
    }

    File f = LittleFS.open(LOGO_FILE, "r");

    if(!f) {
        Serial.println("[LOGO] logo.bin acilamadi");
        return false;
    }

    size_t fileSize = f.size();

    if(fileSize == 0) {
        Serial.println("[LOGO] bos logo");
        f.close();
        return false;
    }

    if(logoRamData) {
        free(logoRamData);
        logoRamData = NULL;
        logoRamSize = 0;
    }

    logoRamData = (uint8_t *)malloc(fileSize);

    if(!logoRamData) {
        Serial.printf("[LOGO] malloc fail size=%u\n", fileSize);
        f.close();
        return false;
    }

    size_t readed = f.read(logoRamData, fileSize);
    f.close();

    if(readed != fileSize) {
        Serial.printf("[LOGO] eksik okundu read=%u size=%u\n",
                      readed,
                      fileSize);

        free(logoRamData);
        logoRamData = NULL;
        logoRamSize = 0;

        return false;
    }

    logoRamSize = fileSize;

    dynamicLogoDsc.header.always_zero = 0;
    dynamicLogoDsc.header.w = ui_img_5inclogo2_png.header.w;
    dynamicLogoDsc.header.h = ui_img_5inclogo2_png.header.h;
    dynamicLogoDsc.header.cf = ui_img_5inclogo2_png.header.cf;
    dynamicLogoDsc.data_size = logoRamSize;
    dynamicLogoDsc.data = logoRamData;

    Serial.printf("[LOGO] RAM logo hazir size=%u\n", logoRamSize);

    return true;
}

static void setLogoSource(lv_obj_t * img)
{
    if(!img) return;

    if(logoRamData || loadLogoFromFsToRam()) {
        lv_img_cache_invalidate_src(&dynamicLogoDsc);
        lv_img_set_src(img, &dynamicLogoDsc);
        Serial.println("[LOGO] dynamic logo aktif");
    }
    else {
        lv_img_set_src(img, &ui_img_5inclogo2_png);
        Serial.println("[LOGO] default logo aktif");
    }
}

static void refreshAllLogoImages()
{
    setLogoSource(ui_Image1);
    setLogoSource(ui_Image16);
    setLogoSource(ui_Image31);
    setLogoSource(ui_Image46);
}

static bool updateLogoByName(const char * logoName)
{
    if(!logoName || strlen(logoName) == 0) {
        Serial.println("[LOGO] logoName bos");
        return false;
    }

    char path[120];

    snprintf(
        path,
        sizeof(path),
        "/logos/%s.bin",
        logoName
    );

    Serial.printf("[LOGO] logo name: %s\n", logoName);
    Serial.printf("[LOGO] logo path: %s\n", path);

    bool ok = downloadLogoFileRaw(path);

    if(!ok) {
        Serial.println("[LOGO] logo download fail");
        return false;
    }

    if(logoRamData) {
        free(logoRamData);
        logoRamData = NULL;
        logoRamSize = 0;
    }

    if(!loadLogoFromFsToRam()) {
        Serial.println("[LOGO] RAM yukleme fail");
        return false;
    }

    refreshAllLogoImages();

    Serial.println("[LOGO] logo ekrana basildi");

    return true;
}

static void dynamicLogoInitFromMqttConfig()
{
    dynamicLogoFsInit();

    if(LittleFS.exists(LOGO_FILE)) {
        Serial.println("[LOGO] mevcut logo LittleFS'ten yukleniyor");

        if(loadLogoFromFsToRam()) {
            refreshAllLogoImages();
        }
        else {
            Serial.println("[LOGO] LittleFS logo okunamadi, default kullaniliyor");
            refreshAllLogoImages();
        }
    }
    else {
        Serial.println("[LOGO] logo.bin yok, default logo kullaniliyor");
        refreshAllLogoImages();
    }

    const char * logoUpdate = mqttDataValue.getLOGO_UPDATE();

    if(strcmp(logoUpdate, "1") != 0) {
        Serial.println("[LOGO] MQTT logo update yok");
        return;
    }

    const char * logoName = mqttDataValue.getLOGO_NAME();

    Serial.printf("[LOGO] MQTT logo update aktif, name=%s\n", logoName);

    mqttDataValue.setLOGO_UPDATE("0");
    Serial.println("[LOGO] LOGO_UPDATE deneme baslarken sifirlandi");

    bool ok = updateLogoByName(logoName);

    if(ok) {
        mqttDataValue.setACTIVE_LOGO(logoName);
        Client& client = logoSelectClient();

        String url ="/logo_report?id=" +String(mqttDataValue.getESP_ID()) +"&logo=" +String(logoName) +"&status=written";

        if(client.connected())
            client.stop();

        if(client.connect(LOGO_HOST, LOGO_PORT)) {

            client.print(String("GET ") + url + " HTTP/1.1\r\n" +"Host: " + LOGO_HOST + "\r\n" +"Connection: close\r\n\r\n");

            Serial.println("[LOGO] logo_report gonderildi");

            delay(300);

            client.stop();
        }
       

        Serial.printf("[LOGO] ACTIVE_LOGO yazildi: %s\n", logoName);
        Serial.println("[LOGO] MQTT logo update OK");
    }
    else {
        Serial.println("[LOGO] MQTT logo update basarisiz, tekrar denemeyecek");
    }

   ESP.restart();
}

static void dynamicLogoInit()
{
    dynamicLogoInitFromMqttConfig();
}

#endif