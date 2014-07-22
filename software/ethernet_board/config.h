#ifdef __cplusplus
extern "C" {
#endif
#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H
#include <inttypes.h>

#define STR1(x)  #x
#define STR(x)  STR1(x)

#define EEPROM_MAGIC 171
#define DB_COOKIE_SIZE 100
#define DB_NAME_SIZE 50
#define DB_DOC_SIZE 25
#define DB_FUNC_SIZE 25

struct config {
  uint8_t mac[6] ;
  uint8_t ip[4] ;
  uint8_t subnet;
  uint8_t gw[4];
  uint16_t port;
  int8_t send_db;
  uint8_t ip_db[4];
  uint16_t port_db;
  char cookie_db[DB_COOKIE_SIZE];
  char name_db[DB_NAME_SIZE];
  char doc_db[DB_DOC_SIZE];
  char func_db[DB_FUNC_SIZE];
};

extern struct config cfg;

uint8_t config_write(struct config * cfg);
void config_read(struct config * cfg);
#endif
#ifdef __cplusplus
}
#endif
