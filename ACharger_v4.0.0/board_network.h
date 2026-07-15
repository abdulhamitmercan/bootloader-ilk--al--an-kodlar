#ifndef board_network_H
#define board_network_H

#include <Arduino.h>
#include <MicroOcpp.h>
#include <TinyGsmClient.h>
//#include <sslbrk.h>
#define NET_TYPE_ALL   0
#define NET_TYPE_GSM   1
#define NET_TYPE_WIFI  2
#define NET_TYPE_ETH   3


extern const char* WrapperMqttGetWIFI_SSID(void);
extern const char* WrapperMqttGetWIFI_PASS(void);
extern const char* WrapperMqttGetCONNECTION_TYPE(void);
extern const char* WrapperMqttGetESP_ID(void);
extern void WrapperMqttSetCOMMSUPPORT_NETWORK(const char* v);
extern void WrapperMqttSetSCAN_LIST(const char* v);

uint16_t netType = NET_TYPE_GSM;// DEFAULT 
uint16_t comsupportedNetwork = NET_TYPE_GSM;// DEFAULT 

/*----------- GSM parametreleri --------------*/
const char apn[]      = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

static bool wifiReconnectInProgress = false;
static unsigned long wifiReconnectStartMs = 0;
static uint8_t wifiReconnectAttempt = 0;
static unsigned long lastDotTime = 0;

const unsigned long WIFI_RECONNECT_TIMEOUT = 20000UL; // 20 sn
const uint8_t WIFI_RECONNECT_MAX_ATTEMPT = 10;
#define MODEM_RESET_PIN 19

uint16_t GPRS_RETRY_LIMIT = 30;

int gprsRetryCount = 0;
bool gprsConnected = false;

#define MODEM_RST 5
#define SerialAT  Serial1

TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem, 1); //broker
//TinyGsmClient gsm_transport_layer(modem);
  
WiFiClient    wifiClient;
EthernetClient ethernetClient;
/*----------- ➊ Wi-Fi tarafı (yeni) ----------*/
      

//#ifdef USE_WIFI_NET  
  /* default SSID / Şifreyi isterseniz burada değiştirin */
  #ifndef WIFI_SSID
    #define WIFI_SSID "BLUETECH"
   // #define WIFI_SSID "OTOPARK"
  #endif
  #ifndef WIFI_PASS
    #define WIFI_PASS "Bluetechev"
   // #define WIFI_PASS "20262026"
  #endif
  char WIFI_SSid[100] = {0};
  char WIFI_Pass[100] = {0};
 //yardımcılar  
 static int globalValue = 0;

int setwifiOnce(void)
{
    static bool initialized = false;

    if (!initialized) {
        globalValue = 2;
        initialized = true;
    }

    return globalValue;
}

void assignUint16_network(const char* s, uint16_t* dst) {
    if (!s || !dst) return;

    while (*s && isspace((unsigned char)*s)) ++s;

    errno = 0;
    char* end = NULL;
    unsigned long v = strtoul(s, &end, 0); 

    if (end != s && errno != ERANGE && v <= 65535UL) {
        *dst = (uint16_t)v;
    }
}



