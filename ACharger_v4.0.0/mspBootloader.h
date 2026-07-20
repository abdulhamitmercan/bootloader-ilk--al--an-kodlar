#pragma once

#include <Arduino.h>
#include <Client.h>
#include <LittleFS.h>

#include "mqttProtocol.h"
#include "AbdOta.h"
#include "board_network.h"

#if (RUN_MODE == 2)

static AbdOta mspAbdOta;

#define MSP_LOCAL_FILE "/msp.txt"

static String getSafeMspCurrentVersion()
{
    String version = mqttDataValue.getMSP_SW_VER();
    version.trim();

    if (version.isEmpty()) {
        return "msp@0.0.0";
    }

    if (version.startsWith("msp@")) {
        version = version.substring(4);
    }

    version.replace("SW ", "");
    version.replace("SW", "");
    version.trim();

    if (version.startsWith("v") || version.startsWith("V")) {
        version.remove(0, 1);
    }

    version.replace(" ", "");
    version.trim();

    if (version.isEmpty()) {
        version = "0.0.0";
    }

    return "msp@" + version;
}

static String getSafeMspHardwareVersion()
{   
    
    String hw = mqttDataValue.getMSP_HW_VER();
    hw.trim();
    hw.toLowerCase();

    int vPos = hw.indexOf('v');

    if (vPos >= 0) {
        hw = hw.substring(vPos + 1);
    }

    String result;

    for (size_t i = 0; i < hw.length(); i++) {
        char c = hw[i];

        if (c >= '0' && c <= '9') {
            result += c;
        }
    }

    if (result.isEmpty()) {
        return "0";
    }

    return result;
    
}

static void mspDownloadProgress(size_t downloaded, size_t total)
{
    static int lastPercent = -1;

    int percent =
        total > 0
            ? (int)((100ULL * downloaded) / total)
            : 0;

    if (percent == lastPercent) {
        return;
    }

    lastPercent = percent;

    Serial.printf("[MSP DOWNLOAD] %d%% %u/%u\n", percent, (unsigned int)downloaded, (unsigned int)total);

    write_Screen_DownloadStatusmsp(downloaded, total, percent);
}

static void mspOtaSetupCommon()
{
    mspAbdOta.setCurrentVersion(getSafeMspCurrentVersion());

    mspAbdOta.setManifestUrl("http://85.105.100.7:5050/firmware.json");

    mspAbdOta.setDeviceId(mqttDataValue.getESP_ID());

    mspAbdOta.setApiKey(mqttDataValue.getOTA_KEY());

    mspAbdOta.onProgress(mspDownloadProgress);
}

static bool mspTcpHealthCheck()
{
    if (!recoverNetwork()) {
        return false;
    }

    Client& client = otaSelectClient();

    String path = "/firmware.json?id=";
    path += mqttDataValue.getESP_ID();
    path += "&ver=";
    path += getSafeMspCurrentVersion();
    path += "&hw=";
    path += getSafeMspHardwareVersion();
    path += "&key=";
    path += mqttDataValue.getOTA_KEY();

    return testTcpConnectionOnce(client, "85.105.100.7", 5050, path, 15000, 10000);
}

static bool parseMspUrl(const String& url, String& host, uint16_t& port, String& path)
{
    if (!url.startsWith("http://")) {
        return false;
    }

    String work = url.substring(7);
    int slash = work.indexOf('/');

    String hostPort =
        slash >= 0
            ? work.substring(0, slash)
            : work;

    path =
        slash >= 0
            ? work.substring(slash)
            : "/";

    host = hostPort;
    port = 80;

    int colon = hostPort.lastIndexOf(':');

    if (colon >= 0) {
        host = hostPort.substring(0, colon);
        port =
            (uint16_t)hostPort
                .substring(colon + 1)
                .toInt();
    }

    return !host.isEmpty() && port > 0;
}

static String extractMspJsonString(const String& json, const String& key)
{
    String pattern = "\"" + key + "\"";
    int keyPos = json.indexOf(pattern);
    if (keyPos < 0) return "";

    int colonPos = json.indexOf(':', keyPos + pattern.length());
    if (colonPos < 0) return "";

    int firstQuote = json.indexOf('"', colonPos + 1);
    if (firstQuote < 0) return "";

    int secondQuote = json.indexOf('"', firstQuote + 1);
    if (secondQuote < 0) return "";

    return json.substring(firstQuote + 1, secondQuote);
}

static AbdOtaInfo checkMspUpdateWithHardware(Client& client)
{
    AbdOtaInfo info;
    info.currentVersion = getSafeMspCurrentVersion();

    String hardwareVersion = getSafeMspHardwareVersion();

    String path = "/firmware.json?id=";
    path += mqttDataValue.getESP_ID();
    path += "&ver=";
    path += info.currentVersion;
    path += "&hw=";
    path += hardwareVersion;
    path += "&key=";
    path += mqttDataValue.getOTA_KEY();

    Serial.print("[MSP OTA] manifestPath=");
    Serial.println(path);

    if (client.connected()) client.stop();

    if (!client.connect("85.105.100.7", 5050)) {
        Serial.println("[MSP OTA] manifest connect failed");
        return info;
    }

    client.print(
        String("GET ") + path + " HTTP/1.1\r\n" +
        "Host: 85.105.100.7\r\n" +
        "Connection: close\r\n\r\n"
    );

    uint32_t startedAt = millis();

    while (!client.available()) {
        if (!client.connected() || millis() - startedAt > 10000UL) {
            client.stop();
            Serial.println("[MSP OTA] manifest timeout");
            return info;
        }

        delay(5);
    }

    String statusLine = client.readStringUntil('\n');
    statusLine.trim();

    bool httpOk =
        statusLine.startsWith("HTTP/1.1 200") ||
        statusLine.startsWith("HTTP/1.0 200");

    if (!httpOk) {
        Serial.print("[MSP OTA] manifest HTTP=");
        Serial.println(statusLine);
        client.stop();
        return info;
    }

    while (client.connected() || client.available()) {
        String line = client.readStringUntil('\n');

        if (line == "\r" || line.length() == 0) {
            break;
        }
    }

    String body;
    uint32_t lastDataMs = millis();

    while (client.connected() || client.available()) {
        while (client.available()) {
            body += (char)client.read();
            lastDataMs = millis();
        }

        if (millis() - lastDataMs > 3000UL) break;
        delay(1);
    }

    client.stop();

    info.rawJson = body;

    String availableText = extractMspJsonString(body, "available");
    info.serverVersion = extractMspJsonString(body, "version");
    info.firmwareUrl = extractMspJsonString(body, "url");
    info.reportUrl = extractMspJsonString(body, "report_url");
    info.available = availableText == "true" || availableText == "1";

    return info;
}


