/*
 * This file is part of the libopencm3 project.
 *
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
#include <libopencm3/lpc43xx/cgu.h>
#include <libopencm3/lpc43xx/uart.h>

#include "../jellybean_conf.h"

void gpio_setup(void)
{
	/* Configure all GPIO as Input (safe state) */
	GPIO0_DIR = 0;
	GPIO1_DIR = 0;
	GPIO2_DIR = 0;
	GPIO3_DIR = 0;
	GPIO4_DIR = 0;
	GPIO5_DIR = 0;
	GPIO6_DIR = 0;
	GPIO7_DIR = 0;

	/* Configure SCU Pin Mux as GPIO */
	scu_pinmux(SCU_PINMUX_LED1, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_LED2, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_LED3, SCU_GPIO_FAST);
	
	scu_pinmux(SCU_PINMUX_EN1V8, SCU_GPIO_FAST);
	
	scu_pinmux(SCU_PINMUX_BOOT0, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_BOOT1, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_BOOT2, SCU_GPIO_FAST);
	scu_pinmux(SCU_PINMUX_BOOT3, SCU_GPIO_FAST);

	/* Configure UART0 Peripheral */
	scu_pinmux(SCU_UART0_RX, (SCU_UART_RX_TX | SCU_CONF_FUNCTION1));
	scu_pinmux(SCU_UART0_TX, (SCU_UART_RX_TX | SCU_CONF_FUNCTION1));

	/* Configure UART3 Peripheral */
	scu_pinmux(SCU_UART3_RX, (SCU_UART_RX_TX | SCU_CONF_FUNCTION2));
	scu_pinmux(SCU_UART3_TX, (SCU_UART_RX_TX | SCU_CONF_FUNCTION2));

	/* Configure GPIO as Output */
	GPIO2_DIR |= (PIN_LED1|PIN_LED2|PIN_LED3); /* Configure GPIO2[1/2/8] (P4_1/2 P6_12) as output. */
	GPIO3_DIR |= PIN_EN1V8; /* GPIO3[6] on P6_10  as output. */
}


#define WAIT_RX_MAX_CYCLES (100000)

int main(void)
{
	int i;
	u8 uart_val;
	u8 uart_rx_val;
    u16 uart_divisor;
    u8 uart_divaddval;
    u8 uart_mulval;
    uart_error_t error;
	uart_rx_data_ready_t uart_rx_ready;

	gpio_setup();

    uart_divisor = 125;
    uart_divaddval = 1;
    uart_mulval = 4;
	/* 
        Baudrate 115200 => ='PCLK' / ( 16* ((('Divisor')*(1+('DivAddVal'/'MulVal'))) )) with PCLK=PLL1=288MHz 
        288000000/(16*125*(1+(1/4))) = 115200 bauds
    */

    /* Init UART0 @ 115200bauds */
	uart_init(UART0_NUM,
              UART_DATABIT_8, UART_STOPBIT_1, UART_PARITY_NONE,
              uart_divisor, uart_divaddval, uart_mulval);

    /* Init UART3 @ 115200bauds */
	uart_init(UART3_NUM,
              UART_DATABIT_8, UART_STOPBIT_1, UART_PARITY_NONE,
              uart_divisor, uart_divaddval, uart_mulval);

	uart_val = 0x0;

	while (1)
	{
		uart_write(UART0_NUM, uart_val);
		uart_write(UART3_NUM, uart_val);

		/* Read & check Written Data */
		/* Check UART0 */
		uart_rx_val = uart_read_timeout(UART0_NUM, WAIT_RX_MAX_CYCLES, &error);
		if( !(uart_rx_val==uart_val && UART_NO_ERROR==error) )
		{
            gpio_set(PORT_LED1_3, PIN_LED2); /* LED2 on means error on UART0 */
		}

		/* Check UART3 */
		for(i=0; i<WAIT_RX_MAX_CYCLES; i++)
		{
			uart_rx_ready = uart_rx_data_ready(UART3_NUM);
			if( UART_RX_DATA_ERROR == uart_rx_ready )
			{
				gpio_set(PORT_LED1_3, PIN_LED3); /* LED3 on means error on UART3 */
			}
			if( UART_RX_DATA_READY == uart_rx_ready )
			{
				uart_rx_val = uart_read(UART3_NUM);
				if( uart_rx_val != uart_val )
				{
					gpio_set(PORT_LED1_3, PIN_LED2); /* LED2 on means error on UART0 */
				}else
				{
					/* No error */
					break;
				}
			}
		}

		gpio_set(PORT_LED1_3, PIN_LED1); /* LED1 on */

		for (i = 0; i < 1000; i++) /* Wait a bit. */
			__asm__("nop");

		gpio_clear(PORT_LED1_3, PIN_LED1); /* LED1 off */

		uart_val++;
	}

	return 0;
}
