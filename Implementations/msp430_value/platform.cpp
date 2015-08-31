/* Provide all the implementation-specific functions defined in platform.h to make a basic Arduino/Wiring platform workable. */

#include <Arduino.h>
#include <s_printf.h>

// C linkage
extern "C" {

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


