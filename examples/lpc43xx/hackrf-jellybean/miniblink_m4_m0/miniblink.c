/*
* This file is part of the libopencm3 project.
*
* Copyright (C) 2010 Uwe Hermann <uwe@hermann-uwe.de>
* Copyright (C) 2012 Michael Ossmann <mike@ossmann.com>
* Copyright (C) 2012 Benjamin Vernoux <titanmkd@gmail.com>
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

#include <libopencm3/lpc43xx/gpio.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/ipc.h>

#include "../jellybean_conf.h"
#include "m0_bin.h"

void gpio_setup(void)
{
	/* Configure SCU Pin Mux as GPIO */
	scu_pinmux(SCU_PINMUX_LED1, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_LED2, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_LED3, SCU_GPIO_FAST);

	scu_pinmux(SCU_PINMUX_EN1V8, SCU_GPIO_FAST);

	scu_pinmux(SCU_PINMUX_BOOT0, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_BOOT1, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_BOOT2, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_BOOT3, SCU_GPIO_FAST);

	/* Configure all GPIO as Input (safe state) */
	GPIO0_DIR = 0;
	GPIO1_DIR = 0;
	GPIO2_DIR = 0;
	GPIO3_DIR = 0;
	GPIO4_DIR = 0;
	GPIO5_DIR = 0;
	GPIO6_DIR = 0;
	GPIO7_DIR = 0;

	/* Configure GPIO as Output */
	GPIO2_DIR |= (PIN_LED1|PIN_LED2|PIN_LED3); /* Configure GPIO2[1/2/8] (P4_1/2 P6_12) as output. */
	GPIO3_DIR |= PIN_EN1V8; /* GPIO3[6] on P6_10  as output. */
}

void m0_startup(void)
{
	u32 *src, *dest;

	/* Halt M0 core (in case it was running) */
	ipc_halt_m0();

	/* Copy M0 code from M4 embedded addr to final addr M0 */
	dest = &cm0_exec_baseaddr;
	for(src = (u32 *)&m0_bin[0]; src < (u32 *)(&m0_bin[0]+m0_bin_size); )
	{
		*dest++ = *src++;
	}

	ipc_start_m0( (u32)(&cm0_exec_baseaddr) );
}

u32 boot0, boot1, boot2, boot3;

int main(void)
{
	int i;
	gpio_setup();

	/* Set 1V8 */
	gpio_set(PORT_EN1V8, PIN_EN1V8);

	/* Start M0 */
	m0_startup();

	/* Blink LED2 on the board and Read BOOT0/1/2/3 pins. */
	while (1) 
	{
		boot0 = BOOT0_STATE;
		boot1 = BOOT1_STATE;
		boot2 = BOOT2_STATE;
		boot3 = BOOT3_STATE;

		gpio_set(PORT_LED1_3, (PIN_LED2)); /* LEDs on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
		gpio_clear(PORT_LED1_3, (PIN_LED2)); /* LED off */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
