#include <stdint.h>
#include <msp430.h>

void initCLK();
void initUART0();
void enableTUSB3410();
void disableTUSB3410();
void initTimer();

void enableI2C();
void disableI2C();
uint16_t readI2Cmemory();
