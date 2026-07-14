#include <msp430.h>
#include <string.h>
#include "boot_config.h"
#include "flash_driver.h"

#define MAX_SEGMENT_INDEX ((APP_HIGH_END + FLASH_SEGMENT_SIZE - 1UL) / FLASH_SEGMENT_SIZE)
#define ERASED_BITMAP_SIZE ((MAX_SEGMENT_INDEX + 7UL) / 8UL)

static uint8_t erasedSegments[ERASED_BITMAP_SIZE];

static bool ranges_overlap(uint32_t a0, uint32_t a1, uint32_t b0, uint32_t b1)
{
    return (a0 < b1) && (a1 > b0);
}

uint8_t flash_read8(uint32_t address)
{
    return (uint8_t)__data20_read_char(address);
}

static void flash_write8(uint32_t address, uint8_t value)
{
    __data20_write_char(address, value);
}

static bool segment_marked(uint32_t segmentAddress)
{
    uint32_t index = segmentAddress / FLASH_SEGMENT_SIZE;
    return (erasedSegments[index >> 3] & (uint8_t)(1U << (index & 7U))) != 0U;
}

static void mark_segment(uint32_t segmentAddress)
{
    uint32_t index = segmentAddress / FLASH_SEGMENT_SIZE;
    erasedSegments[index >> 3] |= (uint8_t)(1U << (index & 7U));
}

static bool erase_main_segment(uint32_t segmentAddress)
{
    if ((segmentAddress & (FLASH_SEGMENT_SIZE - 1UL)) != 0UL)
        return false;

    if (ranges_overlap(segmentAddress, segmentAddress + FLASH_SEGMENT_SIZE,
                       BOOT_CODE_START, BOOT_CODE_END) ||
        ranges_overlap(segmentAddress, segmentAddress + FLASH_SEGMENT_SIZE,
                       REAL_VECTOR_SEG_START, REAL_VECTOR_SEG_END))
        return false;

    while ((FCTL3 & BUSY) != 0U) { }
    FCTL3 = FWKEY;
    FCTL1 = FWKEY | ERASE;
    flash_write8(segmentAddress, 0xFFU);
    while ((FCTL3 & BUSY) != 0U) { }
    FCTL1 = FWKEY;
    FCTL3 = FWKEY | LOCK;
    return true;
}

static bool erase_info_d(void)
{
    while ((FCTL3 & BUSY) != 0U) { }
    FCTL3 = FWKEY;
    FCTL1 = FWKEY | ERASE;
    flash_write8(BOOT_STATE_ADDR, 0xFFU);
    while ((FCTL3 & BUSY) != 0U) { }
    FCTL1 = FWKEY;
    FCTL3 = FWKEY | LOCK;
    return true;
}

void flash_session_begin(void)
{
    memset(erasedSegments, 0, sizeof(erasedSegments));
}

bool flash_address_allowed(uint32_t address, uint8_t length)
{
    uint32_t end;
    bool low;
    bool high;

    if ((length == 0U) || (length > BOOT_MAX_DATA_LEN))
        return false;
    end = address + (uint32_t)length;
    if (end < address)
        return false;

    /* Vector and low application flash are one continuous writable range. */
    low = (address >= APP_VECTOR_START) && (end <= APP_LOW_END);
    high = (address >= APP_HIGH_START) && (end <= APP_HIGH_END);
    if (!low && !high)
        return false;

    if (ranges_overlap(address, end, BOOT_CODE_START, BOOT_CODE_END) ||
        ranges_overlap(address, end, REAL_VECTOR_SEG_START, REAL_VECTOR_SEG_END))
        return false;
    return true;
}

bool flash_prepare_and_write(uint32_t address, const uint8_t *data, uint8_t length)
{
    uint32_t cursor = address;
    uint32_t end = address + (uint32_t)length;
    uint8_t i;

    if (!flash_address_allowed(address, length))
        return false;

    while (cursor < end)
    {
        uint32_t segment = cursor & ~(FLASH_SEGMENT_SIZE - 1UL);
        if (!segment_marked(segment))
        {
            if (!erase_main_segment(segment))
                return false;
            mark_segment(segment);
        }
        cursor = segment + FLASH_SEGMENT_SIZE;
    }

    while ((FCTL3 & BUSY) != 0U) { }
    FCTL3 = FWKEY;
    FCTL1 = FWKEY | WRT;
    for (i = 0U; i < length; i++)
    {
        flash_write8(address + (uint32_t)i, data[i]);
        while ((FCTL3 & BUSY) != 0U) { }
    }
    FCTL1 = FWKEY;
    FCTL3 = FWKEY | LOCK;

    for (i = 0U; i < length; i++)
        if (flash_read8(address + (uint32_t)i) != data[i])
            return false;
    return true;
}

bool boot_state_set(uint8_t state)
{
    erase_info_d();
    while ((FCTL3 & BUSY) != 0U) { }
    FCTL3 = FWKEY;
    FCTL1 = FWKEY | WRT;
    flash_write8(BOOT_STATE_ADDR, state);
    while ((FCTL3 & BUSY) != 0U) { }
    FCTL1 = FWKEY;
    FCTL3 = FWKEY | LOCK;
    return flash_read8(BOOT_STATE_ADDR) == state;
}

uint8_t boot_state_get(void)
{
    return flash_read8(BOOT_STATE_ADDR);
}
