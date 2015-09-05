/* AbstractWiring - platform.h concrete driver adapter for MSP430 small "value line" chips using GCC. */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <msp430.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

/* platform.h must provide:
 * definition of F_CPU for libraries that require it
 * function prototypes for interrupts() and noInterrupts() to enable/disable global IRQs
 * function prototypes for pinMode, digitalWrite, digitalRead, analogRead, analogWrite, analogReference, analogFrequency
 * function prototypes for delay, delayMicroseconds, optionally sleep, suspend, wakeup
 * function prototypes for attachInterrupt, detachInterrupt
 * function prototypes for micros() and millis()
 * function prototypes for pulseIn(), tone() and noTone()
 * function prototypes for makeWord(uint16_t), makeWord(uint8_t h, uint8_t l)
 * provide access to any Serial port implementations; it's strongly recommended this file provide an
 *   "extern <some derivative class type of AbstractSerial> Serial;" so all apps including Arduino.h will
 *   have access to the "Serial" object.
 */

#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE __attribute__((noinline))

#define F_CPU 16000000L

enum PortselMode {
    PORT_SELECTION_NONE = 0,
    PORT_SELECTION_0,
    PORT_SELECTION_1,
    PORT_SELECTION_0_AND_1
};

typedef const volatile uint8_t & u8_CSFR;
typedef volatile uint8_t & u8_SFR;
typedef const volatile unsigned int & u16_CSFR;
typedef volatile unsigned int & u16_SFR;


// C linkage (no function overloading)
#ifdef __cplusplus
extern "C" {
#endif

void sysinit(unsigned long mclk);  // This implementation requires main() run sysinit() after DCO, BCSCTL1, BCSCTL2 and GIE are ready to roll.

ALWAYS_INLINE void interrupts() { __bis_SR_register(GIE); }
ALWAYS_INLINE void noInterrupts() { __bic_SR_register(GIE); }

static const int P1_0 = 1;
static const int P1_1 = 2;
static const int P1_2 = 3;
static const int P1_3 = 4;
static const int P1_4 = 5;
static const int P1_5 = 6;
static const int P1_6 = 7;
static const int P1_7 = 8;
static const int P2_0 = 9;
static const int P2_1 = 10;
static const int P2_2 = 11;
static const int P2_3 = 12;
static const int P2_4 = 13;
static const int P2_5 = 14;
static const int P2_6 = 15;
static const int P2_7 = 16;
#ifdef __MSP430_HAS_PORT3_R__
static const int P3_0 = 17;
static const int P3_1 = 18;
static const int P3_2 = 19;
static const int P3_3 = 20;
static const int P3_4 = 21;
static const int P3_5 = 22;
static const int P3_6 = 23;
static const int P3_7 = 24;
#ifdef __MSP430_HAS_PORT4_R__
static const int P4_0 = 25;
static const int P4_1 = 26;
static const int P4_2 = 27;
static const int P4_3 = 28;
static const int P4_4 = 29;
static const int P4_5 = 30;
static const int P4_6 = 31;
static const int P4_7 = 32;
#endif
#endif

void pinMode(int, int);
void digitalWrite(int, uint8_t);
int digitalRead(int);
uint16_t analogRead(int);
void analogWrite(int, int);
void analogReference(int);
void analogFrequency(uint16_t);

void delay(uint32_t milliseconds);
void delayMicroseconds(uint16_t us);
void sleep(uint32_t milliseconds);
void suspend(void);
void wakeup(void);

void attachInterrupt(int, void (*)(void), int mode);
void detachInterrupt(int);
void maskInterrupt(int);
void unmaskInterrupt(int);
unsigned long micros();
unsigned long millis();
extern volatile uint32_t _sys_millis;
extern volatile uint16_t _sys_micros;
extern volatile boolean _sys_asleep;

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout = 1000000L);
void tone(int _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(int _pin);

extern uint8_t _bitvect[];  // faster _BV for apps that only need an 8-bit number
#define _BV(x) (1 << x)

char * itoa( int value, char *string, int radix ) ;
char * ltoa( long value, char *string, int radix ) ;
char * utoa( unsigned long value, char *string, int radix ) ;
char * ultoa( unsigned long value, char *string, int radix ) ;
char * dtostrf (double val, signed char width, unsigned char prec, char *sout);

void set_pxsel(u8_SFR, u8_SFR, enum PortselMode, uint8_t);

#ifdef __cplusplus
}; /* extern "C" */
#endif

// C++ linkage (function overloading allowed)
uint16_t makeWord(uint16_t);
uint16_t makeWord(uint8_t, uint8_t);



#endif
