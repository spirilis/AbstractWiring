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

	//SPI.setDataMode(SPI_MODE3);
	//SPI.setBitOrder(LSBFIRST);

	pinMode(4, INPUT_PULLUP);
	attachInterrupt(4, myCallback, FALLING);
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);

	while(1) {
		//SPI.transfer(0xAA);
		//delay(150);
		//SPI.transfer(0x82);
		//delay(150);
		//SPI.transfer9(0x123);
		//delay(150);
		SPI.transfer16(0x6933);
		delay(150);
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_locked = true;
}
