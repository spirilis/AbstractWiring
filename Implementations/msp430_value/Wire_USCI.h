/* Wire_USCI - I2C driver for USCI_B on MSP430 Value Line devices */

#ifndef WIRE_USCI_H
#define WIRE_USCI_H

#include <AbstractWiring.h>
#include <TwoWire_USCI_EXTISR.h>
#include <usci_isr.h>


enum USCI_TwoWire_state {
    TWI_IDLE = 0,
    TWI_MRX,
    TWI_MTX,
    TWI_SRX,
    TWI_STX
};

enum USCI_TwoWire_error {
    TWI_ERROR_NONE = 0,
    TWI_ERROR_MTX_ADDR_NACK,
    TWI_ERROR_MRX_ADDR_NACK,
    TWI_ERROR_NACK,
    TWI_ERROR_BUS_BUSY
};



template <
    unsigned int usci_b_instance,
    u8_SFR ucbctl0,
    u8_SFR ucbctl1,
    u8_SFR ucbbr0,
    u8_SFR ucbbr1,
    u8_SFR ucbstat,
    u16_SFR i2coa,
    u16_SFR i2csa,
    u8_SFR ucbtxbuf,
    u8_CSFR ucbrxbuf,
    u8_SFR stateie,
    u8_SFR stateifg,
    u8_SFR txrxie,
    u8_SFR txrxifg,
    u8_SFR pxsel,
    u8_SFR pxsel2,
    enum PortselMode pxsel_specification,
    uint8_t pxbits,
    size_t txbuf_len,
    size_t rxbuf_len >

class Wire_USCI : public TwoWire_USCI_EXTISR {
    private:
        uint32_t _clock;
        volatile enum USCI_TwoWire_state twi_state;
        volatile enum USCI_TwoWire_error twi_error;
        TWOWIRE_SLAVE_TX_CALLBACK stx_callback;
        TWOWIRE_SLAVE_RX_CALLBACK srx_callback;
        volatile uint8_t txbuf[txbuf_len], rxbuf[rxbuf_len];
        volatile uint16_t txhead, txtail, rxhead, rxtail;
        boolean is_slave;

    public:
        NEVER_INLINE
        Wire_USCI() {
            isr_usci_twowire_instance[usci_b_instance] = this;
            stx_callback = NULL;
            srx_callback = NULL;
            twi_state = TWI_IDLE;
            _clock = 100000UL;  // Default I2C speed = 100KHz
            is_slave = false;
        };

        // IRQ matters (and, really, the most important part of the entire codebase)
        NEVER_INLINE
        boolean isr_handle_txrx(void) {
            if (txrxifg & UCB0TXIFG) {
                if (twi_state == TWI_MTX) {
                    // MTX
                    // If there is data to send, then send it; otherwise, stop.
                    if (txhead < txtail) {
                        ucbtxbuf = txbuf[txhead++];
                        return false;
                    } else {  // Last byte just sent?
                        twi_state = TWI_IDLE;
                        ucbctl1 |= UCTXSTP;
                        txrxifg &= ~UCB0TXIFG;
                        return true;  // Signal ISR to wake up CPU
                    }
                } else if (twi_state == TWI_MRX) {
                    // STX
                    // Cool feature idea to add here - "auto answer" mode
                    if (txhead == txtail)
                        ucbctl1 |= UCTXNACK;  // No more bytes to send; NACK the bus
                    else
                        ucbtxbuf = txbuf[txhead++];
                    return false;
                }
            }

            if (txrxifg & UCB0RXIFG) {
                if (twi_state == TWI_MRX) {
                    // MRX
                    rxbuf[rxhead++] = ucbrxbuf;
                    if ( (rxtail - rxhead) == 1 ) {  // Ready to read last byte; inform USCI to send NACK+STOP after
                        ucbctl1 |= UCTXSTP;
                        return false;
                    }
                    if (rxhead >= rxtail) {
                        rxhead = 0;  // Reset head to 0 in preparation for user code to read the buffer
                        twi_state = TWI_IDLE;
                        return true;
                    }
                } else {
                    // SRX
                    if (rxtail < rxbuf_len) {  // Still room available?
                        rxbuf[rxtail++] = ucbrxbuf;
                    } else {
                        ucbctl1 |= UCTXNACK;  // No; send NACK and ignore byte.
                    }
                }
            }
            return false;
        };

