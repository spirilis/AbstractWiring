/* AbstractWiring Serial implementation for all UART implementations, software or hardware. */

#ifndef ABSTRACTSERIAL_H_INCLUDED
#define ABSTRACTSERIAL_H_INCLUDED

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
};

#endif /* ABSTRACTSERIAL_H_INCLUDED */
