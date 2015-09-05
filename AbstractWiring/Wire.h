/* AbstractWiring TwoWire implementation for all I2C implementations. */

#ifndef WIRE_H_INCLUDED
#define WIRE_H_INCLUDED

#include <AbstractWiring.h>
#include <Stream.h>


// Callback function types
typedef void(*TWOWIRE_SLAVE_TX_CALLBACK)(void);
typedef void(*TWOWIRE_SLAVE_RX_CALLBACK)(size_t);

class TwoWire : public Stream {
    public:
        virtual void begin(void) = 0;
        virtual void begin(uint8_t) = 0;
        virtual void begin(int addr) { begin((uint8_t) addr); };  // Override for >8-bit I2C addresses

        virtual void end(void) = 0;

        virtual void beginTransmission(uint8_t) = 0;
        virtual void beginTransmission(int addr) { beginTransmission((uint8_t)addr); };  // Override for >8-bit I2C addresses
        virtual boolean endTransmission(void);

        virtual uint8_t requestFrom(uint8_t addr, uint8_t len) = 0;
        virtual int requestFrom(int addr, int len) { return requestFrom((uint8_t)addr, (uint8_t)len); };  // Override for >8-bit I2C addresses

        virtual size_t write(uint8_t) = 0;
        virtual size_t write(const uint8_t *buf, size_t len) {
            size_t x = 0;
            for (x=0; x < len; x++) {
                if (write(buf[x]) == 0)
                    return x;
            }
            return x;
        };

        virtual int available(void) = 0;
        virtual int read(void) = 0;
        virtual int peek(void) = 0;
        virtual void flush(void) = 0;
        using Print::write;

        virtual void onReceive(TWOWIRE_SLAVE_RX_CALLBACK) = 0;
        virtual void onRequest(TWOWIRE_SLAVE_TX_CALLBACK) = 0;
};

#endif /* WIRE_H_INCLUDED */
