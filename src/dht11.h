/*
 * dht11.h
 *
 *  Created on: Mar 20, 2013
 *      Author: roxxor
 */

#ifndef DHT11_H_
#define DHT11_H_

#include <stdint.h>
#include <msp430.h>

// Port PIN the DHT11 is connected to
#define DHTPOUT	P4OUT
#define DHTPDIR	P4DIR
#define DHTPSEL	P4SEL
#define DHTPIN	BIT2


typedef union {
	uint64_t rawData;
	struct {
		uint8_t checkSum;
		uint8_t decTemperature;
		uint8_t intTemperature;
		uint8_t decHumidity;
		uint8_t intHumidity;
	} formated;
} dht11Data_t;

uint8_t getData(dht11Data_t* recvData);

#endif /* DHT11_H_ */
