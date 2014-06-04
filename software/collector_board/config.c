#include "config.h"
#include <avr/eeprom.h>
#include <string.h>


struct {
	uint8_t magic;
	uint16_t start_pointer;
	uint8_t twi_addr;
	
} eevars EEMEM = {
	.magic = EEPROM_MAGIC,
	.start_pointer = 3,
	.twi_addr = 42
	
}; 

void config_check_eeprom(){
	if (eeprom_read_byte(0)!=EEPROM_MAGIC){

	}
}

void config_reset_eeprom(){
	eeprom_update_byte(0, EEPROM_MAGIC);
	eeprom_update_word(1, 3);
	eeprom_update_byte(3, 42);
}

uint8_t config_write(struct config * cfg){
	uint16_t eeaddr = eeprom_read_word(1);
	while(1){
		// 
		eeprom_update_block(cfg, eeaddr, sizeof(struct config));
		struct config newcfg;
		eeprom_read_block(&newcfg, eeaddr, sizeof(struct config) );
		if( ! memcmp(&newcfg, cfg, sizeof(struct config)) ){
			eeprom_update_word(1, eeaddr);
			return 1;
		}
		eeaddr += sizeof(struct config);
		if(eeaddr >= 1023 - sizeof(struct config)){
			break;
		}
	}
	return 0;
}

void config_read(struct config * cfg){
	uint16_t eeaddr = eeprom_read_word(1);
	eeprom_read_block(cfg, eeaddr, sizeof(struct config) );
	return;
}