        NEVER_INLINE
        boolean isr_handle_control(void) {
            // Arbitration lost (master mode)
            if (stateifg & UCALIFG) {
                stateifg &= ~UCALIFG;
                twi_state = TWI_IDLE;
                twi_error = TWI_ERROR_BUS_BUSY;
                return true;
            }

            // NACK received
            if (stateifg & UCNACKIFG) {
                stateifg &= ~UCNACKIFG;
                if (twi_state == TWI_MTX || twi_state == TWI_MRX) {
                    // Could be data NACK or address NACK, hard to tell.
                    twi_error = TWI_ERROR_NACK;
                }
                twi_state = TWI_IDLE;
                return true;
            }

            // START condition
            if (stateifg & UCSTTIFG) {
                stateifg &= ~UCSTTIFG;
                twi_error = TWI_ERROR_NONE;

                if (ucbctl1 & UCTR) {
                    // Slave TX mode
                    twi_state = TWI_STX;
                    txhead = 0;
                    txtail = 0;
                    
                    // On Slave Transmit callback
                    if (stx_callback != NULL)
                        stx_callback();
                    // No data to send? NACK.
                    if (txtail == 0) {
                        ucbctl1 |= UCTXNACK;
                        twi_state = TWI_IDLE;
                    }
                } else {
                    // Slave RX mode
                    twi_state = TWI_SRX;
                    rxhead = 0;
                    rxtail = 0;
                }
            }

            // STOP condition
            if (stateifg & UCSTPIFG) {
                stateifg &= ~UCSTPIFG;

                if (twi_state == TWI_SRX) {
                    // Completion of slave RX - Run callback to process data
                    // rxtail has total length, rxhead should be 0...
                    if (srx_callback != NULL && rxhead < rxtail) {
                        // Zero-terminate data if possible (Arduino does this)
                        if (rxtail < rxbuf_len)
                            rxbuf[rxtail] = '\0';
                        srx_callback(rxtail);
                    }
                }
                twi_state = TWI_IDLE;
                return true;
            }
            return false;
        };

        // Administrative matters
        NEVER_INLINE
        void setSpeed(uint32_t bitrate) {
            _clock = bitrate;
            configClock(bitrate);
        };

        NEVER_INLINE
        void begin(void) {
            ucbctl1 = UCSSEL_2 | UCSWRST;
            ucbctl0 = UCMST | UCMODE_3 | UCSYNC;
            i2coa = 0x0000;
            i2csa = 0x0000;
            twi_state = TWI_IDLE;
            twi_error = TWI_ERROR_NONE;
            is_slave = false;
            txhead = 0;
            txtail = 0;
            rxhead = 0;
            rxtail = 0;

            configClock(_clock);

            set_pxsel(pxsel, pxsel2, pxsel_specification, pxbits);
            ucbctl1 &= ~UCSWRST;
        };

        NEVER_INLINE
        void begin(int addr) {
            ucbctl1 = UCSSEL_2 | UCSWRST;
            ucbctl0 = UCMODE_3 | UCSYNC;
            i2coa = addr & 0x1FF;
            if (i2coa & 0x180)
                ucbctl0 |= UCA10;
            i2csa = 0x0000;
            twi_state = TWI_IDLE;
            twi_error = TWI_ERROR_NONE;
            is_slave = true;
            txhead = 0;
            txtail = 0;
            rxhead = 0;
            rxtail = 0;

            configClock(_clock);

            set_pxsel(pxsel, pxsel2, pxsel_specification, pxbits);

            stateie |= UCNACKIE | UCSTPIE | UCSTTIE | UCALIE;
            txrxie |= UCB0TXIE | UCB0RXIE;

            ucbctl1 &= ~UCSWRST;
        };

        void begin(uint8_t addr) { begin (addr); };

        NEVER_INLINE
        void end(void) {
            ucbctl1 |= UCSWRST;
            twi_state = TWI_IDLE;
            twi_error = TWI_ERROR_NONE;
            set_pxsel(pxsel, pxsel2, PORT_SELECTION_NONE, pxbits);
        };

        NEVER_INLINE
        void configClock(unsigned long _i2cbitrate) {
            uint32_t smclk = F_CPU;
            uint16_t br = (uint16_t) (smclk / _i2cbitrate);

            ucbbr0 = (uint8_t) br;
            asm volatile("swpb %[b]" : [b] "=r" (br) : "r" (br));
            ucbbr1 = (uint8_t) br;
        };

        void onReceive(TWOWIRE_SLAVE_RX_CALLBACK cb) {
            srx_callback = cb;
        };

        void onRequest(TWOWIRE_SLAVE_TX_CALLBACK cb) {
            stx_callback = cb;
        };

        // Buffer usage matters
        int available(void) { return rxtail-rxhead; };
        void flush(void) { rxhead = rxtail; };

        NEVER_INLINE
        int read(void) {
            if ( (rxtail - rxhead) <= 0 )
                return -1;
            return rxbuf[rxhead++];
        };

        NEVER_INLINE
        int peek(void) {
            if ( (rxtail - rxhead) <= 0 )
                return -1;
            return rxbuf[rxhead];
        };

        NEVER_INLINE
        size_t write(uint8_t c) {
            if (txtail >= txbuf_len)
                return 0;
            txbuf[txtail++] = c;
            return 1;
        };

        // Connection management
        NEVER_INLINE
        void beginTransmission(int i2caddr) {
            i2csa = i2caddr & 0x1FF;
            if (i2caddr & 0x180)
                ucbctl0 |= UCSLA10;
            else
                ucbctl0 &= ~UCSLA10;
            txhead = 0;
            txtail = 0;
            twi_state = TWI_IDLE;
            twi_error = TWI_ERROR_NONE;
        };

