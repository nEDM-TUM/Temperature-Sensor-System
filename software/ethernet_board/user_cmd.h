#ifndef USER_CMD_H
#define USER_CMD_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#define MAX_CMD_LEN 40

struct cmd{
  const char * name;
  void (*handle)(void);
  // TODO add option
};


void handleCMD(uint8_t sock);
#endif
