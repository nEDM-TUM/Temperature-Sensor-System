#ifdef __cplusplus
extern "C" {
#endif
#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H
#include <inttypes.h>

#define EEPROM_MAGIC 171

struct config {
  uint8_t mac[6] ;
  uint8_t ip[4] ;
  uint8_t subnet;
  uint8_t gw[4];
  uint16_t port;
  uint8_t ip_db[4];
  uint16_t port_db;
};

extern struct config cfg;

uint8_t config_write(struct config * cfg);
void config_read(struct config * cfg);
#endif
#ifdef __cplusplus
}
#endif
