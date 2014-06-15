#ifndef USER_CMD_H
#define USER_CMD_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#define MAX_CMD_LEN 40

struct cmd{
  const char * name;
  int8_t (*handle)(void);
  const char * param_format PROGMEM;
};


void handleCMD(uint8_t sock);
#endif
