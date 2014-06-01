#ifndef ETHERNET_TWI_H
#define ETHERNET_TWI_H

#include <inttypes.h>

#define SLA 0x78

#define IDLE 0
#define COMMAND 1
#define WAIT_ADDRESS 2
#define TRANSMIT 3
#define START_MEASUREMENT 4

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2

uint8_t bufferpointer;
uint8_t * end_of_transmit;

uint8_t measurement_data[8][5];
uint8_t interpreted_data[48];

void twi_init(void);
void twi_handle(void);
#endif
