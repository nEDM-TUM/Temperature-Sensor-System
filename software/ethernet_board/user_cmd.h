#ifndef USER_CMD_H
#define USER_CMD_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#define MAX_CMD_LEN 10

// CMD handler state
#define FAILED_PARAMS_PARSE 0
#define SUCCESS_PARAMS_PARSE 1 
#define SUSPEND 2
#define NO_PARAMS_PARSE 3

struct cmd{
  const char * name;
  int8_t (*handle)(void);
  const char * param_format PROGMEM;
};


uint8_t ui_handleCMD(uint8_t sock);
#endif
