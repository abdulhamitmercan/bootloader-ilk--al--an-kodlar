#ifndef BROKER_H
#define BROKER_H

#include <PubSubClient.h>
#include "board_network.h"


#define SerialMon Serial
#define SerialAT Serial1

#ifdef ESP_ID
#undef ESP_ID
#endif



class MqttFrameHandler {
private:
    uint8_t cmdType;
    uint8_t msgType;
    char data[257]; // 256 + '\0'

public:
    MqttFrameHandler() { clear(); }

    void clear() {
        cmdType = 0;
        msgType = 0;
        memset(data, 0, sizeof(data));
    }

    void setAll(uint8_t cmd, uint8_t msg, const char* d) {
    setCmdType(cmd);
    setMsgType(msg);
    setData(d);
}
    // ---- Getter'lar ----
    uint8_t     getCmdType() const { return cmdType; }
    uint8_t     getMsgType() const { return msgType; }
    const char* getData()    const { return data; }

    // ---- Setter'lar (ayrı ayrı) ----
    void setCmdType(uint8_t v) { cmdType = v; }
    void setMsgType(uint8_t v) { msgType = v; }
    void setData(const char* d) {
        if (!d) { data[0] = '\0'; return; }
        strncpy(data, d, 256);
        data[256] = '\0';
    }
    //binary veri için
    void setDataBytes(const uint8_t* b, size_t n) {
        if (!b) { data[0] = '\0'; return; }
        if (n > 256) n = 256;
        memcpy(data, b, n);
        data[n] = '\0';
    }

    // ---- Encode / Decode ----
    int encode(char* buffer, size_t bufSize) const {
        size_t dLen = strlen(data);
        if (dLen + 4 > bufSize) return -1;
        int idx = 0;
        buffer[idx++] = 'c';
        buffer[idx++] = cmdType;
        buffer[idx++] = msgType;
        memcpy(&buffer[idx], data, dLen);
        idx += (int)dLen;
        buffer[idx++] = 'v';
        buffer[idx] = '\0';
        return idx;
    }

    bool decode(const uint8_t* payload, unsigned int length) {
        if (!payload || length < 4) return false;
        if (payload[0] != 'c' || payload[length - 1] != 'v') return false;
        cmdType = payload[1];
        msgType = payload[2];
        int dLen = (int)length - 4;
        if (dLen > 256) dLen = 256;
        memcpy(data, &payload[3], dLen);
        data[dLen] = '\0';
        return true;
    }

};

#include "mqttProtocol.h"

// MQTT Broker
const char* broker = "85.105.100.7";
const int   port   = 1883;
const char* mqttUser = "abdulhamit";
const char* mqttPass = "q12345";


/*
#if (netType == NET_TYPE_GSM)
// MQTT client → socket 1
TinyGsmClient mqttClient(modem, 1);
PubSubClient mqtt(mqttClient);


#elif (netType == NET_TYPE_WIFI)
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);


#elif (netType == NET_TYPE_ETH)
EthernetClient ethernetClient;  
PubSubClient mqtt(ethernetClient);
#endif
TinyGsmClient gsmClient(modem, 1);
WiFiClient    wifiClient;
EthernetClient ethernetClient;
*/



// MQTT nesnesi 
PubSubClient mqtt;

void mqttSelectClient() {
  switch (netType) {
  case NET_TYPE_ALL: {
    switch (comsupportedNetwork) {
      case NET_TYPE_GSM:  mqtt.setClient(gsmClient);     break;
      case NET_TYPE_WIFI: mqtt.setClient(wifiClient);    break;
      case NET_TYPE_ETH:  mqtt.setClient(ethernetClient);break;
      default:             break;
      }
  break; 
  }   

  case NET_TYPE_GSM:  mqtt.setClient(gsmClient);      break;
  case NET_TYPE_WIFI: mqtt.setClient(wifiClient);     break;
  case NET_TYPE_ETH:  mqtt.setClient(ethernetClient); break;
  default: break;
}

}
bool activeMode = false;


MqttFrameHandler sendFrame;
MqttFrameHandler receiveFrame;

#define TOPIC_MAX_LEN 64
char topicControl[TOPIC_MAX_LEN];
char topicFromServer[TOPIC_MAX_LEN];
char topicToServer[TOPIC_MAX_LEN];
char topicStatus[TOPIC_MAX_LEN];

void appendStringa(char *dest, const char *src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}

void buildTopics() {
  char *p;  
   const char* id = mqttDataValue.getESP_ID();
   
p = topicControl;    *p = 0; appendStringa(p, id); appendStringa(p, "/control");
p = topicFromServer; *p = 0; appendStringa(p, id); appendStringa(p, "/from_server");
p = topicToServer;   *p = 0; appendStringa(p, id); appendStringa(p, "/to_server");
p = topicStatus;     *p = 0; appendStringa(p, id); appendStringa(p, "/status");
}

