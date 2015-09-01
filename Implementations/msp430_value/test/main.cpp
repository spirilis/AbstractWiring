#include <AbstractWiring.h>

void myCallback(void);

volatile boolean is_locked = false;

int main()
{
	WDTCTL = WDTPW | WDTHOLD;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	sysinit(16000000UL);

	pinMode(4, INPUT_PULLUP);
	attachInterrupt(4, myCallback, FALLING);

	while(1) {
		digitalWrite(1, HIGH);
		if (is_locked) LPM4;
		delay(250);
		digitalWrite(1, LOW);
		delay(250);
	}
	return 0;
}

void myCallback(void)
{
	digitalWrite(1, HIGH);
	is_locked = true;
}
