#ifndef MAIN_H
#define MAIN_H

#include <inttypes.h>

#define SLA 0x78
#define CRC8 49



void printarray(uint8_t * arr, uint8_t len);
void print_interpreted_data(uint8_t ** data);
void loop(void);
void io_init(void);
int main (void);
#endif
