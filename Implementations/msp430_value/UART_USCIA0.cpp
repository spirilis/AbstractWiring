/* USCI_A0 UART implementation for MSP430 Value Line */

#include <UART_USCIA0.h>
#include <usci_isr.h>

UART_USCIA0::UART_USCIA0(void)
{
}

void UART_USCIA0::begin(unsigned long bitrate)
{
    usci_isr_installer();
    isr_uscia0_uart_instance = this;

    _baud = bitrate;
    UCA0CTL1 = UCSWRST;
    UCA0CTL1 |= UCSSEL_2;
    UCA0CTL0 = 0x00;  // Parity = none, 8 bits, 1 stop bit, LSB first.
    UCA0ABCTL = 0x00;

    configClock(bitrate);

    UCA0CTL1 &= ~UCSWRST;
    P1SEL |= BIT1 | BIT2;
    P1SEL2 |= BIT1 | BIT2;
    IE2 |= UCA0RXIE;

    buffer.tx_head = 0;
    buffer.tx_tail = 0;
    buffer.rx_head = 0;
    buffer.rx_tail = 0;
}

void UART_USCIA0::configClock(unsigned long bitrate)
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

    UCA0BR0 = (uint8_t) div16;
    asm volatile("swpb %[d]" : [d] "=r" (div16) : "r" (div16));
    UCA0BR1 = (uint8_t) div16;
    UCA0MCTL = (uint8_t) (oversampling ? UCOS16 : 0x00) | mod;
}

void UART_USCIA0::end(void)
{
    IE2 &= ~(UCA0TXIE | UCA0RXIE);
    P1SEL &= ~(BIT1 | BIT2);
    P1SEL2 &= ~(BIT1 | BIT2);
    UCA0CTL1 |= UCSWRST;

    buffer.tx_head = 0;
    buffer.tx_tail = 0;
    buffer.rx_head = 0;
    buffer.rx_tail = 0;
}

void UART_USCIA0::isr_send_char(void)
{
    if (buffer.tx_head == buffer.tx_tail) {
        // Buffer empty; disable interrupts
        IE2 &= ~UCA0TXIE;
        P1OUT &= ~BIT0;
        return;
    }

    P1OUT |= BIT0;
    uint8_t c = buffer.txbuffer[buffer.tx_tail];
    buffer.tx_tail = (buffer.tx_tail + 1) % TX_BUFFER_SIZE;
    UCA0TXBUF = c;
}

void UART_USCIA0::isr_get_char(void)
{
    uint8_t c = UCA0RXBUF;
    unsigned int i = (buffer.rx_head + 1) % RX_BUFFER_SIZE;

    if (i != buffer.rx_tail) {
        buffer.rxbuffer[buffer.rx_head] = c;
        buffer.rx_head = i;
    }   // else ... ignore the char (buffer full)
}

int UART_USCIA0::available(void)
{
    return (RX_BUFFER_SIZE + buffer.rx_head - buffer.rx_tail) % RX_BUFFER_SIZE;
}

int UART_USCIA0::peek(void)
{
    if (buffer.rx_head == buffer.rx_tail)
        return -1;
    return buffer.rxbuffer[buffer.rx_tail];
}

int UART_USCIA0::read(void)
{
    if (buffer.rx_head == buffer.rx_tail)
        return -1;

    uint8_t c = buffer.rxbuffer[buffer.rx_tail];
    buffer.rx_tail = (unsigned int)(buffer.rx_tail + 1) % RX_BUFFER_SIZE;
    return c;
}

void UART_USCIA0::flush(void)
{
    while (buffer.tx_head != buffer.tx_tail)
        ;
}

size_t UART_USCIA0::write(uint8_t c)
{
    unsigned int i = (buffer.tx_head + 1) % TX_BUFFER_SIZE;

    // If the output buffer is full, we must wait.
    if (i == buffer.tx_tail) {
        // What to do next?
        // First: Is the peripheral suspended?
        if (UCA0CTL1 & UCSWRST)
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
    IE2 |= UCA0TXIE;
    return 1;
}

UART_USCIA0::operator bool()
{
    if (UCA0CTL1 & UCSWRST)
        return false;
    return true;
}

void UART_USCIA0::set7Bit(boolean yn)
{
    UCA0CTL1 |= UCSWRST;
    if (yn)
        UCA0CTL0 |= UC7BIT;
    else
        UCA0CTL0 &= ~UC7BIT;
    UCA0CTL1 &= ~UCSWRST;
}

void UART_USCIA0::setStopBits(int s)
{
    if (s < 1 || s > 2)
        return;
    UCA0CTL1 |= UCSWRST;
    if (s == 2)
        UCA0CTL0 |= UCSPB;
    else
        UCA0CTL0 &= ~UCSPB;
    UCA0CTL1 &= ~UCSWRST;
}

void UART_USCIA0::setParity(enum SerialParity setting)
{
    UCA0CTL1 |= UCSWRST;
    switch (setting) {
        case PARITY_NONE:
            UCA0CTL0 &= ~UCPEN;
            break;
        case PARITY_ODD:
            UCA0CTL0 |= UCPEN;
            UCA0CTL0 &= ~UCPAR;
            break;
        case PARITY_EVEN:
            UCA0CTL0 |= UCPEN;
            UCA0CTL0 |= UCPAR;
    }
    UCA0CTL1 &= ~UCSWRST;
}

void UART_USCIA0::sendBreak(void)
{
    if (UCA0CTL1 & UCSWRST)
        return;

    uint8_t txie_save = IE2 & UCA0TXIE;
    IE2 &= ~UCA0TXIE;  // Avoid running the TX interrupt handler so we can fit in our BREAK

    while (UCA0STAT & UCBUSY)
        ;  // Busy-wait until the last-transmitted character has finished sending

    UCA0CTL1 |= UCTXBRK;
    UCA0TXBUF = 0x00;  // send it!  UCTXBRK is automatically cleared per TI document SLAU144
    IE2 |= txie_save;
}

void UART_USCIA0::attachBreakInterrupt(SERIAL_BREAK_CALLBACK callback)
{
    if (callback == NULL)
        return;

    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCBRKIE;
    breakcb = callback;
    UCA0CTL1 &= ~UCSWRST;
}

void UART_USCIA0::detachBreakInterrupt(void)
{
    UCA0CTL1 |= UCSWRST;
    breakcb = NULL;
    UCA0CTL1 &= ~UCBRKIE;
    UCA0CTL1 &= ~UCSWRST;
}
