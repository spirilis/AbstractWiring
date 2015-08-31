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

ALWAYS_INLINE void interrupts() { __bis_SR_register(GIE); }
ALWAYS_INLINE void noInterrupts() { __bic_SR_register(GIE); }

void pinMode(int, int);
void digitalWrite(int, uint8_t);
int digitalRead(int);
uint16_t analogRead(int);
void analogWrite(int, int);
void analogReference(int);
void analogFrequency(uint16_t);

void delay(uint32_t milliseconds);
void delayMicroseconds(unsigned int us);
void sleep(uint32_t milliseconds);
void suspend(void);
void wakeup(void);

void attachInterrupt(int, void (*)(void), int mode);
void detachInterrupt(int);
unsigned long micros();
unsigned long millis();

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout = 1000000L);
void tone(int _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(int _pin);

uint16_t makeWord(uint16_t);
uint16_t makeWord(uint8_t, uint8_t);


#endif
