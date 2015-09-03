/* MSP430 SPI Master implementation */

#ifndef SPI_USCI_H
#define SPI_USCI_H


#include <AbstractWiring.h>
#include <SPI.h>

static const uint8_t _usci_spi_mode_bits[] = { UCCKPH, 0, (UCCKPL | UCCKPH), UCCKPL };

template <
    u8_SFR ucxctl0,
    u8_SFR ucxctl1,
    u8_SFR ucxbr0,
    u8_SFR ucxbr1,
    u8_SFR ucxstat,
    u8_SFR ucxtxbuf,
    u8_CSFR ucxrxbuf,
    u8_SFR sclk_pxdir,  // The pxdir, pxout, pxin's are all needed for 9-bit mode
    u8_SFR sclk_pxout,
    u8_SFR sclk_pxsel,
    u8_SFR sclk_pxsel2,
    const uint8_t sclk_pxbits,
    enum PortselMode sclk_pxsel_specification,
    u8_SFR mosi_pxdir,
    u8_SFR mosi_pxout,
    u8_SFR mosi_pxsel,
    u8_SFR mosi_pxsel2,
    const uint8_t mosi_pxbits,
    enum PortselMode mosi_pxsel_specification,
    u8_SFR miso_pxdir,
    u8_CSFR miso_pxin,
    u8_SFR miso_pxsel,
    u8_SFR miso_pxsel2,
    const uint8_t miso_pxbits,
    enum PortselMode miso_pxsel_specification >

class SPI_USCI : public SPIClass {
    private:
        SPISettings _settings, _settings_old;
        int _mask_irq;
        boolean _transaction_semaphore;

        uint8_t _derive_ctl0_bits(SPISettings & s) {
            uint8_t mode = _usci_spi_mode_bits[s._datamode];
            uint8_t msb = (s._bitorder == MSBFIRST ? UCMSB : 0);
            return UCMST | msb | mode;
        }

#ifdef SPI_ENABLE_EXTENDED_API
        uint16_t _manual_9th_bit(uint16_t inw) {
            uint16_t retw = 0;

            if (ucxctl0 & UCCKPL)
                sclk_pxout |= sclk_pxbits;
            else
                sclk_pxout &= ~sclk_pxbits;
            sclk_pxdir |= sclk_pxbits;
            set_pxsel(sclk_pxsel, sclk_pxsel2, PORT_SELECTION_NONE, sclk_pxbits);

            if (ucxctl0 & UCCKPH) {
                if (inw & 0x100)
                    mosi_pxout |= mosi_pxbits;
                else
                    mosi_pxout &= ~mosi_pxbits;
                mosi_pxdir |= mosi_pxbits;
                miso_pxdir &= ~miso_pxbits;
                set_pxsel(mosi_pxsel, mosi_pxsel2, PORT_SELECTION_NONE, mosi_pxbits);
                set_pxsel(miso_pxsel, miso_pxsel2, PORT_SELECTION_NONE, miso_pxbits);

                sclk_pxout ^= sclk_pxbits;
                if (miso_pxin & miso_pxbits)
                    retw = 0x100;
                set_pxsel(sclk_pxsel, sclk_pxsel2, sclk_pxsel_specification, sclk_pxbits);
                // ^ I found by experimentation with the logic analyzer that the "best" way to
                // release SCLK to its resting state is just to hand control back to USCI ... otherwise
                // a 2nd "phantom SCLK pulse" results between the XOR and the pxsel setting.
            } else {
                sclk_pxout ^= sclk_pxbits;
                if (inw & 0x100)
                    mosi_pxout |= mosi_pxbits;
                else
                    mosi_pxout &= ~mosi_pxbits;
                mosi_pxdir |= mosi_pxbits;
                miso_pxdir &= ~miso_pxbits;
                set_pxsel(mosi_pxsel, mosi_pxsel2, PORT_SELECTION_NONE, mosi_pxbits);
                set_pxsel(miso_pxsel, miso_pxsel2, PORT_SELECTION_NONE, miso_pxbits);

                set_pxsel(sclk_pxsel, sclk_pxsel2, sclk_pxsel_specification, sclk_pxbits);
                if (miso_pxin & miso_pxbits)
                    retw = 0x100;
            }
            set_pxsel(mosi_pxsel, mosi_pxsel2, mosi_pxsel_specification, mosi_pxbits);
            set_pxsel(miso_pxsel, miso_pxsel2, miso_pxsel_specification, miso_pxbits);
            return retw;
        };
#endif /* SPI_ENABLE_EXTENDED_API */

