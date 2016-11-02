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

#ifndef LIBUCMX_GPIO_H
#define LIBUCMX_GPIO_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/nrf/memorymap.h>

#define GPIO			(GPIO_BASE)

#define GPIO_OUT			MMIO32(GPIO_BASE + 0x504)
#define GPIO_OUTSET			MMIO32(GPIO_BASE + 0x508)
#define GPIO_OUTCLR			MMIO32(GPIO_BASE + 0x50C)

#define GPIO_IN			MMIO32(GPIO_BASE + 0x510)

#define GPIO_DIR			MMIO32(GPIO_BASE + 0x514)
#define GPIO_DIRSET			MMIO32(GPIO_BASE + 0x518)
#define GPIO_DIRCLR			MMIO32(GPIO_BASE + 0x51C)

#define GPIO_PIN_CNF(N)			MMIO32(GPIO_BASE + 0x700 + 0x4 * (N))

#define GPIOPIN0			(1 << 0)
#define GPIOPIN1			(1 << 1)
#define GPIOPIN2			(1 << 2)
#define GPIOPIN3			(1 << 3)
#define GPIOPIN4			(1 << 4)
#define GPIOPIN5			(1 << 5)
#define GPIOPIN6			(1 << 6)
#define GPIOPIN7			(1 << 7)
#define GPIOPIN8			(1 << 8)
#define GPIOPIN9			(1 << 9)
#define GPIOPIN10			(1 << 10)
#define GPIOPIN11			(1 << 11)
#define GPIOPIN12			(1 << 12)
#define GPIOPIN13			(1 << 13)
#define GPIOPIN14			(1 << 14)
#define GPIOPIN15			(1 << 15)
#define GPIOPIN16			(1 << 16)
#define GPIOPIN17			(1 << 17)
#define GPIOPIN18			(1 << 18)
#define GPIOPIN19			(1 << 19)
#define GPIOPIN20			(1 << 20)
#define GPIOPIN21			(1 << 21)
#define GPIOPIN22			(1 << 22)
#define GPIOPIN23			(1 << 23)
#define GPIOPIN24			(1 << 24)
#define GPIOPIN25			(1 << 25)
#define GPIOPIN26			(1 << 26)
#define GPIOPIN27			(1 << 27)
#define GPIOPIN28			(1 << 28)
#define GPIOPIN29			(1 << 29)
#define GPIOPIN30			(1 << 30)
#define GPIOPIN31			(1 << 31)

#define GPIOPINN(n)			(1 << (n))

#define PIN_CNF_DIR			(1 << 0)
#define PIN_CNF_INPUT			(1 << 1)

#define PIN_CNF_PULL_SHIFT			(2)
#define PIN_CNF_PULL_MASK			(3 << PIN_CNF_PULL_SHIFT)
#define PIN_CNF_PULL_MASKED(V)			(((V) << PIN_CNF_PULL_SHIFT) & PIN_CNF_PULL_MASK)

#define PIN_CNF_DRIVE_SHIFT			(8)
#define PIN_CNF_DRIVE_MASK			(7 << PIN_CNF_DRIVE_SHIFT)
#define PIN_CNF_DRIVE_MASKED(V)			(((V) << PIN_CNF_DRIVE_SHIFT) & PIN_CNF_DRIVE_MASK)

#define PIN_CNF_SENSE_SHIFT			(16)
#define PIN_CNF_SENSE_MASK			(3 << PIN_CNF_SENSE_SHIFT)
#define PIN_CNF_SENSE_MASKED(V)			(((V) << PIN_CNF_SENSE_SHIFT) & PIN_CNF_SENSE_MASK)

/* GPIO Tasks and Events (GPIOTE) */
#define GPIO_TASK_OUT(n)			MMIO32(GPIOTE_BASE + 0x4 * (n))
#define GPIO_TASK_OUT0			GPIO_TASK_OUT(0)
#define GPIO_TASK_OUT1			GPIO_TASK_OUT(1)
#define GPIO_TASK_OUT2			GPIO_TASK_OUT(2)
#define GPIO_TASK_OUT3			GPIO_TASK_OUT(3)

#define GPIO_EVENT_IN(n)			MMIO32(GPIOTE_BASE + 0x100 + 0x4 * (n))
#define GPIO_EVENT_IN0			GPIO_EVENT_IN(0)
#define GPIO_EVENT_IN1			GPIO_EVENT_IN(1)
#define GPIO_EVENT_IN2			GPIO_EVENT_IN(2)
#define GPIO_EVENT_IN3			GPIO_EVENT_IN(3)

#define GPIO_EVENT_PORT			MMIO32(GPIOTE_BASE + 0x17C)

