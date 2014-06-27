#undef TEMPL_TMR_OVF_VECT
#undef TEMPL_PCINT_VECT
#undef TEMPL_TCCRA
#undef TEMPL_TCCRB
#undef TEMPL_PCMSK
#undef TEMPL_TCNT
#undef TEMPL_PIN
#undef TEMPL_TCRIT
#undef TEMPL_TVAL
#undef TEMPL_LOWTIME
#undef TEMPL_SENSOR_PIN
#undef TEMPL_BYTE_ARRAY
#undef TEMPL_START_MEASSURE
#undef TEMPL_STOP_MEASSURE
#undef TEMPL_PCIF
#undef TEMPL_PCIE
#undef TEMPL_START_TIMER
#undef TEMPL_INIT
#undef TEMPL_TIMSK // Timer/Counter Interrupt Mask Rigister
#undef TEMPL_TIMSK_TOIE // Timer/Counter Overflow Interrupt Enable bit
#undef TEMPL_N_SENSOR_PIN
#if BANK == 1
	uint8_t lowtime1;
	uint8_t bytearr_bank1[5];
	uint8_t sensor_pin_mask1 = (1<<PC0);
	uint8_t nsensor_pin_mask1 = ~(1<<PC0);
	#define TEMPL_TMR_OVF_VECT TIMER2_OVF_vect
	#define TEMPL_PCINT_VECT PCINT1_vect
	#define TEMPL_TCCRA TCCR2A
	#define TEMPL_TCCRB TCCR2B
	#define TEMPL_PCMSK PCMSK1
	#define TEMPL_TCNT TCNT2
	#define TEMPL_PIN PINC
	#define TEMPL_TCRIT OCR2A
	#define TEMPL_TVAL OCR2B
	#define TEMPL_LOWTIME lowtime1
	#define TEMPL_SENSOR_PIN sensor_pin_mask1
	#define TEMPL_N_SENSOR_PIN nsensor_pin_mask1
	#define TEMPL_BYTE_ARRAY bytearr_bank1
	#define TEMPL_START_MEASSURE meassure_start_bank1
	#define TEMPL_STOP_MEASSURE meassure_stop_bank1
	#define TEMPL_PCIF PCIF1
  #define TEMPL_PCIE PCIE1 //enable interrupt for PCINT[14...8]

	#define TEMPL_START_TIMER TEMPL_TCCRB = (1<<CS22)

  #define TEMPL_INIT int_init1
  #define TEMPL_TIMSK TIMSK2
  #define TEMPL_TIMSK_TOIE TOIE2
#elif BANK == 2
	uint8_t lowtime2;
	uint8_t bytearr_bank2[5];
	uint8_t sensor_pin_mask2 = (1<<PD2);
	uint8_t nsensor_pin_mask2 = ~(1<<PD2);
	#define TEMPL_TMR_OVF_VECT TIMER0_OVF_vect
	#define TEMPL_PCINT_VECT PCINT2_vect
	#define TEMPL_TCCRA TCCR0A
	#define TEMPL_TCCRB TCCR0B
	#define TEMPL_PCMSK PCMSK2
	#define TEMPL_TCNT TCNT0
	#define TEMPL_PIN PIND
	#define TEMPL_TCRIT OCR0A
	#define TEMPL_TVAL OCR0B
	#define TEMPL_LOWTIME lowtime2
	#define TEMPL_SENSOR_PIN sensor_pin_mask2
	#define TEMPL_N_SENSOR_PIN nsensor_pin_mask2
	#define TEMPL_BYTE_ARRAY bytearr_bank2
	#define TEMPL_START_MEASSURE meassure_start_bank2
	#define TEMPL_STOP_MEASSURE meassure_stop_bank2
	#define TEMPL_PCIF PCIF2
  #define TEMPL_PCIE PCIE2 //enable interrupt for PCINT[7...0]

	#define TEMPL_START_TIMER TEMPL_TCCRB = (1<<CS01) | (1<<CS00)

  #define TEMPL_INIT int_init2
  #define TEMPL_TIMSK TIMSK0
  #define TEMPL_TIMSK_TOIE TOIE0
#else

#endif