static bool downloadMspTxtResume(Client& client, const String& url)
{
    if (!LittleFS.begin(true)) {
        return false;
    }

    uint32_t downloaded = 0;

    if (LittleFS.exists(MSP_LOCAL_FILE)) {
        File oldFile =
            LittleFS.open(MSP_LOCAL_FILE, "r");

        if (oldFile) {
            downloaded = oldFile.size();
            oldFile.close();
        }
    }

    for (uint8_t attempt = 0; attempt < 10; attempt++) {

        if (!recoverNetwork()) {
            delay(1000);
            continue;
        }

        String host;
        String path;
        uint16_t port = 80;

        if (!parseMspUrl(url, host, port, path)) {
            return false;
        }

        if (client.connected()) {
            client.stop();
        }

        if (!client.connect(host.c_str(), port)) {
            delay(1000);
            continue;
        }

        String request =
            String("GET ") + path + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Connection: close\r\n";

        if (downloaded > 0) {
            request +=
                "Range: bytes=" +
                String(downloaded) +
                "-\r\n";
        }

        request += "\r\n";
        client.print(request);

        uint32_t startedAt = millis();

        while (!client.available()) {
            if (!client.connected() || millis() - startedAt > 10000UL) {
                client.stop();
                break;
            }

            delay(10);
        }

        if (!client.available()) {
            delay(1000);
            continue;
        }

        String statusLine =
            client.readStringUntil('\n');

        statusLine.trim();

        bool partial =
            statusLine.startsWith("HTTP/1.1 206") ||
            statusLine.startsWith("HTTP/1.0 206");

        bool full =
            statusLine.startsWith("HTTP/1.1 200") ||
            statusLine.startsWith("HTTP/1.0 200");

        bool rangeComplete =
            statusLine.startsWith("HTTP/1.1 416") ||
            statusLine.startsWith("HTTP/1.0 416");

        int contentLength = -1;
        int totalSize = -1;

        while (client.connected()) {
            String line =
                client.readStringUntil('\n');

            line.trim();

            if (line.isEmpty()) {
                break;
            }

            String lower = line;
            lower.toLowerCase();

            if (lower.startsWith("content-length:")) {
                String value =
                    line.substring(line.indexOf(':') + 1);

                value.trim();
                contentLength = value.toInt();
            }

            if (lower.startsWith("content-range:")) {
                int slash = line.lastIndexOf('/');

                if (slash >= 0) {
                    totalSize =
                        line.substring(slash + 1)
                            .toInt();
                }
            }
        }

        if (rangeComplete) {
            client.stop();

            if (totalSize > 0 && downloaded == (uint32_t)totalSize) {

                File verify =
                    LittleFS.open(MSP_LOCAL_FILE, "r");

                size_t finalSize =
                    verify ? verify.size() : 0;

                if (verify) {
                    verify.close();
                }

                if (finalSize == (size_t)totalSize) {
                    Serial.printf("[MSP DOWNLOAD] existing file complete %u/%d\n", (unsigned int)finalSize, totalSize);

                    mspDownloadProgress(finalSize, totalSize);

                    return true;
                }
            }

            LittleFS.remove(MSP_LOCAL_FILE);
            downloaded = 0;
            delay(500);
            continue;
        }

        if (!partial && !full) {
            client.stop();
            delay(1000);
            continue;
        }

        if (full && downloaded > 0) {
            LittleFS.remove(MSP_LOCAL_FILE);
            downloaded = 0;
        }

        if (totalSize <= 0) {
            totalSize =
                downloaded +
                (contentLength > 0 ? contentLength : 0);
        }

        File file = LittleFS.open(MSP_LOCAL_FILE, downloaded > 0 ? "a" : "w");

        if (!file) {
            client.stop();
            return false;
        }

        uint8_t buffer[512];
        uint32_t lastDataMs = millis();

        while (downloaded < (uint32_t)totalSize) {

            int availableBytes =
                client.available();

            if (availableBytes > 0) {
                size_t count =
                    (size_t)availableBytes;

                if (count > sizeof(buffer)) {
                    count = sizeof(buffer);
                }

                int readCount =
                    client.read(buffer, count);

                if (readCount > 0) {
                    size_t saved =
                        file.write(buffer, readCount);

                    if (saved != (size_t)readCount) {
                        file.close();
                        client.stop();
                        return false;
                    }

                    downloaded += saved;
                    lastDataMs = millis();

                    mspDownloadProgress(downloaded, totalSize);
                }
            } else {
                if (!client.connected() || millis() - lastDataMs > 15000UL) {
                    break;
                }

                delay(2);
            }
        }

        file.flush();
        file.close();
        client.stop();

        if (downloaded == (uint32_t)totalSize) {
            return true;
        }

        delay(1000);
    }

    return false;
}