/*----------- wifi başlatma  ------*/
static inline bool wifi_init(const char* ssid, const char* pass) {
  Serial.println("[Wi-Fi] INIT START");

  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  WiFi.mode(WIFI_STA);

  WiFi.disconnect(false, false);
  delay(1000);

  Serial.println("[Wi-Fi] Scan basliyor...");

  int n = WiFi.scanNetworks();

  Serial.print("[SCAN DEBUG] n=");
  Serial.println(n);

  String foundList = "";
  bool dynamicFound = false;
  bool defaultFound = false;

  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    if (foundList.indexOf(s) == -1) {
      if (foundList.length() > 0) foundList += ",";
      foundList += s;
      foundList += "(" + String(rssi) + ")";
    }

    if (s == ssid) dynamicFound = true;
    if (s == WIFI_SSID) defaultFound = true;
  }

  Serial.print("[Wi-Fi] Bulunan aglar: ");
  Serial.println(foundList);

  const size_t MAX_LEN = 2047; // \0 için 1 byte bırak
  if (foundList.length() > MAX_LEN) foundList = foundList.substring(0, MAX_LEN);
  WrapperMqttSetSCAN_LIST(foundList.c_str());

  WiFi.scanDelete();

  if (dynamicFound) {
    Serial.print("[Wi-Fi] Dinamik bulundu -> ");
    Serial.println(ssid);

    WiFi.disconnect(false, false);
    delay(2000);

    WiFi.begin(ssid, pass);

    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
      Serial.print(".");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
      Serial.println(" OK");
      Serial.print("[Wi-Fi] IP: ");
      Serial.println(WiFi.localIP());
      Serial.print(F("\nMAC: ")); 
      Serial.println(WrapperMqttGetESP_ID());
      return true;
    }

    Serial.println(" FAIL");
  }

  if (strcmp(ssid, WIFI_SSID) != 0 && defaultFound) {
    Serial.print("[Wi-Fi] Default bulundu -> ");
    Serial.println(WIFI_SSID);

    WiFi.disconnect(false, false);
    delay(2000);

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
      Serial.print(".");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
      Serial.println(" OK");
      Serial.print("[Wi-Fi] IP: ");
      Serial.println(WiFi.localIP());
      Serial.print(F("\nMAC: ")); 
      Serial.println(WrapperMqttGetESP_ID());
      return true;
    }

    Serial.println(" FAIL");
  }

  Serial.println("[Wi-Fi] Scan ile bulunamadi, direkt baglanma deneniyor");

  WiFi.disconnect(false, false);
  delay(1000);
  WiFi.begin(ssid, pass);

  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
    Serial.println(" OK");
    Serial.print("[Wi-Fi] IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  if (strcmp(ssid, WIFI_SSID) != 0) {
    WiFi.disconnect(false, false);
    delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
      Serial.print(".");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
      Serial.println(" OK");
      Serial.print("[Wi-Fi] IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
  }

  Serial.println("[Wi-Fi] INIT RETURN FALSE");
  return false;
}

static inline bool wifi_init2(const char* ssid, const char* pass) {
    int i = 0;

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    WiFi.mode(WIFI_STA);

    for (int attempt = 0; attempt < 3; attempt++) {
        WiFi.disconnect(false, false);
        delay(2000);

        Serial.print(F("[Wi-Fi] Connecting "));
        WiFi.begin(ssid, pass);

        for (i = 0; i < 40 && WiFi.status() != WL_CONNECTED; ++i) {
            Serial.print('.');
            delay(500);
        }

        if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
            Serial.println(" OK");
            Serial.print(F("[Wi-Fi] IP: "));
            Serial.println(WiFi.localIP());
            Serial.print(F("\nMAC: ")); 
            Serial.println(WrapperMqttGetESP_ID());
            return true;
        }

        Serial.println(" FAIL");
        delay(5000);
    }

    for (int attempt = 0; attempt < 2; attempt++) {
        WiFi.disconnect(false, false);
        delay(2000);

        Serial.print(F("[Wi-Fi] Default Connecting "));
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        for (i = 0; i < 40 && WiFi.status() != WL_CONNECTED; ++i) {
            Serial.print('.');
            delay(500);
        }

        if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
            Serial.println(" OK");
            Serial.print(F("[Wi-Fi] IP: "));
            Serial.println(WiFi.localIP());
            Serial.println("default ssid pass");

            return true;
        }

        Serial.println(" FAIL");
        delay(5000);
    }

    return false;
}
/*-----------  ethernet başlatma  ------*/
   static inline bool ethernet_init() {
    Serial.begin(115200);
    Serial.print(F("[main] Wait for Ethernet: "));
    int counter= 0;
    // Ethernet bağlantısını başlat
        const int retryLimit = 5;
    bool hardwareOk = false;

    for (int i = 0; i < retryLimit; i++) {
       /* if (Ethernet.hardwareStatus() != EthernetNoHardware) {
            hardwareOk = true;
            break;
        }*/

        Serial.println(F("Ethernet donanımı bulunamadı, yeniden deneniyor..."));
        delay(300);
    }

    if (!hardwareOk) {
        Serial.println(F("Ethernet donanımı bulunamadı! (5 deneme yapıldı)"));
        return false;
    }
   
    
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("Ethernet DHCP ile yapılandırılamadı."));

        /* Ethernet donanımının olup olmadığını kontrol et
            if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println(F("Ethernet donanımı bulunamadı!"));
            return false;
        }*/
      
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println(F("Ethernet kablosu bağlı değil."));
            return false;
        }
    } else {
        Serial.print(F("  DHCP ile IP alındı: "));
        Serial.println(Ethernet.localIP());
    }

    Serial.println(F(" connected!"));
    return true;

   }

