#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include "configuration.h"


uint8_t config_write(struct config * cfg);
void config_read(struct config * cfg);
#endif