static bool sendMspReport(Client& client, const String& reportUrl, const String& status)
{
    if (reportUrl.isEmpty()) {
        return false;
    }

    String url = reportUrl;
    url +=
        url.indexOf('?') >= 0
            ? "&status="
            : "?status=";

    url += status;

    String host;
    String path;
    uint16_t port = 80;

    if (!parseMspUrl(url, host, port, path)) {
        return false;
    }

    if (client.connected()) {
        client.stop();
    }

    if (!client.connect(host.c_str(), port)) {
        if (!recoverNetwork() || !client.connect(host.c_str(), port)) {
            return false;
        }
    }

    client.print(String("GET ") + path + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    uint32_t startedAt = millis();

    while (!client.available()) {
        if (!client.connected() || millis() - startedAt > 10000UL) {
            client.stop();
            return false;
        }

        delay(5);
    }

    String statusLine =
        client.readStringUntil('\n');

    statusLine.trim();
    client.stop();

    return
        statusLine.startsWith("HTTP/1.1 200") ||
        statusLine.startsWith("HTTP/1.0 200");
}

static bool prepareMspDownload(AbdOtaInfo& info)
{
    mqttDataValue.setOTA_UPDATE_ACTIVE("69");

    String currentMspVersion = getSafeMspCurrentVersion();
    String currentMspHardware = getSafeMspHardwareVersion();

    Serial.print("[MSP OTA] currentVersion=");
    Serial.println(currentMspVersion);
    Serial.print("[MSP OTA] hardwareVersion=");
    Serial.println(currentMspHardware);



    if (!mspTcpHealthCheck()) {
        mqttDataValue.setOTA_UPDATE_ACTIVE("63");
        return false;
    }

    Client& client = otaSelectClient();

    info = checkMspUpdateWithHardware(client);

    Serial.println("----- MSP OTA INFO -----");
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
    Serial.println("------------------------");

    String expectedMspFile = "/msp_hw" + currentMspHardware + ".txt";

    if (!info.serverVersion.startsWith("msp@") ||
        info.reportUrl.indexOf("/msp_report") < 0 ||
        (info.available && info.firmwareUrl.indexOf(expectedMspFile) < 0)) {

        Serial.println("[MSP OTA] INVALID MANIFEST: ESP firmware returned");

        Serial.print("[MSP OTA] serverVersion=");
        Serial.println(info.serverVersion);

        Serial.print("[MSP OTA] firmwareUrl=");
        Serial.println(info.firmwareUrl);

        Serial.print("[MSP OTA] reportUrl=");
        Serial.println(info.reportUrl);

        mqttDataValue.setOTA_UPDATE_ACTIVE("63");
        return false;
    }

    if (!info.available) {
        sendMspReport(client, info.reportUrl, "msp_no_new_firmware");

        mqttDataValue.setOTA_UPDATE_ACTIVE("62");
        return true;
    }

    bool ok = downloadMspTxtResume(client, info.firmwareUrl);

    if (!ok) {
        sendMspReport(client, info.reportUrl, "msp_download_failed");

        mqttDataValue.setOTA_UPDATE_ACTIVE("63");
        return false;
    }

    Serial.println("[MSP OTA] DOWNLOAD SUCCESS");

    return true;
}

/* MSP UART */
#define MSP_SERIAL Serial2
#define MSP_RX_PIN 18
#define MSP_TX_PIN 17
#define MSP_BAUD   9600

/* MSP reset */
#define MSP_RESET_PIN 19
#define MSP_RESET_HOLD_MS 1000
#define MSP_AFTER_RESET_MS 150

/* Boot protokolü */
#define BOOT_H1      0xAA
#define BOOT_H2      0x55
#define CMD_ERASE    0x02
#define CMD_WRITE    0x03
#define CMD_DONE     0x04
#define RSP_ACK      0x80
#define RSP_NACK     0x81
#define MAX_DATA_LEN 16



enum class MspUpdatePhase : uint8_t {
    IDLE,
    STARTED,
    COUNTING,
    RESETTING,
    ERASING,
    WRITING,
    FINISHING,
    SUCCESS,
    FAILED
};

enum class MspUpdateError : uint8_t {
    NONE = 0,
    WIFI = 1,
    HTTP_CONNECT = 2,
    HTTP_RESPONSE = 3,
    TXT_FORMAT = 4,
    EMPTY_FIRMWARE = 5,
    MSP_ERASE = 6,
    MSP_WRITE = 7,
    MSP_DONE = 8,
    BYTE_COUNT = 9,
    HTTP_TIMEOUT = 10
};

struct MspWriteProgress {
    MspUpdatePhase phase = MspUpdatePhase::IDLE;
    MspUpdateError error = MspUpdateError::NONE;

    uint32_t totalBytes = 0;
    uint32_t writtenBytes = 0;
    uint32_t packetCount = 0;
    uint32_t currentAddress = 0;

    uint8_t percent = 0;
};


static MspWriteProgress mspProgressInfo;
static uint8_t mspLastReportedPercent = 255;
static MspUpdatePhase mspLastReportedPhase = MspUpdatePhase::IDLE;
static uint32_t mspLastReportMs = 0;


static const char *mspPhaseText(MspUpdatePhase phase)
{
    switch (phase) {
        case MspUpdatePhase::IDLE:      return "IDLE";
        case MspUpdatePhase::STARTED:   return "STARTED";
        case MspUpdatePhase::COUNTING:  return "COUNTING";
        case MspUpdatePhase::RESETTING: return "RESETTING";
        case MspUpdatePhase::ERASING:   return "ERASING";
        case MspUpdatePhase::WRITING:   return "WRITING";
        case MspUpdatePhase::FINISHING: return "FINISHING";
        case MspUpdatePhase::SUCCESS:   return "SUCCESS";
        case MspUpdatePhase::FAILED:    return "FAILED";
        default:                     return "UNKNOWN";
    }
}

static const char *mspNackReasonText(uint8_t reason)
{
    switch (reason) {
        case 0x10: return "invalid length";
        case 0x11: return "CRC error";
        case 0x12: return "address not allowed";
        case 0x13: return "flash erase/write/verify error";
        case 0x15: return "boot-state write error";
        case 0x16: return "update not started";
        case 0x17: return "application invalid";
        case 0x20: return "unknown command";
        case 0x21: return "parser overflow";
        default:   return "unknown reason";
    }
}




/* ===================== CRC16-CCITT ===================== */

static uint16_t crc16Update(uint16_t crc, uint8_t data)
{
    crc ^= (uint16_t)data << 8;

    for (uint8_t i = 0; i < 8; i++) {
        crc = (crc & 0x8000U)
                  ? (uint16_t)((crc << 1) ^ 0x1021U)
                  : (uint16_t)(crc << 1);
    }

    return crc;
}

static uint16_t crc16Calculate(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;

    for (size_t i = 0; i < length; i++) {
        crc = crc16Update(crc, data[i]);
    }

    return crc;
}

/* ===================== MSP UART ===================== */

static void clearMspUartRx()
{
    while (MSP_SERIAL.available()) {
        MSP_SERIAL.read();
    }
}

static bool waitBootResponse(uint8_t expectedCommand, uint32_t timeoutMs = 6000)
{
    uint8_t frame[8];
    uint8_t index = 0;
    uint32_t startedAt = millis();

    while ((uint32_t)(millis() - startedAt) < timeoutMs) {
        while (MSP_SERIAL.available()) {
            uint8_t value = (uint8_t)MSP_SERIAL.read();

            if (index == 0 && value != (uint8_t)'b') {
                continue;
            }

            frame[index++] = value;

            if (index == sizeof(frame)) {
                uint8_t checksum =
                    frame[0] ^ frame[1] ^ frame[2] ^
                    frame[3] ^ frame[4] ^ frame[5];

                if (expectedCommand != CMD_WRITE) {
                    Serial.print("[MSP RX] ");
                    for (uint8_t i = 0; i < sizeof(frame); i++) {
                        if (frame[i] < 0x10) Serial.print('0');
                        Serial.print(frame[i], HEX);
                        Serial.print(' ');
                    }
                    Serial.println();
                }

                if (frame[0] != (uint8_t)'b' || frame[7] != (uint8_t)'k' || frame[6] != checksum) {
                    index = 0;
                    continue;
                }

                if (frame[1] == RSP_ACK) {
                    if (frame[3] != expectedCommand) {
                        index = 0;
                        continue;
                    }
                    return true;
                }

                if (frame[1] == RSP_NACK) {
                    Serial.printf("[MSP] NACK 0x%02X: %s\n", frame[3], mspNackReasonText(frame[3]));
                    return false;
                }

                index = 0;
            }
        }

        delay(1);
    }

    Serial.printf("[MSP] ACK timeout cmd=0x%02X\n", expectedCommand);
    return false;
}

static bool sendBootPacket(uint8_t command, uint32_t address, const uint8_t *data, uint8_t length, uint8_t retryCount = 3)
{
    if (length > MAX_DATA_LEN) {
        return false;
    }

    uint8_t packet[10 + MAX_DATA_LEN];
    size_t packetLength = 10U + length;

    packet[0] = BOOT_H1;
    packet[1] = BOOT_H2;
    packet[2] = command;
    packet[3] = (uint8_t)(address >> 24);
    packet[4] = (uint8_t)(address >> 16);
    packet[5] = (uint8_t)(address >> 8);
    packet[6] = (uint8_t)address;
    packet[7] = length;

    for (uint8_t i = 0; i < length; i++) {
        packet[8 + i] = data[i];
    }

    uint16_t crc = crc16Calculate(packet, 8U + length);
    packet[8 + length] = (uint8_t)(crc >> 8);
    packet[9 + length] = (uint8_t)crc;

    for (uint8_t attempt = 1; attempt <= retryCount; attempt++) {
        clearMspUartRx();

        MSP_SERIAL.write(packet, packetLength);
        MSP_SERIAL.flush();

        if (waitBootResponse(command)) {
            return true;
        }

        Serial.printf("[MSP] retry %u/%u cmd=0x%02X addr=0x%05lX len=%u\n", attempt, retryCount, command, (unsigned long)address, length);

        delay(100);
    }

    return false;
}

static void hardResetMsp()
{
    pinMode(MSP_RESET_PIN, OUTPUT);


    digitalWrite(MSP_RESET_PIN, HIGH);
    delay(MSP_RESET_HOLD_MS);
    digitalWrite(MSP_RESET_PIN, LOW);
    delay(MSP_AFTER_RESET_MS);
    clearMspUartRx();
}



/* ===================== TI-TXT parser ===================== */

static int hexNibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool parseHexAddress(const String &text, uint32_t &address)
{
    if (text.isEmpty()) return false;

    uint32_t value = 0;

    for (size_t i = 0; i < text.length(); i++) {
        int nibble = hexNibble(text[i]);
        if (nibble < 0) return false;
        value = (value << 4) | (uint32_t)nibble;
    }

    address = value;
    return true;
}

static bool parseHexByte(const String &token, uint8_t &value)
{
    if (token.length() != 2) return false;

    int high = hexNibble(token[0]);
    int low = hexNibble(token[1]);

    if (high < 0 || low < 0) return false;

    value = (uint8_t)((high << 4) | low);
    return true;
}

/*
 * Tek bir TI-TXT satırındaki veri byte'larını sayar.
 * @adres ve q satırlarını saymaz.
 */
static bool countDataTokensInLine(String line, uint32_t &dataCount, bool &endReached)
{
    line.trim();

    if (line.isEmpty()) return true;

    if (line[0] == 'q' || line[0] == 'Q') {
        endReached = true;
        return true;
    }

    if (line[0] == '@') {
        uint32_t dummyAddress = 0;
        String addressText = line.substring(1);
        addressText.trim();
        return parseHexAddress(addressText, dummyAddress);
    }

    int position = 0;

    while (position < (int)line.length()) {
        while (position < (int)line.length() && (line[position] == ' ' || line[position] == '\t')) {
            position++;
        }

        if (position >= (int)line.length()) break;

        int start = position;
        while (position < (int)line.length() && line[position] != ' ' && line[position] != '\t') {
            position++;
        }

        String token = line.substring(start, position);
        uint8_t value = 0;

        if (!parseHexByte(token, value)) {
            Serial.print("[TI-TXT] invalid token while counting: ");
            Serial.println(token);
            return false;
        }

        dataCount++;
    }

    return true;
}

static bool countFirmwareDataBytes(uint32_t &dataCount)
{
    File file = LittleFS.open(MSP_LOCAL_FILE, "r");

    if (!file) {
        Serial.println("[TI-TXT] local file open fail");
        return false;
    }

    dataCount = 0;
    bool endReached = false;
    String line;

    while (file.available()) {
        char c = (char)file.read();

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            if (!countDataTokensInLine(line, dataCount, endReached)) {
                file.close();
                return false;
            }

            line = "";

            if (endReached) {
                file.close();
                return dataCount > 0;
            }
        } else {
            if (line.length() >= 512) {
                file.close();
                return false;
            }

            line += c;
        }
    }

    if (!line.isEmpty()) {
        if (!countDataTokensInLine(line, dataCount, endReached)) {
            file.close();
            return false;
        }
    }

    file.close();
    return dataCount > 0;
}

