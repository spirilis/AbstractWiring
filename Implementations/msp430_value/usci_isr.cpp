/* USCI ISRs for MSP430G2xxx devices */

#include <AbstractWiring.h>
#include <UART_USCI.h>


UART_USCI *isr_uscia0_uart_instance = NULL;

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
            isr_uscia0_uart_instance->isr_send_char();
        }
    }

    if (IFG2 & UCB0TXIFG) {
        if ( (UCB0CTL0 & UCMODE_3) == UCMODE_3 ) {
            // I2C
            // TODO
        } else {
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
            isr_uscia0_uart_instance->isr_get_char();
        }
    }

    if (still_asleep != _sys_asleep)
        __bic_SR_register_on_exit(LPM4_bits);
}


}; /* extern "C" */
