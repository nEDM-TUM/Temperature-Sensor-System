#ifndef MAIN_H
#define MAIN_H

#include <inttypes.h>


extern struct config cfg;

void printarray(uint8_t * arr, uint8_t len);
void loop(void);
void io_init(void);
int main (void);
#endif
