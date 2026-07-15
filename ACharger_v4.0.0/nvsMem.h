#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifndef NVS_NS
#define NVS_NS "ocpp"
#endif

namespace Nvsmem {


inline SemaphoreHandle_t& mutex() {
    static SemaphoreHandle_t mtx = nullptr;

    if (mtx == nullptr) {
        mtx = xSemaphoreCreateMutex();
    }
    return mtx;
}

//  lock guard
class LockGuard {
public:
    LockGuard() {
        SemaphoreHandle_t m = mutex();
        if (m) {
            xSemaphoreTake(m, portMAX_DELAY);
        }
    }

    ~LockGuard() {
        SemaphoreHandle_t m = mutex();
        if (m) {
            xSemaphoreGive(m);
        }
    }
};

// SADECE OKU
inline String loadString(const char* key) {
    LockGuard lock;

    Preferences pref;
    if (!pref.begin(NVS_NS, true)) {
        return "";
    }

    String current = pref.getString(key, "");
    pref.end();
    return current;
}

// STRING ILE YAZ / FARKLIYSA YAZ
inline String loadString(const char* key, const String& value) {
    LockGuard lock;

    Preferences pref;
    if (!pref.begin(NVS_NS, false)) {
        return value;
    }

    String current = pref.getString(key, "");

    if (current != value) {
        pref.putString(key, value);
        current = value;
    }

    pref.end();
    return current;
}

// CONST CHAR* ILE YAZ / FARKLIYSA YAZ
inline String loadString(const char* key, const char* value) {
    String tmp = value ? String(value) : String("");
    return loadString(key, tmp);
}

// YOKSA DEFAULT YAZ, VARSA OKU
inline String getOrInit(const char* key, const String& defVal) {
    LockGuard lock;

    Preferences pref;
    if (!pref.begin(NVS_NS, false)) {
        return defVal;
    }

    String current = pref.getString(key, "");

    if (current.isEmpty()) {
        pref.putString(key, defVal);
        current = defVal;
    }

    pref.end();
    return current;
}

// AYRI YAZMA FONKSİYONU 
inline bool writeString(const char* key, const String& value) {
    LockGuard lock;

    Preferences pref;
    if (!pref.begin(NVS_NS, false)) {
        return false;
    }

    String current = pref.getString(key, "");
    if (current != value) {
        pref.putString(key, value);
    }

    pref.end();
    return true;
}

//  SADECE OKUMA
inline String readString(const char* key, const String& defVal = "") {
    LockGuard lock;

    Preferences pref;
    if (!pref.begin(NVS_NS, true)) {
        return defVal;
    }

    String current = pref.getString(key, defVal);
    pref.end();
    return current;
}
/*
 * Proje   : EVSE – MQTT / OCPP Haberleşme
 * Dosya   : nvsmem.h
 * Amaç    : ESP32 NVS (Preferences) üzerinde güvenli String okuma/yazma
 *           sarmalayıcısı sağlamak; çoklu task erişimlerinde sıraya alma
 *           ile çakışmayı azaltmak.
 *
 * Yazar   : Abdulhamit Mercan
 * Tarih   : Ağustos 2025
 *
 * Özet    :
 * - Nvsmem::loadString(key): sadece OKU.
 * - Nvsmem::loadString(key, value): farklıysa YAZ, güncel değeri döndür.
 * - Nvsmem::getOrInit(key, defVal): yoksa defVal ile başlat, varsa mevcutu döndür.
 * - Nvsmem::readString(key, defVal): sadece oku, boşsa varsayılanı döndür.
 * - Nvsmem::writeString(key, value): farklıysa yaz.
 *
 * Yapılandırma:
 * - NVS_NS (varsayılan "ocpp") – namespace ≤ 15 karakter.
 * - Key’ler de ≤ 15 karakter (ESP32 NVS kısıtı).
 *
 * Uyarılar:
 * - Fonksiyonlar mutex ile korunur; farklı task’lardan erişim sıraya alınır.
 * - Sık değişen runtime parametrelerini NVS’ye yazmayın; flash ömrü etkilenir.
 * - Runtime veriler RAM’de tutulmalı, NVS sadece kalıcı config için kullanılmalıdır.
 */
} // namespace Nvsmem



