#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#define MAX_SERVER_SOCK_NUM 3
#define MAX_CMD_LEN 40
#define MAX_PARAM_LEN 6
#define MAX_PARAM_COUNT 6
#define MAX_RESPONSE_LEN 5
#define DEFINED_CMD_COUNT 2

void beginService(void);
void serve(void);
void setupServer(void);
#endif

