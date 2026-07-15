#ifndef TIMELIB_H
#define TIMELIB_H

#include <stdio.h>

uint16_t timeLib_isTooLong(uint16_t *counter, uint16_t timeout){
  *counter = *counter + 1;
  if(*counter > timeout){
    return 1;
  }
  else{
    return 0;
  }
}

void timeLib_clearCounter(uint16_t *counter){
  *counter &= 0x0000;
}

#endif 