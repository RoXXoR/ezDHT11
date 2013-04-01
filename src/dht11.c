/*
 * dht11.c
 *
 *  Created on: Mar 20, 2013
 *      Author: roxxor
 *
 */

#include "dht11.h"

uint8_t verifyChecksum(dht11Data_t* recvData) {
	uint8_t calcChecksum;
	calcChecksum = recvData->formated.decHumidity
			+ recvData->formated.decTemperature + recvData->formated.intHumidity
			+ recvData->formated.intTemperature;
	return calcChecksum ^ recvData->formated.checkSum; // should be 0 in case of success otherwise the wrong bits are set to 1
}

uint16_t prevCCR = 0;
uint8_t receivedBit = 0;
uint64_t receivedData = 0;
uint8_t initReceived = 0;

void initGlobals() {
	prevCCR = 0;
	receivedBit = 0;
	receivedData = 0;
	initReceived = 0;
}


uint8_t getData(dht11Data_t* recvData) {
	DHTPOUT &= ~DHTPIN;

	initGlobals();
	// Config Timer
	TB0CCTL0 = CCIE;                         	// TRCCR0 interrupt enabled
	TB0CCR0 = 54000;							// 18ms @ 3MHz
	TB0CTL = TBSSEL_2 | ID_2 | MC_1 | TBCLR;	// SMCLK/4 = 3MHz, upmode, clear TBR
	DHTPDIR |= DHTPIN;							// set to output

	__bis_SR_register(LPM0_bits + GIE);

	recvData->rawData = receivedData;

	return verifyChecksum(recvData); 			// data is only valid if checkSum is OK
}

#pragma vector=TIMERB0_VECTOR
__interrupt void TIMER0_BO_ISR() {
	DHTPOUT |= DHTPIN;							// drive DHT to high
	TB0CTL = 0;									// stop timer
	DHTPDIR &= ~DHTPIN;							// set to DHT pin to input
	DHTPSEL |= DHTPIN;							// enable CCIxA
	TB0CCTL2 = CM_2 | CAP | CCIE | CCIS_0;		// capture on falling edge
	TB0CTL = TBSSEL_2 | ID_2 | MC_2 | TBCLR;	// SMCLK/4, cont., clear TBR
}

#pragma vector=TIMERB1_VECTOR
__interrupt void TIMER0_B1_ISR() {
	if (!initReceived) {	// DHT Init signal
		if (510 < (TBCCR2 - prevCCR)) {			// 170us @ 3MHz
			initReceived = 1;
		}
	} else {
		receivedData <<= 1;
		if (375 < (TBCCR2 - prevCCR)) {			// 100
			receivedData |= 1;
		}
		receivedBit++;
		if (receivedBit >= 40) {				// End signal
			TB0CTL = 0;		// stop timer
			__bic_SR_register_on_exit(LPM4_bits + GIE);
		}
	}
	prevCCR = TBCCR2;
	TB0CCTL2 &= ~CCIFG;
}
