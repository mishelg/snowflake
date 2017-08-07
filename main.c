/*
 * main.c - Driver for graphics display for SnowFlake Mini
 *
 * Version: 1.00 Initial version 04-20-2011
 * Version: 1.01 cleanup 04-21-2011
 * Version: 1.02 added write support 04-05-2012
 *
 */
#include "msp430g2553.h"
#include <msp430.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "diskio.h"
#include "pff.h"
#include "spi.h"
#include "graphics.h"
#include "display.h"
char Buffer[256];
char size[8];
FRESULT res;
WORD s1, s2, ofs;
FATFS fs;			/* File system object */
/**
 * setup() - initialize timers and clocks
 */
#define NUM_BYTES  32                    // How many bytes?
char *PRxData,*Start;                     // Pointer to RX data
volatile unsigned char TXByteCtr, RXByteCtr, RX = 0;
char RxBuffer[32];       // Allocate 128 byte of RAM
volatile unsigned char Mode=1;

void I2C_init()
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
	UCB0I2COA = 0x48;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
}
void Flash_init()
{
	P2DIR |= BIT0;
	P2DIR |= BIT1;
	P2DIR |= BIT2;
	P2OUT |= BIT0;
	P2OUT |= BIT1;
	Flash_csl();
	spi_send(0x01);// Write status register instruction
	spi_send(0x41);// Enables sequential write/read mode and disable hold pin
	Flash_csh();
	Flash_csl();
	spi_send(0x06);
	Flash_csh();
}
void setup() {
    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
    //DCOCTL += 6; // the default calibrated clock on my mcu is slow
    //P1DIR |= BIT4; P1SEL |= BIT4;   // output SMCLK for measurement on P1.4
	/*BCSCTL1 =CALBC1_8MHZ;							//set range
	DCOCTL = CALDCO_8MHZ;							//set DCO step+ modulation
	BCSCTL3 |= LFXT1S_2;							//LFXT1 = VLO
	IFG1 &= ~OFIFG;									//clear OSCFault flag
	BCSCTL2 |= SELM_0 +DIVM_0 +DIVS_0;				//MCLK = DCO/8, SMCLK = DCO/8*/
}

/**
 * loop() - this routine runs over and over
 *
 * Wait for data to arrive. When something is available,
 * read it from the ring_buffer and echo it back to the
 * sender.
 */

