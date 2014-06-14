#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#define MAX_SERVER_SOCK_NUM 3
#define MAX_CMD_LEN 40
#define MAX_PARAM_LEN 6
#define MAX_PARAM_COUNT 6
#define DEFINED_CMD_COUNT 2

void beginService(void);
void serve(void);
void setupServer(void);

void ui_loop();


void handleIp(uint8_t sock, char* paramsStr);
void handlePort(uint8_t sock, char* paramsStr);
void handleMac(uint8_t sock, char* paramsStr);
void handleGw(uint8_t sock, char* paramsStr);
void handleReset(uint8_t sock, char* paramsStr);
void handleIpDB(uint8_t sock, char* paramsStr);
void handlePortDB(uint8_t sock, char* paramsStr);
void handleTwiaddr(uint8_t sock, char* paramsStr);
void handleDoMeasurement(uint8_t sock, char* paramsStr);
void handleScan(uint8_t sock, char * paramsStr);

#endif

