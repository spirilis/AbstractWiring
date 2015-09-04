/* USCI ISRs for MSP430G2xxx devices */

#include <AbstractWiring.h>
#include <UART_USCI.h>


UART_USCI_EXTISR *isr_usci_uart_instance[1] = { NULL };
TwoWire_USCI_EXTISR *isr_usci_twowire_instance[1] = { NULL };

extern "C" {

void usci_isr_installer(void) { ; }

__attribute__((interrupt(USCIAB0TX_VECTOR)))
void USCIAB0_TX(void)
{
    boolean still_asleep = _sys_asleep;

    if (IFG2 & UCA0TXIFG) {
        if (UCA0CTL0 & UCSYNC) {
            // SPI Slave Mode
            // TODO
        } else {
            // UART
            isr_usci_uart_instance[0]->isr_send_char();
        }
    }

    if ( (UCB0CTL0 & UCMODE) == UCMODE_3 ) {
        if (IFG2 & (UCB0TXIFG | UCB0RXIFG)) {
            // I2C
            isr_usci_twowire_instance[0]->isr_handle_txrx();
        }
    } else {
        if (IFG2 & UCB0TXIFG) {
            // SPI Slave
            // TODO
        }
    }

    if (still_asleep != _sys_asleep)
        __bic_SR_register_on_exit(LPM4_bits);
}

__attribute__((interrupt(USCIAB0RX_VECTOR)))
void USCIAB0_RX(void)
{
    boolean still_asleep = _sys_asleep;

    if (IFG2 & UCA0RXIFG) {
        if (UCA0CTL0 & UCSYNC) {
            // SPI Slave Mode
            // TODO
        } else {
            // UART
            isr_usci_uart_instance[0]->isr_get_char();
        }
    }

    if ( (UCB0CTL0 & UCMODE) == UCMODE_3 ) {
        if (UCB0STAT & (UCNACKIFG | UCSTPIFG | UCSTTIFG | UCALIFG)) {
            // I2C
            isr_usci_twowire_instance[0]->isr_handle_control();
        }
    } else {
        if (IFG2 & UCB0RXIFG) {
            // SPI Slave Mode
            // TODO
        }
    }

    if (still_asleep != _sys_asleep)
        __bic_SR_register_on_exit(LPM4_bits);
}


}; /* extern "C" */
