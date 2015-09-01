/* Dummy function to pull in usci_isr.cpp contents */

#ifndef USCI_ISR_H
#define USCI_ISR_H

#include <UART_USCI_EXTISR.h>

#ifdef __cplusplus
extern "C" {
#endif

void usci_isr_installer();
extern UART_USCI_EXTISR *isr_uscia0_uart_instance;

#ifdef __cplusplus
};  /* extern "C" */
#endif

#endif /* USCI_ISR_H */
