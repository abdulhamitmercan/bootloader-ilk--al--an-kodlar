#include "crc16.h"

uint16_t crc16_ccitt(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t n;
    uint8_t bit;

    for (n = 0U; n < length; n++)
    {
        crc ^= ((uint16_t)data[n] << 8);
        for (bit = 0U; bit < 8U; bit++)
        {
            crc = (crc & 0x8000U) ?
                (uint16_t)((crc << 1) ^ 0x1021U) :
                (uint16_t)(crc << 1);
        }
    }
    return crc;
}