static void mspWriteProgressUpdate();
static bool waitMspNetworkReconnect(uint32_t timeoutMs = 120000UL);
static void processMspWriteScreenUpdate();
static volatile bool mspWriteWaitNetwork = false;
static volatile bool mspWriteNetworkFailed = false;

/* ===================== TI-TXT -> MSP yazma ===================== */


static bool flushDataBlock(uint32_t blockAddress, uint8_t *block, uint8_t &blockLength)
{
    if (blockLength == 0) return true;

    processMspWriteScreenUpdate();

    if (mspWriteNetworkFailed) return false;
    if (!waitMspNetworkReconnect()) return false;

    if (!sendBootPacket(CMD_WRITE, blockAddress, block, blockLength)) {
        Serial.printf("[TI-TXT] WRITE failed addr=0x%05lX len=%u\n", (unsigned long)blockAddress, blockLength);
        return false;
    }

    mspProgressInfo.writtenBytes += blockLength;
    mspProgressInfo.packetCount++;
    mspProgressInfo.currentAddress = blockAddress;
    mspProgressInfo.phase = MspUpdatePhase::WRITING;

    mspWriteProgressUpdate();

    if (mspWriteNetworkFailed) return false;

    blockLength = 0;
    return true;
}

static bool processTiTxtWriteLine(String line, bool &haveAddress, uint32_t &currentAddress, uint32_t &blockAddress, uint8_t *block, uint8_t &blockLength, bool &endReached)
{
    line.trim();

    if (line.isEmpty()) return true;

    if (line[0] == 'q' || line[0] == 'Q') {
        if (!flushDataBlock(blockAddress, block, blockLength)) {
            return false;
        }

        endReached = true;
        return true;
    }

    if (line[0] == '@') {
        if (!flushDataBlock(blockAddress, block, blockLength)) {
            return false;
        }

        String addressText = line.substring(1);
        addressText.trim();

        if (!parseHexAddress(addressText, currentAddress)) {
            Serial.print("[TI-TXT] invalid address: ");
            Serial.println(line);
            return false;
        }

        haveAddress = true;
        Serial.printf("[TI-TXT] address=0x%05lX\n", (unsigned long)currentAddress);
        return true;
    }

    if (!haveAddress) {
        Serial.println("[TI-TXT] data before @address");
        return false;
    }

    int position = 0;

    while (position < (int)line.length()) {
        while (position < (int)line.length() && (line[position] == ' ' || line[position] == '\t')) {
            position++;
        }

        if (position >= (int)line.length()) break;

        int start = position;

        while (position < (int)line.length() && line[position] != ' ' && line[position] != '\t') {
            position++;
        }

        String token = line.substring(start, position);
        uint8_t value = 0;

        if (!parseHexByte(token, value)) {
            Serial.print("[TI-TXT] invalid byte: ");
            Serial.println(token);
            return false;
        }

        if (blockLength == 0) {
            blockAddress = currentAddress;
        }

        if (currentAddress != blockAddress + blockLength) {
            if (!flushDataBlock(blockAddress, block, blockLength)) {
                return false;
            }
            blockAddress = currentAddress;
        }

        block[blockLength++] = value;
        currentAddress++;

        if (blockLength == MAX_DATA_LEN) {
            if (!flushDataBlock(blockAddress, block, blockLength)) {
                return false;
            }
        }
    }

    return true;
}

