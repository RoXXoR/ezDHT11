#include <msp430.h> 
#include <stdint.h>
/*
 * main.c
 */
void initCLK();
void initUART0();
void initTUSB3410();
void initTimer();

uint8_t counter = 0;
uint16_t divider = 25;

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	initCLK();
	initTUSB3410();
	initUART0();
	initTimer();
	__bis_SR_register(LPM0_bits | GIE);
	return 0;
}

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

void initTUSB3410() {
	P5SEL |= BIT5;				// SMCLK out	-> 12MHz XIN
	P4OUT |= BIT6;				// set output to high
	P4DIR |= BIT6;				// set to output -> release reset
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

	P3SEL |= BIT6 + BIT7;		// P3.4,5 = USART0 TXD/RXD

	ME2 |= UTXE1 + URXE1;                     // Enable USART0 TXD/RXD
	U1CTL |= CHAR;                            // 8-bit character
	U1TCTL |= SSEL1;                          // UCLK = SMCLK
	U1BR0 = 0x71;                             // 12Mhz/19200 = 625
	U1BR1 = 0x02;                             //
	U1MCTL = 0x00;                            // no modulation
	U1CTL &= ~SWRST;                          // Initialize USART state machine
//	IE2 |= URXIE1;                            // Enable USART0 RX interrupt

}
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

