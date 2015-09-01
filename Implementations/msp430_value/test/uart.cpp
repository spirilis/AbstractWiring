#include <AbstractWiring.h>
#include <UART_USCIA0.h>

void myCallback(void);

volatile boolean is_locked = false;

template <
    u8_SFR ucactl0,
    u8_SFR ucactl1,
    u8_SFR ucamctl,
    u8_SFR ucaabctl,
    u8_SFR ucabr0,
    u8_SFR ucabr1,
    u8_SFR ucastat,
    u8_SFR ucatxbuf,
    u8_SFR ucarxbuf,
    u8_SFR ucaie,
    uint8_t ucatxie,
    uint8_t ucarxie
    volatile char * txbuffer,
    volatile char * rxbuffer,
    size_t tx_buffer_size,
    size_t rx_buffer_size, 
    u8_SFR pxsel, 
    u8_SFR pxsel2,
    enum PortselMode pxsel_specification,
    uint8_t pxbits >

char txbuf[16], rxbuf[16];

UART_USCI <UCA0CTL0, UCA0CTL1, UCA0MCLK, UCA0ABCTL, UCA0BR0, UCA0BR1, UCA0STAT, UCA0TXBUF, UCA0RXBUF, IE2, UCA0TXIE, UCA0RXIE, txbuf, rxbuf, 16, 16, P1SEL, P1SEL2, PORT_SELECTION_0_AND_1, BIT1|BIT2> Serial;


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
