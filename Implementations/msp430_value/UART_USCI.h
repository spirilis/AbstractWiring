/* USCI Abstract UART class - defines ISR handler functions */

#include <AbstractWiring.h>
#include <AbstractSerial.h>
#include <Stream.h>

class UART_USCI : public AbstractSerial {
    public:
        virtual void isr_send_char(void);   // Utility function used by IRQ handler
        virtual void isr_get_char(void);    // Utility function used by IRQ handler
};
