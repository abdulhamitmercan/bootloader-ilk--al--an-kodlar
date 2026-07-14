#ifndef BOOT_CONFIG_H_
#define BOOT_CONFIG_H_

#include <stdint.h>

/* UART packet protocol */
#define BOOT_H1                 0xAAU
#define BOOT_H2                 0x55U
#define BOOT_CMD_ERASE          0x02U
#define BOOT_CMD_WRITE          0x03U
#define BOOT_CMD_DONE           0x04U
#define BOOT_RSP_ACK            0x80U
#define BOOT_RSP_NACK           0x81U
#define BOOT_MAX_DATA_LEN       16U

/* acCharger bootloader-linker memory map */
#define APP_VECTOR_START        0x0000DF80UL
#define APP_VECTOR_END          0x0000E000UL
#define APP_VECTOR_RESET        0x0000DFFEUL
#define APP_VECTOR_SEGMENT      0x0000DE00UL
#define APP_LOW_START           0x0000E000UL
#define APP_LOW_END             0x0000FE00UL
#define APP_HIGH_START          0x00010000UL
#define APP_HIGH_END            0x0002BFF8UL

/* Bootloader and real vector areas. */
#define BOOT_CODE_START         0x0000C000UL
#define BOOT_CODE_END           0x0000DE00UL
#define REAL_VECTOR_SEG_START   0x0000FE00UL
#define REAL_VECTOR_SEG_END     0x00010000UL

#define RAM_VECTOR_START        0x00005B80UL
#define VECTOR_TABLE_SIZE       0x0080U
#define FLASH_SEGMENT_SIZE      512UL

/* INFO D is reserved for boot state. */
#define BOOT_STATE_ADDR         0x00001800UL
#define BOOT_STATE_UPDATING     0x55U
#define BOOT_STATE_VALID        0xA5U

/* Approximately 8 seconds at the polling loop used by main. */
#define BOOT_STARTUP_WAIT_LOOPS 800000UL

#endif