/*-----------  GSM başlatma  ------*/

static inline bool gsm_init() {
  Serial.print(F("\nMAC: ")); 
  Serial.println(WrapperMqttGetESP_ID());
  delay(1000);
  SerialAT.begin(115200, SERIAL_8N1, 44, 43);
  SerialAT.flush();
  //modem.poweroff();
  delay(1000);
  modem.restart();
  modem.init();
  //initGsmParams(&gsm_transport_layer);
  String modemInfo = modem.getModemInfo();
  Serial.println("[GSM] " + modemInfo);


  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      delay(10000);
      return false;
  }
  if (modem.isGprsConnected()) {
       // Multi-socket ayarı
      modem.sendAT("+CIPMUX=1");
      modem.waitResponse();
      modem.sendAT("+CIPRXGET=1");
      modem.waitResponse();

      delay(100);
      Serial.println("[GSM] GPRS bağlı");
      return true;
  }
  return false;
}

static inline bool netInit(const char* ssid, const char* pass) {
  assignUint16_network(WrapperMqttGetCONNECTION_TYPE(), &netType);
  
  if(netType != NET_TYPE_ALL) networkSource = netType;
  switch (netType) {
    case NET_TYPE_ALL: {
      //if (ethernet_init())     { comsupportedNetwork = NET_TYPE_ETH; networkSource=NET_TYPE_ETH; WrapperMqttSetCOMMSUPPORT_NETWORK("NET_TYPE_ETH");return true; }
      if (wifi_init(ssid, pass)) { comsupportedNetwork = NET_TYPE_WIFI; networkSource=NET_TYPE_WIFI; WrapperMqttSetCOMMSUPPORT_NETWORK("NET_TYPE_WIFI");return true; }
      if (gsm_init())            { comsupportedNetwork = NET_TYPE_GSM;  networkSource=NET_TYPE_GSM; WrapperMqttSetCOMMSUPPORT_NETWORK("NET_TYPE_GSM");return true; }
      return false;
    }
    case NET_TYPE_ETH:
      comsupportedNetwork = NET_TYPE_ETH;
      return ethernet_init();
    case NET_TYPE_WIFI:
      comsupportedNetwork = NET_TYPE_WIFI;
      return wifi_init(ssid, pass);
    case NET_TYPE_GSM:
      comsupportedNetwork = NET_TYPE_GSM;
      return gsm_init();
    default:
      return false;
  }
}

bool connectGPRS() {
  SerialAT.println("AT");
  if (SerialAT.find("OK")) {
      
  }
  else{
    return false;
  }
  //modem.gprsDisconnect();
  //delay(200);
  modem.restart();
  if (!modem.waitForNetwork(60000)) {
    return false;
  }

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    return false;
  }

  return true;
}



void gsmReconnect(bool isInternet, bool* gprsConnected) {
  if (!isInternet) {
    *gprsConnected = false;
    gprsRetryCount++;
    if (gprsRetryCount >= GPRS_RETRY_LIMIT) {
      //resetModem();
      delay(3000);
      *gprsConnected = connectGPRS();
      gprsRetryCount = 0;
    } else {
      *gprsConnected = false;
    }

  } else {
    if (!(*gprsConnected)) {
    }
    *gprsConnected = true;
    gprsRetryCount = 0;
  }
}

