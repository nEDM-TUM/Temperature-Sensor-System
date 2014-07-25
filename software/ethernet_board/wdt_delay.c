#include "wdt_delay.h"
#include <util/delay.h>
#include <avr/wdt.h>

void wdt_delay_ms(uint16_t delay){
  for(;delay>0;delay--){
    _delay_ms(1);
    wdt_reset();
  }
}
