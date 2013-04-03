#include "ez430U.h"
#include "dht11.h"
/*
 * main.c
 */

int main(void) {

	dht11Data_t sensorData;
	uint16_t tusbSignature;

	unsigned char formatedString[] = "\rT: XX H: XX";		// formated string for output

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	// init clock system (SMCLK = MCLK = XT2 = 12MHz)
	initCLK();

	// set all P4 pins to input
	P4DIR = 0x00;
	P4OUT = 0x00;

	// find out if tusb3410 will use default values
	disableTUSB3410();	// -> avoid access from second I2C master
	enableI2C();
	readI2Cmemory((uint16_t) 0x00, (uint8_t*) &tusbSignature, 2);	// read first two bytes
	if (tusbSignature == 0x3410) {
		writeI2Cmemory((uint16_t) 0x0000, 0xFFFF);	// overwrite first two bytes to 0xFFFF
	}

	disableI2C();
	enableTUSB3410();
	initUART0();

	// enter main loop
	while (1) {
		// wait 1s in LPM0
		initTimer();
		__bis_SR_register(LPM0_bits | GIE);

		// send data only if CRC of sensor data is correct
		if ( getData(&sensorData) == 0 ) {
			int2String(sensorData.formated.intTemperature, &formatedString[4]);		// overwrite first XX in string
			int2String(sensorData.formated.intHumidity, &formatedString[10]);		// overwrite second XX in string
			sendString(formatedString, sizeof(formatedString));
		}
	}

	// let's cross fingers we will never reach this
	return 0;
}

