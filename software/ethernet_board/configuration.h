#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "user_cmd.h"

#define MAX_SERVER_SOCK_NUM 3

// set eeprom to store the whole configuration
#define EEPROM

struct config {
  uint8_t mac[6] ;
  uint8_t ip[4] ;
  uint8_t subnet;
  uint8_t gw[4];
  uint16_t port;
  uint8_t ip_db[4];
  uint16_t port_db;
};

extern config cfg;

extern uint8_t data_request[MAX_SERVER_SOCK_NUM];
extern uint32_t measure_interval;

void beginService(void);
void serve(void);
void setupServer(void);

void ui_loop();

void dataAvailable(struct dummy_packet * received, uint8_t src_addr);
#endif

