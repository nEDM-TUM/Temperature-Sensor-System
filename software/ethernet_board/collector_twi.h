#ifndef COLLECTOR_TWI_H
#define COLLECTOR_TWI_H
#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
	
#define CRC8 49

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2
#define CMD_LED_ON 3
#define CMD_LED_OFF 4

#define TWI_RCV_FIN 0
#define TWI_RCV_START 1 
#define TWI_RCV_RECEIVE 2
#define TWI_RCV_ERROR 3

extern uint8_t scanresults[20];
extern uint8_t num_boards;

uint8_t twi_try_lock_bus();
void twi_free_bus();
void twi_interpret(uint8_t * data);
void printarray(uint8_t * arr, uint8_t len);
uint8_t twi_computeCRC(uint8_t * data, uint8_t len, uint8_t crc);
uint8_t twi_wait(void);
uint8_t twi_wait_timeout(uint16_t milliseconds);
uint8_t twi_start(void);
uint8_t twi_start_measurement(uint8_t addr);
uint8_t twi_receive_data(uint8_t address, uint8_t * buffer, uint8_t len);
uint8_t twi_try_receive_data(uint8_t address, uint8_t * buffer, uint8_t len, uint8_t state);
uint8_t twi_scan(uint8_t * result, uint8_t max_results);
uint8_t twi_set_address(uint8_t addr, uint8_t new_addr);
uint8_t twi_set_led(uint8_t addr, uint8_t on, uint8_t num);


#ifdef __cplusplus
}
#endif
#endif
