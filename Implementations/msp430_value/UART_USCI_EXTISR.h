/* UART_USCI intermediate class defining the ISR handlers - so that the ISRs can reference a non-templated class name */

#ifndef UART_USCI_EXTISR_H
#define UART_USCI_EXTISR_H


#include <AbstractSerial.h>

class UART_USCI_EXTISR : public AbstractSerial {
    public:
        virtual void isr_send_char(void) = 0;       // Utility function used by ISR handler
        virtual void isr_get_char(void) = 0;        // Utility function used by ISR handler
};


#endif /* UART_USCI_EXTISR_H */