/*#pragma once
#include <Arduino.h>
#include <Preferences.h>

// ---- NVS namespace (≤15 char)
#ifndef NVS_NS
#define NVS_NS "ocpp"
#endif

// ---- Key'ler (≤15 char) — tek dosyada toplandı
#define KEY_ESP_ID        "ESP_ID_"
#define KEY_OCPP_URL      "OCPP_URL_"
#define KEY_OCPP_ID       "OCPP_ID_"
#define KEY_QR_ID         "QR_ID_"
#define KEY_MAX_POWER     "MAX_POWER_"
#define KEY_WIFI_SSID     "WIFI_SSID_"
#define KEY_WIFI_PASS     "WIFI_PASS_"
#define KEY_CONN_TYPE     "CONN_TYPE_"
#define KEY_UI_TEL_NUM    "UI_TEL_NUM_"
#define KEY_UI_COMP_NAME  "UI_COMP_NAME_"
#define KEY_UI_APP_URL    "UI_APP_URL_"
#define KEY_UI_PLAY_URL   "UI_PLAY_URL_"
#define KEY_UI_LOGO_TEXT  "UI_LOGO_TEXT_"
#define KEY_OTA_ACTIVE    "OTA_ACTIVE_"
#define KEY_CONF_MEASTYPE "CONF_MEASTYPE_"
#define KEY_CONF_MODE     "CONF_MODE_"
#define KEY_CONF_MODEL    "CONF_MODEL_"

namespace Nvsmem {
  inline Preferences& prefs() {
    static Preferences p;
    return p;
  }

  // Tek API:
  // - value boşsa: SADECE OKU
  // - value doluysa: GEREKİRSE YAZ (farklıysa yazar; aynıysa yazmaz)
  inline String loadString(const char* key, const String& value = "") {
    const bool writeMode = (value.length() > 0);
    prefs().begin(NVS_NS, writeMode ? false : true);

    String current = prefs().getString(key, "");
    if (writeMode && current != value) {
      prefs().putString(key, value);
      current = value;
    }

    prefs().end();
    return current; // her zaman depodaki güncel değer
  }

  // Yoksa default ile başlat (ilk kurulum için pratik)
  inline String getOrInit(const char* key, const String& defVal) {
    prefs().begin(NVS_NS, true);
    const bool exists = prefs().isKey(key);
    prefs().end();
    return exists ? loadString(key) : loadString(key, defVal);
  }

  // const char* isteyen yerler için küçük yardımcı (tek thread için güvenli)
  inline const char* cstr(const char* key, const char* defVal = "") {
    static String cache;
    cache = loadString(key);
    if (cache.isEmpty() && defVal && *defVal) {
      cache = loadString(key, defVal);
    }
    return cache.c_str();
  }
}*/

/*
 * Proje   : EVSE – MQTT / OCPP Haberleşme
 * Dosya   : nvsmem.h
 * Amaç    : ESP32 NVS (Preferences) üzerinde basit String okuma/yazma sarmalayıcısı
 *           sağlamak; default ile başlatma ve const char* dönüş kolaylığı vermek.
 *
 * Yazar   : Abdulhamit Mercan
 * Tarih   : Ağustos 2025
 *
 * Özet    :
 * - Nvsmem::loadString(key, [value]): value boşsa OKU; doluysa farklıysa YAZ.
 * - Nvsmem::getOrInit(key, defVal): yoksa defVal ile başlat, varsa mevcutu döndür.
 * - Nvsmem::cstr(key, defVal): tek thread için uygun küçük yardımcı (geçici cache).
 *
 * Yapılandırma:
 * - NVS_NS (varsayılan "ocpp") – namespace ≤ 15 karakter.
 * - Key’ler de ≤ 15 karakter (ESP32 NVS kısıtı).
 *
 * Uyarılar:
 * - cstr() içindeki static String cache çoklu thread/ISR için uygun değildir.
 * - Sık değişen parametreleri NVS’ye yazmayın; flash ömrü etkilenir.
 */
