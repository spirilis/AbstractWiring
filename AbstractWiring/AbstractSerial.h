/* AbstractWiring Serial implementation for all UART implementations, software or hardware. */

#ifndef ABSTRACTSERIAL_H_INCLUDED
#define ABSTRACTSERIAL_H_INCLUDED

#include <AbstractWiring.h>
#include <Stream.h>

enum SerialParity {
    PARITY_NONE = 0,
    PARITY_ODD,
    PARITY_EVEN
};

typedef void(*SERIAL_BREAK_CALLBACK)(void);

class AbstractSerial : public Stream {
    public:
        virtual void begin(unsigned long bitrate) = 0;
        virtual void end(void) = 0;

        virtual int available(void) = 0;
        virtual int peek(void) = 0;
        virtual int read(void) = 0;
        virtual void flush(void) = 0;
        virtual size_t write(uint8_t) = 0;
        using Print::write;
        operator bool();
	void isr_send_char(void); // Utility function used by ISR

        // Extended API - optional - if implemented by subclass, override next function to return true
        virtual boolean hasExtendedAPI(void) { return false; }

        virtual void set7Bit(boolean yn) { };
        virtual void setStopBits(int s) { };
        virtual void setParity(enum SerialParity par) { };
        virtual void sendBreak(void) { };
        virtual void attachBreakInterrupt(SERIAL_BREAK_CALLBACK) { };
        virtual void detachBreakInterrupt(void) { };
};

#endif /* ABSTRACTSERIAL_H_INCLUDED */