void wifiReconnect(bool isInternet, bool* gprsConnected, const char* ssid, const char* pass) {

  if (isInternet) {
    *gprsConnected = true;
    gprsRetryCount = 0;
    wifiReconnectInProgress = false;
    wifiReconnectAttempt = 0;
    return;
  }

  *gprsConnected = false;
  gprsRetryCount++;

  Serial.println("no internet");

  if (gprsRetryCount < GPRS_RETRY_LIMIT) return;

  if (!wifiReconnectInProgress) {

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    WiFi.mode(WIFI_STA);

    WiFi.disconnect(false, false);

    Serial.print(F("[Wi-Fi] Reconnecting "));
    Serial.println(wifiReconnectAttempt + 1);

    WiFi.begin(ssid, pass);

    wifiReconnectInProgress = true;
    wifiReconnectStartMs = millis();
    lastDotTime = millis();
    return;
  }

  if (millis() - lastDotTime >= 500) {
    Serial.print(".");
    lastDotTime = millis();
  }

  if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
    Serial.println(" OK");
    Serial.print(F("[Wi-Fi] IP: "));
    Serial.println(WiFi.localIP());

    *gprsConnected = true;
    gprsRetryCount = 0;
    wifiReconnectInProgress = false;
    wifiReconnectAttempt = 0;
    return;
  }

  if (millis() - wifiReconnectStartMs >= WIFI_RECONNECT_TIMEOUT) {
    Serial.println(" FAIL");

    wifiReconnectInProgress = false;
    wifiReconnectAttempt++;

    if (wifiReconnectAttempt >= WIFI_RECONNECT_MAX_ATTEMPT) {
      wifiReconnectAttempt = 0;
      gprsRetryCount = 0;
    }
  }
}
static inline bool ethernetHasIP() {
  IPAddress ip = Ethernet.localIP();
  return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);
}

void ethernetReconnect(bool isInternet, bool* gprsConnected) {
  if (!isInternet) {
    *gprsConnected = false;
    gprsRetryCount++;
        if (gprsRetryCount >= GPRS_RETRY_LIMIT) {
      //resetModem();
      delay(3000);

    // Donanım yok
          const int retryLimit = 5;
    bool hardwareOk = false;

    for (int i = 0; i < retryLimit; i++) {
        if (Ethernet.hardwareStatus() != EthernetNoHardware) {
            hardwareOk = true;
            break;
        }

        Serial.println(F("Ethernet donanımı bulunamadı, yeniden deneniyor..."));
        delay(300);
    }

    if (!hardwareOk) {
        Serial.println(F("Ethernet donanımı bulunamadı! (5 deneme yapıldı)"));
        return ;
    }

    // Kablo/link yoksa 
    if (Ethernet.linkStatus() == LinkOFF) {
      if (gprsRetryCount % 10 == 0) {
        Serial.println(F("[ETH] Link OFF (kablo yok/anahtar kapalı).")) ;
        
      }
    } else {
      // Link var -> DHCP durumunu koru/yenile
      if (gprsRetryCount % 5 == 0) {
        int dhcp = Ethernet.maintain();
        // maintain() başarısızlıklarında veya IP yoksa yeniden DHCP iste
        if (!ethernetHasIP() || dhcp == 1 || dhcp == 3) {
          Serial.println(F("[ETH] DHCP yenilemesi başarısız, yeniden isteniyor..."));
          Ethernet.begin(mac);
        } 
      }

      // Hâlâ IP yoksa arada tam dene
      if (!ethernetHasIP() && (gprsRetryCount % 15 == 0)) {
        Serial.println(F("[ETH] IP yok, DHCP yeniden başlatılıyor..."));
        Ethernet.begin(mac);
      }
    }

    // Güvenlik: uzun beklemede tam rebind
    if (gprsRetryCount >= 60) { // çağrı periyoduna göre ~2dk gibi bişi
      Serial.println(F("[ETH] Zorunlu tam DHCP rebind"));
      Ethernet.begin(mac);
      gprsRetryCount = 0;
    }
      }
  } else {
    // İnternet geri geldi
    if (!(*gprsConnected)) {
      Serial.print(F("[ETH] Bağlandı, IP: "));
      Serial.println(Ethernet.localIP());
    }
    *gprsConnected = true;
    gprsRetryCount = 0;
  }
  
}

