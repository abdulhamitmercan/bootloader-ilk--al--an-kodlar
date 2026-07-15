#ifndef RFID_API_H
#define RFID_API_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "debug.h"
#include "EEPROM.h"

#define PN532_IRQ   (19)
#define PN532_RESET (3)

const int DELAY_BETWEEN_CARDS = 5000;
long timeLastCardRead = 0;
boolean readerDisabled = false;
bool rfidAvailable = false;
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

typedef enum {
  RFID_RESPONSE_NONE = 0,
  RFID_RESPONSE_CONTINUE = 2,
  RFID_RESPONSE_FINISHED = 3
} t_rfidResponse;

typedef enum {
  RFID_REQUEST_FINISHED = 0,
  RFID_REQUEST_STARTED = 1
} t_rfidStRequest;

typedef struct {
  uint16_t detected;
  String tag;
  uint16_t request;
  uint16_t response;
  uint16_t status;
} t_rfidParams;

t_rfidParams rfid1;

String mainTag = "ABDULHAMIT";
uint16_t rfidManageEnable = 0;

#define MAX_TAGS 100  // Define the maximum number of tags
#define EEPROM_SIZE 512  // Define the size of EEPROM

// Queue structure for storing valid tags
struct TagQueue {
    String validTags[MAX_TAGS];  // Using String for easier memory management
    uint16_t front;
    uint16_t rear;
    uint16_t size;
};

// EEPROM'a tag'leri kaydetme
void writeTagToEEPROM(uint16_t index, const String& tag) {
    uint16_t address = 1 + index * 32;  // Each tag takes 32 bytes

    for (uint16_t i = 0; i < 32; i++) {
        if (i < tag.length()) {
            EEPROM.write(address + i, tag[i]);  // Write tag to EEPROM
        } else {
            EEPROM.write(address + i, 0);  // Null terminator for empty space
        }
    }
    EEPROM.commit();  // Commit changes to EEPROM
}

// EEPROM'dan tag okuma
void readTagFromEEPROM(uint16_t index, String& tag) {
    uint16_t address = 1 + index * 32;  // Each tag takes 32 bytes
    tag = "";  // Initialize the tag as an empty string

    for (uint16_t i = 0; i < 32; i++) {
        char c = EEPROM.read(address + i);  // Read each byte
        if (c != 0) {
            tag += c;  // Append character to the tag string
        } else {
            break;  // Stop when null terminator is reached
        }
    }
}

// EEPROM'dan tag'leri yükle
void loadTagsFromEEPROM(TagQueue& queue) {
    EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM

    // Queue size from EEPROM
    uint8_t size = EEPROM.read(0);
    if (size == 0 || size > MAX_TAGS) {
        Serial.println("Invalid queue size from EEPROM, initializing...");
        size = 0;  // Ya da başka bir güvenli değer
    }
    queue.size = size;

    for (uint16_t i = 0; i < queue.size; i++) {
        String tag;
        readTagFromEEPROM(i, tag);  // Read tag from EEPROM
        queue.validTags[i] = tag;   // Store the tag in the queue
    }

    EEPROM.end();  // End EEPROM operations
}

// EEPROM'a tag'leri kaydet
void saveTagsToEEPROM(const TagQueue& queue) {
    EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM

    // Write the queue size to the first byte
    EEPROM.write(0, queue.size);  // First byte holds the queue size

    for (uint16_t i = 0; i < queue.size; i++) {
        writeTagToEEPROM(i, queue.validTags[i]);  // Write each tag to EEPROM
    }

    EEPROM.commit();  // Commit changes to EEPROM
    EEPROM.end();  // End EEPROM operations
}

// Queue'yu başlat
void initTagQueue(TagQueue& queue) {
    queue.front = 0;
    queue.rear = 0;
    queue.size = 0;
}

// Queue dolu mu?
bool isQueueFull(const TagQueue& queue) {
    return queue.size == MAX_TAGS;
}

// Queue boş mu?
bool isQueueEmpty(const TagQueue& queue) {
    return queue.size == 0;
}

// Queue'dan tag sil (FIFO)
bool dequeueTag(TagQueue& queue) {
    if (isQueueEmpty(queue)) {
        return false;  // No tag to remove
    }

    // Remove the oldest tag from the front of the queue
    queue.front = (queue.front + 1) % MAX_TAGS;  // Circular increment
    queue.size--;
    return true;
}

bool enqueueTag(TagQueue& queue, const String& tag) {
    if (isQueueFull(queue)) {
        Serial.println("Queue is full, removing the oldest tag...");
        dequeueTag(queue);  // Remove the oldest tag if the queue is full
    }

    // Add the new tag at the rear of the queue
    queue.validTags[queue.rear] = tag;
    queue.rear = (queue.rear + 1) % MAX_TAGS;  // Circular increment
    queue.size++;

    // EEPROM'a kaydet, sadece tag ekleme işlemi sonrası
    saveTagsToEEPROM(queue);

    return true;
}

