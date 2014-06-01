#ifndef MAIN_H
#define MAIN_H

#include <inttypes.h>

#define SLA 0x78
#define CRC8 49


#define IDLE 0
#define COMMAND 1
#define WAIT_ADDRESS 2
#define TRANSMIT 3
#define START_MEASUREMENT 4

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2

void printarray(uint8_t * arr, uint8_t len);
void print_interpreted_data(uint8_t ** data);
void loop(void);
void io_init(void);
int main (void);
#endif
