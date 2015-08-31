/* AbstractWiring SPIClass abstract class for all SPI implementations. */

#ifndef SPI_H_INCLUDED
#define SPI_H_INCLUDED

#include <Arduino.h>

/* Implementation-specific subclass should define the following features if implemented:
 *
 * #define SPI_HAS_TRANSACTION 1
 * #define SPI_HAS_TRANSACTION_SEMAPHORE 1
 * #define SPI_HAS_TRANSFER16 1
 * #define SPI_HAS_TRANSFER9 1
 *
 * All implementations must provide transfer16(), transfer9(), beginTransaction(), endTransaction(), attachInterrupt(), detachInterrupt(),
 * usingInterrupt() but do not necessarily need any code in those functions.
 * If it does have code in those functions, however, it should #define the appropriate flags up above to indicate to the user's firmware that
 * they are available.
 */

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3
#define SPI_MODE_0 SPI_MODE0
#define SPI_MODE_1 SPI_MODE1
#define SPI_MODE_2 SPI_MODE2
#define SPI_MODE_3 SPI_MODE3

// SPISettings is required for transaction support, but your implementation of SPIClass can use it or ignore it.
class SPISettings {
    private:
        uint32_t _clock;
        uint8_t _bitorder;
        uint8_t _datamode;

        friend class SPIClass;

    public:
        SPISettings(uint32_t clockrate, uint8_t bitorder, uint8_t datamode) {
            _clock = clockrate;
            _bitorder = bitorder;
            _datamode = datamode;
        }

        SPISettings() {
            _clock = 4000000UL;
            _bitorder = MSBFIRST;
            _datamode = SPI_MODE0;
        }
}

class SPIClass {
    public:
        // Basic API
        virtual void begin() = 0;
        virtual void end() = 0;
        virtual uint8_t transfer(uint8_t) = 0;
        virtual void setBitOrder(enum bitorder) = 0;
        virtual void setDataMode(enum datamode) = 0;
        virtual void setClockDivider(int clockDiv) = 0;
        virtual void attachInterrupt() { };  // This doesn't do anything in Arduino anyhow
        virtual void detachInterrupt() { };  // This doesn't do anything in Arduino anyhow

        // Extended API features; default implementation is null
        virtual uint16_t transfer16(uint16_t) { };
        virtual uint16_t transfer9(uint16_t) { };
        virtual boolean beginTransaction(SPISettings settings) { };
        virtual void endTransaction(void) { };
        virtual void usingInterrupt(int pin) { };
};

#endif /* SPI_H_INCLUDED */
