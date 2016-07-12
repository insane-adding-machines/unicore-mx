/** @defgroup pwr_file PWR
 *
 * @ingroup STM32F4xx
 *
 * @brief <b>unicore-mx STM32F4xx Power Control</b>
 *
 * @version 1.0.0
 *
 * @author @htmlonly &copy; @endhtmlonly 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * @date 4 March 2013
 *
 * This library supports the power control system for the
 * STM32F4 series of ARM Cortex Microcontrollers by ST Microelectronics.
 *
 * LGPL License Terms @ref lgpl_license
 */
/*
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
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

#include <unicore-mx/stm32/pwr.h>

void pwr_set_vos_scale(enum pwr_vos_scale scale)
{
	if (scale == PWR_SCALE1) {
		PWR_CR |= PWR_CR_VOS | PWR_CR_ODRIVE;
	} else if (scale == PWR_SCALE2) {
		PWR_CR &= ~PWR_CR_VOS;
	}
}
