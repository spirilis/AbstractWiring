/* USCI_A UART implementation for MSP430 */

#include <UART_USCI.h>
#include <usci_isr.h>

UART_USCI::UART_USCI(void)
{
    buffer.txbuffer = txbuffer;
    buffer.rxbuffer = rxbuffer;
}

void UART_USCI::begin(unsigned long bitrate)
{
    usci_isr_installer();
    isr_uscia0_uart_instance = this;

    _baud = bitrate;
    ucactl1 = UCSWRST;
    ucactl1 |= UCSSEL_2;
    ucactl0 = 0x00;  // Parity = none, 8 bits, 1 stop bit, LSB first.
    ucaabctl = 0x00;

    configClock(bitrate);

    ucactl1 &= ~UCSWRST;
    switch (pxsel_specification) {
        case PORT_SELECTION_NONE:
            pxsel &= ~pxbits;
            if (pxsel2 != NULL)
                pxsel2 &= ~pxbits;
            break;
        case PORT_SELECTION_0:
            pxsel |= pxbits;
            if (pxsel2 != NULL)
                pxsel2 &= ~pxbits;
            break;
        case PORT_SELECTION_1:
            pxsel &= ~pxbits;
            if (pxsel2 != NULL)
                pxsel2 |= pxbits;
            break;
        case PORT_SELECTION_0_AND_1:
            pxsel |= pxbits;
            if (pxsel2 != NULL)
                pxsel2 |= pxbits;
    }
    ucaie |= ucarxie;

    buffer.tx_head = 0;
    buffer.tx_tail = 0;
    buffer.rx_head = 0;
    buffer.rx_tail = 0;
}

void UART_USCI::configClock(unsigned long bitrate)
{
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
}

void UART_USCI::end(void)
{
    ucaie &= ~(ucatxie | ucarxie);
    pxsel &= ~(pxbits);
    pxsel2 &= ~(pxbits);
    ucactl1 |= UCSWRST;

    buffer.tx_head = 0;
    buffer.tx_tail = 0;
    buffer.rx_head = 0;
    buffer.rx_tail = 0;
}

void UART_USCI::isr_send_char(void)
{
    if (buffer.tx_head == buffer.tx_tail) {
        // Buffer empty; disable interrupts
        ucaie &= ~ucatxie;
        P1OUT &= ~BIT0;
        return;
    }

    P1OUT |= BIT0;
    uint8_t c = buffer.txbuffer[buffer.tx_tail];
    buffer.tx_tail = (buffer.tx_tail + 1) % tx_buffer_size;
    ucatxbuf = c;
}

void UART_USCI::isr_get_char(void)
{
    uint8_t c = ucarxbuf;
    unsigned int i = (buffer.rx_head + 1) % rx_buffer_size;

    if (i != buffer.rx_tail) {
        buffer.rxbuffer[buffer.rx_head] = c;
        buffer.rx_head = i;
    }   // else ... ignore the char (buffer full)
}

int UART_USCI::available(void)
{
    return (rx_buffer_size + buffer.rx_head - buffer.rx_tail) % rx_buffer_size;
}

int UART_USCI::peek(void)
{
    if (buffer.rx_head == buffer.rx_tail)
        return -1;
    return buffer.rxbuffer[buffer.rx_tail];
}

int UART_USCI::read(void)
{
    if (buffer.rx_head == buffer.rx_tail)
        return -1;

    uint8_t c = buffer.rxbuffer[buffer.rx_tail];
    buffer.rx_tail = (unsigned int)(buffer.rx_tail + 1) % rx_buffer_size;
    return c;
}

void UART_USCI::flush(void)
{
    while (buffer.tx_head != buffer.tx_tail)
        ;
}

size_t UART_USCI::write(uint8_t c)
{
    unsigned int i = (buffer.tx_head + 1) % tx_buffer_size;

    // If the output buffer is full, we must wait.
    if (i == buffer.tx_tail) {
        // What to do next?
        // First: Is the peripheral suspended?
        if (ucactl1 & UCSWRST)
            return 0;  // Not getting anywhere busy-waiting on a suspended UART!
        // Second, are interrupts disabled?
        if ( !(__get_SR_register() & GIE) )
            return 0;  // No point in waiting for TX while the USCI IRQ can't fire!
            // ^ This should catch a common pitfall of Arduino users: Running Serial.print inside an interrupt routine.
        // Else ... just wait.
        while (i == buffer.tx_tail)
            ;
    }

    buffer.txbuffer[buffer.tx_head] = c;
    buffer.tx_head = i;
    ucaie |= ucatxie;
    return 1;
}

UART_USCI::operator bool()
{
    if (ucactl1 & UCSWRST)
        return false;
    return true;
}

void UART_USCI::set7Bit(boolean yn)
{
    ucactl1 |= UCSWRST;
    if (yn)
        ucactl0 |= UC7BIT;
    else
        ucactl0 &= ~UC7BIT;
    ucactl1 &= ~UCSWRST;
}

void UART_USCI::setStopBits(int s)
{
    if (s < 1 || s > 2)
        return;
    ucactl1 |= UCSWRST;
    if (s == 2)
        ucactl0 |= UCSPB;
    else
        ucactl0 &= ~UCSPB;
    ucactl1 &= ~UCSWRST;
}

void UART_USCI::setParity(enum SerialParity setting)
{
    ucactl1 |= UCSWRST;
    switch (setting) {
        case PARITY_NONE:
            ucactl0 &= ~UCPEN;
            break;
        case PARITY_ODD:
            ucactl0 |= UCPEN;
            ucactl0 &= ~UCPAR;
            break;
        case PARITY_EVEN:
            ucactl0 |= UCPEN;
            ucactl0 |= UCPAR;
    }
    ucactl1 &= ~UCSWRST;
}

void UART_USCI::sendBreak(void)
{
    if (ucactl1 & UCSWRST)
        return;

    uint8_t txie_save = ucaie & ucatxie;
    ucaie &= ~ucatxie;  // Avoid running the TX interrupt handler so we can fit in our BREAK

    while (ucastat & UCBUSY)
        ;  // Busy-wait until the last-transmitted character has finished sending

    ucactl1 |= UCTXBRK;
    ucatxbuf = 0x00;  // send it!  UCTXBRK is automatically cleared per TI document SLAU144
    ucaie |= txie_save;
}

void UART_USCI::attachBreakInterrupt(SERIAL_BREAK_CALLBACK callback)
{
    if (callback == NULL)
        return;

    ucactl1 |= UCSWRST;
    ucactl1 |= UCBRKIE;
    breakcb = callback;
    ucactl1 &= ~UCSWRST;
}

void UART_USCI::detachBreakInterrupt(void)
{
    ucactl1 |= UCSWRST;
    breakcb = NULL;
    ucactl1 &= ~UCBRKIE;
    ucactl1 &= ~UCSWRST;
}