void SD_font_init(char *font)
{
	SD_csl();
	res = disk_initialize();
	SD_csh();

	SD_csl();
	pf_mount(&fs);
	SD_csh();

	SD_csl();
	pf_open(font);
	SD_csh();
	ofs = fs.fptr;

}
void SD_img_init(char *img)
{
	SD_csl();
	res = disk_initialize();
	SD_csh();

	SD_csl();
	pf_mount(&fs);
	SD_csh();

	SD_csl();
	pf_open(img);
	SD_csh();
	ofs = fs.fptr;
}
void SD_img_Write_Full()
{
	int i,j;
	for (j=0;j<10;j++)
	{
		SD_csl();
		res = pf_read(Buffer, sizeof(Buffer), &s1);
		SD_csh();
		for(i=0;i<256;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		Flash_csl();
		spi_send(0x02);
		spi_send(j);
		spi_send(0x00);
		for(i=0;i<128;i++)
		{
			spi_send((Buffer[2*i]*16+Buffer[2*i+1]));
		}
		Flash_csh();
		SD_csl();
		res = pf_read(Buffer, sizeof(Buffer), &s1);
		SD_csh();
		for(i=0;i<256;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		Flash_csl();
		spi_send(0x02);
		spi_send(j);
		spi_send(128);
		for(i=0;i<128;i++)
		{
			spi_send((Buffer[2*i]*16+Buffer[2*i+1]));
		}
		Flash_csh();
	}
}
void SD_img_Write(unsigned int x, unsigned int y)
{
	int i,j,k;
	unsigned int l=0,b=0,le=0;
	unsigned char temp, temp2;
	SD_csl();
	res = pf_read(size, sizeof(size), &s1);
	SD_csh();
	l=(int)((size[3]-48)+(size[2]-48)*10+(size[1]-48)*100+(size[0]-48)*1000);
	b=(int)((size[7]-48)+(size[6]-48)*10+(size[5]-48)*100+(size[4]-48)*1000);
	if (l == 200 && b == 96)
	{
		SD_img_Write_Full();
		return;
	}
	else if (l+x>200 || b+y>96)
	{
		return;
	}
	if(l%8==0)
	{
		le=(l/8);
	}
	else
	{
		le=(l/8)+1;
	}
	for (k=0;k<b;k++)
	{
		SD_csl();
		res = pf_read(Buffer, 2*(le), &s1);
		SD_csh();
		for(i=0;i<2*le;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		for(i=0;i<le;i++)
		{
			Buffer[i]=(char)Buffer[2*i]*16+Buffer[2*i+1];
		}
		for(i=0;i<le-1;i++)
		{
			temp=Buffer[i]<<(x%8);
			temp2=Buffer[i+1]>>(8-x%8);
			Buffer[2*le+i+1]=temp+temp2;
		}
		Flash_csl();
		spi_send(0x03);
		spi_send(((y+k)*25+((x+l)/8))/256);
		spi_send(((y+k)*25+((x+l)/8))%256);
		Buffer[le]=spi_receive();
		Flash_csh();
		//Buffer[3*le-1]=Buffer[3*le-1]+Buffer[le];
		Flash_csl();
		spi_send(0x03);
		spi_send(((y+k)*25+(x/8))/256);
		spi_send(((y+k)*25+(x/8))%256);
		Buffer[le]=spi_receive();
		Flash_csh();
		temp=Buffer[le]>>(x%8);
		temp=temp<<(x%8);
		temp2=Buffer[0]>>(8-x%8);
		Buffer[2*le]=temp+temp2;
		Flash_csl();
		spi_send(0x02);
		spi_send(((y+k)*25+(x/8))/256);
		spi_send(((y+k)*25+(x/8))%256);
		for(i=0;i<le;i++)
		{
			spi_send(Buffer[2*le+i]);
		}
		Flash_csh();
	}
}
void SD_Font_Write()
{
	int i,j;
	for (j=0;j<5;j++)
	{
		SD_csl();
		res = pf_read(Buffer, sizeof(Buffer), &s2);
		SD_csh();
		for(i=0;i<256;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		Flash_csl();
		spi_send(0x02);
		spi_send(10+j);
		spi_send(0x00);
		for(i=0;i<128;i++)
		{
			spi_send(Buffer[2*i]*16+Buffer[2*i+1]);
		}
		Flash_csh();
		SD_csl();
		res = pf_read(Buffer, sizeof(Buffer), &s2);
		SD_csh();
		for(i=0;i<256;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		Flash_csl();
		spi_send(0x02);
		spi_send(10+j);
		spi_send(128);
		for(i=0;i<128;i++)
		{
			spi_send(Buffer[2*i]*16+Buffer[2*i+1]);
		}
		Flash_csh();
	}
}
void SD_Font10x16_Write()
{
	int i,j;
	for (j=0;j<8;j++)
	{
		SD_csl();
		res = pf_read(Buffer, sizeof(Buffer), &s2);
		SD_csh();
		for(i=0;i<256;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		Flash_csl();
		spi_send(0x02);
		spi_send(10+j);
		spi_send(0x00);
		for(i=0;i<128;i++)
		{
			spi_send(Buffer[2*i]*16+Buffer[2*i+1]);
		}
		Flash_csh();
		SD_csl();
		res = pf_read(Buffer, sizeof(Buffer), &s2);
		SD_csh();
		for(i=0;i<256;i++)
		{
			if(Buffer[i]>='0'&&Buffer[i]<='9')
			{
				Buffer[i]=Buffer[i]-48;
			}
			else
			{
				Buffer[i]=Buffer[i]-55;
			}
		}
		Flash_csl();
		spi_send(0x02);
		spi_send(10+j);
		spi_send(128);
		for(i=0;i<128;i++)
		{
			spi_send(Buffer[2*i]*16+Buffer[2*i+1]);
		}
		Flash_csh();
	}
}
void main (void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	volatile unsigned int i,j;
	unsigned char temp,temp2,temp3,temp4;
	j=1;
	setup();
	spi_initialize();
	disp_init();
	Flash_init();
	I2C_init();
    pwrOn();
    initDriver();
    screen_clear();
    while(1)
	{
		RX = 1;
		IE2 &= ~UCB0TXIE;                         // Disable TX interrupt
		IE2 |= UCB0RXIE;                          // Enable RX interrupt
		TXByteCtr=0;
		RXByteCtr=0;
		PRxData = (char *)RxBuffer;    // Start of RX buffer
		Start= RxBuffer;
		__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
		_disable_interrupt();
		switch(*Start++)
		{
			case 'S': break;
			case 'B': screen_black();
						break;
			case 'C': screen_clear();
						break;
			case 'I':	//invert();
					  temp=*Start++;
					  temp2=*Start++;
					  temp=temp+temp2;
					  temp2=*Start++;
					  temp3=*Start++;
					  temp4=*Start++;
					  temp3=temp3+temp4;
					  temp4=*Start++;
					  invertxy(temp,temp2,temp3,temp4);
						break;
			case 'L':
					switch(*Start++)
					{
						case 'F':SD_font_init(Start);
								 SD_Font10x16_Write();
								break;
					}
					break;
			case 'W':
				switch(*Start++)
				{
				case 'I':
						temp=*Start++;
						temp2=*Start++;
						temp=temp+temp2;
						temp2=*Start++;
						SD_img_init(Start);
						SD_img_Write(temp,temp2);
						break;
				case 'F':
						temp=*Start++;
						temp2=*Start++;
						temp=temp+temp2;
						temp2=*Start++;
						drawstring10x16(temp,temp2,Start);
						break;
				case 'S':
						temp=*Start++;
						for (i=0;i<(temp-48);i++)
						{
							screenWrite();
						}
						//pwrOff();
						break;
				}
						break;
			default: 	break;
		}
		_enable_interrupt();
		RX=0;
		IE2 |= UCB0TXIE;
		IE2 &= ~UCB0RXIE;
	}
}

//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move data from MSP430 memory to the
// I2C master. PTxData points to the next byte to be transmitted, and TXByteCtr
// keeps track of the number of bytes transmitted.
//------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if(RX == 0)
  {
	  UCB0TXBUF = 'D';      // Transmit data at address PTxData
	  TXByteCtr++;                              // Increment TX byte counter
  }
  if(RX == 1){*PRxData++ = UCB0RXBUF;       // Move RX data to address PRxData
  RXByteCtr++;                              // Increment RX byte count
  if(RXByteCtr >= NUM_BYTES){               // Received enough bytes to switch
	  __bic_SR_register_on_exit(CPUOFF);       // Exit LPM0 if data was
  }
 }
}

//------------------------------------------------------------------------------
// The USCI_B0 state ISR is used to wake up the CPU from LPM0 in order to do
// processing in the main program after data has been transmitted. LPM0 is
// only exit in case of a (re-)start or stop condition when actual data
// was transmitted.
//------------------------------------------------------------------------------
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
  if(RX == 0){ UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags

}
  if(TXByteCtr)
  {
	  IE2 &= ~UCB0TXIE;
  }
  if(RX == 1){UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags
  }

}
