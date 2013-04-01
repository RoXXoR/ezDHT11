#include "ez430U.h"

uint8_t counter = 0;
uint16_t divider = 25;
uint8_t i2cDataArray[34];
uint8_t i2cDataCnt;
uint8_t *pI2CByte;

void initCLK() {
	BCSCTL1 &= ~XT2OFF;			// XT2on

	BCSCTL2 |= SELM_2 | DIVM_0 | SELS;	// MCLK = SMCLK = XT2

	while (IFG1 & OFIFG) {
		IFG1 &= ~OFIFG;			// Clear OSCFault flag
	}
}

void initTimer() {
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 50000;
	TACTL = TASSEL_2 + ID_3 + MC_2;           // SMCLK/8, contmode
}

void enableTUSB3410() {
	P5SEL |= BIT5;				// SMCLK out	-> 12MHz XIN
	P4OUT |= BIT6;				// set output to high
	P4DIR |= BIT6;				// set to output -> release reset
}

void disableTUSB3410() {
	P4OUT &= ~BIT6;				// set output to low
	P5SEL &= ~BIT5;				// diable SMCLK out
}

void initUART0() {
	P3SEL |= BIT4 + BIT5;		// P3.4,5 = USART0 TXD/RXD

	ME1 |= UTXE0 + URXE0;                     // Enable USART0 TXD/RXD
	U0CTL |= CHAR;                            // 8-bit character
	U0TCTL |= SSEL1;                          // UCLK = SMCLK
	U0BR0 = 0x71;                             // 12Mhz/19200 = 625
	U0BR1 = 0x02;                             //
	U0MCTL = 0x00;                            // no modulation
	U0CTL &= ~SWRST;                          // Initialize USART state machine
//	IE1 |= URXIE0;                            // Enable USART0 RX interrupt
}

void enableI2C() {
	P3SEL |= BIT1 | BIT3;	// select SDA and SCL on P3.1 and P3.3

	U0CTL |= I2C + SYNC;	// enable I2C mode
	U0CTL &= ~I2CEN;

	// SMCLK = 12MHz / 30 = 400kHz
	I2CTCTL = I2CSSEL_2;
	//I2CPSC = 0x02;			// divide clock by 3
	I2CSCLL = 0x0D;
	I2CSCLH = 0x0D;

	I2CSA = EEPROM_I2CSA;

	U0CTL |= I2CEN;			// Enable I2C
}

void disableI2C() {
	P3SEL &= ~(BIT1 | BIT3);
	P3DIR &= ~(BIT1 | BIT3); // deselect I2C port setting and switch to input (high-impedance)
}

void startI2Cread(uint8_t cnt) {
	I2CTCTL &= ~I2CTRX;						// read mode
	U0CTL |= MST;                           // Master mode

	I2CIFG &= ~RXRDYIFG;
	I2CIE &= ~TXRDYIE;
	I2CIE |= RXRDYIE;

	I2CNDAT = cnt;

	I2CTCTL |= I2CSTT;
	while (I2CTCTL & I2CSTT)
		;
	I2CTCTL |= I2CSTP;
	__bis_SR_register(LPM0_bits + GIE);
}

void startI2Cwrite(uint8_t cnt, uint8_t stop) {
	I2CTCTL |= I2CTRX;						// write mode
	U0CTL |= MST;                           // Master mode

	I2CIFG &= ~TXRDYIFG;
	I2CIE &= ~RXRDYIE;
	I2CIE |= TXRDYIE;

	I2CNDAT = cnt;

	I2CTCTL |= I2CSTT;

	if (stop) {
		while (I2CTCTL & I2CSTT)
			;
		I2CTCTL |= I2CSTP;
	}

	__bis_SR_register(LPM0_bits + GIE);
}

uint8_t readI2Cmemory(uint16_t start_address, uint8_t* data, uint8_t size) {
	i2cDataArray[1] = (start_address & 0xFF00) >> 8;
	i2cDataArray[0] = (start_address & 0x00FF);
	i2cDataCnt = 2;

	startI2Cwrite(i2cDataCnt, 0);

	pI2CByte = data;
	startI2Cread(size);

	while (I2CTCTL & I2CSTP)
		;

	return 0;
}

uint8_t writeI2Cmemory(uint16_t start_address, uint16_t word) {
	i2cDataArray[3] = (start_address & 0xFF00) >> 8;
	i2cDataArray[2] = (start_address & 0x00FF);
	i2cDataArray[1] = (word & 0x00FF);
	i2cDataArray[0] = (word & 0xFF00) >> 8;
	i2cDataCnt = 4;

	startI2Cwrite(i2cDataCnt, 1);

	while (I2CTCTL & I2CSTP)
		;

	return 0;
}

// send string via USART0
uint8_t sendString(unsigned char* string, uint8_t length) {
	while (length--) {
		while (!(IFG1 & UTXIFG0));
		U0TXBUF = *string++;
	}
	return length;
}

// convert 8bit integer to 2 char BCD ASCII
void int2String(uint8_t input, unsigned char* output) {
	*output++ = ('0' + input/10);
	*output = ('0' + input%10);
}

/*********************************************/
/*   Interrupt service routines start here   */
/*********************************************/

/*
 #pragma vector=USART1RX_VECTOR
 __interrupt void USART1RX_ISR(void) {
 while (!(IFG2 & UTXIFG1))
 ;                // USART0 TX buffer ready?
 U1TXBUF = U1RXBUF;                          // RXBUF0 to TXBUF0
 }
 */

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A(void) {
	if (divider == 1) {
		//U0TXBUF = counter++;
		__bic_SR_register_on_exit(LPM0_bits);
		divider = 25;
	} else {
		divider--;
	}
	CCR0 += 60000;                            // Add Offset to CCR0
}

#pragma vector=USART0TX_VECTOR
__interrupt void I2C_ISR(void) {
	switch (__even_in_range(I2CIV, I2CIV_STT)) {
	case I2CIV_AL: /* I2C interrupt vector: Arbitration lost (ALIFG) */
		break;
	case I2CIV_NACK: /* I2C interrupt vector: No acknowledge (NACKIFG) */
		break;
	case I2CIV_OA: /* I2C interrupt vector: Own address (OAIFG) */
		break;
	case I2CIV_ARDY: /* I2C interrupt vector: Access ready (ARDYIFG) */
		break;
	case I2CIV_RXRDY: /* I2C interrupt vector: Receive ready (RXRDYIFG) */
		*pI2CByte++ = I2CDRB;
		__bic_SR_register_on_exit(LPM0_bits);
		break;
	case I2CIV_TXRDY: /* I2C interrupt vector: Transmit ready (TXRDYIFG) */
		I2CDRB = i2cDataArray[(i2cDataCnt--) - 1];	// load data to send

		if (!i2cDataCnt) {
			I2CIE &= ~TXRDYIE;                  // disable interrupts
			I2CIFG &= ~TXRDYIFG;                // Clear USCI_B0 TX int flag
			__bic_SR_register_on_exit(LPM0_bits);
			// Exit LPM0
		}
		break;
	case I2CIV_GC: /* I2C interrupt vector: General call (GCIFG) */
		break;
	case I2CIV_STT: /* I2C interrupt vector: Start condition (STTIFG) */
		break;
	}
}
