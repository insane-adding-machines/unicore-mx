/*
 * Copyright (C) 2015 Daniele Lacamera <root at danielinux.net>
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

#include <unicore-mx/lpc17xx/uart.h>

void usart_send(uint32_t usart, uint16_t data)
{
	UART_THR(usart) = data;
}

uint16_t usart_recv(uint32_t usart)
{
	return UART_RBR(usart) & 0xff;
}

void usart_wait_send_ready(uint32_t usart)
{
	while ((UART_LSR(usart) & UART_TH_LS) == 0);
}

void usart_wait_recv_ready(uint32_t usart)
{
	while ((UART_LSR(usart) & UART_RDR_LS) != 0);
}

void usart_send_blocking(uint32_t usart, uint16_t data)
{
	usart_wait_send_ready(usart);
	usart_send(usart, data);
}

bool usart_is_recv_ready(uint32_t usart)
{
	return ((UART_LSR(usart) & UART_RDR_LS) == 0);
}

bool usart_is_send_ready(uint32_t usart)
{
	return ((UART_LSR(usart) & UART_TH_LS) != 0);
}

uint16_t usart_recv_blocking(uint32_t usart)
{
	usart_wait_recv_ready(usart);
	return usart_recv(usart);
}

void usart_enable_rx_interrupt(uint32_t usart)
{
	UART_IER(usart) |= UART_RBR_IE;
}

void usart_enable_tx_interrupt(uint32_t usart)
{
	UART_IER(usart) |= UART_THR_IE;
}

void usart_disable_rx_interrupt(uint32_t usart)
{
	UART_IER(usart) &= (~UART_RBR_IE);
}

void usart_disable_tx_interrupt(uint32_t usart)
{
	UART_IER(usart) &= (~UART_THR_IE);
}

void usart_clear_rx_interrupt(uint32_t usart)
{
    /* Interrupt is automatically cleared by reading the IIR */
}

void usart_clear_tx_interrupt(uint32_t usart)
{
    /* Interrupt is automatically cleared by reading the IIR */
}

bool usart_get_interrupt_source(uint32_t usart, uint32_t flag)
{
    return ((UART_IIR(usart) & flag) != 0);
}
