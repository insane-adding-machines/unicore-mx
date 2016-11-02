/** @addtogroup gpio_defines
 *
 * @brief <b>Access functions for the NRF51 I/O Controller</b>
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

#include <unicore-mx/nrf/gpio.h>

/** @brief Atomic set output
 *
 * @param[in] gpios uint32_t
 */
void gpio_set(uint32_t gpios)
{
	GPIO_OUTSET = gpios;
}

/** @brief Atomic clear output
 *
 * @param[in] gpios uint32_t
 */
void gpio_clear(uint32_t gpios)
{
	GPIO_OUTCLR = gpios;
}

/** @brief Toggle output
 *
 * @param[in] gpios uint32_t
 */
void gpio_toggle(uint32_t gpios)
{
	uint32_t reg_val = GPIO_OUT;
	GPIO_OUTCLR = reg_val & gpios;
	GPIO_OUTSET = (~reg_val) & gpios;
}

/** @brief Read GPIO values
 *
 * @param[in] gpios uint32_t
 */
uint32_t gpio_get(uint32_t gpios)
{
	return GPIO_IN & gpios;
}

/** @brief Set direction (input/output) and pull for a set of gpios.
 *
 * @param[in] gpios uint32_t
 * @param[in] dir uint8_t
 * @param[in] pull uint8_t
 */
void gpio_setup_mode(uint32_t gpios, uint8_t dir, uint8_t pull)
{
	if (GPIO_DIR_INPUT == dir) {
		GPIO_DIRCLR = gpios;
	} else {
		GPIO_DIRSET = gpios;
	}

	uint8_t i = 0;
	while (gpios) {
		if (gpios & 1) {
			uint32_t reg_pin_cnf = GPIO_PIN_CNF(i);
			reg_pin_cnf &= ~PIN_CNF_PULL_MASK;
			GPIO_PIN_CNF(i) = reg_pin_cnf | PIN_CNF_PULL_MASKED(pull);
		}
		++i;
		gpios >>= 1;
	}
}

/** @brief Set drive for gpios.
 *
 * Some peripherals require this to be configured in a certain way.
 *
 * @param[in] gpios uint32_t
 * @param[in] drive uint8_t
 */
void gpio_set_drive(uint32_t gpios, uint8_t drive)
{
	uint8_t i = 0;
	while (gpios) {
		if (gpios & 1) {
			uint32_t reg_pin_cnf = GPIO_PIN_CNF(i);
			reg_pin_cnf &= ~PIN_CNF_DRIVE_MASK;
			GPIO_PIN_CNF(i) = reg_pin_cnf | PIN_CNF_DRIVE_MASKED(drive);
		}
		++i;
		gpios >>= 1;
	}
}

/** @brief Configure Task in GPIO TE Module
 *
 * @param[in] task_num uint8_t Task number (0-3)
 * @param[in] pin_num uint8_t GPIO Pin number (0-31)
 * @param[in] polarity uint8_t polarity Operation to perform when task is triggered.
 * @param[in] init uint8_t Initial state of the pin
 */
void gpio_configure_task(uint8_t task_num,
		uint8_t pin_num, uint8_t polarity, uint8_t init)
{
	GPIO_TE_CONFIG(task_num) = GPIO_TE_CONFIG_MODE_MASKED(GPIO_TE_MODE_TASK)
		| GPIO_TE_CONFIG_PSEL_MASKED(pin_num)
		| GPIO_TE_CONFIG_POLARITY_MASKED(polarity)
		| init;
}

/** @brief Configure Event in GPIO TE Module
 *
 * @param[in] event_num uint8_t Event number (0-3)
 * @param[in] pin_num uint8_t GPIO Pin number (0-31)
 * @param[in] uint8_t Operation to perform when task is triggered.
 */
void gpio_configure_event(uint8_t event_num, uint8_t pin_num, uint8_t polarity)
{
	GPIO_TE_CONFIG(event_num) = GPIO_TE_CONFIG_MODE_MASKED(GPIO_TE_MODE_EVENT)
		| GPIO_TE_CONFIG_PSEL_MASKED(pin_num)
		| GPIO_TE_CONFIG_POLARITY_MASKED(polarity);
}

/** @brief Enable GPIO interrupts
 *
 * @param[in] mask uint32_t which interrupts to enable.
 */
void gpio_enable_interrupts(uint32_t mask)
{
	GPIO_INTENSET = mask;
}

/** @brief Disable GPIO interrupts
 *
 * @param[in] mask uint32_t which interrupts to disable.
 */
void gpio_disable_interrupts(uint32_t mask)
{
	GPIO_INTENCLR = mask;
}

/** @brief Disable all GPIO interrupts
 *
 * @param[in] mask uint32_t which interrupts to disable.
 */
void gpio_clear_interrupts(void)
{
	GPIO_INTENCLR = 0xffffffff;
}