    public:
        void begin(void) {
            ucxctl1 |= UCSWRST;
            ucxctl0 = _derive_ctl0_bits(_settings);
            configClock(_settings._clock);

            set_pxsel(sclk_pxsel, sclk_pxsel2, sclk_pxsel_specification, sclk_pxbits);
            set_pxsel(mosi_pxsel, mosi_pxsel2, mosi_pxsel_specification, mosi_pxbits);
            set_pxsel(miso_pxsel, miso_pxsel2, miso_pxsel_specification, miso_pxbits);

            ucxctl1 = UCSSEL_2;
            _transaction_semaphore = false;
            _mask_irq = 0;
        };

        void end(void) {
            ucxctl1 |= UCSWRST;
            ucxctl0 &= ~UCSYNC;
            set_pxsel(sclk_pxsel, sclk_pxsel2, PORT_SELECTION_NONE, sclk_pxbits);
            set_pxsel(mosi_pxsel, mosi_pxsel2, PORT_SELECTION_NONE, mosi_pxbits);
            set_pxsel(miso_pxsel, miso_pxsel2, PORT_SELECTION_NONE, miso_pxbits);
        };

        uint8_t transfer(uint8_t inb) {
            while (ucxstat & UCBUSY)
                ;
            ucxtxbuf = inb;
            while (ucxstat & UCBUSY)
                ;
            return ucxrxbuf;
        };

        void setBitOrder(unsigned int msblsb) {
            _settings._bitorder = msblsb;

            uint8_t was_ucrst = ucxctl1 & UCSWRST;
            ucxctl1 |= UCSWRST;

            ucxctl0 = _derive_ctl0_bits(_settings);

            ucxctl1 = (ucxctl1 & ~UCSWRST) | was_ucrst;
        };

        void setDataMode(unsigned int mode) {
            _settings._datamode = mode;

            uint8_t was_ucrst = ucxctl1 & UCSWRST;
            ucxctl1 |= UCSWRST;

            ucxctl0 = _derive_ctl0_bits(_settings);

            ucxctl1 = (ucxctl1 & ~UCSWRST) | was_ucrst;
        };

        void setClockDivider(int clockDiv) {
            _settings._clock = 16000000UL / clockDiv;  // Clock div is referenced from Arduino, dividing 16MHz
            configClock(_settings._clock);
        };

        void configClock(unsigned long bitrate) {
            uint32_t smclk = F_CPU;
            uint16_t clkdiv = smclk / bitrate;

            if (smclk % bitrate != 0) {
                /* Partial division will leave us with a divider that produces too fast a clock
                 *
                 * Also this condition will always match if smclk < spi_master_clk,
                 * ensuring clockdiv of /1 will always end up the case there.
                 */
                clkdiv++;
            }

            uint8_t was_ucrst = ucxctl1 & UCSWRST;
            ucxctl1 |= UCSWRST;

            // Set clock divider as UCxBR0 (LSB) and UCxBR1 (MSB)
            ucxbr0 = (uint8_t)clkdiv;
            asm volatile ("swpb %[c]" : [c] "=r" (clkdiv) : "r" (clkdiv));
            ucxbr1 = (uint8_t)clkdiv;

            ucxctl1 = (ucxctl1 & ~UCSWRST) | was_ucrst;
        };

