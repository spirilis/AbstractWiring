#include <AbstractWiring.h>
#include <UART_USCIA0.h>

void myCallback(void);

volatile boolean is_locked = false;

UART_USCIA0 Serial;

int main()
{
	//static unsigned int i = 0;

	WDTCTL = WDTPW | WDTHOLD;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	sysinit(16000000UL);
	Serial.begin(9600);

	pinMode(4, INPUT_PULLUP);
	attachInterrupt(4, myCallback, FALLING);
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);

	int c;

	while(1) {
		if (Serial.available()) {
			c = Serial.read();
			if (c >= 0)
				Serial.write(c);
		}
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_locked = true;
}
