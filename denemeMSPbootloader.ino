#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <Preferences.h>

/*
 * ESP32 -> MSP430F67751 TI-TXT updater
 *
 * Akış:
 *  1) NVS state REQUEST veya RUNNING ise güncelleme başlar.
 *  2) msp.txt ilk kez okunur ve gerçek firmware byte sayısı hesaplanır.
 *  3) MSP resetlenir, ERASE gönderilir.
 *  4) msp.txt ikinci kez açılır ve adresli WRITE paketleri gönderilir.
 *  5) Her yüzde değişiminde ilerleme sunucuya bildirilir.
 *  6) DONE başarılıysa NVS state SUCCESS yapılır.
 *
 * NVS durumları:
 *  IDLE     = 0x0000
 *  REQUEST  = 0xA55A
 *  RUNNING  = 0x61A1
 *  SUCCESS  = 0xD0E0
 *  FAILED   = 0xFA11
 */

/* ===================== Kullanıcı ayarları ===================== */

#define WIFI_SSID "TurkTelekom_TPA842_2.4GHz"
#define WIFI_PASS "z9v9eW4gzpL9"

#define HTTP_HOST "85.105.100.7"
#define HTTP_PORT 5050
#define HTTP_PATH "/msp.txt"

/*
 * Sunucuda ilerleme alacak endpoint.
 *
 * ESP şu JSON'u POST eder:
 * {
 *   "deviceId":"onc1-1",
 *   "state":"WRITING",
 *   "totalBytes":18838,
 *   "writtenBytes":9424,
 *   "progress":50,
 *   "packets":589,
 *   "address":"0x12340",
 *   "error":0
 * }
 *
 * Endpoint henüz yoksa 0 yap. İlerleme Serial'da yine görünür.
 */
#define ENABLE_HTTP_PROGRESS 1
#define PROGRESS_PATH "/msp-progress"
#define DEVICE_ID "onc1-1"

/* MSP UART */
#define MSP_SERIAL Serial2
#define MSP_RX_PIN 18
#define MSP_TX_PIN 17
#define MSP_BAUD   9600

/* MSP reset */
#define MSP_RESET_PIN 19
#define MSP_RESET_ACTIVE_HIGH 1
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

/* NVS */
#define NVS_NAMESPACE "msp_ota"
#define NVS_STATE_KEY "state"

static constexpr uint16_t MSP_STATE_IDLE    = 0x0000;
static constexpr uint16_t MSP_STATE_REQUEST = 0xA55A;
static constexpr uint16_t MSP_STATE_RUNNING = 0x61A1;
static constexpr uint16_t MSP_STATE_SUCCESS = 0xD0E0;
static constexpr uint16_t MSP_STATE_FAILED  = 0xFA11;

/* ===================== Durum ve hata tipleri ===================== */

enum class UpdatePhase : uint8_t {
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

enum class UpdateError : uint8_t {
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

struct MspProgress {
    UpdatePhase phase = UpdatePhase::IDLE;
    UpdateError error = UpdateError::NONE;

    uint32_t totalBytes = 0;
    uint32_t writtenBytes = 0;
    uint32_t packetCount = 0;
    uint32_t currentAddress = 0;

