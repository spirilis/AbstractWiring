/* TwoWire_USCI intermediate class defining the ISR handlers - so that the ISRs can reference a non-templated class name */

#ifndef TWOWIRE_USCI_EXTISR_H
#define TWOWIRE_USCI_EXTISR_H


#include <Wire.h>

class TwoWire_USCI_EXTISR : public TwoWire {
    public:
        virtual boolean isr_handle_txrx(void) = 0;       // Utility function used by ISR handler
        virtual boolean isr_handle_control(void) = 0;    // Utility function used by ISR handler
        // These functions return boolean indicating:
        // true = Wake CPU from LPM
        // false = Don't wake CPU
};


#endif /* TWOWIRE_USCI_EXTISR_H */
