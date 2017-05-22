/** @addtogroup timer_defines
 *
 * @brief <b>Access functions for the NRF51 Timer/Counter </b>
 * @ingroup NRF51_defines
 * LGPL License Terms @ref lgpl_license
 * @author @htmlonly &copy; @endhtmlonly 2016
 * Maxim Sloyko <maxims@google.com>
 *
 */

/*
 * This file is part of the unicore-mx project.
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

#include <unicore-mx/nrf/timer.h>

/** @brief Set timer mode (counter/timer)
 *
 * @param[in] timer uint32_t timer base
 * @param[in] mode enum timer_mode
 */
void timer_set_mode(uint32_t timer, enum timer_mode mode)
{
	TIMER_MODE(timer) = mode;
}

/** @brief Set timer bit mode (width)
 *
 * @param[in] timer uint32_t timer base
 * @param[in] bitmode enum timer_bitmode
 */
void timer_set_bitmode(uint32_t timer, enum timer_bitmode bitmode)
{
	TIMER_BITMODE(timer) = bitmode;
}

/** @brief Start the timer
 *
 * @param[in] timer uint32_t timer base
 */
void timer_start(uint32_t timer)
{
	periph_trigger_task(TIMER_TASK_START(timer));
}

/** @brief Stop the timer
 *
 * @param[in] timer uint32_t timer base
 */
void timer_stop(uint32_t timer)
{
	periph_trigger_task(TIMER_TASK_STOP(timer));
}

/** @brief Clear the timer
 *
 * @param[in] timer uint32_t timer base
 */
void timer_clear(uint32_t timer)
{
	periph_trigger_task(TIMER_TASK_CLEAR(timer));
}

/** @brief Set prescaler value
 *
 * @param[in] timer uint32_t timer base
 * @param[in] presc uint8_t prescaler value
 */
void timer_set_prescaler(uint32_t timer, uint8_t presc)
{
	TIMER_PRESCALER(timer) = presc & TIMER_PRESCALER_MASK;
}

/** @brief Set compare register
 *
 * @param[in] timer uint32_t timer base
 * @param[in] compare_num uint8_t compare number (0-3)
 * @param[in] compare_num uint32_t compare value
 */
void timer_set_compare(uint32_t timer, uint8_t compare_num, uint32_t compare_val)
{
	if (compare_num > 3) {
		return;
	}

	TIMER_CC(timer, compare_num) = compare_val;
}