    uint8_t percent = 0;
};

Preferences preferences;
WiFiClient firmwareClient;

static MspProgress progressInfo;
static uint8_t lastReportedPercent = 255;
static UpdatePhase lastReportedPhase = UpdatePhase::IDLE;
static uint32_t lastReportMs = 0;

/* ===================== Yardımcı metinler ===================== */

static const char *phaseText(UpdatePhase phase)
{
    switch (phase) {
        case UpdatePhase::IDLE:      return "IDLE";
        case UpdatePhase::STARTED:   return "STARTED";
        case UpdatePhase::COUNTING:  return "COUNTING";
        case UpdatePhase::RESETTING: return "RESETTING";
        case UpdatePhase::ERASING:   return "ERASING";
        case UpdatePhase::WRITING:   return "WRITING";
        case UpdatePhase::FINISHING: return "FINISHING";
        case UpdatePhase::SUCCESS:   return "SUCCESS";
        case UpdatePhase::FAILED:    return "FAILED";
        default:                     return "UNKNOWN";
    }
}

static const char *nackReasonText(uint8_t reason)
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

/* ===================== NVS tetik sistemi ===================== */

static uint16_t readMspUpdateState()
{
    preferences.begin(NVS_NAMESPACE, true);
    uint16_t state = preferences.getUShort(NVS_STATE_KEY, MSP_STATE_IDLE);
    preferences.end();
    return state;
}

static void writeMspUpdateState(uint16_t state)
{
    preferences.begin(NVS_NAMESPACE, false);
    preferences.putUShort(NVS_STATE_KEY, state);
    preferences.end();

    Serial.printf("[NVS] MSP state=0x%04X\n", state);
}

/*
 * Ana projede MQTT komutu geldiğinde bu fonksiyonu çağır:
 *
 * requestMspUpdate();
 * ESP restart şart değil. loop içindeki serviceMspUpdate() çalıştırabilir.
 */
void requestMspUpdate()
{
    writeMspUpdateState(MSP_STATE_REQUEST);
}

/* ===================== İlerleme bildirimi ===================== */

static uint8_t calculatePercent(uint32_t written, uint32_t total)
{
    if (total == 0) return 0;

    uint64_t value = ((uint64_t)written * 100ULL) / total;
    if (value > 100ULL) value = 100ULL;
    return (uint8_t)value;
}

static void postProgressToServer(const MspProgress &info)
{
#if ENABLE_HTTP_PROGRESS
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    WiFiClient reportClient;
    HTTPClient http;

    String url = String("http://") + HTTP_HOST + ":" +
                 String(HTTP_PORT) + PROGRESS_PATH;

    if (!http.begin(reportClient, url)) {
        Serial.println("[PROGRESS] HTTP begin failed");
        return;
    }

    http.addHeader("Content-Type", "application/json");
    http.setTimeout(2500);

    char addressText[16];
    snprintf(addressText, sizeof(addressText),
             "0x%05lX", (unsigned long)info.currentAddress);

    String body;
    body.reserve(240);
    body += "{";
    body += "\"deviceId\":\"" + String(DEVICE_ID) + "\",";
    body += "\"state\":\"" + String(phaseText(info.phase)) + "\",";
    body += "\"totalBytes\":" + String(info.totalBytes) + ",";
    body += "\"writtenBytes\":" + String(info.writtenBytes) + ",";
    body += "\"progress\":" + String(info.percent) + ",";
    body += "\"packets\":" + String(info.packetCount) + ",";
    body += "\"address\":\"" + String(addressText) + "\",";
    body += "\"error\":" + String((uint8_t)info.error);
    body += "}";

    int statusCode = http.POST(body);

    Serial.printf("[PROGRESS] POST %d %s %u%% %lu/%lu\n",
                  statusCode,
                  phaseText(info.phase),
                  info.percent,
                  (unsigned long)info.writtenBytes,
                  (unsigned long)info.totalBytes);

    http.end();
#else
    (void)info;
#endif
}

static void reportProgress(bool force = false)
{
    progressInfo.percent =
        calculatePercent(progressInfo.writtenBytes,
                         progressInfo.totalBytes);

    bool phaseChanged = progressInfo.phase != lastReportedPhase;
    bool percentChanged = progressInfo.percent != lastReportedPercent;
    bool intervalPassed = (uint32_t)(millis() - lastReportMs) >= 1500U;

    if (!force && !phaseChanged && !percentChanged && !intervalPassed) {
        return;
    }

    Serial.printf(
        "[MSP OTA] state=%s progress=%u%% bytes=%lu/%lu packets=%lu addr=0x%05lX error=%u\n",
        phaseText(progressInfo.phase),
        progressInfo.percent,
        (unsigned long)progressInfo.writtenBytes,
        (unsigned long)progressInfo.totalBytes,
        (unsigned long)progressInfo.packetCount,
        (unsigned long)progressInfo.currentAddress,
        (unsigned int)progressInfo.error);

    postProgressToServer(progressInfo);

    lastReportedPhase = progressInfo.phase;
    lastReportedPercent = progressInfo.percent;
    lastReportMs = millis();
}

static void setFailure(UpdateError error)
{
    progressInfo.phase = UpdatePhase::FAILED;
    progressInfo.error = error;
    reportProgress(true);
    writeMspUpdateState(MSP_STATE_FAILED);
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

static bool waitBootResponse(uint8_t expectedCommand,
                             uint32_t timeoutMs = 6000)
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

                Serial.print("[MSP RX] ");
                for (uint8_t i = 0; i < sizeof(frame); i++) {
                    if (frame[i] < 0x10) Serial.print('0');
                    Serial.print(frame[i], HEX);
                    Serial.print(' ');
                }
                Serial.println();

                if (frame[0] != (uint8_t)'b' ||
                    frame[7] != (uint8_t)'k' ||
                    frame[6] != checksum) {
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
                    Serial.printf("[MSP] NACK 0x%02X: %s\n",
                                  frame[3],
                                  nackReasonText(frame[3]));
                    return false;
                }

                index = 0;
            }
        }

