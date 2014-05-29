#ifndef MAIN_H
#define MAIN_H


#define SLA 0x78
#define CRC8 49


#define IDLE 0
#define COMMAND 1
#define WAIT_ADDRESS 2
#define TRANSMIT 3
#define START_MEASUREMENT 4

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2

uint8_t check_parity(uint8_t value, uint8_t parity);
int16_t analyze(uint8_t * buf);
uint16_t analyze_hum_temp(uint8_t * buf);
uint16_t analyze_hum_hum(uint8_t * buf);
void twi_init(void);
void handle_communications(void);
void printarray(uint8_t * arr, uint8_t len);
uint8_t verifyCRC(uint8_t * data, int8_t len);
void interpret(uint8_t * data);
void do_measurement(void);
void print_interpreted_data(uint8_t ** data);
void loop(void);
void io_init(void);
int main (void);
#endif
