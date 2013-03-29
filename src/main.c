#include "ez430U.h"
/*
 * main.c
 */
void initCLK();
void initUART0();
void enableTUSB3410();
void disableTUSB3410();
void initTimer();

volatile uint8_t received[0x800];

int main(void) {
	enum {
		readI2Cmem, sendUART
	} command;
	uint16_t i, ii;
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	command = readI2Cmem;

	initCLK();

	switch (command) {
	case (readI2Cmem):
		// utilizes USART0 in I2C mode
		disableTUSB3410();	// -> avoid access from second I2C master
		enableI2C();

		for (i = 0; i < 0x800; i += 0x80) {
			readI2Cmemory((uint16_t) i, (uint8_t*) &received[i], 0x80);
		}
		__no_operation();		// put breakpoint here to read out memory
		for (i = 0x800; i < 0x1000; i += 0x80) {
			readI2Cmemory((uint16_t) i, (uint8_t*) &received[i-0x800], 0x80);
		}
		__no_operation();		// put breakpoint here to read out memory
		for (i = 0x1000; i < 0x1800; i += 0x80) {
			readI2Cmemory((uint16_t) i, (uint8_t*) &received[i-0x1000], 0x80);
		}
		__no_operation();		// put breakpoint here to read out memory
		for (i = 0x1800; i < 0x2000; i += 0x80) {
			readI2Cmemory((uint16_t) i, (uint8_t*) &received[i-0x1800], 0x80);
		}
		__no_operation();		// put breakpoint here to read out memory
		disableI2C();
		enableTUSB3410();	// -> re-enable usb again
		break;
	case (sendUART):
		break;
	default:
		break;
	}

//	initTUSB3410();
//	initUART0();
//	initTimer();
//	__bis_SR_register(LPM0_bits | GIE);

	while (1)
		;
	return 0;
}

