#include "msp430g2553.h"
#include "spi.h"
#include "display.h"
#include "integer.h"
#define byte unsigned char
volatile unsigned int RXData;
unsigned char Screen_Buffer[25];
//#define spi_xfer_byte spi_send
void disp_init()
{
	P1DIR |= PWMPin;                     //
	P2DIR |= FlashPin;				  //PWM, discharge and RST
	P3DIR |= csPin+ dischargePin + rstPin;
	P2OUT|= FlashPin;
	P3OUT|= rstPin;
	P3OUT&= ~dischargePin;
}

void SPISend(unsigned char a, unsigned char b)
{
	P3OUT |= csPin;
	__delay_cycles(200);//adjust to hit 10us
	P3OUT &= ~csPin;
	spi_xfer_byte(0x70);
	spi_xfer_byte(a);
	P3OUT |= csPin;
	__delay_cycles(200);//adjust to hit 10us
	P3OUT &= ~csPin;
	spi_xfer_byte(0x72);
	spi_xfer_byte(b);
	P3OUT |= csPin;
}

void SPISendInc(unsigned char a, unsigned char b)
{
	P3OUT |= csPin;
	__delay_cycles(200);//adjust to hit 10us
	P3OUT &= ~csPin;
	spi_xfer_byte(0x70);
	spi_xfer_byte(a);
	P3OUT |= csPin;
	__delay_cycles(200);//adjust to hit 10us
	P3OUT &= ~csPin;
	spi_xfer_byte(0x72);
	spi_xfer_byte(b);
}

void pwrOn(void)
{
	_delay_cycles(80000);//adjust to hit 10 ms
	P3OUT |= csPin;
	P3OUT |= rstPin;
	_delay_cycles(40000);//adjust to hit 5 ms
	P3OUT &= ~rstPin;
	_delay_cycles(40000);//adjust to hit 5 ms
	P3OUT |= rstPin;
	_delay_cycles(40000);//adjust to hit 5 ms
}

void initDriver(void)
{
	volatile unsigned int i;
	SPISendInc(0x01,0x00);
	P3OUT &= ~csPin;
	/*spi_xfer_byte(0x00);
	spi_xfer_byte(0x00);
	spi_xfer_byte(0x00);
	spi_xfer_byte(0x00);
	spi_xfer_byte(0x0F);
	spi_xfer_byte(0xFF);
	spi_xfer_byte(0x00);*/
	spi_xfer_byte(0x00);
	spi_xfer_byte(0x00);
	spi_xfer_byte(0x00);
	spi_xfer_byte(0x01);
	spi_xfer_byte(0xFF);
	spi_xfer_byte(0xE0);
	spi_xfer_byte(0x00);
	P3OUT |= csPin;
	SPISend(0x06,0x1F);
    SPISend(0x07,0x9d);
    SPISend(0x08,0x00);
    SPISendInc(0x09,0xC0);
	spi_xfer_byte(0x00);
	P3OUT |= csPin;
    SPISend(0x04,0x03);
    SPISend(0x03,0x01);
    SPISend(0x03,0x00);
    for (i=0;i<2400;i++)
    {
    	P1OUT^= BIT0;
    }
    P1OUT&= ~BIT0;
	SPISend(0x05,0x01);
	for (i=0;i<16000;i++)
	    {
	    	P1OUT^= BIT0;
	    }
	P1OUT&= ~BIT0;
	SPISend(0x05,0x03);
    _delay_cycles(240000);//delay 30ms
    SPISend(0x05,0x0F);
    _delay_cycles(240000);//delay 30ms
    SPISend(0x02,0x24);
	//spi_xfer_byte(0x03);
	//spi_xfer_byte(0x00);
	//spi_xfer_byte(0xF0);
	//spi_xfer_byte(0x00);
	_delay_cycles(240000);//delay 30ms
	P3OUT |= csPin;
}

unsigned char expo(int a)//Returns 2^a
{
	unsigned char r=1;
	int i;
	r=r<<a;
	return r;
}

byte bufferProcess2(byte Buffer)
{
	byte a;
	a=Buffer;
	a = (a & 0x0F) << 4 | (a & 0xF0) >> 4;
	a = (a & 0x33) << 2 | (a & 0xCC) >> 2;
	a = (a & 0x55) << 1 | (a & 0xAA) >> 1;
	a = a & 0xAA;
	a=a>>1;
	a = a + 0xAA;
  return a;
}
byte bufferProcess(byte Buffer)
{
  volatile byte a;
  volatile byte c;
  a=Buffer;
  a = a & 0xAA;
  a=a>>1;
  a = a | 0xAA;
  return a;
}

