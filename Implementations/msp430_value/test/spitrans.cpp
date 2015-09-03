#include <AbstractWiring.h>
#include <SPI_USCI.h>

void myCallback(void);

volatile boolean is_locked = false;

SPI_USCI<UCB0CTL0,UCB0CTL1,UCB0BR0,UCB0BR1,UCB0STAT,UCB0TXBUF,UCB0RXBUF,P1DIR,P1OUT,P1SEL,P1SEL2,BIT5,PORT_SELECTION_0_AND_1,P1DIR,P1OUT,P1SEL,P1SEL2,BIT7,PORT_SELECTION_0_AND_1,P1DIR,P1IN,P1SEL,P1SEL2,BIT6,PORT_SELECTION_0_AND_1> SPI;

int main()
{
	//static unsigned int i = 0;

	WDTCTL = WDTPW | WDTHOLD;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	sysinit(16000000UL);
	SPI.begin();

	pinMode(4, INPUT_PULLUP);
	attachInterrupt(4, myCallback, FALLING);
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);

	SPISettings a(8000000UL, MSBFIRST, SPI_MODE0);
	SPISettings b(4000000UL, MSBFIRST, SPI_MODE1);

	while(1) {
		SPI.beginTransaction(a);
		SPI.transfer(0xAA);
		delay(150);
		SPI.transfer(0x82);
		SPI.endTransaction();
		delay(150);

		SPI.beginTransaction(b);
		SPI.transfer(0xD3);
		delay(150);
		SPI.transfer(0x8E);
		SPI.endTransaction();
		delay(150);
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_locked = true;
}
