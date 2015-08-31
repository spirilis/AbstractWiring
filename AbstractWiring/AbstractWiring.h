/* Abstract Wiring/Arduino/Energia base classes for truly portable Wiring implementations that may be integrated into
 * any microcontroller vendor's native SDK environment.
 */

#ifndef ABSTRACTWIRING_H
#define ABSTRACTWIRING_H

/* platform.h should include basic stuff required for the microcontroller-specific SDK, e.g. #include <msp430.h> or such.
 *
 * platform.h should provide at the minimum:
 * definition of F_CPU for libraries that require it
 * function prototypes for interrupts() and noInterrupts() to enable/disable global IRQs
 *   void interrupts();
 *   void noInterrupts();
 * function prototypes for pinMode, digitalWrite, digitalRead, analogRead, analogWrite, analogReference
 *   void pinMode(int, int);
 *   void digitalWrite(int, uint8_t);
 *   int digitalRead(int);
 *   uint16_t analogRead(int);
 *   void analogWrite(int, int);
 *   void analogReference(int);
 * function prototypes for delay, delayMicroseconds, optionally sleep, suspend, wakeup
 *   void delay(uint32_t milliseconds);
 *   void delayMicroseconds(unsigned int us);
 *   void sleep(uint32_t milliseconds);
 *   void suspend(void);
 *   void wakeup(void);
 * function prototypes for attachInterrupt, detachInterrupt
 *   void attachInterrupt(int, void (*)(void), int mode);
 *   void detachInterrupt(int);
 * function prototypes for micros() and millis()
 *   unsigned long micros();
 *   unsigned long millis();
 * function prototypes for pulseIn(), tone() and noTone()
 *   unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout = 1000000L);
 *   void tone(int _pin, unsigned int frequency, unsigned long duration = 0);
 *   void noTone(int _pin);
 * function prototypes for makeWord(uint16_t), makeWord(uint8_t h, uint8_t l)
 *   unsigned int makeWord(uint16_t w);
 *   unsigned int makeWord(uint8_t h, uint8_t l);
 *
 * provide access to any Serial port implementations; it's strongly recommended this file provide an
 *   "extern <some derivative class type of AbstractSerial> Serial;" so all apps including Arduino.h will
 *   have access to the "Serial" object.
 *
 *
 * Since these are all very implementation-specific, platform.h should broker their exposure to the general-purpose libraries
 * the user might use with this system.
 *
 * Different implementation codebases may handle these differently, in fancier ways or tailored to specific subsets of the available
 * hardware.  It's left intentionally vague and abstract so libraries can guarantee certain functionality will behave 
 *
 * Also, new.h and new.cpp should be implemented by each implementation to provide the C++ primitives for creating & destroying objects.
 */
#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif


#define LOW 0
#define HIGH 1
#define LSBFIRST 0
#define MSBFIRST 1
#define RISING 0
#define FALLING 1
#define CHANGE 2
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 4

#define true 1
#define false 0

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105


typedef uint8_t boolean;
typedef uint8_t byte;
typedef uint16_t word;

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))


#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
#include <WCharacter.h>
#include <WString.h>

// makeWord() is defined in platform.h in case the CPU has a super-fast way of doing it.
#define word(...) makeWord(__VA_ARGS__)

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long map(long, long, long, long, long);
#include <dtostrf.h>

#endif /* __cplusplus */


#endif /* ABSTRACTWIRING_H */
