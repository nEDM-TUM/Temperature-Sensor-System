#ifndef PACKET_H
#define PACKET_H

#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_TYPE_TSIC 0
#define PACKET_TYPE_HYT 1

struct packet_header{
	// length of payload (without header and without crc)
	uint8_t	type	:	2;
	uint8_t reserved	: 3;
	uint8_t	connected	:	1;
	uint8_t	error	: 2;
};

struct tsic_packet{
	struct packet_header header;
	int16_t temperature;
	uint16_t padding;
	uint8_t crc;
};

struct hyt_packet{
	struct packet_header header;
	int16_t temperature;
	int16_t humidity;
	uint8_t crc;
};

struct dummy_packet{
	struct packet_header header;
	int16_t pl1;
	int16_t pl2;
	uint8_t crc;
};
#ifdef __cplusplus
}
#endif
#endif
