#ifndef FLASH_DRIVER_H_
#define FLASH_DRIVER_H_
#include <stdint.h>
#include <stdbool.h>
void flash_session_begin(void);
bool flash_address_allowed(uint32_t address, uint8_t length);
bool flash_prepare_and_write(uint32_t address, const uint8_t *data, uint8_t length);
uint8_t flash_read8(uint32_t address);
bool boot_state_set(uint8_t state);
uint8_t boot_state_get(void);
#endif