static bool streamFirmwareToMsp()
{
    File file = LittleFS.open(MSP_LOCAL_FILE, "r");

    if (!file) {
        Serial.println("[TI-TXT] local firmware open fail");
        return false;
    }

    bool haveAddress = false;
    bool endReached = false;
    uint32_t currentAddress = 0;
    uint32_t blockAddress = 0;
    uint8_t block[MAX_DATA_LEN];
    uint8_t blockLength = 0;
    String line;

    while (file.available()) {
        char c = (char)file.read();

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            if (!processTiTxtWriteLine(line, haveAddress, currentAddress, blockAddress, block, blockLength, endReached)) {
                file.close();
                return false;
            }

            line = "";

            if (endReached) {
                file.close();
                return true;
            }
        } else {
            if (line.length() >= 512) {
                file.close();
                return false;
            }

            line += c;
        }
    }

    if (!line.isEmpty()) {
        if (!processTiTxtWriteLine(line, haveAddress, currentAddress, blockAddress, block, blockLength, endReached)) {
            file.close();
            return false;
        }
    }

    bool ok =
        flushDataBlock(blockAddress, block, blockLength);

    file.close();

    return ok && mspProgressInfo.writtenBytes > 0;
}




#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#define MSP_STATUS_HOST "85.105.100.7"
#define MSP_STATUS_PORT 5050

struct MspWriteReportItem {
    uint8_t percent;
    uint32_t written;
    uint32_t total;
};

static QueueHandle_t mspWriteReportQueue = nullptr;
static TaskHandle_t mspWriteReportTaskHandle = nullptr;
static volatile bool mspWriteReportTaskRunning = false;
static volatile bool mspWriteReportStopRequested = false;
static int mspLastWritePercent = -1;
static int mspLastServerReportedPercent = -1;

static volatile int mspPendingScreenPercent = -1;
static volatile bool mspPendingScreenUpdate = false;

