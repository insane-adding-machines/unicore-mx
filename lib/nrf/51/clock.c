/** @addtogroup clock_defines
 *
 * @brief <b>Access functions for the NRF51 Clock Controller </b>
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

#include <unicore-mx/nrf/clock.h>
#include <unicore-mx/nrf/periph.h>

/** @brief Start Low Frequency Clock
 *
 * @param[in] wait bool: If true, will busy wait for the clock to start.
 */
void clock_start_lfclk(bool wait)
{
	periph_trigger_task(CLOCK_TASK_LFCLKSTART);
	if (wait) {
		while(!(CLOCK_LFCLKSTAT & CLOCK_LFCLKSTAT_STATE));
	}
}

/** @brief Stop Low Frequency Clock */
void clock_stop_lfclk()
{
	periph_trigger_task(CLOCK_TASK_LFCLKSTOP);
}

/** @brief Start High Frequency Crystal Oscillator.
 *
 * @details Oscillator needs to be running for the radio to work.
 *
 * @param[in] wait bool If true, will busy wait for the clock to start.
 */
void clock_start_hfclk(bool wait)
{
	periph_trigger_task(CLOCK_TASK_HFCLKSTART);
	if (wait) {
		while(!(CLOCK_HFCLKSTAT & CLOCK_HFCLKSTAT_STATE));
	}
}

/** @brief Stop High Frequency Crystal Oscillator */
void clock_stop_hfclk()
{
	periph_trigger_task(CLOCK_TASK_HFCLKSTOP);
}

/** @brief Select nominal frequency of external crystal for HFCLK.
 *
 * @details This register has to match the actual crystal used in design to
 * enable correct behaviour.
 *
 * @param[in] freq enum clock_xtal_freq
 * */
void clock_set_xtal_freq(enum clock_xtal_freq freq)
{
	CLOCK_XTALFREQ = freq;
}

/** @brief Low Frequency Clock Source.
 *
 * @param[in] lfclk_src enum clock_lfclk_src
 */
void clock_set_lfclk_src(enum clock_lfclk_src lfclk_src)
{
	CLOCK_LFCLKSRC = lfclk_src;
}
