#include "ez430U.h"

uint8_t counter = 0;
uint16_t divider = 25;

void initCLK() {
	BCSCTL1 &= ~XT2OFF;			// XT2on

	while (IFG1 & OFIFG) {
		IFG1 &= ~OFIFG;			// Clear OSCFault flag
	}

	BCSCTL2 |= SELM_2 * DIVM_1 + SELS;	// MCLK*2 = SMCLK = XT2

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

	P3SEL |= BIT6 + BIT7;		// P3.6,7 = USART1 TXD/RXD

	ME2 |= UTXE1 + URXE1;                     // Enable USART0 TXD/RXD
	U1CTL |= CHAR;                            // 8-bit character
	U1TCTL |= SSEL1;                          // UCLK = SMCLK
	U1BR0 = 0x71;                             // 12Mhz/19200 = 625
	U1BR1 = 0x02;                             //
	U1MCTL = 0x00;                            // no modulation
	U1CTL &= ~SWRST;                          // Initialize USART state machine
//	IE2 |= URXIE1;                            // Enable USART0 RX interrupt

}

void enableI2C() {
	P3SEL |= BIT1 | BIT3;	// select SDA and SCL on P3.1 and P3.3

}

void disableI2C() {
	P3SEL &= ~(BIT1 | BIT3);
	P3DIR &= ~(BIT1 | BIT3);	// deselect I2C port setting and switch to input (high-impedance)
}

uint16_t readI2Cmemory(){

	return 0;		// should return No. of bytes received
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
		U0TXBUF = counter;
		U1TXBUF = counter++;
		divider = 25;
	} else {
		divider--;
	}
	CCR0 += 60000;                            // Add Offset to CCR0
}
