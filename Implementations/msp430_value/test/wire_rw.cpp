#include <AbstractWiring.h>
#include <UART_USCI.h>
#include <Wire_USCI.h>

void myCallback(void);

volatile boolean is_ready = false;

UART_USCI <0, UCA0CTL0, UCA0CTL1, UCA0MCTL, UCA0ABCTL, UCA0BR0, UCA0BR1, UCA0STAT, UCA0TXBUF, UCA0RXBUF, IE2, UCA0TXIE, UCA0RXIE, 16, 2, P1SEL, P1SEL2, PORT_SELECTION_0_AND_1, BIT1|BIT2> Serial;
Wire_USCI<0, UCB0CTL0, UCB0CTL1, UCB0BR0, UCB0BR1, UCB0STAT, UCB0I2COA, UCB0I2CSA, UCB0TXBUF, UCB0RXBUF, UCB0I2CIE, UCB0STAT, IE2, IFG2, P1SEL, P1SEL2, PORT_SELECTION_0_AND_1, BIT6|BIT7, 24, 16> Wire;

int main()
{
	int i = 0;

	WDTCTL = WDTPW | WDTHOLD;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	sysinit(16000000UL);
	Wire.begin();
	//Wire.setSpeed(600000UL);
	Serial.begin(9600);

	pinMode(4, INPUT_PULLUP);
	attachInterrupt(4, myCallback, FALLING);
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);

	while (!is_ready) ;

	while(1) {
		Serial.println("Begin-"); Serial.flush();

		Wire.beginTransmission(0x28);
		Wire.write(0x00); Wire.write(0x26);
		Wire.print("Text goes here");
		if (Wire.endTransmission()) {
			Serial.println("Sent 14 bytes to 0x28 at address 0x26"); Serial.flush();
			Wire.beginTransmission(0x28);
			Wire.write(0x00); Wire.write(0x26);  // Re-set the RF430CL330H chip's pointer
			Wire.endTransmission();
		} else {
			Serial.println("Error sending I2C data"); Serial.flush();
		}
		delay(250);

		// Read back
		i = Wire.requestFrom(0x28, 14);
		if (i) {
			Serial.print("Reading back: "); Serial.flush();
			while (Wire.available())
				Serial.write(Wire.read());
			Serial.println();
		} else {
			Serial.println("Error reading from I2C 0x28"); Serial.flush();
		}

		delay(4000);
	}
	return 0;
}

void myCallback(void)
{
	//digitalWrite(1, LOW);
	is_ready = true;
}
