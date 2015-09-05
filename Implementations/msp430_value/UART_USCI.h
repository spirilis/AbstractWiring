/* MSP430 USCI UART implementation - genericized with templates for all USCI implementations */

#ifndef UART_USCI_H
#define UART_USCI_H

#include <AbstractWiring.h>
#include <UART_USCI_EXTISR.h>
#include <usci_isr.h>


template <
    int usci_a_instance,
    u8_SFR ucactl0,
    u8_SFR ucactl1,
    u8_SFR ucamctl,
    u8_SFR ucaabctl,
    u8_SFR ucabr0,
    u8_SFR ucabr1,
    u8_SFR ucastat,
    u8_SFR ucatxbuf,
    u8_CSFR ucarxbuf,
    u8_SFR ucaie,
    uint8_t ucatxie,
    uint8_t ucarxie,
    size_t tx_buffer_size,
    size_t rx_buffer_size,
    u8_SFR pxsel,
    u8_SFR pxsel2,
    enum PortselMode pxsel_specification,
    uint8_t pxbits >

class UART_USCI : public UART_USCI_EXTISR {
    private:
	volatile char txbuffer[tx_buffer_size], rxbuffer[rx_buffer_size];
        volatile unsigned int tx_head, tx_tail;
        volatile unsigned int rx_head, rx_tail;
        unsigned long _baud;
        volatile SERIAL_BREAK_CALLBACK breakcb;

    public:
        UART_USCI() {
            isr_usci_uart_instance[usci_a_instance] = this;
	};

	NEVER_INLINE
        void begin(unsigned long bitrate) {
            usci_isr_installer();

            _baud = bitrate;
            ucactl1 = UCSWRST;
            ucactl1 |= UCSSEL_2;
            ucactl0 = 0x00;  // Parity = none, 8 bits, 1 stop bit, LSB first.
            ucaabctl = 0x00;

            configClock(bitrate);

            ucactl1 &= ~(UCSWRST);
            set_pxsel(pxsel, pxsel2, pxsel_specification, pxbits);
            ucaie |= ucarxie;

            tx_head = 0;
            tx_tail = 0;
            rx_head = 0;
            rx_tail = 0;
    	};

	NEVER_INLINE
        void end(void) {
            ucaie &= ~(ucatxie | ucarxie);
            set_pxsel(pxsel, pxsel2, PORT_SELECTION_NONE, pxbits);
            ucactl1 |= UCSWRST;

            tx_head = 0;
            tx_tail = 0;
            rx_head = 0;
            rx_tail = 0;
        };

        int available(void) { return (rx_buffer_size + rx_head - rx_tail) % rx_buffer_size; };
        int peek(void) { if (rx_head == rx_tail) return -1; return rxbuffer[rx_tail]; };

	NEVER_INLINE
        int read(void) {
            if (rx_head == rx_tail)
                return -1;

            uint8_t c = rxbuffer[rx_tail];
            rx_tail = (unsigned int)(rx_tail + 1) % rx_buffer_size;
            return c;
        };

        void flush(void) { while (tx_head != tx_tail) ; };

	NEVER_INLINE
        size_t write(uint8_t c) {
            unsigned int i = (tx_head + 1) % tx_buffer_size;

            // If the output buffer is full, we must wait.
            if (i == tx_tail) {
                // What to do next?
                // First: Is the peripheral suspended?
                if (ucactl1 & UCSWRST)
                    return 0;  // Not getting anywhere busy-waiting on a suspended UART!
                // Second, are interrupts disabled?
                if ( !(__get_SR_register() & GIE) )
                    return 0;  // No point in waiting for TX while the USCI IRQ can't fire!
                    // ^ This should catch a common pitfall of Arduino users: Running Serial.print inside an interrupt routine.
                // Else ... just wait.
                while (i == tx_tail)
                    ;
            }

            txbuffer[tx_head] = c;
            tx_head = i;
            ucaie |= ucatxie;
            return 1;
        };

        operator bool() { if (ucactl1 & UCSWRST) return false; return true; };

