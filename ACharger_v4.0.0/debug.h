#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include "broker.h"
// Define your debug serial object here
#define DEBUG_SERIAL Serial1

// Define debug levels
#define DL_NONE 0x00     // suppress all output to the console
#define DL_VERBOSE 0x01  // all output

// Default debug level if not defined
#ifndef DBG_LEVEL
#define DBG_LEVEL DL_VERBOSE
#endif

// Define debug print function macro
#if DBG_LEVEL >= DL_VERBOSE
#define DEBUG_PRINT(fmt, ...) DEBUG_SERIAL.printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) ((void)0)
#endif
void addErrorIfNotExists(const char* newError) {
    String currentErrors = String(mqttDataValue.getERROR());

    // Eğer boşsa direkt ekle
    if (currentErrors.length() == 0) {
        mqttDataValue.setERROR(newError);
        return;
    }

    // Zaten var mı kontrol et (tam eşleşme için virgüllü kontrol)
    String search1 = "," + currentErrors + ",";
    String search2 = "," + String(newError) + ",";

    if (search1.indexOf(search2) == -1) {
        // Yoksa ekle
        currentErrors += ",";
        currentErrors += newError;
        mqttDataValue.setERROR(currentErrors.c_str());
    }
}
#endif // DEBUG_H