void TEMPL_INIT (){
	// enable interrupt will be done when starting measurement
	PCICR |= ( 1 << TEMPL_PCIE ); //Pin change interrupt enable
	// enable timer overflow interrupt
	TEMPL_TIMSK = ( 1 << TEMPL_TIMSK_TOIE ); 
  // Timer init (reset to default)
	TEMPL_TCCRA = 0;
	TEMPL_TCCRB = 0;
}

// Timer overflow interrupt handler
ISR(TEMPL_TMR_OVF_VECT){
	TEMPL_TCCRB = 0; //disable timer
	// we use the fact, that TSIC always sends 5 zeroes at the beginning
	// to determine, if the transmission was complete
	// as it might happen, that we start meassuring just before the
	// second start bit. We then have to wait for the next transmission
  //uint8_t b = !(TEMPL_BYTE_ARRAY [2] & (1<<2));
  //printf("b: 0x%x\n\r",b);
#ifdef DEBUG
	printf ("%x %x %x %d\n\r", TEMPL_BYTE_ARRAY [2], TEMPL_BYTE_ARRAY [1], TEMPL_BYTE_ARRAY [0], s);
#endif
	if (!(TEMPL_BYTE_ARRAY [2] & (1<<2)) || !(TEMPL_BYTE_ARRAY [0] & (1<<7))){
		// we have seen a start bit AND are finished with transmission
		// => stop measurement
		// disable interrupt for this pin
		// -> unmask:
		TEMPL_PCMSK &= TEMPL_N_SENSOR_PIN;
		// TODO: check, if we have to clear possibly arrived interrupts
		// this this would be done by writing 1 to the corresponding bit:
		// PCIFR |= (1<< PCIF1);

	}else{
		// we will receive the rest of the data in the next iteration
		// hence we have to clean up the buffer, as we have to have a defined state
		// to distinguish between humidity and temperature sensor
		TEMPL_BYTE_ARRAY [4] = 0xff;
		TEMPL_BYTE_ARRAY [3] = 0xff;
		TEMPL_BYTE_ARRAY [2] = 0xff;
		TEMPL_BYTE_ARRAY [1] = 0xff;
		TEMPL_BYTE_ARRAY [0] = 0xff;
		// we also have to set the timer value to initial value
		// this has to be done for spike detection to work correctly
		// this is needed, that we do not get a start bit at the beginning
		// TODO: this triggers an immediate overflow
		// it might be a better idea to set this to 128
		TEMPL_TCNT = 0xff;
		// low time mightalso be 0xff -> this might cause problems
		// if transmission starts again. so set it to default state 0:
		TEMPL_LOWTIME = 0;
	}
}

