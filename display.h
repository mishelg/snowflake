#define csPin BIT1
#define dischargePin BIT3
#define rstPin BIT0
#define PWMPin BIT0
#define FlashPin BIT2
#define byte unsigned char
#define spi_xfer_byte spi_send

void disp_init();

void SPISend(unsigned char a, unsigned char b);

void SPISendInc(unsigned char a, unsigned char b);

void pwrOn(void);

void initDriver(void);

unsigned char expo(int a);

byte bufferProcess2(byte Buffer);

byte bufferProcess(byte Buffer);

void screenWrite();

void pwrOff();
