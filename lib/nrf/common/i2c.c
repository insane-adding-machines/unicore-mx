/** @addtogroup i2c_defines
 *
 * @brief <b>Access functions for the NRF51 I2C </b>
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

#include <unicore-mx/nrf/i2c.h>

/** @brief Enable I2C peripheral
 *
 * @param[in] i2c uint32_t i2c peripheral base
 */
void i2c_enable(uint32_t i2c)
{
    I2C_ENABLE(i2c) = I2C_ENABLE_VALUE;
}

/** @brief Disable I2C peripheral
 *
 * @param[in] i2c uint32_t i2c peripheral base
 */
void i2c_disable(uint32_t i2c)
{
    I2C_ENABLE(i2c) = 0;
}

/** @brief Start I2C transmission.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] data uint8_t the first byte to send.
 */
void i2c_start_tx(uint32_t i2c, uint8_t data)
{
    periph_trigger_task(I2C_TASK_STARTTX(i2c));
    I2C_TXD(i2c) = data;
}

/** @brief Start I2C reception.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 */
void i2c_start_rx(uint32_t i2c)
{
    periph_trigger_task(I2C_TASK_STARTRX(i2c));
}

/** @brief Signal stop on I2C line.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 */
void i2c_send_stop(uint32_t i2c)
{
    periph_trigger_task(I2C_TASK_STOP(i2c));
}

/** @brief Select Fast (400kHz) mode.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 */
void i2c_set_fast_mode(uint32_t i2c)
{
    I2C_FREQUENCY(i2c) = I2C_FREQUENCY_400K;
}

/** @brief Select Standard (100kHz) mode.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 */
void i2c_set_standard_mode(uint32_t i2c)
{
    I2C_FREQUENCY(i2c) = I2C_FREQUENCY_100K;
}

/** @brief Set I2C frequency.
 *
 * In addition to Standard (100kHz) and Fast (400kHz) modes
 * this peripheral also supports 250kHz mode.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] freq uint32_t frequency constant. See defines for details
 *     and note that this is not actually a frequency in Hz or kHz.
 */
void i2c_set_frequency(uint32_t i2c, uint32_t freq)
{
    I2C_FREQUENCY(i2c) = freq;
}

/** @brief Enable interrupts.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] interrupts uint32_t interrupts mask.
 */
void i2c_enable_interrupts(uint32_t i2c, uint32_t interrupts)
{
    periph_enable_interrupts(i2c, interrupts);
}

/** @brief Disable interrupts.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] interrupts uint32_t interrupts mask.
 */
void i2c_disable_interrupts(uint32_t i2c, uint32_t interrupts)
{
    periph_disable_interrupts(i2c, interrupts);
}

/** @brief Write Data to TXD register to be sent.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] data uint8_t byte to send next.
 */
void i2c_send_data(uint32_t i2c, uint8_t data)
{
    I2C_TXD(i2c) = data;
}

/** @brief Read Data from RXD register.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @returns uint8_t data from RXD register.
 */
uint8_t i2c_get_data(uint32_t i2c)
{
    return (uint8_t)I2C_RXD(i2c);
}

/** @brief Select GPIO pins to be used by this peripheral.
 *
 * This needs to be configured when no transaction is in progress.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] i2c scl_pin SCL pin (0-31).
 * @param[in] i2c sda_pin SDA pin (0-31).
 */
void i2c_select_pins(uint32_t i2c, uint8_t scl_pin, uint8_t sda_pin)
{
    I2C_PSELSCL(i2c) = scl_pin;
    I2C_PSELSDA(i2c) = sda_pin;
}

/** @brief Set 7bit I2C address of the device you wish to communicate with.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] addr uint8_t device address (7bit).
 */
void i2c_set_address(uint32_t i2c, uint8_t addr)
{
    I2C_ADDRESS(i2c) = addr;
}

/** @brief Enable shortcuts.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] shorts uint32_t Shortcuts to enable.
 */
void i2c_enable_shorts(uint32_t i2c, uint32_t shorts)
{
    periph_enable_shorts(i2c, shorts);
}

/** @brief Disable shortcuts.
 *
 * Because this peripheral only supports two mutually exclusive
 * shortcuts, the second argument is ignored and all shortcuts are
 * cleared.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 * @param[in] shorts uint32_t Shortcuts to disable.
 */
void i2c_disable_shorts(uint32_t i2c, uint32_t shorts)
{
    periph_disable_shorts(i2c, shorts);
}

/** @brief Resume I2C transaction.
 *
 * This function is unusual, but required to implement
 * i2c exchange with this peripheral.
 *
 * @param[in] i2c uint32_t i2c peripheral base.
 */
void i2c_resume(uint32_t i2c)
{
    periph_trigger_task(I2C_TASK_RESUME(i2c));
}


