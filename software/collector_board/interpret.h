#ifndef INTERPRET_H
#define INTERPRET_H

#include <inttypes.h>

#define PACKET_TYPE_TSIC 0
#define PACKET_TYPE_HYT 1

struct packet_header{
	// length of payload (without header and without crc)
	uint8_t len	: 3;
	uint8_t	type	:	2;
	uint8_t	connected	:	1;
	uint8_t	error	: 2;
};

struct tsic_packet{
	struct packet_header header;
	int16_t temperature;
	uint8_t crc;
};
struct hyt_packet{
	struct packet_header header;
	int16_t temperature;
	int16_t humidity;
	uint8_t crc;
};


uint8_t interpret_analyzeTSIC(uint8_t * buf, int16_t * result);
int16_t interpret_analyzeHYTtemp(uint8_t * buf);
int16_t interpret_analyzeHYThum(uint8_t * buf);
void interpret_detectPrintSingle(uint8_t * data);
void interpret_detectPrintAll(uint8_t (* data)[5], uint8_t connected);
uint8_t interpret_generatePacket(uint8_t * data, uint8_t connected, uint8_t * buffer);
#endif
