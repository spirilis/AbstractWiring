/* Provide all the implementation-specific functions defined in platform.h to make a basic Arduino/Wiring platform workable. */

#include <Arduino.h>
#include <s_printf.h>
#include <ADC10.h>

// ADC instance
ADC10 adc;

// C linkage
extern "C" {

// MSP430 Value-Line GPIO implementation:
// Pin# equals P1.0 thru P4.7, with 1-8 = P1.0-P1.7, 9-16 = P2.0-P2.7, 17-24 = P3.0-P3.7, 25-32 = P4.0-P4.7

enum msp430_sfr {
    PxIN = 1, PxOUT, PxDIR, PxREN, PxSEL, PxSEL2
};

// avoid bitshifts with this
const uint8_t _bitvect[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};


// This stuff would normally be split out into a pins.h or pins.c file (e.g. pins_energia.h from Energia)
#define CVOL (const volatile uint8_t *)
#define VOL (volatile uint8_t *)

const volatile uint8_t * const _pxin[] = {CVOL &P1IN, CVOL &P2IN,
#ifdef __MSP430_HAS_PORT3_R__
CVOL &P3IN,
#else
NULL,
#endif
#ifdef __MSP430_HAS_PORT4_R__
CVOL &P4IN,
#else
NULL
#endif
};

const volatile uint8_t * const _pxout[] = {CVOL &P1OUT, CVOL &P2OUT,
#ifdef __MSP430_HAS_PORT3_R__
CVOL &P3OUT,
#else
NULL,
#endif
#ifdef __MSP430_HAS_PORT4_R__
CVOL &P4OUT
#else
NULL
#endif
};

const volatile uint8_t * const _pxdir[] = {CVOL &P1DIR, CVOL &P2DIR,
#ifdef __MSP430_HAS_PORT3_R__
CVOL &P3DIR,
#else
NULL,
#endif
#ifdef __MSP430_HAS_PORT4_R__
CVOL &P4DIR,
#else
NULL
#endif
};

const volatile uint8_t * const _pxren[] = {CVOL &P1REN, CVOL &P2REN,
#ifdef __MSP430_HAS_PORT3_R__
CVOL &P3REN,
#else
NULL,
#endif
#ifdef __MSP430_HAS_PORT4_R__
CVOL &P4REN
#else
NULL
#endif
};

const volatile uint8_t * const _pxsel[] = {CVOL &P1SEL, CVOL &P2SEL,
#ifdef __MSP430_HAS_PORT3_R__
CVOL &P3SEL,
#else
NULL,
#endif
#ifdef __MSP430_HAS_PORT4_R__
CVOL &P4SEL,
#else
NULL
#endif
};

const volatile uint8_t * const _pxsel2[] = {CVOL &P1SEL2, CVOL &P2SEL2,
#ifdef __MSP430_HAS_PORT3_R__
CVOL &P3SEL2,
#else
NULL,
#endif
#ifdef __MSP430_HAS_PORT4_R__
CVOL &P4SEL2,
#else
NULL
#endif
};

static volatile uint8_t * _msp430_get_port(int pin, enum msp430_sfr sfr)
{
    int lidx = (pin - 1) / 8;    

    switch (sfr) {
        case PxIN:
            return VOL _pxin[lidx];
            break;
        case PxOUT:
            return VOL _pxout[lidx];
            break;
        case PxDIR:
            return VOL _pxdir[lidx];
            break;
        case PxREN:
            return VOL _pxren[lidx];
            break;
        case PxSEL:
            return VOL _pxsel[lidx];
            break;
        case PxSEL2:
            return VOL _pxsel2[lidx];
            break;
    }
    return NULL;
}
//////////////////////


void pinMode(int pin, int mode)
{
    if (pin < 1 || pin > 32)
        return;
    int pxidx = (pin - 1) % 8;
    uint8_t pxbit = _bitvect[pxidx];

    volatile uint8_t *pxdir = _msp430_get_port(pin, PxDIR);
    volatile uint8_t *pxren = _msp430_get_port(pin, PxREN);
    volatile uint8_t *pxsel = _msp430_get_port(pin, PxSEL);
    volatile uint8_t *pxsel2 = _msp430_get_port(pin, PxSEL2);
    *pxsel &= ~pxbit;
    *pxsel2 &= ~pxbit;

    switch (mode) {
        case INPUT:
        case INPUT_PULLUP:
        case INPUT_PULLDOWN:
            *pxdir &= ~pxbit;
            if (mode > INPUT) {
                volatile uint8_t *pxout = _msp430_get_port(pin, PxOUT);
                if (mode == INPUT_PULLUP) {
                    *pxout |= pxbit;
                    *pxren |= pxbit;
                } else {
                    *pxout &= ~pxbit;
                    *pxren |= pxbit;
                }
            } else {
                *pxren &= ~pxbit;
            }
            break;
        case OUTPUT:
            *pxdir |= pxbit;
            break;
    }
}

void digitalWrite(int pin, uint8_t value)
{
    if (pin < 1 || pin > 32)
        return;
    uint8_t pxbit = _bitvect[(pin - 1) % 8];

    volatile uint8_t *pxout = _msp430_get_port(pin, PxOUT);
    if (value)
        *pxout |= pxbit;
    else
        *pxout &= ~pxbit;
}

int digitalRead(int pin)
{
    if (pin < 1 || pin > 32)
        return 0;
    uint8_t pxbit = _bitvect[(pin - 1) % 8];

    volatile uint8_t *pxin = _msp430_get_port(pin, PxIN);
    if (*pxin & pxbit)
        return HIGH;
    else
        return LOW;
}

uint16_t analogRead(int channel)
{
    return adc.sample(channel);
}

void analogReference(enum AdcVRef ref)
{
    adc.setReference(ref);
}

void analogWrite(int pin, int pwmvalue)
{
}

void analogFrequency(uint16_t freq)
{
}

volatile boolean _sys_asleep;

void delay(uint32_t ms)
{
    register uint32_t ms_start = _sys_millis;
    _sys_asleep = true;
    while ( _sys_asleep && ((_sys_millis - ms_start) < ms) )
        LPM0;
    _sys_asleep = false;
}

void delayMicroseconds(uint16_t us)
{
    uint32_t i=0;

    // this is highly inaccurate, was too lazy to come up with a "real" implementation or rip off Energia's
    for (i=0; i < us; i++) {
        __delay_cycles(2);
    }
}

void sleep(uint32_t ms)
{
    delay(ms);
}

void suspend(void)
{
    _sys_asleep = true;
    while (_sys_asleep)
        LPM4;
    _sys_asleep = false;
}

void wakeup(void)
{
    _sys_asleep = false;
}

typedef void(*msp430_irq_callback)(void);
struct msp430_irq_mode_is_change {
	uint8_t p1;
	uint8_t p2;
};

volatile struct msp430_irq_mode_is_change intVectMode;
volatile msp430_irq_callback intVect_P1[8];
volatile msp430_irq_callback intVect_P2[8];

void attachInterrupt(int pin, void (*userFunc)(void), int mode)
{
    if (pin > 16)
        return;

    int pxidx = (pin - 1) % 8;
    uint8_t pxbit = _bitvect[pxidx];

    if (pin < 9) {
        // P1
        intVect_P1[pxidx] = userFunc;
        intVectMode.p1 &= ~pxbit;

        if (mode == RISING)
            P1IES &= ~pxbit;
        if (mode == FALLING)
            P1IES |= pxbit;
        P1IFG &= ~pxbit;
        if (mode == CHANGE) {
            P1IES = (P1IES & ~pxbit) | (P1IN & pxbit);
            intVectMode.p1 |= pxbit;
        }
        P1IE |= pxbit;
        return;
    }
    // P2
    intVect_P2[pxidx] = userFunc;
    intVectMode.p2 &= ~pxbit;
    if (mode == RISING)
        P2IES &= ~pxbit;
    if (mode == FALLING)
        P2IES |= pxbit;
    P2IFG &= ~pxbit;
    if (mode == CHANGE) {
        P2IES = (P2IES & ~pxbit) | (P2IN & pxbit);
        intVectMode.p2 |= pxbit;
    }
    P2IE |= pxbit;
    return;
}

void detachInterrupt(int pin)
{
    if (pin > 16)
        return;

    int pxidx = (pin - 1) % 8;
    uint8_t pxbit = _bitvect[pxidx];

    if (pin < 9) {
        P1IE &= ~pxbit;
        intVect_P1[pxidx] = NULL;
        return;
    }
    P2IE &= ~pxbit;
    intVect_P2[pxidx] = NULL;
    return;
}

// maskInterrupt and unmaskInterrupt are used by SPI_USCI for usingInterrupt() Transaction support.
void maskInterrupt(int pin)
{
    if (pin > 16)
        return;

    int pxidx = (pin - 1) % 8;
    uint8_t pxbit = _bitvect[pxidx];

    if (pin < 9) {
        P1IE &= ~pxbit;
        return;
    }
    P2IE &= ~pxbit;
}

void unmaskInterrupt(int pin)
{
    if (pin > 16)
        return;

    int pxidx = (pin - 1) % 8;
    uint8_t pxbit = _bitvect[pxidx];

    if (pin < 9) {
        if (intVect_P1[pxidx] != NULL)
            P1IE |= pxbit;
        return;
    }
    if (intVect_P2[pxidx] != NULL)
        P2IE |= pxbit;
}

__attribute__((interrupt(PORT1_VECTOR)))
void P1_ISR(void)
{
    uint16_t i = 0, j = 0x01;
    while (!(j & 0x0100)) {
        if ((P1IFG & j) && (P1IE & j)) {
            intVect_P1[i]();
            P1IFG &= ~j;
            if (intVectMode.p1 & j)
                P1IES ^= j;
        }
        i++;
        j *= 2;
    }
    if (!_sys_asleep)
        __bic_SR_register_on_exit(LPM4_bits);
}

__attribute__((interrupt(PORT2_VECTOR)))
void P2_ISR(void)
{
    uint16_t i = 0, j = 0x01;
    while (!(j & 0x0100)) {
        if ((P2IFG & j) && (P2IE & j)) {
            intVect_P2[i]();
            P2IFG &= ~j;
            if (intVectMode.p2 & j)
                P2IES ^= j;
        }
        i++;
        j *= 2;
    }
    if (!_sys_asleep)
        __bic_SR_register_on_exit(LPM4_bits);
}

unsigned long micros(void)
{
	return _sys_millis * 1000UL + _sys_micros;
}

unsigned long millis(void)
{
	return _sys_millis;
}

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout)
{
	return 0;
}

