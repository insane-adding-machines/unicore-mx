/** @defgroup pwr_defines PWR Defines
 *
 * @brief <b>Defined Constants and Types for the STM32F0xx PWR Control</b>
 *
 * @ingroup STM32F0xx_defines
 *
 * @version 1.0.0
 *
 * @date 5 December 2012
 *
 * LGPL License Terms @ref lgpl_license
 */

/*
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

#ifndef UNICOREMX_PWR_H
#define UNICOREMX_PWR_H

#include <unicore-mx/stm32/common/pwr_common_all.h>

/*****************************************************************************/
/* Module definitions                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Register definitions                                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Register values                                                           */
/*****************************************************************************/

/* EWUP: Enable WKUP2 pin */
#define PWR_CSR_EWUP2			(1 << 9)

/* EWUP: Enable WKUP1 pin */
#define PWR_CSR_EWUP1			(1 << 8)

/*****************************************************************************/
/* API definitions                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* API Functions                                                             */
/*****************************************************************************/

BEGIN_DECLS

END_DECLS

#endif

