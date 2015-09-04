/* Wire_USCI - I2C driver for USCI_B on MSP430 Value Line devices */

#ifndef WIRE_USCI_H
#define WIRE_USCI_H

#include <AbstractWiring.h>
#include <TwoWire_USCI_EXTISR.h>

template <
>

class Wire_USCI : public TwoWire_USCI_EXTISR {
    public:
        void isr_handle_txrx(void) {
        };

        void isr_handle_control(void) {
        };

        void begin(void) {
        };

        void begin(int addr, boolean has_10bit) {
        };

        void begin(int addr) { begin(addr, true); };
        void begin(uint8_t addr) { begin (addr, false); };
        void end(void) {
        };
};


#endif /* WIRE_USCI_H */