void Flash_Buffer(byte y)
{
	volatile byte i;
	Screen_csh();
	Flash_csl();
	spi_send(0x03);
	spi_send(0x00+(unsigned char) ((y-1)*25/256));
	spi_send(0x00+((y-1)*25)-((unsigned char) ((y-1)*25/256)*256));
	for (i=0;i<25;i++)
	{
		//Screen_Buffer[i]=0;
		Screen_Buffer[i]=spi_receive();
	}
	Flash_csh();
	Screen_csl();
}
void screenWrite()
{
	volatile byte y,i;
	for (y=1;y<=96;)
	{
		Flash_Buffer(y);
		/*for(i=0;i<33;i++)
		{
			Screen_Buffer[i]=Draw_Buffer[y*33+i];
		}*/
		SPISend(0x04,0x02);
		SPISendInc(0x0A,bufferProcess2(Screen_Buffer[(24)]));
		for (i=1;i<=24;i++)
		{
		  spi_xfer_byte(bufferProcess2(Screen_Buffer[(24-i)]));
		}
		for ( i=1;i<=((y-1)/4);i++)
		{
		  spi_xfer_byte(0x00);
		}
		if (y%4==1)
		{
		  spi_xfer_byte(0xC0);
		}
		else if (y%4==2)
		{
		  spi_xfer_byte(0x30);
		}
		else if (y%4==3)
		{
		  spi_xfer_byte(0x0C);
		}
		else if (y%4==0)
		{
		  spi_xfer_byte(0x03);
		}
		for (i=((y-1)/4)+2;i<=24;i++)
		{
		  spi_xfer_byte(0x00);
		}
		for (i=0;i<=24;i++)
		{
		  spi_xfer_byte(bufferProcess(Screen_Buffer[i]));
		}
		spi_xfer_byte(0x00);
		y++;
		SPISend(0x02,0x25);
	}
}

void pwrOff()
{
  volatile unsigned int y,i;
  for (y=1;y<=96;)
  {
    SPISend(0x04,0x03);
    SPISendInc(0x0A,0x00);
    for (i=1;i<=24;i++)
    {
      spi_xfer_byte(0x00);
    }
    for (i=1;i<=((y-1)/4);i++)
    {
      spi_xfer_byte(0x00);
    }
    if (y%4==1)
    {
      spi_xfer_byte(0xC0);
    }
    else if (y%4==2)
    {
      spi_xfer_byte(0x30);
    }
    else if (y%4==3)
    {
      spi_xfer_byte(0x0C);
    }
    else if (y%4==0)
    {
      spi_xfer_byte(0x03);
    }
    for (i=((y-1)/4)+1;i<=24;i++)
    {
      spi_xfer_byte(0x00);
    }
    for (i=1;i<=25;i++)
    {
      spi_xfer_byte(0x00);
    }
    spi_xfer_byte(0x00);
    y++;
    SPISend(0x02,0x2F);
  }
  //End of nothing frame.
  SPISend(0x04,0x03);
  SPISendInc(0x0A,0x00);
  for (i=1;i<=24;i++)
  {
    spi_xfer_byte(0x00);
  }
  for (i=1;i<=24;i++)
  {
    spi_xfer_byte(0x00);
  }
  for (i=1;i<=25;i++)
  {
    spi_xfer_byte(0x00);
  }
  spi_xfer_byte(0x00);
  SPISend(0x02,0x2F);
  _delay_cycles(240000);//delay 30ms
  SPISend(0x03,0x01);
  SPISend(0x02,0x05);
  SPISend(0x05,0x00);
  SPISend(0x07,0x0D);
  for (i=1;i<=10;i++)
  {
    SPISend(0x04,0xFC);
    SPISend(0x04,0x00);
  }
  SPISend(0x04,0xFC);
  _delay_cycles(240000);//delay 30ms
  SPISend(0x04,0x00);
  P3OUT |= csPin;
  P3OUT |= rstPin;
  P3OUT |= dischargePin;
  _delay_cycles(1200000);//delay 30ms
  P3OUT &= ~dischargePin;
}
