#ifndef NETWORKING_H
#define NETWORKING_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "user_cmd.h"
#include "collector_twi.h"

#define DB_CLIENT_SOCK 0
#define FIRST_SERVER_SOCK 1
#define MAX_SERVER_SOCK_NUM 3
#define UI_READY 0
#define UI_TWILOCK 1

// set eeprom to store the whole configuration
//#define EEPROM


extern uint8_t ui_state;

extern uint8_t data_request[MAX_SERVER_SOCK_NUM];
extern uint32_t measure_interval;
extern void (*twi_access_fun)();

void beginService(void);
void serve(void);
void setupServer(void);

void ui_loop();

void dataAvailable(struct dummy_packet * received, uint8_t src_addr);
void sendResultToDB(struct dummy_packet *packets);
#endif