static void processMspWriteScreenUpdate()
{
    if (!mspPendingScreenUpdate) return;

    int percent = mspPendingScreenPercent;
    mspPendingScreenUpdate = false;

    write_Screen_WritingStatus(percent);
}

static uint8_t mspCalculateWritePercent(uint32_t written, uint32_t total)
{
    if (total == 0) return 0;
    uint64_t value = ((uint64_t)written * 100ULL) / total;
    if (value > 100ULL) value = 100ULL;
    return (uint8_t)value;
}

static inline Client& mspWriteReportClient()
{
    switch (netType) {
        case NET_TYPE_ALL:
            switch (comsupportedNetwork) {
                case NET_TYPE_GSM:  { static TinyGsmClient client(modem, 3); return client; }
                case NET_TYPE_WIFI: { static WiFiClient client; return client; }
                case NET_TYPE_ETH:  { static EthernetClient client; return client; }
                default: break;
            }
            break;

        case NET_TYPE_GSM:  { static TinyGsmClient client(modem, 3); return client; }
        case NET_TYPE_WIFI: { static WiFiClient client; return client; }
        case NET_TYPE_ETH:  { static EthernetClient client; return client; }
        default: break;
    }

    static WiFiClient fallbackClient;
    return fallbackClient;
}

static bool mspWriteUsesGsm()
{
    if (netType == NET_TYPE_GSM) return true;

    return netType == NET_TYPE_ALL &&
           comsupportedNetwork == NET_TYPE_GSM;
}

static bool waitMspNetworkReconnect(uint32_t timeoutMs)
{
    if (!mspWriteWaitNetwork) return true;

    Serial.println("[MSP WRITE] paused, waiting network");

    uint32_t startedAt = millis();

    while (mspWriteWaitNetwork) {
        if (recoverNetwork()) {
            Client& client = mspWriteReportClient();

            if (client.connected()) client.stop();

            if (client.connect(MSP_STATUS_HOST, MSP_STATUS_PORT)) {
                client.stop();
                mspWriteWaitNetwork = false;

                Serial.println("[MSP WRITE] network reconnected, continue");
                return true;
            }
        }

        if (millis() - startedAt >= timeoutMs) {
            Serial.println("[MSP WRITE] network reconnect timeout");
            mspWriteNetworkFailed = true;
            return false;
        }

        delay(1000);
        yield();
    }

    return true;
}

static bool sendMspWritePercent(uint8_t percent, uint32_t written, uint32_t total)
{
    if (mspWriteWaitNetwork) return false;

    Client& client = mspWriteReportClient();

    if (client.connected()) client.stop();

    if (!client.connect(MSP_STATUS_HOST, MSP_STATUS_PORT)) {
        client.stop();
        mspWriteWaitNetwork = true;
        Serial.println("[MSP REPORT] network lost, MSP write will pause");
        return false;
    }

    mspWriteWaitNetwork = false;

    String path = "/msp_status?id=";
    path += mqttDataValue.getESP_ID();
    path += "&progress=";
    path += String(percent);
    path += "&written=";
    path += String(written);
    path += "&total=";
    path += String(total);
    path += "&key=";
    path += mqttDataValue.getOTA_KEY();

    client.print(String("GET ") + path + " HTTP/1.1\r\n" + "Host: " + MSP_STATUS_HOST + "\r\n" + "Connection: close\r\n\r\n");

    write_Screen_WritingStatus(percent);

    uint32_t startedAt = millis();

    while (!client.available()) {
        if (!client.connected() || millis() - startedAt > 5000UL) {
            client.stop();
            mspWriteWaitNetwork = true;
            Serial.println("[MSP REPORT] connection lost, MSP write will pause");
            return false;
        }

        delay(2);
    }

    String statusLine = client.readStringUntil('\n');
    statusLine.trim();
    client.stop();

    return statusLine.startsWith("HTTP/1.1 200") ||
           statusLine.startsWith("HTTP/1.0 200");
}

