#ifndef USER_CMD_H
#define USER_CMD_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#define MAX_CMD_LEN 40

struct cmd{
  const char * name;
  void (*handle)(uint8_t, char*);
};

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

// XXX cmdLen should be always the length of the registered cmd array below! 
#define DEFINED_CMD_COUNT 10;
struct cmd cmds[]={
  {"ip", handleIp},
  {"port", handlePort},
  {"mac", handleMac},
  {"gw", handleGw},
  {"reset", handleReset},
  {"ip-db", handleIpDB},
  {"port-db", handlePortDB},
  {"twiaddr", handleTwiaddr},
  {"s", handleScan},
  {"m", handleDoMeasurement}
};

void handleCMD(uint8_t sock);
#endif
