#include <Arduino.h>

int main()
{
	WDTCTL = WDTPW | WDTHOLD;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	while(1) {
		digitalWrite(3, HIGH);
	}
}