static void mspWriteReportTask(void* parameter)
{
    mspWriteReportTaskRunning = true;
    MspWriteReportItem item;

    while (true) {
        if (xQueueReceive(mspWriteReportQueue, &item, pdMS_TO_TICKS(300)) == pdTRUE) {

            sendMspWritePercent(item.percent, item.written, item.total);

            continue;
        }

        if (mspWriteReportStopRequested && uxQueueMessagesWaiting(mspWriteReportQueue) == 0) {
            break;
        }
    }

    mspWriteReportTaskRunning = false;
    mspWriteReportTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

static bool startMspWriteReportTask()
{
    mspWriteReportTaskRunning = false;
    mspWriteReportStopRequested = false;
    return true;
}

static void queueMspWritePercent(uint8_t percent, uint32_t written, uint32_t total)
{
    (void)percent;
    (void)written;
    (void)total;
}

static void stopMspWriteReportTask()
{
    mspWriteReportStopRequested = true;
}

static void waitMspWriteReports(uint32_t timeoutMs = 120000UL)
{
    (void)timeoutMs;
    processMspWriteScreenUpdate();
}

static void mspWriteProgressUpdate()
{
    uint8_t percent = mspCalculateWritePercent(mspProgressInfo.writtenBytes, mspProgressInfo.totalBytes);

    mspProgressInfo.percent = percent;

    if (percent == mspLastWritePercent) return;

    mspLastWritePercent = percent;

    Serial.printf("[MSP WRITE] %u%% %lu/%lu\n", percent, (unsigned long)mspProgressInfo.writtenBytes, (unsigned long)mspProgressInfo.totalBytes);

    write_Screen_WritingStatus(percent);

    uint8_t reportPercent = percent;

    if (mspWriteUsesGsm()) {
        reportPercent = percent >= 100 ? 100 : (uint8_t)((percent / 10U) * 10U);

        if (reportPercent == 0 || reportPercent == mspLastServerReportedPercent) {
            return;
        }
    }

    if (reportPercent == mspLastServerReportedPercent) return;
    mspLastServerReportedPercent = reportPercent;

    while (!sendMspWritePercent(reportPercent, mspProgressInfo.writtenBytes, mspProgressInfo.totalBytes)) {
        if (!waitMspNetworkReconnect()) {
            Serial.println("[MSP REPORT] network reconnect failed");
            return;
        }

        delay(50);
        yield();
    }

}

static void mspDeleteFileAndRestart(const char* reason)
{
    Serial.print("[MSP OTA] ABORT: ");
    Serial.println(reason);

    stopMspWriteReportTask();
    waitMspWriteReports(5000);

    LittleFS.remove(MSP_LOCAL_FILE);

    mqttDataValue.setOTA_UPDATE_ACTIVE("63");

    Serial.println("[MSP OTA] /msp.txt deleted");
    Serial.println("[ESP] restart in 1500 ms");

    Serial.flush();
    delay(1500);
    ESP.restart();
}

static bool writeDownloadedMspToDevice()
{
    mspWriteWaitNetwork = false;
    mspWriteNetworkFailed = false;
    mspPendingScreenPercent = -1;
    mspPendingScreenUpdate = false;
    mspProgressInfo = MspWriteProgress{};
    mspLastWritePercent = -1;
    mspLastServerReportedPercent = -1;

    uint32_t firmwareDataBytes = 0;

    if (!countFirmwareDataBytes(firmwareDataBytes) || firmwareDataBytes == 0) {
        Serial.println("[MSP WRITE] invalid TI-TXT");
        return false;
    }

    mspProgressInfo.totalBytes = firmwareDataBytes;
    mspProgressInfo.writtenBytes = 0;
    mspProgressInfo.packetCount = 0;
    mspProgressInfo.currentAddress = 0;
    mspProgressInfo.percent = 0;

    Serial.printf("[TI-TXT] firmware data bytes=%lu\n", (unsigned long)firmwareDataBytes);

    write_Screen_WritingStatus(0);
    startMspWriteReportTask();

    MSP_SERIAL.end();
    delay(50);

    MSP_SERIAL.begin(MSP_BAUD, SERIAL_8N1, MSP_RX_PIN, MSP_TX_PIN);

    MSP_SERIAL.setTimeout(20);

    while (MSP_SERIAL.available()) {
        MSP_SERIAL.read();
    }

    hardResetMsp();

    if (!sendBootPacket(CMD_ERASE, 0, nullptr, 0)) {
        Serial.println("[MSP WRITE] ERASE FAIL");
        stopMspWriteReportTask();
        waitMspWriteReports();
        return false;
    }

    if (!streamFirmwareToMsp()) {
        Serial.println("[MSP WRITE] WRITE FAIL");
        stopMspWriteReportTask();
        waitMspWriteReports();
        return false;
    }

    if (mspProgressInfo.writtenBytes != mspProgressInfo.totalBytes) {
        Serial.printf("[MSP WRITE] byte mismatch %lu/%lu\n", (unsigned long)mspProgressInfo.writtenBytes, (unsigned long)mspProgressInfo.totalBytes);
        stopMspWriteReportTask();
        waitMspWriteReports();
        return false;
    }

    if (!sendBootPacket(CMD_DONE, 0, nullptr, 0)) {
        Serial.println("[MSP WRITE] DONE FAIL");
        stopMspWriteReportTask();
        waitMspWriteReports();
        return false;
    }

    mspProgressInfo.writtenBytes =
        mspProgressInfo.totalBytes;

    mspWriteProgressUpdate();

    if (mspWriteNetworkFailed) {
        Serial.println("[MSP WRITE] final progress report reconnect failed");
        stopMspWriteReportTask();
        waitMspWriteReports();
        return false;
    }

    stopMspWriteReportTask();
    waitMspWriteReports();
    processMspWriteScreenUpdate();

    return true;
}

bool startMspUpdate()
{
    if (LittleFS.begin(true)) {
        if (LittleFS.exists(MSP_LOCAL_FILE)) {
            LittleFS.remove(MSP_LOCAL_FILE);
            Serial.println("[MSP OTA] old msp.txt deleted");
        }
    } 
    
    AbdOtaInfo info;

    if (!prepareMspDownload(info)) {
        LittleFS.remove(MSP_LOCAL_FILE);
        mqttDataValue.setOTA_UPDATE_ACTIVE("63");

        Serial.println("[MSP OTA] DOWNLOAD/PREPARE FAIL");
        Serial.println("[MSP OTA] /msp.txt deleted");
        Serial.flush();
        delay(1500);
        ESP.restart();

        return false;
    }

    if (!info.available) {
        return true;
    }

    bool writeOk = writeDownloadedMspToDevice();

    Client& reportClient = otaSelectClient();

    if (!writeOk) {
        sendMspReport(reportClient, info.reportUrl, "msp_write_failed");

        mspDeleteFileAndRestart("MSP WRITE FAIL");

        return false;
    }

    bool finalReportOk = sendMspReport(reportClient, info.reportUrl, "msp_write_success");

    if (!finalReportOk) {
        Serial.println("[MSP REPORT] final report failed, recoverNetwork()");

        if (recoverNetwork()) {
            Client& retryClient = otaSelectClient();

            finalReportOk = sendMspReport(retryClient, info.reportUrl, "msp_write_success");
        }
    }

    LittleFS.remove(MSP_LOCAL_FILE);
    mqttDataValue.setOTA_UPDATE_ACTIVE("62");

    Serial.println("[MSP] UPDATE SUCCESS");
    Serial.println("[ESP] restart in 1500 ms");

    Serial.flush();
    delay(1500);
    ESP.restart();

    return true;
}

#endif



/*
 * Proje   : EVSE – MSP430 OTA Firmware Güncelleme Sistemi
 * Dosya   : mspBootloader.h
 * Amaç    : ESP32 üzerinden MSP430 firmware güncellemesi
 *           (donanım seçimi, resume destekli indirme ve UART bootloader yazma)
 *
 * Yazar   : Abdulhamit Mercan
 * Tarih   : 2026
 *
 * Özellikler:
 * - MSP yazılım sürümünü otomatik temizleme
 * - MSP donanım sürümünü otomatik algılama
 * - Donanıma göre doğru firmware dosyasını seçme
 * - HW v3.1 → msp_hw31.txt
 * - HW v2.1 → msp_hw21.txt
 * - Yeni donanımlar için dinamik HW sürüm desteği
 * - Resume destekli MSP firmware indirme
 * - WiFi / GSM / Ethernet destekli client seçimi
 * - TCP health check
 * - Network koparsa otomatik recover
 * - LittleFS üzerinde geçici firmware saklama
 * - TI-TXT formatını satır satır parse etme
 * - MSP430 bootloader üzerinden ERASE / WRITE / DONE işlemleri
 * - CRC16-CCITT paket doğrulama
 * - ACK / NACK kontrolü
 * - MSP yazma yüzdesini servera raporlama
 * - MSP yazma yüzdesini ekranda gösterme
 * - Hata durumunda firmware dosyasını silme
 * - Başarı veya hata sonunda ESP restart
 *
 * Akış:
 * 1. getSafeMspCurrentVersion()
 *    → MSP yazılım sürümü okunur ve temizlenir
 *
 * 2. getSafeMspHardwareVersion()
 *    → MSP donanım sürümü okunur
 *    → Örnek: "HW v3.1" değeri "31" formatına çevrilir
 *
 * 3. mspTcpHealthCheck()
 *    → OTA server bağlantısı test edilir
 *
 * 4. checkMspUpdateWithHardware()
 *    → ESP ID, MSP yazılım sürümü ve MSP donanım sürümü servera gönderilir
 *
 * 5. Server firmware seçimi
 *    → hw=31 ise msp_hw31.txt
 *    → hw=21 ise msp_hw21.txt
 *
 * 6. downloadMspTxtResume()
 *    → MSP firmware dosyası LittleFS'e indirilir
 *    → Bağlantı koparsa Range ile kaldığı yerden devam edilir
 *
 * 7. countFirmwareDataBytes()
 *    → TI-TXT içerisindeki gerçek firmware byte miktarı hesaplanır
 *
 * 8. hardResetMsp()
 *    → MSP resetlenir ve bootloader moduna alınır
 *
 * 9. sendBootPacket(CMD_ERASE)
 *    → MSP uygulama flash alanı silinir
 *
 * 10. streamFirmwareToMsp()
 *     → TI-TXT satırları parse edilir
 *     → Veriler 16 byte bloklar halinde MSP'ye yazılır
 *
 * 11. mspWriteProgressUpdate()
 *     → Yazma yüzdesi hesaplanır
 *     → Servera raporlanır
 *     → Ekrandaki slider güncellenir
 *
 * 12. sendBootPacket(CMD_DONE)
 *     → Firmware yazma işlemi tamamlanır
 *     → MSP uygulamasının geçerliliği doğrulanır
 *
 * 13. sendMspReport()
 *     → Başarı veya hata sonucu servera gönderilir
 *
 * 14. ESP.restart()
 *     → İşlem sonucu ne olursa olsun ESP kontrollü şekilde yeniden başlatılır
 *
 * Network Yönetimi:
 * - recoverNetwork():
 *      aktif bağlantıyı yeniden kurmaya çalışır
 *
 * - waitMspNetworkReconnect():
 *      MSP yazma sırasında internet koparsa en fazla 120 saniye bekler
 *
 * - Ağ 120 saniye içinde gelirse:
 *      → aynı yüzde tekrar servera gönderilir
 *      → MSP yazma kaldığı bloktan devam eder
 *
 * - Ağ 120 saniye içinde gelmezse:
 *      → yazma işlemi başarısız sayılır
 *      → hata üst fonksiyonlara taşınır
 *      → geçici firmware dosyası silinir
 *      → ESP yeniden başlatılır
 *
 * Client Seçimi:
 * - mspWriteReportClient():
 *      aktif network tipine göre doğru Client nesnesini döndürür
 *
 * Desteklenen Ağlar:
 * - WiFi
 * - GSM
 * - Ethernet
 *
 * Kritik Noktalar:
 * - MSP yazma yüzdesi ayrı FreeRTOS taskından gönderilmez
 * - GSM modemine aynı anda birden fazla task erişmez
 * - LVGL fonksiyonları güvenli task içerisinden çağrılır
 * - Yanlış donanıma ait MSP firmware dosyası kabul edilmez
 * - Manifest içindeki firmware URL'si donanım sürümüyle doğrulanır
 * - Eksik veya bozuk TI-TXT dosyası MSP'ye yazılmaz
 * - Her WRITE paketinden sonra MSP ACK cevabı beklenir
 * - ACK alınmazsa paket yeniden gönderilir
 * - Retry sayısı dolarsa güncelleme başarısız olur
 * - MSP erase sonrası oluşan herhangi bir hata kontrollü şekilde işlenir
 *
 * Firmware Dosyaları:
 * - HW v2.1 → msp_hw21.txt
 * - HW v3.1 → msp_hw31.txt
 *
 * Yeni Donanım Ekleme:
 * - Örnek HW v4.1:
 *      → ESP tarafından hw=41 gönderilir
 *      → firmware.json içine "41" kaydı eklenir
 *      → public klasörüne msp_hw41.txt eklenir
 *
 * Server Gereksinimleri:
 * - HTTP Range desteği
 * - 206 Partial Content desteği
 * - Donanıma göre firmware seçimi
 * - /msp_status endpoint'i
 * - /msp_report endpoint'i
 *
 * Notlar:
 * - MSP OTA başlamadan önce eski /msp.txt dosyası silinir
 * - Yeni firmware yoksa durum servera bildirilir
 * - Giriş durumunda OTA_UPDATE_ACTIVE = 69 yapılır
 * - Başarı durumunda OTA_UPDATE_ACTIVE = 62 yapılır
 * - Hata durumunda OTA_UPDATE_ACTIVE = 63 yapılır
 * - Güncelleme tamamlandıktan sonra geçici dosya silinir
 */
