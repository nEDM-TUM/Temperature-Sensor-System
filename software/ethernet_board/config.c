#include "config.h"
#include <avr/eeprom.h>
#include <string.h>

struct config cfg;

struct {
	uint8_t magic;
	uint16_t start_pointer;
	struct config init_cfg;
	
} eevars EEMEM = {
	.magic = EEPROM_MAGIC,
	.start_pointer = 3,
	.init_cfg ={
  /*.mac = */{0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B},
  /*.ip = */{10,0,1, 100},
  /*.subnet = */16,
  /*.gw = */{10, 0, 1, 1},
  /*.port = */8888,
  /*.ip_db = */{10, 0, 1, 99},
  /*.port_db = */8888
						}
};

uint8_t config_write(struct config * pcfg){
	void * eeaddr = eeprom_read_word((void *)1);
	while(1){
		eeprom_update_block(pcfg, eeaddr, sizeof(struct config));
		// FIXME: this wastes lots of RAM on stack.
		// TODO: implement memcmp for eeprom
		struct config newcfg;
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
