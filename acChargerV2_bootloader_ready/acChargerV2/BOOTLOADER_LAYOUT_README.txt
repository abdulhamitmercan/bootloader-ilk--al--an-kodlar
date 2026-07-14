acChargerV2 bootloader-compatible linker layout

Active linker: lnk_msp430f67751.cmd

Memory layout:
  Application RAM       0x1C00 - 0x5B7F
  RAM vector reserve    0x5B80 - 0x5BFF
  Virtual app vectors   0xDF80 - 0xDFFF
  Application low flash 0xE000 - 0xFDFF
  Bootloader protected  0xFE00 - 0xFFFF
  Application high flash 0x10000 - 0x2BFF7

Bootloader must use:
  APP_VECTOR_START = 0xDF80
  APP_VECTOR_RESET = 0xDFFE
  APP_START = 0xE000
  RAM_VECTOR_START = 0x5B80
  PROTECTED_START = 0xFE00
  PROTECTED_END = 0x10000

Important:
- Clean and rebuild the project in CCS.
- Generate a new acChargerV2.txt after rebuilding.
- Do not use the old Debug/acChargerV2.txt; it still contains the previous 0xC000/0xFFxx layout.
- Verify the new map before UART update.