        delay(1);
    }

    Serial.printf("[MSP] ACK timeout cmd=0x%02X\n",
                  expectedCommand);
    return false;
}

static bool sendBootPacket(uint8_t command,
                           uint32_t address,
                           const uint8_t *data,
                           uint8_t length,
                           uint8_t retryCount = 3)
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

        Serial.printf(
            "[MSP] retry %u/%u cmd=0x%02X addr=0x%05lX len=%u\n",
            attempt,
            retryCount,
            command,
            (unsigned long)address,
            length);

        delay(100);
    }

    return false;
}

static void hardResetMsp()
{
    pinMode(MSP_RESET_PIN, OUTPUT);

#if MSP_RESET_ACTIVE_HIGH
    digitalWrite(MSP_RESET_PIN, HIGH);
    delay(MSP_RESET_HOLD_MS);
    digitalWrite(MSP_RESET_PIN, LOW);
#else
    digitalWrite(MSP_RESET_PIN, LOW);
    delay(MSP_RESET_HOLD_MS);
    digitalWrite(MSP_RESET_PIN, HIGH);
#endif

    delay(MSP_AFTER_RESET_MS);
    clearMspUartRx();
}

/* ===================== Wi-Fi ve HTTP ===================== */

static bool connectWiFi(uint32_t timeoutMs = 30000)
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("[WiFi] connecting");
    uint32_t startedAt = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if ((uint32_t)(millis() - startedAt) >= timeoutMs) {
            Serial.println(" timeout");
            return false;
        }

        Serial.print('.');
        delay(500);
    }

    Serial.println();
    Serial.print("[WiFi] IP=");
    Serial.println(WiFi.localIP());
    return true;
}

static bool openFirmwareHttp(WiFiClient &client,
                             int &contentLength)
{
    client.stop();

    if (!client.connect(HTTP_HOST, HTTP_PORT)) {
        Serial.println("[HTTP] connect failed");
        return false;
    }

    client.print(String("GET ") + HTTP_PATH + " HTTP/1.1\r\n" +
                 "Host: " + HTTP_HOST + "\r\n" +
                 "Connection: close\r\n" +
                 "Accept: text/plain\r\n\r\n");

    uint32_t startedAt = millis();

    while (!client.available()) {
        if ((uint32_t)(millis() - startedAt) > 10000U) {
            Serial.println("[HTTP] response timeout");
            client.stop();
            return false;
        }
        delay(10);
    }

    String status = client.readStringUntil('\n');
    status.trim();

    Serial.print("[HTTP] ");
    Serial.println(status);

    if (!status.startsWith("HTTP/1.1 200") &&
        !status.startsWith("HTTP/1.0 200")) {
        client.stop();
        return false;
    }

    contentLength = -1;

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        line.trim();

        if (line.isEmpty()) {
            break;
        }

        String lower = line;
        lower.toLowerCase();

        if (lower.startsWith("content-length:")) {
            String value = line.substring(line.indexOf(':') + 1);
            value.trim();
            contentLength = value.toInt();
        }

        if (lower.startsWith("transfer-encoding:") &&
            lower.indexOf("chunked") >= 0) {
            Serial.println("[HTTP] chunked transfer unsupported");
            client.stop();
            return false;
        }
    }

    Serial.printf("[HTTP] Content-Length=%d\n", contentLength);
    return true;
}

