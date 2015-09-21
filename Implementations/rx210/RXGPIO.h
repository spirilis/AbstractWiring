/* GPIO type for Renesas RX (useful for hardcoding port+bit information
 * for use by other templated C++ class libraries)
 *
 * Applicable for RX2xx and RX63x chips only thus far.
 */

#ifndef RXGPIO_H
#define RXGPIO_H

#include <AbstractWiring.h>
#include <iodefine.h>
#include <rx_peripherals.h>

// RX GPIO Ports - generic struct for the basics
struct rx_gpio_port_t {
	uint8_t PDR;
	char deadspace0[31];
	uint8_t PODR;
	char deadspace1[31];
	uint8_t PIDR;
	char deadspace2[31];
	uint8_t PMR;
	char deadspace3[31];
};

enum rx_gpio_port {
	RX_PORT0 = 0,
	RX_PORT1,
	RX_PORT2,
	RX_PORT3,
	RX_PORT4,
	RX_PORT5,
	RX_PORT6,
	RX_PORT7,
	RX_PORT8,
	RX_PORT9,
	RX_PORTA,
	RX_PORTB,
	RX_PORTC,
	RX_PORTD,
	RX_PORTE,
	RX_PORTF,
	RX_PORTG,
	RX_PORTH,
	RX_PORTJ,
	RX_PORTK,
	RX_PORTL,
};

typedef volatile struct rx_gpio_port_t rx_portstruct;

template <const rx_gpio_port portnum>
struct RXGPIO_PORT {
	static const rx_gpio_port _portnum = portnum;

	__inline static rx_portstruct * portdef() {
		if (_portnum == RX_PORT0)
			return (rx_portstruct *)&SFRBASE_PORT0;
		if (_portnum == RX_PORT1)
			return (rx_portstruct *)&SFRBASE_PORT1;
		if (_portnum == RX_PORT2)
			return (rx_portstruct *)&SFRBASE_PORT2;
		if (_portnum == RX_PORT3)
			return (rx_portstruct *)&SFRBASE_PORT3;
		if (_portnum == RX_PORT4)
			return (rx_portstruct *)&SFRBASE_PORT4;
		if (_portnum == RX_PORT5)
			return (rx_portstruct *)&SFRBASE_PORT5;
		if (_portnum == RX_PORT6)
			return (rx_portstruct *)&SFRBASE_PORT6;
		if (_portnum == RX_PORT7)
			return (rx_portstruct *)&SFRBASE_PORT7;
		if (_portnum == RX_PORT8)
			return (rx_portstruct *)&SFRBASE_PORT8;
		if (_portnum == RX_PORT9)
			return (rx_portstruct *)&SFRBASE_PORT9;
		if (_portnum == RX_PORTA)
			return (rx_portstruct *)&SFRBASE_PORTA;
		if (_portnum == RX_PORTB)
			return (rx_portstruct *)&SFRBASE_PORTB;
		if (_portnum == RX_PORTC)
			return (rx_portstruct *)&SFRBASE_PORTC;
		if (_portnum == RX_PORTD)
			return (rx_portstruct *)&SFRBASE_PORTD;
		if (_portnum == RX_PORTE)
			return (rx_portstruct *)&SFRBASE_PORTE;
		if (_portnum == RX_PORTF)
			return (rx_portstruct *)&SFRBASE_PORTF;
#ifdef PORTG
		if (_portnum == RX_PORTG)
			return (rx_portstruct *)&SFRBASE_PORTG;
#endif
#ifdef PORTH
		if (_portnum == RX_PORTH)
			return (rx_portstruct *)&SFRBASE_PORTH;
#endif
#ifdef PORTI
		if (_portnum == RX_PORTI)
			return (rx_portstruct *)&SFRBASE_PORTI;
#endif
#ifdef PORTJ
		if (_portnum == RX_PORTJ)
			return (rx_portstruct *)&SFRBASE_PORTJ;
#endif
#ifdef PORTK
		if (_portnum == RX_PORTK)
			return (rx_portstruct *)&SFRBASE_PORTK;
#endif
#ifdef PORTL
		if (_portnum == RX_PORTL)
			return (rx_portstruct *)&SFRBASE_PORTL;
#endif

		return (rx_portstruct *)&SFRBASE_PORT0;
	}

	__inline static volatile uint8_t & PIN() { return portdef()->PIDR; }
	__inline static volatile uint8_t & POUT() { return portdef()->PODR; }
	__inline static volatile uint8_t & PDIR() { return portdef()->PDR; }
};

// RX GPIO Pins (based on the PORT)
template <
	typename PORT,
	uint8_t BIT >
struct RXGPIO_PIN {
	static const uint8_t _pinmask = (1 << BIT);

	__inline static volatile uint8_t & PIN() { return PORT::PIN(); }
	__inline static volatile uint8_t & POUT() { return PORT::POUT(); }
	__inline static volatile uint8_t & PDIR() { return PORT::PDIR(); }

	__inline static boolean read() {
		if (PORT::PIN() & _pinmask)
			return 1;
		return 0;
	};

	__inline static void write(boolean value) {
		if (value)
			PORT::POUT() |= _pinmask;
		else
			PORT::POUT() &= ~_pinmask;
	};

	__inline static void setMode(boolean outyn) {
		if (outyn)
			PORT::PDIR() |= _pinmask;
		else
			PORT::PDIR() &= ~_pinmask;
	}
};

#endif /* RXGPIO_H */