// Queue'da tag var mı? (doğru kontrol)
bool isTagInQueue(const TagQueue& queue, const String& tag) {
    for (uint16_t i = 0; i < queue.size; i++) {
        uint16_t idx = (queue.front + i) % MAX_TAGS;
        if (queue.validTags[idx] == tag) {
            return true;
        }
    }
    return false;  // Tag not found
}

// EEPROM'dan tag'ı sil (sıfırlama işlemi)
void clearTagFromEEPROM(uint16_t index) {
    uint16_t address = 1 + index * 32;  // Her tag için 32 byte ayrılacak

    // Tag'ı EEPROM'dan sıfırlayın
    for (uint16_t i = 0; i < 32; i++) {
        EEPROM.write(address + i, 0);  // Tag'ı sıfırla
    }
    EEPROM.commit();  // EEPROM'a kaydetme işlemini tamamla
}

// Queue'dan belirli bir tag'ı sil (FIFO)
bool removeTagFromQueue(TagQueue& queue, const String& tag) {
    for (uint16_t i = 0; i < queue.size; i++) {
        uint16_t idx = (queue.front + i) % MAX_TAGS;
        if (queue.validTags[idx] == tag) {
            // Tag'ı boşalt
            queue.validTags[idx] = "";  // Tag'ı boşaltıyoruz

            // Queue boyutunu güncelle
            queue.size--;

            // EEPROM'dan da sil
            clearTagFromEEPROM(i);

            // EEPROM'u güncelle
            saveTagsToEEPROM(queue);

            return true;  // Başarıyla silindi
        }
    }
    return false;  // Tag bulunamadı
}




// Main RFID check
bool checkMainRfid(const String& readTag) {
    return readTag == mainTag;
}

// Check charge permission based on the tag
bool chargePermission(const String& readTag, const TagQueue& queue) {
    // Eğer tag boşsa veya null ise, şarj başlatma
    if (readTag == "") {
        return false;  // Geçersiz tag, şarj başlatılmamalı
    }

    for (uint16_t i = 0; i < queue.size; i++) {
        if (readTag == queue.validTags[i]) {
            return true;  // Tag queue içinde mevcut, şarj başlatılabilir
        }
    }
    return false;  // Tag queue içinde mevcut değil, şarj başlatılamaz
}




String handleCardDetected() {
  uint8_t success = false;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  String cardid = "";

  success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
  if (success) {
    for (int i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) {
        cardid += "0";  // Eğer değer tek basamaklıysa başına 0 ekle
      }
      String hexValue = String(uid[i], HEX);  // Hexadecimal değeri string'e çevir
      hexValue.toUpperCase();  // Hexadecimal string'i büyük harfe dönüştür
      cardid += hexValue;       // Büyük harfe dönüştürülmüş değeri ekle
    }
  }

  timeLastCardRead = millis();
  readerDisabled = true;  // Reader'ı geçici olarak devre dışı bırak

  return cardid;
}


String startListeningToNFC() {
  String cardid = "";
  
  // Polling for card presence
  if (nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A)) {
    cardid = handleCardDetected();  // Return the card ID as a string
  }

  return cardid;
}

void rfid_init() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("[RFID] PN532 bulunamadi! RFID devre disi.");
    rfidAvailable = false;
     addErrorIfNotExists("E0");
#if RUN_MODE == 1
    while (1);
#endif
    return;
  }
  Serial.println("[RFID] PN532 OK.");
  rfidAvailable = true;
 
}

void rfid_api_update(t_rfidParams *rfid) {
  if (rfid->request == RFID_REQUEST_STARTED) {
    if (rfid->response == RFID_RESPONSE_FINISHED) {
      rfid->request = RFID_REQUEST_FINISHED;
    }
    rfid->status = 1;
  } else {
    if (rfid->response == RFID_RESPONSE_NONE) {
      rfid->status = 0;
    }
  }
}

void rfidRead(t_rfidParams *rfid, int timeout) {
  // If the reader is disabled, wait for the specified delay
  if (readerDisabled) {
    if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS) {
      readerDisabled = false;
      String idTag = startListeningToNFC();
      
      if (idTag.length() > 0) {
        rfid->tag = idTag;        // Update the tag ID in the rfid struct
        rfid->detected = 1;       // Set detected flag to true
      }
    }
  } else {
    // Poll for card detection continuously
    String idTag = startListeningToNFC();
    if (idTag.length() > 0) {
      rfid->tag = idTag;        // Update the tag ID in the rfid struct
      rfid->detected = 1;       // Set detected flag to true
    }
  }
}

uint16_t isRfidProcessOngoing(t_rfidParams *rfid) {
  return rfid->status;
}

void rfid_FinishRequest(t_rfidParams *rfid) {
  rfid->request = RFID_REQUEST_FINISHED;
}

void rfid_resetParams(t_rfidParams *rfid){
  rfid->status = 0;
  rfid->detected = 0;
  rfid->tag = "";
}

#endif
