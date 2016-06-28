/*
 * Copyright (C) 2015 Daniele Lacamera <root at danielinux dot net>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LPC17XX_UART_H
#define LPC17XX_UART_H

#include <unicore-mx/cm3/common.h>

/* --- Universal Asynchronous Receiver Transmitter (UART) */
#define UART_RBR(x)             MMIO32((x) + 0x0000)
#define UART_THR(x)             MMIO32((x) + 0x0000)
#define UART_DLL(x)             MMIO32((x) + 0x0000)
#define UART_DLM(x)             MMIO32((x) + 0x0004)
#define UART_IER(x)             MMIO32((x) + 0x0004)
#define UART_IIR(x)             MMIO32((x) + 0x0008)
#define UART_FCR(x)             MMIO32((x) + 0x0008)
#define UART_LCR(x)             MMIO32((x) + 0x000C)
#define UART_LSR(x)             MMIO32((x) + 0x0014)
#define UART_SCR(x)             MMIO32((x) + 0x001C)
#define UART_ACR(x)             MMIO32((x) + 0x0020)
#define UART_ICR(x)             MMIO32((x) + 0x0024)
#define UART_FDR(x)             MMIO32((x) + 0x0028)
#define UART_TER(x)             MMIO32((x) + 0x0030)

/* UART Interrupt Enable Register (UART_IER) */
/* Bits [31:10] - Reserved */
#define UART_ABT_IE             (0x01 << 9)
#define UART_ABE_IE             (0x01 << 8)
/* Bits 7:3 reserved */
#define UART_LSR_IE             (0x01 << 2)
#define UART_THR_IE             (0x01 << 1)
#define UART_RBR_IE             (0x01 << 0)

/* UART Interrupt Identification Register (UART_IIR) */
/* Bits [31:10] - Reserved */
#define UART_ABT_II             (0x01 << 9)
#define UART_ABE_II             (0x01 << 8)
/* Bits 5:4 reserved */
#define UART_IIR_MASK             (0x07 << 1)
/* Interrupt type: */
    #define UART_IIR_RLS          (0x03 << 1)
    #define UART_IIR_RDA          (0x02 << 1)
    #define UART_IIR_CTI          (0x06 << 1)
    #define UART_IIR_THE          (0x01 << 1)
#define UART_IEN_II             (0x01 << 0)


/* compatibility API for interrupt source detection */
#define USART_SR_TXE    UART_IIR_THE
#define USART_SR_RXNE   UART_IIR_RDA

/* UART Line Status Register (UART_LSR) */
/* Bits [31:8] - Reserved */
#define UART_RXFE_LS            (0x01 << 7)
#define UART_TE_LS              (0x01 << 6)
#define UART_TH_LS              (0x01 << 5)
#define UART_BI_LS              (0x01 << 4)
#define UART_FE_LS              (0x01 << 3)
#define UART_PE_LS              (0x01 << 2)
#define UART_OE_LS              (0x01 << 1)
#define UART_RDR_LS             (0x01 << 0)



/* TODO: Other registries, according to datasheet */

enum usart_stopbits {
    USART_STOPBITS_1,
    USART_STOPBITS_1_5,
    USART_STOPBITS_2,
};

enum usart_parity {
    USART_PARITY_NONE,
    USART_PARITY_ODD,
    USART_PARITY_EVEN,
};

enum usart_mode {
    USART_MODE_DISABLED,
    USART_MODE_RX,
    USART_MODE_TX,
    USART_MODE_TX_RX,
};

enum usart_flowcontrol {
    USART_FLOWCONTROL_NONE,
    USART_FLOWCONTROL_RTS_CTS,
};

void usart_send(uint32_t usart, uint16_t data);
uint16_t usart_recv(uint32_t usart);
void usart_wait_send_ready(uint32_t usart);
void usart_wait_recv_ready(uint32_t usart);
bool usart_is_send_ready(uint32_t usart);
bool usart_is_recv_ready(uint32_t usart);
void usart_send_blocking(uint32_t usart, uint16_t data);
uint16_t usart_recv_blocking(uint32_t usart);
void usart_enable_rx_interrupt(uint32_t usart);
void usart_disable_rx_interrupt(uint32_t usart);
void usart_clear_rx_interrupt(uint32_t usart);
void usart_enable_tx_interrupt(uint32_t usart);
void usart_disable_tx_interrupt(uint32_t usart);
void usart_clear_tx_interrupt(uint32_t usart);
bool usart_get_interrupt_source(uint32_t usart, uint32_t flag);


#endif