/* ===================== TI-TXT parser ===================== */

static int hexNibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool parseHexAddress(const String &text,
                            uint32_t &address)
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

static bool parseHexByte(const String &token,
                         uint8_t &value)
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
static bool countDataTokensInLine(String line,
                                  uint32_t &dataCount,
                                  bool &endReached)
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
        while (position < (int)line.length() &&
               (line[position] == ' ' || line[position] == '\t')) {
            position++;
        }

        if (position >= (int)line.length()) break;

        int start = position;
        while (position < (int)line.length() &&
               line[position] != ' ' &&
               line[position] != '\t') {
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
    WiFiClient countClient;
    int contentLength = -1;

    if (!openFirmwareHttp(countClient, contentLength)) {
        return false;
    }

    dataCount = 0;
    bool endReached = false;
    String line;
    uint32_t lastDataAt = millis();

    while (countClient.connected() || countClient.available()) {
        while (countClient.available()) {
            char c = (char)countClient.read();
            lastDataAt = millis();

            if (c == '\r') continue;

            if (c == '\n') {
                if (!countDataTokensInLine(line,
                                           dataCount,
                                           endReached)) {
                    countClient.stop();
                    return false;
                }

                line = "";

                if (endReached) {
                    countClient.stop();
                    return dataCount > 0;
                }
            } else {
                if (line.length() >= 512) {
                    countClient.stop();
                    return false;
                }
                line += c;
            }
        }

        if ((uint32_t)(millis() - lastDataAt) > 15000U) {
            countClient.stop();
            return false;
        }

        delay(1);
    }

    if (!line.isEmpty()) {
        if (!countDataTokensInLine(line,
                                   dataCount,
                                   endReached)) {
            return false;
        }
    }

    countClient.stop();
    return dataCount > 0;
}

/* ===================== TI-TXT -> MSP yazma ===================== */

static bool flushDataBlock(uint32_t blockAddress,
                           uint8_t *block,
                           uint8_t &blockLength)
{
    if (blockLength == 0) return true;

    if (!sendBootPacket(CMD_WRITE,
                        blockAddress,
                        block,
                        blockLength)) {
        Serial.printf(
            "[TI-TXT] WRITE failed addr=0x%05lX len=%u\n",
            (unsigned long)blockAddress,
            blockLength);
        return false;
    }

    progressInfo.writtenBytes += blockLength;
    progressInfo.packetCount++;
    progressInfo.currentAddress = blockAddress;
    progressInfo.phase = UpdatePhase::WRITING;

    reportProgress();

    blockLength = 0;
    return true;
}

static bool processTiTxtWriteLine(String line,
                                  bool &haveAddress,
                                  uint32_t &currentAddress,
                                  uint32_t &blockAddress,
                                  uint8_t *block,
                                  uint8_t &blockLength,
                                  bool &endReached)
{
    line.trim();

    if (line.isEmpty()) return true;

    if (line[0] == 'q' || line[0] == 'Q') {
        if (!flushDataBlock(blockAddress,
                            block,
                            blockLength)) {
            return false;
        }

        endReached = true;
        return true;
    }

    if (line[0] == '@') {
        if (!flushDataBlock(blockAddress,
                            block,
                            blockLength)) {
            return false;
        }

        String addressText = line.substring(1);
        addressText.trim();

        if (!parseHexAddress(addressText,
                             currentAddress)) {
            Serial.print("[TI-TXT] invalid address: ");
            Serial.println(line);
            return false;
        }

        haveAddress = true;
        Serial.printf("[TI-TXT] address=0x%05lX\n",
                      (unsigned long)currentAddress);
        return true;
    }

    if (!haveAddress) {
        Serial.println("[TI-TXT] data before @address");
        return false;
    }

    int position = 0;

    while (position < (int)line.length()) {
        while (position < (int)line.length() &&
               (line[position] == ' ' ||
                line[position] == '\t')) {
            position++;
        }

        if (position >= (int)line.length()) break;

        int start = position;

        while (position < (int)line.length() &&
               line[position] != ' ' &&
               line[position] != '\t') {
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
            if (!flushDataBlock(blockAddress,
                                block,
                                blockLength)) {
                return false;
            }
            blockAddress = currentAddress;
        }

        block[blockLength++] = value;
        currentAddress++;

        if (blockLength == MAX_DATA_LEN) {
            if (!flushDataBlock(blockAddress,
                                block,
                                blockLength)) {
                return false;
            }
        }
    }

    return true;
}

