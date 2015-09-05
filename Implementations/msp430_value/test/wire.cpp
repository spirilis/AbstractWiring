#include <AbstractWiring.h>
#include <Wire_USCI.h>

void myCallback(void);

volatile boolean is_locked = false;

Wire_USCI<UCB0CTL0, UCB0CTL1, UCB0BR0, UCB0BR1, UCB0STAT, UCB0I2COA, UCB0I2CSA, UCB0TXBUF, UCB0RXBUF, UCB0I2CIE, UCB0STAT, IE2, IFG2, P1SEL, P1SEL2, PORT_SELECTION_0_AND_1, BIT6|BIT7, 16, 16> Wire;

int main()
{
	//static unsigned int i = 0;

	WDTCTL = WDTPW | WDTHOLD;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	sysinit(16000000UL);
	Wire.begin();

	pinMode(4, INPUT_PULLUP);
	attachInterrupt(4, myCallback, FALLING);
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);

	while(1) {
		Wire.beginTransmission(0x29);
		Wire.print("Hi there");
		if (Wire.endTransmission())
			digitalWrite(1, HIGH);
		else
			digitalWrite(1, LOW);
		delay(250);
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_locked = true;
}