        void beginTransmission(uint8_t i2caddr) { beginTransmission((int) i2caddr); };

        NEVER_INLINE
        boolean endTransmission(void) {
            if (txhead >= txtail)
                return false;  // Nothing to send!

            ucbctl1 |= UCSWRST;
            ucbctl0 |= UCMST;
            ucbctl1 = UCSSEL_2 | UCTR | UCSWRST;
            ucbctl1 &= ~UCSWRST;
            twi_state = TWI_MTX;
            twi_error = TWI_ERROR_NONE;

            stateie |= UCSTTIE | UCSTPIE | UCALIE | UCNACKIE;
            txrxie |= UCB0TXIE | UCB0RXIE;

            // Check for timeout waiting for client to respond
            uint32_t mstart = millis();
            ucbctl1 |= UCTXSTT;  // Initiate START condition

            while ( (ucbctl1 & UCTXSTT) && (millis() - mstart) < 50 )  // Wait until START+ADDR is finished sending...
                ;
            if ( (millis() - mstart) > 49 ) {  // Address NACK or fault in slave?
                ucbctl1 = UCSSEL_2 | UCSWRST;
                twi_state = TWI_IDLE;
                twi_error = TWI_ERROR_MTX_ADDR_NACK;
                if (is_slave) {
                    ucbctl0 &= ~UCMST;
                    ucbctl1 &= ~UCSWRST;  // Restore slave mode functionality
                }
                return false;
            }

            while (twi_state != TWI_IDLE && twi_error == TWI_ERROR_NONE)
                LPM0;

            while (ucbctl1 & UCTXSTP)  // Wait for STOP condition to complete before resetting bus
                ;

            if (is_slave) {
                ucbctl1 |= UCSWRST;
                ucbctl0 &= ~UCMST;
                ucbctl1 &= ~UCSWRST;
            } else {
                ucbctl1 = UCSSEL_2 | UCSWRST;
                stateie &= ~(UCSTTIE | UCSTPIE | UCALIE | UCNACKIE);
                txrxie &= ~(UCB0TXIE | UCB0RXIE);
            }

            if (twi_error != TWI_ERROR_NONE)
                return false;
            return true;
        };

        NEVER_INLINE
        int requestFrom(int addr, int len) {
            if (len < 1 || (size_t)len > rxbuf_len)
                return 0;  // Nothing to do!

            rxhead = 0;
            rxtail = len;

            ucbctl1 |= UCSWRST;
            i2csa = addr & 0x1FF;
            if (addr & 0x180)
                ucbctl0 |= UCSLA10;
            else
                ucbctl0 &= ~UCSLA10;
            ucbctl0 |= UCMST;
            ucbctl1 = UCSSEL_2 | UCSWRST;
            ucbctl1 &= ~UCSWRST;

            twi_state = TWI_MRX;
            twi_error = TWI_ERROR_NONE;

            stateie |= UCSTTIE | UCSTPIE | UCALIE | UCNACKIE;
            txrxie |= UCB0TXIE | UCB0RXIE;

            // Check for timeout waiting for client to respond
            uint32_t mstart = millis();
            ucbctl1 |= UCTXSTT;  // Initiate START condition

            while ( (ucbctl1 & UCTXSTT) && (millis() - mstart) < 50 )  // Wait until START+ADDR is finished sending...
                ;
            if ( (millis() - mstart) > 49 ) {  // Address NACK or fault in slave?
                ucbctl1 = UCSSEL_2 | UCSWRST;
                twi_state = TWI_IDLE;
                twi_error = TWI_ERROR_MRX_ADDR_NACK;
                if (is_slave) {
                    ucbctl0 &= ~UCMST;
                    ucbctl1 &= ~UCSWRST;  // Restore slave mode functionality
                }
                return 0;
            }

            if (len == 1)
                ucbctl1 |= UCTXSTP;  // STOP needs to be sent right away if it's just 1 byte!

            while (twi_state != TWI_IDLE && twi_error == TWI_ERROR_NONE)
                LPM0;

            while (ucbctl1 & UCTXSTP)  // Wait for STOP condition to complete before resetting bus
                ;

            if (is_slave) {
                ucbctl1 |= UCSWRST;
                ucbctl0 &= ~UCMST;
                ucbctl1 &= ~UCSWRST;
            } else {
                ucbctl1 = UCSSEL_2 | UCSWRST;
                stateie &= ~(UCSTTIE | UCSTPIE | UCALIE | UCNACKIE);
                txrxie &= ~(UCB0TXIE | UCB0RXIE);
            }

            if (twi_error != TWI_ERROR_NONE)
                return 0;
            return rxtail;
        };

        uint8_t requestFrom(uint8_t addr, uint8_t len) { return requestFrom((int) addr, (int) len); };
};


#endif /* WIRE_USCI_H */
