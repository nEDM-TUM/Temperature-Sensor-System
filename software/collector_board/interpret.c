#include "interpret.h"


int16_t interpret_analyzeTSIC(uint8_t * buf){
	uint16_t result;
	uint8_t err = 0;
	uint8_t resth = ((buf[2]<<5) | (buf[3]>>3));
	uint8_t restl = ((buf[3]<<7) | (buf[4]>>1));
	if (!checksum_checkParity(restl, buf[4] & 0x1) ){
		err = 1;
		//printf("PARITY ERROR low\n\r");
	}
	if (!checksum_checkParity(resth, (buf[3]>>2) & 0x1 ) ){
		err = 1;
		//printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		err = 1;
		//printf("FORMAT ERROR\n\r");
	}
  result =( resth <<8)|restl;

  int32_t result32 = (int32_t)(result);

  int32_t cels = result32*100L*70L/2047L - 1000L;

	if(err){
		return 0;
	}else{
		return (int16_t)cels;
	}

}

int16_t interpret_analyzeHYTtemp(uint8_t * buf){
	uint16_t data;
	int32_t data32;
	int32_t result;
	uint8_t tempH = buf[2];
	uint8_t tempL = buf[3];
  data = ((tempH<<6) | (tempL>>2));
  data32 = (int32_t)(data);
  result = data32*16500L;
  result = (result >> 14) - 4000L;
	return(int16_t)result;
}

int16_t interpret_analyzeHYThum(uint8_t * buf){
	uint16_t data;
	int32_t data32;
	int32_t result;
	uint8_t capH = buf[0] & ~((1<<7)|(1<<6));
	uint8_t capL = buf[1];
  data= (capH << 8) | capL;
  data32 = (int32_t)(data);
  result = data32*10000L;
  result = result >> 14;
	return(int16_t)result;
}

void interpret_detectPrintSingle(uint8_t * data){
	if (!(data[0] & (1<<7))){
		//printf("received: \n\r");
		//printarray(data, 5);
		//printf("\n\r");
		// this is a humidity sensor
    // check crc checksum:
		//printf("verify crc...\n\r");
    if (checksum_computeCRC(data, 4, data[4])!=0){
      printf("CRC error\n\r");
    }
		//printf("done\n\r");
		int16_t cels = analyze_hum_temp(data);
		int16_t hum = analyze_hum_hum(data);
		printf(" T = %d, H = %d", cels, hum);

	}else{
		// this is a temperature sensor
		int16_t cels = analyze(data);
		printf("T = %d", cels);
	}
}

void interpret_detectPrintAll(uint8_t * data){
	uint8_t s;
	for(s = 0; s<8; s++){
		printf("P%u: ", s+1);
		if(connected & (1<<s) ){
			interpret_detectPrintSingle(data[s]);
		}else{
			printf("X = xxxx");
		}
		printf(" | ");
	}
  printf("\n\r");

}
