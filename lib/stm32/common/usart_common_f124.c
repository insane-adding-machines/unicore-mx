/** @addtogroup usart_file

@author @htmlonly &copy; @endhtmlonly 2009 Uwe Hermann <uwe@hermann-uwe.de>

This library supports the USART/UART in the STM32F series
of ARM Cortex Microcontrollers by ST Microelectronics.

Devices can have up to 3 USARTs and 2 UARTs.

*/

/*
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
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

/**@{*/

#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/stm32/rcc.h>

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/** @brief USART Return Interrupt Source.

Returns true if the specified interrupt flag (IDLE, RXNE, TC, TXE or OE) was
set and the interrupt was enabled. If the specified flag is not an interrupt
flag, the function returns false.

@todo  These are the most important interrupts likely to be used. Others
relating to LIN break, and error conditions in multibuffer communication, need
to be added for completeness.

@param[in] usart unsigned 32 bit. USART block register address base @ref
usart_reg_base
@param[in] flag Unsigned int32. Status register flag  @ref usart_sr_flags.
@returns boolean: flag and interrupt enable both set.
*/

bool usart_get_interrupt_source(uint32_t usart, uint32_t flag)
{
	uint32_t flag_set = (USART_SR(usart) & flag);
	/* IDLE, RXNE, TC, TXE interrupts */
	if ((flag >= USART_SR_IDLE) && (flag <= USART_SR_TXE)) {
		return ((flag_set & USART_CR1(usart)) != 0);
	/* Overrun error */
	} else if (flag == USART_SR_ORE) {
		return flag_set && (USART_CR3(usart) & USART_CR3_CTSIE);
	}

	return false;
}

/**@}*/