static bool streamFirmwareToMsp()
{
    int contentLength = -1;

    if (!openFirmwareHttp(firmwareClient,
                          contentLength)) {
        return false;
    }

    bool haveAddress = false;
    bool endReached = false;
    uint32_t currentAddress = 0;
    uint32_t blockAddress = 0;
    uint8_t block[MAX_DATA_LEN];
    uint8_t blockLength = 0;
    String line;
    uint32_t lastDataAt = millis();

    while (firmwareClient.connected() ||
           firmwareClient.available()) {
        while (firmwareClient.available()) {
            char c = (char)firmwareClient.read();
            lastDataAt = millis();

            if (c == '\r') continue;

            if (c == '\n') {
                if (!processTiTxtWriteLine(
                        line,
                        haveAddress,
                        currentAddress,
                        blockAddress,
                        block,
                        blockLength,
                        endReached)) {
                    firmwareClient.stop();
                    return false;
                }

                line = "";

                if (endReached) {
                    firmwareClient.stop();
                    return true;
                }
            } else {
                if (line.length() >= 512) {
                    firmwareClient.stop();
                    return false;
                }
                line += c;
            }
        }

        if ((uint32_t)(millis() - lastDataAt) > 15000U) {
            Serial.println("[HTTP] firmware body timeout");
            firmwareClient.stop();
            return false;
        }

        delay(1);
    }

    if (!line.isEmpty()) {
        if (!processTiTxtWriteLine(
                line,
                haveAddress,
                currentAddress,
                blockAddress,
                block,
                blockLength,
                endReached)) {
            return false;
        }
    }

    if (!flushDataBlock(blockAddress,
                        block,
                        blockLength)) {
        return false;
    }

    firmwareClient.stop();
    return progressInfo.writtenBytes > 0;
}

/* ===================== Tam güncelleme akışı ===================== */