#define GPIO_INTEN			MMIO32(GPIOTE_BASE + 0x300)
#define GPIO_INTENSET			MMIO32(GPIOTE_BASE + 0x304)
#define GPIO_INTENCLR			MMIO32(GPIOTE_BASE + 0x308)

#define GPIO_TE_CONFIG(n)			MMIO32(GPIOTE_BASE + 0x510 + 0x4 * (n))
#define GPIO_TE_CONFIG0			GPIO_TE_CONFIG(0)
#define GPIO_TE_CONFIG1			GPIO_TE_CONFIG(1)
#define GPIO_TE_CONFIG2			GPIO_TE_CONFIG(2)
#define GPIO_TE_CONFIG3			GPIO_TE_CONFIG(3)

/* Register Details */
#define GPIO_INTEN_IN(n)			(1 << (n))
#define GPIO_INTEN_IN0			(1 << 0)
#define GPIO_INTEN_IN1			(1 << 1)
#define GPIO_INTEN_IN2			(1 << 2)
#define GPIO_INTEN_IN3			(1 << 3)

#define GPIO_INTEN_PORT			(1 << 31)

#define GPIO_TE_CONFIG_MODE_SHIFT			(0)
#define GPIO_TE_CONFIG_MODE_MASK			(3 << GPIO_TE_CONFIG_MODE_SHIFT)
#define GPIO_TE_CONFIG_MODE_MASKED(V)			(((V) << GPIO_TE_CONFIG_MODE_SHIFT) & GPIO_TE_CONFIG_MODE_MASK)

#define GPIO_TE_CONFIG_PSEL_SHIFT			(8)
#define GPIO_TE_CONFIG_PSEL_MASK			(0x1f << GPIO_TE_CONFIG_PSEL_SHIFT)
#define GPIO_TE_CONFIG_PSEL_MASKED(V)			(((V) << GPIO_TE_CONFIG_PSEL_SHIFT) & GPIO_TE_CONFIG_PSEL_MASK)

#define GPIO_TE_CONFIG_POLARITY_SHIFT			(16)
#define GPIO_TE_CONFIG_POLARITY_MASK			(3 << GPIO_TE_CONFIG_POLARITY_SHIFT)
#define GPIO_TE_CONFIG_POLARITY_MASKED(V)			(((V) << GPIO_TE_CONFIG_POLARITY_SHIFT) & GPIO_TE_CONFIG_POLARITY_MASK)

#define GPIO_TE_CONFIG_OUTINIT			(1 << 20)

#define GPIO_DIR_INPUT			(0)
#define GPIO_DIR_OUTPUT			(1)

#define GPIO_INPUT_CONNECT			(0)
#define GPIO_INPUT_DISCONNECT		(1)

#define GPIO_PULL_NONE			(0)
#define GPIO_PULL_DOWN			(1)
#define GPIO_PULL_UP			(3)

#define GPIO_DRIVE_S0S1			(0)
#define GPIO_DRIVE_H0S1			(1)
#define GPIO_DRIVE_S0H1			(2)
#define GPIO_DRIVE_H0H1			(3)
#define GPIO_DRIVE_D0S1			(4)
#define GPIO_DRIVE_D0H1			(5)
#define GPIO_DRIVE_S0D1			(6)
#define GPIO_DRIVE_H0D1			(7)

#define GPIO_TE_MODE_DISABLED			(0)
#define GPIO_TE_MODE_EVENT			(1)
#define GPIO_TE_MODE_TASK			(3)

#define GPIO_TE_POLARITY_NONE			(0)
#define GPIO_TE_POLARITY_LO_TO_HI			(1)
#define GPIO_TE_POLARITY_HI_TO_LO			(2)
#define GPIO_TE_POLARITY_TOGGLE			(3)

#define GPIO_TE_OUTINIT_LOW			(1)
#define GPIO_TE_OUTINIT_HIGH			GPIO_TE_CONFIG_OUTINIT

BEGIN_DECLS

void gpio_set(uint32_t gpios);
void gpio_clear(uint32_t gpios);
void gpio_toggle(uint32_t gpios);
uint32_t gpio_get(uint32_t gpios);

void gpio_setup_mode(uint32_t gpios, uint8_t dir, uint8_t pull);

void gpio_set_drive(uint32_t gpios, uint8_t drive);

void gpio_configure_task(uint8_t task_num,
		uint8_t pin_num, uint8_t polarity, uint8_t init);

void gpio_configure_event(uint8_t event_num, uint8_t pin_num, uint8_t polarity);

void gpio_enable_interrupts(uint32_t mask);
void gpio_disable_interrupts(uint32_t mask);
void gpio_clear_interrupts(void);

END_DECLS

#endif	/* LIBUCMX_GPIO_H */