        boolean beginTransaction(SPISettings settings) {
            // For atomicity in semaphore check
            __bic_SR_register(GIE);
            if (_transaction_semaphore) {
                __bis_SR_register(GIE);
                return false;
            }
            _transaction_semaphore = true;
            if (_mask_irq && _mask_irq != 255)
                maskInterrupt(_mask_irq);
            // Re-enable GIE but only if _mask_irq != 255
            if (_mask_irq != 255)
                __bis_SR_register(GIE);

            _settings_old.copy(_settings);
            _settings.copy(settings);

            ucxctl1 |= UCSWRST;
            ucxctl0 = _derive_ctl0_bits(_settings);
            configClock(_settings._clock);
            ucxctl1 &= ~UCSWRST;

            return true;
        };

        void endTransaction(void) {
            __bic_SR_register(GIE);
            _transaction_semaphore = false;
            if (_mask_irq && _mask_irq != 255)
                unmaskInterrupt(_mask_irq);

            // Restore SPI settings under GIE=0 protection in case an IRQ fires which uses SPI and expects
            // the default SPISettings parameters to be active.
            _settings.copy(_settings_old);
            ucxctl1 |= UCSWRST;
            ucxctl0 = _derive_ctl0_bits(_settings);
            configClock(_settings._clock);
            ucxctl1 &= ~UCSWRST;

            __bis_SR_register(GIE);
        };

        void usingInterrupt(int pin) {
            if (pin == 255) {
                _mask_irq = 255;
                return;
            }
            if (pin < 1 || pin > 16)
                return;
            _mask_irq = pin;
        };

#ifdef SPI_ENABLE_EXTENDED_API
        boolean hasExtendedAPI(void) { return true; };

        uint16_t transfer16(uint16_t inw) {
            uint16_t retw;

            while (ucxstat & UCBUSY)
                ;
            if (_settings._bitorder == MSBFIRST) {
                // Send MSB
                asm volatile ("swpb %[c]" : [c] "=r" (inw) : "r" (inw));  // inw now flip-flopped
                ucxtxbuf = (uint8_t)inw;
                while (ucxstat & UCBUSY)
                    ;
                // Retrieve MSB
                retw = ucxrxbuf;
                asm volatile ("swpb %[c]" : [c] "=r" (retw) : "r" (retw)); // retw now correct

                // Send LSB
                asm volatile ("swpb %[c]" : [c] "=r" (inw) : "r" (inw));  // inw now correct
                ucxtxbuf = (uint8_t)inw;
                while (ucxstat & UCBUSY)
                    ;
                // Retrieve LSB
                retw |= ucxrxbuf;  // retw is in correct order for return
            } else {
                // Send LSB
                ucxtxbuf = (uint8_t)inw;
                while (ucxstat & UCBUSY)
                    ;
                // Retrieve LSB
                retw = ucxrxbuf;

                // Send MSB
                asm volatile ("swpb %[c]" : [c] "=r" (inw) : "r" (inw));  // inw now flip-flopped
                ucxtxbuf = (uint8_t)inw;
                while (ucxstat & UCBUSY)
                    ;
                // Retrieve MSB
                asm volatile ("swpb %[c]" : [c] "=r" (retw) : "r" (retw)); // retw now flip-flopped
                retw |= ucxrxbuf;
                // Flip retw back to correct order
                asm volatile ("swpb %[c]" : [c] "=r" (retw) : "r" (retw)); // retw now correct for return
            }
            return retw;
        };

        uint16_t transfer9(uint16_t inw) {
            uint16_t retw;
            while (ucxstat & UCBUSY)
                ;

            if (_settings._bitorder == MSBFIRST) {
                // First bit is the manually-sent 9th bit
                retw = _manual_9th_bit(inw);
                retw |= transfer((uint8_t)inw);
            } else {
                retw = transfer((uint8_t)inw);
                retw |= _manual_9th_bit(inw);
            }
            return retw;
        };
#endif /* SPI_ENABLE_EXTENDED_API */
};


#endif /* SPI_USCI_H */