static bool runMspUpdate()
{
    progressInfo = MspProgress{};
    lastReportedPercent = 255;
    lastReportedPhase = UpdatePhase::IDLE;
    lastReportMs = 0;

    progressInfo.phase = UpdatePhase::STARTED;
    reportProgress(true);

    writeMspUpdateState(MSP_STATE_RUNNING);

    if (!connectWiFi()) {
        setFailure(UpdateError::WIFI);
        return false;
    }

    /*
     * Birinci HTTP geçişi:
     * ASCII dosya boyutunu değil, MSP'ye gerçekten yazılacak
     * binary byte sayısını hesaplar.
     */
    progressInfo.phase = UpdatePhase::COUNTING;
    reportProgress(true);

    uint32_t firmwareDataBytes = 0;

    if (!countFirmwareDataBytes(firmwareDataBytes)) {
        setFailure(UpdateError::TXT_FORMAT);
        return false;
    }

    if (firmwareDataBytes == 0) {
        setFailure(UpdateError::EMPTY_FIRMWARE);
        return false;
    }

    progressInfo.totalBytes = firmwareDataBytes;
    progressInfo.writtenBytes = 0;
    progressInfo.packetCount = 0;
    progressInfo.currentAddress = 0;
    progressInfo.percent = 0;

    Serial.printf("[TI-TXT] firmware data bytes=%lu\n",
                  (unsigned long)firmwareDataBytes);

    progressInfo.phase = UpdatePhase::RESETTING;
    reportProgress(true);

    /*
     * İkinci HTTP bağlantısından hemen önce MSP resetlenir.
     * Bootloader'ın UART bekleme penceresi içinde ERASE gider.
     */
    hardResetMsp();

    progressInfo.phase = UpdatePhase::ERASING;
    reportProgress(true);

    if (!sendBootPacket(CMD_ERASE,
                        0,
                        nullptr,
                        0)) {
        setFailure(UpdateError::MSP_ERASE);
        return false;
    }

    progressInfo.phase = UpdatePhase::WRITING;
    reportProgress(true);

    if (!streamFirmwareToMsp()) {
        setFailure(UpdateError::MSP_WRITE);
        return false;
    }

    if (progressInfo.writtenBytes !=
        progressInfo.totalBytes) {
        Serial.printf(
            "[TI-TXT] byte mismatch written=%lu expected=%lu\n",
            (unsigned long)progressInfo.writtenBytes,
            (unsigned long)progressInfo.totalBytes);

        setFailure(UpdateError::BYTE_COUNT);
        return false;
    }

    progressInfo.phase = UpdatePhase::FINISHING;
    reportProgress(true);

    if (!sendBootPacket(CMD_DONE,
                        0,
                        nullptr,
                        0)) {
        setFailure(UpdateError::MSP_DONE);
        return false;
    }

    progressInfo.phase = UpdatePhase::SUCCESS;
    progressInfo.error = UpdateError::NONE;
    progressInfo.writtenBytes = progressInfo.totalBytes;
    progressInfo.percent = 100;

    writeMspUpdateState(MSP_STATE_SUCCESS);
    reportProgress(true);

    Serial.println("[MSP] UPDATE SUCCESS");
    return true;
}

/*
 * Ana program loop() içinde çağrılabilir.
 *
 * REQUEST:
 *   Yeni güncelleme komutu.
 *
 * RUNNING:
 *   ESP önceki güncelleme sırasında resetlenmiş olabilir.
 *   Güvenli şekilde dosyanın tamamını baştan yükler.
 *
 * SUCCESS / FAILED / IDLE:
 *   Güncelleme başlatmaz.
 */
static void serviceMspUpdate()
{
    static bool checkedThisBoot = false;
    static bool updateActive = false;

    if (updateActive) return;

    uint16_t state = readMspUpdateState();

    bool shouldStart =
        state == MSP_STATE_REQUEST ||
        state == MSP_STATE_RUNNING;

    /*
     * Bu bağımsız test sürümünde NVS yalnızca bir kez kontrol edilir.
     * Ana projede MQTT komutunu aldıktan sonra requestMspUpdate()
     * çağırıp checkedThisBoot kontrolünü kaldırabilirsin.
     */
    if (checkedThisBoot && !shouldStart) {
        return;
    }

    checkedThisBoot = true;

    if (!shouldStart) {
        Serial.printf(
            "[MSP OTA] no request, state=0x%04X\n",
            state);
        return;
    }

    updateActive = true;
    bool success = runMspUpdate();
    updateActive = false;

    Serial.println(success
        ? "UPDATE RESULT: SUCCESS"
        : "UPDATE RESULT: FAIL");
}

/* ===================== Arduino ===================== */

void setup()
{
    Serial.begin(115200);
    delay(500);

    MSP_SERIAL.begin(MSP_BAUD,
                     SERIAL_8N1,
                     MSP_RX_PIN,
                     MSP_TX_PIN);

    MSP_SERIAL.setTimeout(20);

    Serial.println();
    Serial.println("ESP32 -> MSP430 TI-TXT updater with progress");

    /*
     * İlk manuel test için aşağıdaki satırı BİR KEZ aç:
     *
     * requestMspUpdate();
     *
     * Başarılı güncellemeden sonra tekrar yorum satırı yap.
     * Ana projede bunun yerine MQTT tetikleyicisi çağıracak.
     */

    serviceMspUpdate();
}

void loop()
{
    /*
     * Ana projede MQTT tetiklerinin çalışması için bunu loop içinde tut.
     * Bu test sürümünde NVS state değişirse de güncellemeyi başlatabilir.
     */
    serviceMspUpdate();
    delay(100);
}