int getCsqOnce() {
  static int lastValid = 99;          

  // 1) TinyGSM ile dene
  int csq = modem.getSignalQuality();  // 0..31, 99 bilinmiyor, <0 hata
  if (csq >= 0 && csq <= 31) {
    lastValid = csq;
    return csq;
  }

  // 2) Ham AT+CSQ fallback (küçük bir retry ile)
  for (int i = 0; i < 2; ++i) {
    // RX temizle
    while (SerialAT.available()) SerialAT.read();
    SerialAT.setTimeout(300);
    SerialAT.println("AT+CSQ");
    if (SerialAT.find("+CSQ: ")) {
      csq = SerialAT.parseInt();              // 0..31 veya 99
      (void)SerialAT.readStringUntil('\n');   // OK ve kalanları yut
      if (csq >= 0 && csq <= 31) {
        lastValid = csq;
        return csq;
      }
    }
    delay(120); 
  }
}

int getDbmOnce() {
  static int lastValid = 0x7FFF;  // 0x7FFF = Bilinmiyor
  
  
  int dbm = WiFi.RSSI();  
  
  if (dbm <= 0 && dbm >= -100) {
    lastValid = dbm;
    return dbm;
  }

  // 3) Eğer Wi-Fi değeri saçmaysa
  if (WiFi.status() != WL_CONNECTED) {
    return 0x7FFF; // Bilinmiyor
  }

 
  return lastValid;
}

static bool readLineUntilTimeout(Stream &s, String &out, uint32_t timeoutMs) {
  uint32_t start = millis();
  out = "";
  while (millis() - start < timeoutMs) {
    while (s.available()) {
      char c = (char)s.read();
      if (c == '\r') continue;
      if (c == '\n') {
        if (out.length() > 0) return true; // bir satır doldu
        // boş satırsa devam
      } else {
        out += c;
      }
    }
    delay(5);
  }
  return out.length() > 0;
}

static void flushInput(Stream &s) {
  while (s.available()) (void)s.read();
}

static String trimNonDigits(const String &in) {
  String out;
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    if (c >= '0' && c <= '9') out += c;
  }
  return out;
}

// SIM800 ICCID: AT+CCID  -> cevap satırında 19-20 hane olur
bool GetICCID(Stream &at, String &iccid, uint32_t timeoutMs = 2000) {
  flushInput(at);
  at.println("AT+CCID");

  String line;
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (!readLineUntilTimeout(at, line, timeoutMs)) break;
    line.trim();
    if (line.length() == 0) continue;
    if (line == "OK" || line.startsWith("AT+CCID")) continue;
    if (line.startsWith("+CCID:")) {
      line.remove(0, strlen("+CCID:"));
      line.trim();
    }
    String digits = trimNonDigits(line);
    if (digits.length() >= 18) { // ICCID genelde 19-20
      iccid = digits;
      return true;
    }
  }
  return false;
}

// SIM800 IMSI: AT+CIMI -> tek satır 15 hane
bool GetIMSI(Stream &at, String &imsi, uint32_t timeoutMs = 2000) {
  flushInput(at);
  at.println("AT+CIMI");

  String line;
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (!readLineUntilTimeout(at, line, timeoutMs)) break;
    line.trim();
    if (line.length() == 0) continue;
    if (line == "OK" || line.startsWith("AT+CIMI")) continue;

    String digits = trimNonDigits(line);
    if (digits.length() == 15) {
      imsi = digits;
      return true;
    }
  }
  return false;
}


