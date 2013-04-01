#include "ez430U.h"
#include "dht11.h"
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
		readI2Cmem, defaultTUSB, restoreTUSB, sendUART, readDHT
	} command;
	uint16_t i;
	dht11Data_t sensorData;
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	command = readDHT;

	initCLK();
	P4DIR = 0x00;
	P4OUT = 0x00;

	switch (command) {
	case (readI2Cmem):
		// utilizes USART0 in I2C mode
		disableTUSB3410();	// -> avoid access from second I2C master
		enableI2C();

		for (i = 0; i < 0x800; i += 0x80) {
			readI2Cmemory((uint16_t) i, (uint8_t*) &received[i], 0x80);
		}
		__no_operation();		// put breakpoint here to read out memory
		disableI2C();
		enableTUSB3410();	// -> re-enable usb again
		break;
	case (defaultTUSB):
		// utilizes USART0 in I2C mode
		disableTUSB3410();	// -> avoid access from second I2C master
		enableI2C();
		writeI2Cmemory((uint16_t) 0x0000, 0xFFFF);
		disableI2C();
		enableTUSB3410();	// -> re-enable usb again
		break;
	case (restoreTUSB):
		// utilizes USART0 in I2C mode
		disableTUSB3410();	// -> avoid access from second I2C master
		enableI2C();
		writeI2Cmemory((uint16_t) 0x0000, 0x3410);
		disableI2C();
		enableTUSB3410();	// -> re-enable usb again
		break;
	case (sendUART):
		initUART0();
		initTimer();
		enableTUSB3410();
		__bis_SR_register(LPM0_bits | GIE);
		__no_operation();
		break;
	case (readDHT):
		getData(&sensorData);
		__no_operation();
		break;
	default:
		break;
	}

	while (1)
		;
	return 0;
}