// Interrupt handler
// IDEA: use OCRnA/B for TEMPL_TCRIT / TEMPL_TVAL storage to reduce stack usage
ISR(TEMPL_PCINT_VECT){
  //icount++;
	// start timer 2: enable with prescaler 64
	// the order AND position of the following 3 instructions
	// is highly important for propper timing
	// The timer should be started as soon as possible
	// After that the timer value should be reset to 0 as soon as possible too
	TEMPL_START_TIMER;
  // Read timer 2
	TEMPL_TVAL = TEMPL_TCNT;
  // Reset timer 2
	TEMPL_TCNT = 0;
  
#ifdef SPIKEDETECT
	// this should filter out vibrations on the wire
	// FIXME: check if we need this. I was not able to construct
	// a situation, where this was called in a lab environment.
	// But maybe with long wires we might have spikes.
	// We need to check, if timer is already running,
	// as this might be the first interrupt, where
	// TEMPL_TVAL is 0 by default.
	if (TEMPL_TVAL < 2){
		//printf("INSTABILITY!\n\r");
		return;
	}
#endif

	//TEMPL_TCCRB = (1<<CS22);
	if(TEMPL_PIN & TEMPL_SENSOR_PIN){
#ifdef DEBUG
		timesr[cr] = TEMPL_TVAL;
		cr++;
#endif
		// PC0 is 1 -> rising edge
		// this is the time, the signal was low:
		TEMPL_LOWTIME = TEMPL_TVAL;
		if (TEMPL_TCRIT != 0xff){
			// here we receive a TEMPL_BYTE_ARRAY.
			// we now know, that we have received a start bit in the past
			// shift all 3 TEMPL_BYTE_ARRAYs to the left:
			// shift first TEMPL_BYTE_ARRAY -> carry flag is the msb
			TEMPL_BYTE_ARRAY [4] = (TEMPL_BYTE_ARRAY [4] << 1);
			// use "rol" to shift the other two bits with carry
			asm("rol %0" : "=r" (TEMPL_BYTE_ARRAY [3]) : "0" (TEMPL_BYTE_ARRAY [3]));
			asm("rol %0" : "=r" (TEMPL_BYTE_ARRAY [2]) : "0" (TEMPL_BYTE_ARRAY [2]));
			asm("rol %0" : "=r" (TEMPL_BYTE_ARRAY [1]) : "0" (TEMPL_BYTE_ARRAY [1]));
			asm("rol %0" : "=r" (TEMPL_BYTE_ARRAY [0]) : "0" (TEMPL_BYTE_ARRAY [0]));
			// now we write the received bit:
			// if this (rising) edge happened before the critical time,
			// we extracted from the start bit, we are dealing with a '1',
			// else with a '0'
			TEMPL_BYTE_ARRAY [4] = TEMPL_BYTE_ARRAY [4] ^ (TEMPL_TVAL < TEMPL_TCRIT);
		}
	}else{
#ifdef DEBUG
		times[cf] = TEMPL_TVAL;
		cf++;
#endif
		// PC0 is 0 -> falling edge
		// we check for a start bit:
		// A start bit should have a duty cycle of 50%
		// We also accept bits as start bits within a small mergin
		// If we have detected a start bit, we use the high time of the
		// bit as the critical time TEMPL_TCRIT
		if(TEMPL_LOWTIME>TEMPL_TVAL){
			if(TEMPL_LOWTIME-TEMPL_TVAL < 3){
				TEMPL_TCRIT = TEMPL_TVAL;
#ifdef DEBUG
				s++;
				tcritb = TEMPL_LOWTIME;
				tcrita = TEMPL_TVAL;
#endif
			}
		}else{
			if(TEMPL_TVAL-TEMPL_LOWTIME < 3){
				TEMPL_TCRIT = TEMPL_TVAL;
#ifdef DEBUG
				s++;
				tcritb = TEMPL_LOWTIME;
				tcrita = TEMPL_TVAL;
#endif
			}
		}
	}
}

void TEMPL_START_MEASSURE (){
	TEMPL_TCRIT = 0xff;
	// this is needed, that we do not get a start bit at the beginning
	// TODO: this triggers an immediate overflow
	// it might be a better idea to set this to 128
	TEMPL_TCNT = 0xff;
	// this is needed, that we do not get a start bit at the beginning
	TEMPL_LOWTIME = 0;
	// this will be shifted through and help us to determine
	// if we have received enough bits
	TEMPL_BYTE_ARRAY [4] = 0xff;
	TEMPL_BYTE_ARRAY [3] = 0xff;
	TEMPL_BYTE_ARRAY [2] = 0xff;
	TEMPL_BYTE_ARRAY [1] = 0xff;
	TEMPL_BYTE_ARRAY [0] = 0xff;
	// if in the meantime interrupts have arrived -> clear them
	PCIFR |= (1<< TEMPL_PCIF);
	//enable interrupt for PCINT[14...8]
	TEMPL_PCMSK = TEMPL_SENSOR_PIN; // PCINT8 -> PC0
	//PCICR = (1<< PCIE1);
	// wait >100ms for meassurement to complete
	// there should not happen too many interrupts, as they extend _delay_ms
}

uint8_t TEMPL_STOP_MEASSURE (){
	// wait >100ms for meassurement to complete
	// there should not happen too many interrupts, as they extend _delay_ms
	if(TEMPL_PCMSK & TEMPL_SENSOR_PIN){
		// interrupt was still enabled
		// -> meassurement was not successful
		// disable interrupts now (to be in consistent state)
		TEMPL_PCMSK &= TEMPL_N_SENSOR_PIN; // PCINT8 -> PC0
		//PCICR &= ~(1<< PCIE1);
		// return error:
		TEMPL_BYTE_ARRAY [4] =0xff;
		TEMPL_BYTE_ARRAY [3] =0xff;
		TEMPL_BYTE_ARRAY [2] =0xff;
		TEMPL_BYTE_ARRAY [1] =0xff;
		TEMPL_BYTE_ARRAY [0] =0xff;
		// printf("ERROR Meassurement not complete\n\r");
		return 0;
	}
	return 1;

}
