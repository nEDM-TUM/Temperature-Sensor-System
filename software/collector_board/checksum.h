#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <inttypes.h>

uint8_t checksum_checkParity(uint8_t value, uint8_t parity);
uint8_t checksum_computeCRC(uint8_t * data, uint8_t len, uint8_t crc);

#endif
