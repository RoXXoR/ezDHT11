#include <stdint.h>
#include <msp430.h>

// Slave Address of I2C EEPROM
#define EEPROM_I2CSA	0x50	// 0b101_0000

// init clock system (SMCLK = MCLK = XT2 = 12MHz)
void initCLK();

void initUART0();
uint8_t sendString(unsigned char* string, uint8_t length);
void int2String(uint8_t input, unsigned char* output);

void enableTUSB3410();
void disableTUSB3410();

void initTimer();

void enableI2C();
void disableI2C();
uint8_t readI2Cmemory(uint16_t start_address, uint8_t* data, uint8_t size);
uint8_t writeI2Cmemory(uint16_t start_address, uint16_t word);