bool isValidWifiField(const char* s, size_t maxLen) {
    if (s == NULL) return false;

    size_t len = strlen(s);
    if (len == 0 || len >= maxLen) return false;

    for (size_t i = 0; i < len; i++) {
        if (!isspace((unsigned char)s[i])) {
            return true;
        }
    }

    return false;
}

void loadWifiCredentials() {
    const char* ssid = WrapperMqttGetWIFI_SSID();
    const char* pass = WrapperMqttGetWIFI_PASS();

    if (isValidWifiField(ssid, sizeof(WIFI_SSid))) {
        strncpy(WIFI_SSid, ssid, sizeof(WIFI_SSid) - 1);
        WIFI_SSid[sizeof(WIFI_SSid) - 1] = '\0';
        Serial.println("[WiFi] SSID MQTT'den alindi");
    } else {
        strncpy(WIFI_SSid, WIFI_SSID, sizeof(WIFI_SSid) - 1);
        WIFI_SSid[sizeof(WIFI_SSid) - 1] = '\0';
        Serial.println("[WiFi] SSID gecersiz veya bos, default kullanildi");
    }

    if (isValidWifiField(pass, sizeof(WIFI_Pass))) {
        strncpy(WIFI_Pass, pass, sizeof(WIFI_Pass) - 1);
        WIFI_Pass[sizeof(WIFI_Pass) - 1] = '\0';
        Serial.println("[WiFi] PASS MQTT'den alindi");
    } else {
        strncpy(WIFI_Pass, WIFI_PASS, sizeof(WIFI_Pass) - 1);
        WIFI_Pass[sizeof(WIFI_Pass) - 1] = '\0';
        Serial.println("[WiFi] PASS gecersiz veya bos, default kullanildi");
    }

    Serial.printf("[WiFi] Final SSID: %s\n", WIFI_SSid);
    Serial.printf("[WiFi] Final PASS: %s\n", WIFI_Pass);
}




static inline bool isInternetAvailable() {
  switch (netType) {
    case NET_TYPE_ALL:
      switch (comsupportedNetwork) {
        case NET_TYPE_GSM:
          return modem.isNetworkConnected() && modem.isGprsConnected();

        case NET_TYPE_WIFI:
          return WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0 && WiFi.gatewayIP()[0] != 0;

        case NET_TYPE_ETH:
          return ethernetHasIP() && Ethernet.linkStatus() != LinkOFF;

        default:
          return false;
      }

    case NET_TYPE_GSM:
      return modem.isNetworkConnected() && modem.isGprsConnected();

    case NET_TYPE_WIFI:
      return WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0 &&WiFi.gatewayIP()[0] != 0;

    case NET_TYPE_ETH:
      return ethernetHasIP() && Ethernet.linkStatus() != LinkOFF;

    default:
      return false;
  }
}

static inline bool recoverNetwork() {
  if (isInternetAvailable()) return true;

  switch (netType) {
    case NET_TYPE_ALL:
      switch (comsupportedNetwork) {
        case NET_TYPE_GSM:
          if (connectGPRS()) return gprsConnected = true;
          if (gsm_init()) return gprsConnected = true;
          return gprsConnected = false;

        case NET_TYPE_WIFI:
          loadWifiCredentials();
          if (wifi_init2(WIFI_SSid, WIFI_Pass)) return gprsConnected = true;
          return gprsConnected = false;

        case NET_TYPE_ETH:
          if (ethernet_init()) return gprsConnected = true;
          return gprsConnected = false;

        default:
          return false;
      }

    case NET_TYPE_GSM:
      if (connectGPRS()) return gprsConnected = true;
      if (gsm_init()) return gprsConnected = true;
      return gprsConnected = false;

    case NET_TYPE_WIFI:
      loadWifiCredentials();
      if (wifi_init2(WIFI_SSid, WIFI_Pass)) return gprsConnected = true;
      return gprsConnected = false;

    case NET_TYPE_ETH:
      if (ethernet_init()) return gprsConnected = true;
      return gprsConnected = false;

    default:
      return false;
  }
}


#endif /* gsm_api_H */
