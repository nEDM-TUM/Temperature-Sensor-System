#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "user_cmd.h"

#define MAX_SERVER_SOCK_NUM 3

void beginService(void);
void serve(void);
void setupServer(void);

void ui_loop();

#endif

