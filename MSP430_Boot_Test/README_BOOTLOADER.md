# MSP430F67751 acCharger UART Bootloader

## Memory map

- `0xC000-0xDDFF`: bootloader code
- `0xDE00-0xDFFF`: application virtual-vector flash segment
- `0xE000-0xFDFF`: application low flash
- `0xFE00-0xFFFF`: protected segment containing real interrupt/reset vectors
- `0x10000-0x2BFF7`: application high flash
- `0x5B80-0x5BFF`: application RAM vector table
- `INFO D / 0x1800`: update-valid state marker

## Build and installation

1. Build this project in CCS.
2. Confirm the map shows `.text` only inside `0xC000-0xDDFF`.
3. Program this bootloader once with JTAG.
4. ESP downloads `msp.txt`, parses TI-TXT addresses, and sends ERASE, WRITE packets, then DONE.

## Packet

`AA 55 CMD ADDR[4] LEN DATA[LEN] CRC16[2]`

- CRC16-CCITT is calculated from `AA` through the final data byte.
- LEN may be 0..16. WRITE requires 1..16.
- Existing 16-byte packets remain compatible.

## NACK reasons

- `0x10`: invalid length
- `0x11`: CRC mismatch
- `0x12`: address denied
- `0x13`: erase/write/verify failed
- `0x15`: boot-state flash write failed
- `0x16`: WRITE/DONE before ERASE
- `0x17`: application reset vector invalid
- `0x20`: unknown command
- `0x21`: parser overflow

## Important

The ESP sender must not flatten TI-TXT at `0xE000`. It must honor every `@address` record and send the final short block with its real LEN rather than padding across protected boundaries.
