#include <AbstractWiring.h>
#include <UART_USCI.h>

void myCallback(void);

volatile boolean is_ready = false;

UART_USCI <0, UCA0CTL0, UCA0CTL1, UCA0MCTL, UCA0ABCTL, UCA0BR0, UCA0BR1, UCA0STAT, UCA0TXBUF, UCA0RXBUF, IE2, UCA0TXIE, UCA0RXIE, 16, 16, P1SEL, P1SEL2, PORT_SELECTION_0_AND_1, BIT1|BIT2> Serial;


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

	while (!is_ready)  // wait for user to press button to begin
		;

	int c;
	uint32_t millis_next = 0;

	while(1) {
		if (Serial.available()) {
			c = Serial.read();
			if (c >= 0)
				Serial.write(c);
		}
		if (millis() > millis_next) {
			millis_next = millis() + 2000;
			Serial.print("Temperature (degrees Celsius): ");
			Serial.println(adc.sample_tempsensor());
		}

		adc.sample(0);  // sample A0
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_ready = true;
}
