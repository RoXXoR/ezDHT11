#include "ez430U.h"
/*
 * main.c
 */
void initCLK();
void initUART0();
void enableTUSB3410();
void disableTUSB3410();
void initTimer();

int main(void) {
	enum {
		readI2Cmem, sendUART
	} command;

	volatile uint8_t received[64];
	uint8_t i;
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	command = readI2Cmem;

	initCLK();

	switch (command) {
	case (readI2Cmem):
		// utilizes USART0 in I2C mode
		disableTUSB3410();	// -> avoid access from second I2C master
		enableI2C();
		readI2Cmemory((uint16_t) i, (uint8_t*)received, 64);
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

