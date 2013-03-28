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
		readI2Cmem,
		sendUART
	} command;

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	command = readI2Cmem;

	switch (command) {
	case (readI2Cmem):
			// utilizes USART0 in I2C mode
			disableTUSB3410();	// -> avoid access from second I2C master
			enableI2C();
			readI2Cmemory();
			disableI2C();
			enableTUSB3410();	// -> re-enable usb again
			break;
	case (sendUART):
			break;
	default:
		break;
	}

	initCLK();
//	initTUSB3410();
//	initUART0();
//	initTimer();
	__bis_SR_register(LPM0_bits | GIE);
	return 0;
}