void tone(int _pin, unsigned int frequency, unsigned long duration)
{
}

void noTone(int _pin)
{
}

char * itoa( int value, char *string, int radix )
{
  return ltoa( value, string, radix ) ;
}

char * ltoa( long value, char *string, int radix )
{
  char tmp[33];
  char *tp = tmp;
  long i;
  unsigned long v;
  int sign;
  char *sp;

  if ( string == NULL )
  {
    return 0 ;
  }

  if (radix > 36 || radix <= 1)
  {
    return 0 ;
  }

  sign = (radix == 10 && value < 0);
  if (sign)
  {
    v = -value;
  }
  else
  {
    v = (unsigned long)value;
  }

  while (v || tp == tmp)
  {
    i = v % radix;
    v = v / radix;
    if (i < 10)
      *tp++ = i+'0';
    else
      *tp++ = i + 'a' - 10;
  }

  sp = string;

  if (sign)
    *sp++ = '-';
  while (tp > tmp)
    *sp++ = *--tp;
  *sp = 0;

  return string;
}

char * utoa( unsigned long value, char *string, int radix )
{
  return ultoa( value, string, radix ) ;
}

char * ultoa( unsigned long value, char *string, int radix )
{
  char tmp[33];
  char *tp = tmp;
  long i;
  unsigned long v = value;
  char *sp;

  if ( string == NULL )
  {
    return 0;
  }

  if (radix > 36 || radix <= 1)
  {
    return 0;
  }
 
  while (v || tp == tmp)
  {
    i = v % radix;
    v = v / radix;
    if (i < 10)
      *tp++ = i+'0';
    else
      *tp++ = i + 'a' - 10;
  }

  sp = string;

 
  while (tp > tmp)
    *sp++ = *--tp;
  *sp = 0;

  return string;
}

char * dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  
  int whole = val;
  float mantissa = val - whole;

  int32_t frac = mantissa * powf(10, prec);
  if(frac < 0) frac = -frac;

  s_printf(fmt, "%%0%dd.%%0%dd", width, prec);
  s_printf(sout, fmt, whole, frac);
  return sout;
}

volatile uint32_t _sys_millis;
volatile uint16_t _sys_micros;
volatile uint16_t _sys_micros_per_wdt;

__attribute__((interrupt(WDT_VECTOR)))
void watchdog_isr (void)
{
    // copy these to local variables so they can be stored in registers
    // (volatile variables must be read from memory on every access)
    register uint32_t m = _sys_millis;
    register uint16_t f = _sys_micros;

    m += _sys_micros_per_wdt / 1000;
    f += _sys_micros_per_wdt % 1000;

    if (f >= 1000) {
        f -= 1000;
        m++;
    }

    _sys_millis = m;
    _sys_micros = f;

    /* Exit from LPM on reti */
    __bic_SR_register_on_exit(LPM4_bits);
}

__attribute__((interrupt(ADC10_VECTOR)))
void ADC10_ISR(void)
{
    __bic_SR_register_on_exit(LPM4_bits);
}

void sysinit(unsigned long mclk)
{
    if (mclk < 8000000UL) {
        WDTCTL = WDTPW | WDTCNTCL | WDTTMSEL | WDTIS1;
        _sys_micros_per_wdt = 512 / (mclk / 1000000UL);
    } else {
        WDTCTL = WDTPW | WDTCNTCL | WDTTMSEL | WDTIS0;
        _sys_micros_per_wdt = 8192 / (mclk / 1000000UL);
    }
    _sys_millis = 0;
    _sys_micros = 0;
    IFG1 &= ~WDTIFG;
    IE1 |= WDTIE;
    __bis_SR_register(GIE);
}

void set_pxsel(u8_SFR sel, u8_SFR sel2, enum PortselMode mode, uint8_t bits) {
    switch (mode) {
        case PORT_SELECTION_NONE:
            sel &= ~bits;
            sel2 &= ~bits;
            break;
        case PORT_SELECTION_0:
            sel |= bits;
            sel2 &= ~bits;
            break;
        case PORT_SELECTION_1:
            sel &= ~bits;
            sel2 |= bits;
            break;
        case PORT_SELECTION_0_AND_1:
            sel |= bits;
            sel2 |= bits;
            break;
    }
}



};  /* extern "C" */

// C++ linkage
uint16_t makeWord(uint16_t w)
{
    return w;
}

uint16_t makeWord(uint8_t h, uint8_t l)
{
    return (h << 8) | l;
}


