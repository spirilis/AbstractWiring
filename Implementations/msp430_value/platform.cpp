/* Provide all the implementation-specific functions defined in platform.h to make a basic Arduino/Wiring platform workable. */

#include <Arduino.h>

void pinMode(int pin, int mode)
{
}

void digitalWrite(int pin, uint8_t value)
{
}

int digitalRead(int pin)
{
	return LOW;
}

uint16_t analogRead(int pin)
{
	return 0;
}

void analogWrite(int pin, int pwmvalue)
{
}

void analogReference(int vref)
{
}

void analogFrequency(uint16_t freq)
{
}

void delay(uint32_t ms)
{
}

void delayMicroseconds(uint16_t us)
{
}

void sleep(uint32_t ms)
{
}

void suspend(void)
{
}

void wakeup(void)
{
}

void attachInterrupt(int pin, void (*userFunc)(void), int mode)
{
}

void detachInterrupt(int pin)
{
}

unsigned long micros(void)
{
	return 0;
}

unsigned long millis(void)
{
	return 0;
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

uint16_t makeWord(uint16_t w)
{
    return w;
}

uint16_t makeWord(uint8_t h, uint8_t l)
{
    return (h << 8) | l;
}
