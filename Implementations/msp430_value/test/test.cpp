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

	while(1) {
		//digitalWrite(1, HIGH);
		if (is_locked) {
			Serial.print("# ");
			//Serial.println(i++);
		}
		delay(250);
		//digitalWrite(1, LOW);
		delay(250);
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_locked = true;
}
