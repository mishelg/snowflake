#include <msp430.h>
#include "msp430g2553.h"
#include <stdint.h>
#include "spi.h"
#include "integer.h"

#ifndef __MSP430_HAS_USCI__
#error "Error! This MCU doesn't have a USCI peripheral"
#endif

#define SPI_MODE_0 UCCKPH | UCMSB| UCMST | UCSYNC /* CPOL=0 CPHA=0 */
#define SPI_MODE_3 UCCKPL | UCMSB| UCMST | UCSYNC /* CPOL=1 CPHA=1 */
#define SD_CS BIT4
#define SCREEN_CS BIT1
#define FLASH_CS BIT2
#define SOMI BIT1
#define SIMO BIT2
#define SCLK BIT4

/**
 * spi_initialize() - Initialize and enable the SPI module
 *
 * P2.0 - CS (active low)
 * P1.4 - SCLK
 * P1.1 - MISO
 * P1.2 - MOSI
 *
 */
void spi_initialize( void )
{

	P3DIR |= SD_CS | FLASH_CS;

	// (1)
	UCA0CTL1 |= UCSWRST;

	// (2)
	P3DIR  |= SCREEN_CS;
	P3OUT  |= SCREEN_CS;
	P1DIR |= SOMI + SIMO + SCLK;
	P1SEL  |= SOMI + SIMO + SCLK;
	P1SEL2 |= SOMI + SIMO + SCLK;

	// (3) 3-pin, 8-bit SPI master
	UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;
	UCA0CTL1 |= UCSSEL_2;   // SMCLK
	UCA0BR0 |= 1;
	UCA0BR1 |= 0;

	// (4)
	UCA0CTL1 &= ~UCSWRST;




  /*UCA0CTL1 |= UCSWRST;
  //UCB0CTL0 |= UCMSB |  UCMST | UCMODE_0 | UCSYNC;
  UCA0CTL0 = UCCKPL + UCMSB + UCMST + UCMODE_0 + UCSYNC;
  UCA0CTL1 = UCSSEL_2 | UCSWRST;

  UCA0BR0 = 16; // 8MHz
  UCA0BR1 = 0;
  P1DIR |= BIT1 | BIT2 | BIT4;
  P1SEL  |= BIT1 | BIT2 | BIT4; // P1.5, P1.6, P1.7 option select
  P1SEL2 |= BIT1 | BIT2 | BIT4;
  UCA0CTL1 &= ~UCSWRST;
  */
  SD_csh();
}

/**
 * spi_send() - send a byte and recv response
 */
uint8_t spi_send(const uint8_t c)
{
	//_delay_cycles(100);
	//SD_csl();
	while (!(UC0IFG & UCA0TXIFG))
		; // wait for previous tx to complete

	UCA0TXBUF = c; // setting TXBUF clears the TXIFG flag
	while (!(UC0IFG & UCA0RXIFG))
		; // wait for an rx character?
	//_delay_cycles(100);
	//SD_csh();
	return UCA0RXBUF; // reading clears RXIFG flag
}

/**
 * spi_receive() - send dummy btye then recv response
 */
uint8_t spi_receive(void) {
	//SD_csl();
	//_delay_cycles(100);
	while (!(UC0IFG & UCA0TXIFG))
		; // wait for any previous xmits to complete

	UCA0TXBUF = 0xFF; // Send dummy packet to get data back.

	while (!(UC0IFG & UCA0RXIFG))
		; // wait to recv a character?
	//_delay_cycles(100);
	//SD_csh();
	return UCA0RXBUF; // reading clears RXIFG flag

}

unsigned char spi_xfer_byte(unsigned char data)
{
  	UCA0TXBUF = data;

	// wait for TX
	while (!(IFG2 & UCA0TXIFG));
	//_delay_cycles(160);

	return UCA0RXBUF;
}

/**
 * spi_setspeed() - set new clock divider for USCI
 *
 * USCI speed is based on the SMCLK divided by BR0 and BR1
 * initially we start slow (400kHz) to conform to SDCard
 * specifications then we speed up once initialized (16Mhz)
 *
 */
void spi_set_divisor(const uint16_t clkdiv)
{
	UCA0CTL1 |= UCSWRST;		// go into reset state
	UCA0BR0 = LOBYTE(clkdiv);
	UCA0BR1 = HIBYTE(clkdiv);
	UCA0CTL1 &= ~UCSWRST;		// release for operation
}
void SD_csh(void)
{
    P3OUT |= SD_CS;
}
void SD_csl(void)
{
    P3OUT &= ~SD_CS;
}
void Screen_csh(void)
{
    P3OUT |= SCREEN_CS;
}
void Screen_csl(void)
{
    P3OUT &= ~SCREEN_CS;
}
void Flash_csh(void)
{
    P2OUT |= FLASH_CS;
}
void Flash_csl(void)
{
    P2OUT &= ~FLASH_CS;
}
