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
	// Arduino 1
  ///*.mac = */{0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5A},
  ///*.ip = */{192, 168, 1, 61},
  ///*.subnet = */24,
  ///*.gw = */{192, 168, 1, 1},
  ///*.port = */8888,
  ///*.send_db = */0,
  ///*.ip_db = */{192., 168, 1, 9},
  ///*.port_db = */5984,
  ///*.cookie_db[DB_COOKIE_SIZE] = */"YXJkdWlub190ZW1wZXJhdHVyZV93cml0ZXI6NTNDRTREMUM6Lf5Oi3ewnCudlP4694jiaMcSXKc",
  ///*.name_db[DB_NAME_SIZE] = */"nedm%2Ftemperature_environment",
  ///*.doc_db[DB_DOC_SIZE] = */"nedm_default",
  ///*.func_db[DB_FUNC_SIZE] = */"insert_with_timestamp",
  ///*.interval = */5000,
	// Arduino 2
  /*.mac = */{0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B},
  /*.ip = */{192, 168, 1, 62},
  /*.subnet = */24,
  /*.gw = */{192, 168, 1, 1},
  /*.port = */8888,
  /*.send_db = */0,
  /*.ip_db = */{192., 168, 1, 9},
  /*.port_db = */5984,
  /*.cookie_db[DB_COOKIE_SIZE] = */"YXJkdWlub190ZW1wZXJhdHVyZV93cml0ZXI6NTNDRTREMUM6Lf5Oi3ewnCudlP4694jiaMcSXKc",
  /*.name_db[DB_NAME_SIZE] = */"nedm%2Ftemperature_environment",
  /*.doc_db[DB_DOC_SIZE] = */"nedm_default",
  /*.func_db[DB_FUNC_SIZE] = */"insert_with_timestamp",
  /*.measure_interval = */5000,
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
