/* USCI_A0 UART implementation for MSP430 Value Line */

#include <AbstractWiring.h>
#include <UART_USCI.h>

#define TX_BUFFER_SIZE 16
#define RX_BUFFER_SIZE 16

struct USCI_UART_Buffer {
    char txbuffer[TX_BUFFER_SIZE];
    char rxbuffer[RX_BUFFER_SIZE];
    volatile uint16_t tx_head, tx_tail;
    volatile uint16_t rx_head, rx_tail;
};

class UART_USCIA0 : public UART_USCI {
    private:
        volatile struct USCI_UART_Buffer buffer;
        unsigned long _baud;
        volatile SERIAL_BREAK_CALLBACK breakcb;

    public:
        UART_USCIA0();
        virtual void begin(unsigned long bitrate);
        virtual void end(void);

        virtual int available(void);
        virtual int peek(void);
        virtual int read(void);
        virtual void flush(void);
        virtual size_t write(uint8_t);
        operator bool();
        void isr_send_char(void);  // Utility function used by IRQ handler
        void isr_get_char(void);    // Utility function used by IRQ handler

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
