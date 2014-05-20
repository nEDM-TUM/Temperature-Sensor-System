#ifndef BOARD_SUPPORT_H
#define BOARD_SUPPORT_H

#include <avr/io.h>


// LEDs:
#define LED1 PB2
#define LED2 PB1
#define LED3 PB0
#define LED4 PD7

#define LED1_PORT PORTB
#define LED2_PORT PORTB
#define LED3_PORT PORTB
#define LED4_PORT PORTD

#define LED1_DDR DDRB
#define LED2_DDR DDRB
#define LED3_DDR DDRB
#define LED4_DDR DDRD

// SWITCHES:
#define SW1 PD6
#define SW1_PORT PORTD
#define SW1_DDR DDRD


#endif
