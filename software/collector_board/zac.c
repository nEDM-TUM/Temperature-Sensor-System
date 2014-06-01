#include "zac.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#undef BANK
#define BANK 1
#include "zac_template.c"
#undef BANK
#define BANK 2
#include "zac_template.c"

uint8_t zac_sampleAll(uint8_t (* buffer)[5]){
	uint8_t s;
	uint8_t i;
	uint8_t connected = 0;
	for(s = 0; s<4; s++){
		sensor_pin_mask1 = (1<< (PC0+s));
		nsensor_pin_mask1 = ~(1<< (PC0+s));
		sensor_pin_mask2 = (1<< (PD2+s));
		nsensor_pin_mask2 = ~(1<< (PD2+s));
		meassure_start_bank1();
		meassure_start_bank2();
		_delay_ms(120);
		if(meassure_stop_bank1()){
			connected |= (1<<s);
		}
		//printf("icount = %u\n\r", icount);
		if(meassure_stop_bank2()){
			connected |= (1<<(4+s));
		}

		for(i=0;i<5;i++){
			buffer[s][i] = bytearr_bank1[i];
			buffer[4+s][i] = bytearr_bank2[i];
		}
	}
	return connected;
}

void zac_init(){
	// TODO: rename this
	// TODO: initialize GPIO pins correctly here
	int_init1();
	int_init2();
}