// MQTT callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    mqttSelectClient();
    String message;
    for (int i = 0; i < length; i++) message += (char)payload[i];

    if (strcmp(topic, topicControl) == 0) {
        if (message == "activate") {
            activeMode = true;
            mqtt.subscribe(topicFromServer);
            mqtt.publish(topicStatus, "ESP aktif");
            return;
        } else if (message == "deactivate") {
            activeMode = false;
            mqtt.unsubscribe(topicFromServer);
            mqtt.publish(topicStatus, "ESP pasif");
            return;
        }
    }

    if (activeMode && strcmp(topic, topicFromServer) == 0) {
        handleMqttProtocol(payload, length, receiveFrame, sendFrame, mqtt, topicToServer);
    }
}

// MQTT reconnect
void mqttReconnect() {
    mqttSelectClient();
    while (!mqtt.connected()) {
        if (mqtt.connect(mqttDataValue.getESP_ID(), mqttUser, mqttPass)) {
            mqtt.subscribe(topicControl);
            mqtt.publish(topicStatus, "ESP MQTT bağlı");
        } else {
            delay(5000);
        }
    }
}
unsigned long lastMqttAttemptMs = 0;
const unsigned long mqttRetryMs = 5000;

void mqttReconnectNonBlocking() {
    mqttSelectClient();
  if (mqtt.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttAttemptMs < mqttRetryMs) return; // bekleme

  lastMqttAttemptMs = now;

 // SerialMon.print("[MQTT] Yeniden bağlanıyor...");
  // İsteğe bağlı: temiz oturum kullanma -> abonelikler broker tarafında kalabilir
  // PubSubClient'ta cleanSession'ı false vermek için farklı overload yoksa default'u kullan.
  if (mqtt.connect(mqttDataValue.getESP_ID(), mqttUser, mqttPass))  {
   // SerialMon.println("OK");
    // Zorunlu abonelik
    mqtt.subscribe(topicControl);

    // Aktif mod açıksa, dinlemeyi geri getir
    if (activeMode) {
      mqtt.subscribe(topicFromServer);
    }

    mqtt.publish(topicStatus, "ESP MQTT yeniden bağlandı");
  } else {
    //SerialMon.print("Hata, rc=");
    //SerialMon.println(mqtt.state());
    // Sıradaki denemeyi bekle (return ile çıkıyoruz)
  } 
}
bool isMqttTcpAlive() {
  mqttSelectClient();

  if (!mqtt.connected()) return false;

  // TCP kontrolünü opsiyonel yap
  switch (netType) {
    case NET_TYPE_GSM:
      if (gsmClient.connected()) return true;
      break;

    case NET_TYPE_WIFI:
      if (wifiClient.connected()) return true;
      break;

    case NET_TYPE_ETH:
      if (ethernetClient.connected()) return true;
      break;

    case NET_TYPE_ALL:
      if (comsupportedNetwork == NET_TYPE_GSM && gsmClient.connected()) return true;
      if (comsupportedNetwork == NET_TYPE_WIFI && wifiClient.connected()) return true;
      if (comsupportedNetwork == NET_TYPE_ETH && ethernetClient.connected()) return true;
      break;
  }

  // fallback → MQTT bağlıysa yine true kabul et
  return true;
}
void mqtt_init()
{
 mqttSelectClient();
  buildTopics();
  mqtt.setServer(broker, port);
  mqtt.setCallback(mqttCallback);
  mqtt.setKeepAlive(60);
  mqtt.setSocketTimeout(10); 
 
}

#endif 


/* 
 * Proje   : EVSE – MQTT / OCPP Haberleşme
 * Dosya   : broker.h
 * Amaç    : MQTT broker bağlantısını yönetmek, topic’leri kurmak ve gelen
 *           çerçeveleri protokol katmanına (mqttProtocol.h) yönlendirmek.
 *
 * Yazar   : Abdulhamit Mercan
 * Tarih   : Ağustos 2025
 *
 * Özet    :
 * - TinyGSM üzerinden modem (gsm_api.h) ile soket açar, PubSubClient ile MQTT bağlanır.
 * - Topic şeması: <ESP_ID>/control | /from_server | /to_server | /status
 * - mqttCallback(): control komutları (activate/deactivate) ve protokol trafiğini işler.
 * - mqttReconnect(): bağlantı koparsa yeniden bağlanmayı dener.
 *
 * Bağımlılıklar:
 * - gsm_api.h (modem nesnesi / TinyGSM konfigürasyonu)
 * - PubSubClient.h
 * - mqttProtocol.h (çerçeve işleme ve DataValue erişimi)
 *
 * Notlar:
 * - ESP_ID benzersiz olmalıdır (default kullanma).
 * - MqttFrameHandler bu dosyada tanımlıdır; encode/decode formatı: 'c' CMD MSG DATA 'v'
 * - Bu dosya kalıcı ayar yazmaz; sadece broker bağlantısı ve yönlendirme yapar.
 */

