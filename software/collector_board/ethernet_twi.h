#ifndef ETHERNET_TWI_H
#define ETHERNET_TWI_H

#define IDLE 0
#define COMMAND 1
#define WAIT_ADDRESS 2
#define TRANSMIT 3
#define START_MEASUREMENT 4

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2

uint8_t bufferpointer;
uint8_t cstate = IDLE;

#endif
