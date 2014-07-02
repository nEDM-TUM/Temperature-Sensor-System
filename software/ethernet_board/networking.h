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

//json formating
#define JSON_PREFIX "{\"type\":\"data\",\"value\":{"
#define JSON_PREFIX_LEN 24
#define JSON_TEMP "\"b%03ds%01dTEMP\":%03d.%02d"
#define JSON_TEMP_LEN 19
#define JSON_TEMP "\"b%03ds%01dTEMP\":%03d.%02d"
#define JSON_HUM "\"b%03ds%01dHUM\":%03d.%02d"
#define JSON_HUM_LEN 18
#define JSON_OUTPUT board_addr, sensor_index, value/100, value%100
extern uint8_t ui_state;

extern uint8_t data_request[MAX_SERVER_SOCK_NUM];
extern uint8_t db_response_request[MAX_SERVER_SOCK_NUM];
extern uint32_t measure_interval;
extern void (*twi_access_fun)();

void net_beginService(void);
void net_setupServer(void);

void net_loop();

void net_dataAvailable(struct dummy_packet * received, uint8_t src_addr);
void net_sendResultToDB(struct dummy_packet *packets, uint8_t src_addr);
#endif

