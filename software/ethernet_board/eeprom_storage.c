#include "eeprom_storage.h"
#include <avr/eeprom.h>
#include <string.h>
#include <inttypes.h>

struct config newcfg;

uint8_t config_write(struct config * pcfg){
	void * eeaddr = eeprom_read_word((void *)1);
	while(1){
		eeprom_update_block(pcfg, eeaddr, sizeof(struct config));
		eeprom_read_block(&newcfg, eeaddr, sizeof(struct config) );
		if( ! memcmp(&newcfg, pcfg, sizeof(struct config)) ){
			return 1;
		}
		eeaddr += sizeof(struct config);
    eeprom_update_word((void *)1, (uint16_t)eeaddr);
		if(eeaddr >= (void*)(1023 - sizeof(struct config) ) ){
			break;
		}
	}
	return 0;
}

void config_read(struct config * pcfg){
	uint16_t eeaddr = eeprom_read_word((void *)1);
	eeprom_read_block(pcfg, (void *)eeaddr, sizeof(struct config) );
	return;
}
