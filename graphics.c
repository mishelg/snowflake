#include "graphics.h"
#include "display.h"
extern char Buffer[];
void SetPixel(unsigned int x, unsigned int y)
{
	unsigned char temp;
	unsigned char temp2;
	Flash_csl();
	spi_send(0x03);
	spi_send((y*25+(x/8))/256);
	spi_send((y*25+(x/8))-(unsigned char)((y*25+(x/8))/256)*256);
	temp=spi_receive();
	Flash_csh();
	temp2=(unsigned char)7-(unsigned char)(x-(x/8)*8);
	Flash_csl();
	spi_send(0x02);
	spi_send((y*25+(x/8))/256);
	spi_send((y*25+(x/8))-(int)((y*25+(x/8))/256)*256);
	spi_send(temp|expo(temp2));
	Flash_csh();
}
void ClearPixel(int x, int y)
{
	unsigned char temp;
	unsigned char temp2;
	Flash_csl();
	spi_send(0x03);
	spi_send((y*25+(x/8))/256);
	spi_send((y*25+(x/8))-(unsigned char)((y*25+(x/8))/256)*256);
	temp=spi_receive();
	Flash_csh();
	temp2=(unsigned char)7-(unsigned char)(x%8);
	//drawchar(132,88,temp2);
	Flash_csl();
	spi_send(0x02);
	spi_send((y*25+(x/8))/256);
	spi_send((y*25+(x/8))-(int)((y*25+(x/8))/256)*256);
	spi_send(temp&~(expo(temp2)));
	Flash_csh();
}
void screen_clear(void)
{
	int i;
	Flash_csl();
	spi_send(0x02);
	spi_send(0x00);
	spi_send(0x00);
	for(i=0; i<2400; i++)
	{
		spi_send(0x00);
	}
	Flash_csh();
}
void screen_black(void)
{
	int i;
	Flash_csl();
	spi_send(0x02);
	spi_send(0x00);
	spi_send(0x00);
	for(i=0; i<2400; i++)
	{
		spi_send(0xFF);
	}
	Flash_csh();
}
void drawchar(int x, int y, char c)
{
	unsigned char temp;
	unsigned int i;
	for (i =0; i<5; i++ )
	{
		Flash_csl();
		spi_send(0x03);
		spi_send(10+(unsigned char)(((c-32)*5+i)/256));
		spi_send((c-32)*5+i-(unsigned char)(((c-32)*5+i)/256)*256);
		temp=spi_receive();
		Flash_csh();
		Flash_csl();
		spi_send(0x02);
		spi_send((y*25+(x/8))/256);
		spi_send((y*25+(x/8))-(unsigned char)((y*25+(x/8))/256)*256);
		spi_send(temp);
		Flash_csh();
		y++;
	}
}
void drawchar10x16(int x, int y, char c)
{
	byte char_buffer[32];//={0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x0E, 0xC0, 0x09, 0x38, 0x01, 0x06, 0x01, 0x38, 0x01, 0xC0, 0x09, 0x00, 0x0E, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	byte b[8];
	int i,j;
	Flash_csl();
	spi_send(0x03);
	spi_send(10+(unsigned char)(((c-32)*20)/256));
	spi_send((c-32)*20-(unsigned char)(((c-32)*20)/256)*256);
	for (i=0;i<20;i++)
	{
		char_buffer[i]=spi_receive();
	}
	Flash_csh();
	for (i=0;i<20;i++)
	{
		for (j=0;j<=7;j++)
		{
			b[j]=char_buffer[i]%2;
			char_buffer[i]=char_buffer[i]/2;
		}
		for(j=7;j>=0;j--)
		{
			if(b[j]==1)
			{
				SetPixel(x+(i/2),y+(i%2)*8+(j));
			}
			else
			{
				ClearPixel(x+(i/2),y+(i%2)*8+(j));
			}
		}
	}
}
void drawstring(int x, int y, char *c)
{
  while(c[0]!=0)
  {
    drawchar(x, y, c[0]);
    c++;
    y+=10;
  }
}
void drawstring10x16(unsigned int x, unsigned int y, char *c)
{
	while(c[0]!=0)
	{
		drawchar10x16(x, y, c[0]);
		c++;
		x+=10;
	}
}
void invert()
{
	unsigned int i,j;
	for (i=0;i<23;i++)
	{
		Flash_csl();
		spi_send(0x03);
		spi_send(i);
		spi_send(0);
		for (j=0;j<256;j++)
		{
			Buffer[j]=spi_receive();
		}
		Flash_csh();
		Flash_csl();
		spi_send(0x02);
		spi_send(i);
		spi_send(0);
		for (j=0;j<256;j++)
		{
			spi_send(255-Buffer[j]);
		}
		Flash_csh();
	}
}
void invertxy(unsigned int x, unsigned int y, unsigned int a, unsigned int b)
{
	if(x+a>200||y+b>96)
	{
		return;
	}
	unsigned int i,j,k;
	unsigned char temp,temp2;
	k=0;
	while(k<b)
	{
		i=((y+k)*25+(x/8))/256;
		Flash_csl();
		spi_send(0x03);
		spi_send(i);
		spi_send(((y+k)*25+(x/8))%256);
		for (j=0;j<(a/8)+1;j++)
		{
			Buffer[j]=spi_receive();
		}
		Flash_csh();
		temp=Buffer[0]>>(8-x%8);
		temp=temp<<(8-x%8);
		temp2=Buffer[0]<<(x%8);
		temp2=~temp2;
		temp2=temp2>>(x%8);
		Buffer[0]=temp+temp2;
		for (j=1;j<(a/8);j++)
		{
			Buffer[j]=~Buffer[j];
		}
		temp=Buffer[(a/8)]>>((x+a)%8);
		temp=temp<<((x+a)%8);
		temp2=Buffer[(a/8)]<<(8-(x+a)%8);
		temp2=~temp2;
		temp2=temp2>>(8-(x+a)%8);
		Buffer[(a/8)]=temp+temp2;
		Buffer[(a/8)]=~Buffer[(a/8)];
		Flash_csl();
		spi_send(0x02);
		spi_send(i);
		spi_send(((y+k)*25+(x/8))%256);
		for(j=0;j<=(a/8);j++)
		{
			spi_send(Buffer[j]);
		}
		Flash_csh();
		k++;
	}
}
