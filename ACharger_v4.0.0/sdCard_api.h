#ifndef SDCARD_API_H
#define SDCARD_API_H

#include <SPI.h>
#include <SD.h>
#include "debug.h"
#define SD_CS_PIN 10
#define DATA_FILE "/energy.txt"
#define TIME_FILE "/time.txt"
// ---------------------------------------------------------------------------
// SD Kart Başlatma
// ---------------------------------------------------------------------------
bool sd_init() {
    if (!SD.begin(SD_CS_PIN)) {
        addErrorIfNotExists("E1");
        return false;
    }

    return true;
}

void writeEnergy(float value) {
    File file = SD.open("/energy.txt", FILE_WRITE);  

    if (!file) {
        return;
    }

    file.printf("%.3f\n", value);
    file.flush();   
    file.close();

}


float readEnergy() {
    File file = SD.open("/energy.txt", FILE_READ);

    if (!file) {
        return -1;
    }

    String line = file.readStringUntil('\n');
    file.close();

    line.trim();
    return line.toFloat();
}




void writeChargeTime(uint32_t value) {
    File file = SD.open(TIME_FILE, FILE_WRITE);
    if (!file) return;

    file.printf("%lu\n", (unsigned long)value);
    file.flush();
    file.close();
}

uint32_t readChargeTime() {
    File file = SD.open(TIME_FILE, FILE_READ);
    if (!file) return 0;

    String line = file.readStringUntil('\n');
    file.close();

    line.trim();
    return (uint32_t)line.toInt();
}

#endif
