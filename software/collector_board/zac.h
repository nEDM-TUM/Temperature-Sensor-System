#ifndef ZAC_H
#define ZAC_H

#include <inttypes.h>

#ifdef DEBUG
uint8_t s = 0;
uint8_t tcrita;
uint8_t tcritb;
uint8_t times[30];
uint8_t cf = 0;
uint8_t timesr[30];
uint8_t cr = 0;
#endif

uint8_t zac_sampleAll(uint8_t * buffer);
void zac_init(void);

#endif
