#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>

#define EEPROM_MAGIC 171

struct config {
	uint8_t twi_addr;
};

extern struct config cfg;

void config_check_eeprom(void);
void config_reset_eeprom(void);
uint8_t config_write(struct config * cfg);
void config_read(struct config * cfg);
#endif
