#ifndef ETHERNET_TWI_H
#define ETHERNET_TWI_H

#include <inttypes.h>

#define SLA 42

#define IDLE 0
#define COMMAND 1
#define WAIT_ADDRESS 2
#define START_MEASUREMENT 4
#define ADDR_FIN 5

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2

uint8_t bufferpointer;
uint8_t * end_of_transmit;

uint8_t measurement_data[8][5];
uint8_t interpreted_data[48];

void twi_init(uint8_t addr);
void twi_handle(void);
#endif
