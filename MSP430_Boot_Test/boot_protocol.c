#include <stdint.h>
#include <stdbool.h>
#include "boot_config.h"
#include "crc16.h"
#include "boot_uart.h"
#include "flash_driver.h"
#include "app_jump.h"
#include "boot_protocol.h"

#define PACKET_MAX_SIZE (10U + BOOT_MAX_DATA_LEN)

static uint8_t packet[PACKET_MAX_SIZE];
static uint8_t index;
static uint8_t expected;
static uint8_t state;
static bool updateStarted;

static void parser_reset(void)
{
    state = 0U;
    index = 0U;
    expected = 0U;
}

static void send_response(uint8_t response, uint8_t parameter, uint8_t d1, uint8_t d2)
{
    uint8_t tx[8];
    tx[0] = (uint8_t)'b';
    tx[1] = response;
    tx[2] = 0U;
    tx[3] = parameter;
    tx[4] = d1;
    tx[5] = d2;
    tx[6] = tx[0] ^ tx[1] ^ tx[2] ^ tx[3] ^ tx[4] ^ tx[5];
    tx[7] = (uint8_t)'k';
    boot_uart_write(tx, 8U);
}

static void ack(uint8_t command, uint8_t d1, uint8_t d2)
{
    send_response(BOOT_RSP_ACK, command, d1, d2);
}

static void nack(uint8_t reason)
{
    send_response(BOOT_RSP_NACK, reason, 0U, 0U);
}

static uint32_t packet_address(void)
{
    return ((uint32_t)packet[3] << 24) |
           ((uint32_t)packet[4] << 16) |
           ((uint32_t)packet[5] << 8)  |
           (uint32_t)packet[6];
}

static void process_packet(void)
{
    uint8_t command = packet[2];
    uint8_t length = packet[7];
    uint16_t received = ((uint16_t)packet[8U + length] << 8) |
                        (uint16_t)packet[9U + length];
    uint16_t calculated = crc16_ccitt(packet, (uint16_t)(8U + length));
    uint32_t address = packet_address();

    if (received != calculated) { nack(0x11U); return; }

    switch (command)
    {
        case BOOT_CMD_ERASE:
            flash_session_begin();
            if (!boot_state_set(BOOT_STATE_UPDATING)) { nack(0x15U); return; }
            updateStarted = true;
            ack(command, 0xEEU, 0U);
            break;

        case BOOT_CMD_WRITE:
            if (!updateStarted) { nack(0x16U); return; }
            if (!flash_address_allowed(address, length)) { nack(0x12U); return; }
            if (!flash_prepare_and_write(address, &packet[8], length)) { nack(0x13U); return; }
            ack(command, packet[8], packet[7U + length]);
            break;

        case BOOT_CMD_DONE:
            if (!updateStarted) { nack(0x16U); return; }
            if (!boot_state_set(BOOT_STATE_VALID)) { nack(0x15U); return; }
            if (!application_is_valid()) { nack(0x17U); return; }
            ack(command, 0xD0U, 0x0EU);
            boot_uart_flush_tx();
            application_jump();
            break;

        default:
            nack(0x20U);
            break;
    }
}

void boot_protocol_reset(void)
{
    parser_reset();
    updateStarted = false;
}

bool boot_protocol_update_started(void)
{
    return updateStarted;
}

void boot_protocol_feed(uint8_t value)
{
    switch (state)
    {
        case 0U:
            if (value == BOOT_H1) { packet[0] = value; state = 1U; }
            break;
        case 1U:
            if (value == BOOT_H2) { packet[1] = value; index = 2U; state = 2U; }
            else if (value != BOOT_H1) { state = 0U; }
            break;
        case 2U:
            if (index >= PACKET_MAX_SIZE) { parser_reset(); nack(0x21U); break; }
            packet[index++] = value;
            if (index == 8U)
            {
                if (packet[7] > BOOT_MAX_DATA_LEN) { parser_reset(); nack(0x10U); break; }
                expected = (uint8_t)(10U + packet[7]);
            }
            if ((expected != 0U) && (index == expected))
            {
                state = 0U;
                index = 0U;
                expected = 0U;
                process_packet();
            }
            break;
        default:
            parser_reset();
            break;
    }
}
