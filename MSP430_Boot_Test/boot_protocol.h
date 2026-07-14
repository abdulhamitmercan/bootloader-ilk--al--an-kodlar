#ifndef BOOT_PROTOCOL_H_
#define BOOT_PROTOCOL_H_
#include <stdint.h>
#include <stdbool.h>
void boot_protocol_reset(void);
void boot_protocol_feed(uint8_t value);
bool boot_protocol_update_started(void);
#endif
