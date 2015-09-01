/* MSP430 USCI UART implementation - genericized with templates for all USCI implementations */

#ifndef UART_USCI_H
#define UART_USCI_H

#include <AbstractWiring.h>
#include <AbstractSerial.h>

struct USCI_UART_Buffer {
    volatile char *txbuffer, *rxbuffer;
    volatile unsigned int tx_head, tx_tail;
    volatile unsigned int rx_head, rx_tail;
};

enum PortselMode {
    PORT_SELECTION_NONE = 0,
    PORT_SELECTION_0,
    PORT_SELECTION_1,
    PORT_SELECTION_0_AND_1
};

typedef const volatile uint8_t * u8_CSFR;
typedef volatile uint8_t * u8_SFR;
typedef const volatile uint16_t * u16_CSFR;
typedef volatile uint16_t * u16_SFR;

template <
    u8_SFR ucactl0,
    u8_SFR ucactl1,
    u8_SFR ucamctl,
    u8_SFR ucaabctl,
    u8_SFR ucabr0,
    u8_SFR ucabr1,
    u8_SFR ucastat,
    u8_SFR ucatxbuf,
    u8_SFR ucarxbuf,
    u8_SFR ucaie,
    uint8_t ucatxie,
    uint8_t ucarxie,
    volatile char * txbuffer,
    volatile char * rxbuffer,
    size_t tx_buffer_size,
    size_t rx_buffer_size,
    u8_SFR pxsel,
    u8_SFR pxsel2,
    enum PortselMode pxsel_specification,
    uint8_t pxbits >

class UART_USCI : public AbstractSerial {
    private:
        volatile struct USCI_UART_Buffer buffer;
        unsigned long _baud;
        volatile SERIAL_BREAK_CALLBACK breakcb;

    public:
        UART_USCI();
        virtual void begin(unsigned long bitrate);
        virtual void end(void);

        virtual int available(void);
        virtual int peek(void);
        virtual int read(void);
        virtual void flush(void);
        virtual size_t write(uint8_t);
        operator bool();
        void isr_send_char(void);       // Utility function used by ISR handler
        void isr_get_char(void);        // Utility function used by ISR handler

        void configClock(unsigned long bitrate);

        // Optional API is implemented
        virtual boolean hasExtendedAPI(void) { return true; };
        virtual void set7Bit(boolean);
        virtual void setStopBits(int);
        virtual void setParity(enum SerialParity);
        virtual void sendBreak(void);
        virtual void attachBreakInterrupt(SERIAL_BREAK_CALLBACK);
        virtual void detachBreakInterrupt(void);
};
    


#endif /* UART_USCI_H */