	NEVER_INLINE
        void isr_send_char(void) {
            if (tx_head == tx_tail) {
                // Buffer empty; disable interrupts
                ucaie &= ~ucatxie;
                return;
            }

            uint8_t c = txbuffer[tx_tail];
            tx_tail = (tx_tail + 1) % tx_buffer_size;
            ucatxbuf = c;
        };

	NEVER_INLINE
        void isr_get_char(void) {
            uint8_t c = ucarxbuf;
            unsigned int i = (rx_head + 1) % rx_buffer_size;

            if (i != rx_tail) {
                rxbuffer[rx_head] = c;
                rx_head = i;
            }   // else ... ignore the char (buffer full)
        };

	NEVER_INLINE
        void configClock(unsigned long bitrate) {
            uint16_t mod;
            uint32_t divider;
            boolean oversampling = false;

            if (F_CPU / _baud >= 48)
                oversampling = true;
            divider = (F_CPU << 4) / _baud;
            if (oversampling) {
                mod = divider & 0xFFF0;               // UCBRFx = INT([(N/16) <96> INT(N/16)] <D7> 16)
                divider >>= 8;
            } else {
                mod = ((divider & 0x0F) + 1) & 0x0E;  // UCBRSx (bit 1-3)
                divider >>= 4;
            }

            uint16_t div16 = (uint16_t) divider;
            ucabr0 = (uint8_t) div16;
            asm volatile("swpb %[d]" : [d] "=r" (div16) : "r" (div16));
            ucabr1 = (uint8_t) div16;
            ucamctl = (uint8_t) (oversampling ? UCOS16 : 0x00) | mod;

        };

        // Optional API is implemented
        boolean hasExtendedAPI(void) { return true; };

	NEVER_INLINE
        void set7Bit(boolean yn) {
            ucactl1 |= UCSWRST;
            if (yn)
                ucactl0 |= UC7BIT;
            else
                ucactl0 &= ~(UC7BIT);
            ucactl1 &= ~(UCSWRST);
        };

	NEVER_INLINE
        void setStopBits(int s) {
            if (s < 1 || s > 2)
                return;
            ucactl1 |= UCSWRST;
            if (s == 2)
                ucactl0 |= UCSPB;
            else
                ucactl0 &= ~(UCSPB);
            ucactl1 &= ~(UCSWRST);
        };

	NEVER_INLINE
        void setParity(enum SerialParity setting) {
            ucactl1 |= UCSWRST;
            switch (setting) {
                case PARITY_NONE:
                    ucactl0 &= ~(UCPEN);
                    break;
                case PARITY_ODD:
                    ucactl0 |= UCPEN;
                    ucactl0 &= ~(UCPAR);
                    break;
                case PARITY_EVEN:
                    ucactl0 |= UCPEN;
                    ucactl0 |= UCPAR;
            }
            ucactl1 &= ~(UCSWRST);
        };

	NEVER_INLINE
        void sendBreak(void) {
            if (ucactl1 & UCSWRST)
                return;

            uint8_t txie_save = ucaie & ucatxie;
            ucaie &= ~ucatxie;  // Avoid running the TX interrupt handler so we can fit in our BREAK

            while (ucastat & UCBUSY)
                ;  // Busy-wait until the last-transmitted character has finished sending

            ucactl1 |= UCTXBRK;
            ucatxbuf = 0x00;  // send it!  UCTXBRK is automatically cleared per TI document SLAU144
            ucaie |= txie_save;
        };

	NEVER_INLINE
        void attachBreakInterrupt(SERIAL_BREAK_CALLBACK callback) {
            if (callback == NULL)
                return;

            ucactl1 |= UCSWRST;
            ucactl1 |= UCBRKIE;
            breakcb = callback;
            ucactl1 &= ~(UCSWRST);
        };

	NEVER_INLINE
        void detachBreakInterrupt(void) {
            ucactl1 |= UCSWRST;
            breakcb = NULL;
            ucactl1 &= ~(UCBRKIE);
            ucactl1 &= ~(UCSWRST);
        };
};


#endif /* UART_USCI_H */
