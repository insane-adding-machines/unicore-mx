/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2015 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/tools.h>
#include <unicore-mx/stm32/st_usbfs.h>
#include <unicore-mx/usbd/usbd.h>
#include "../usbd_private.h"
#include "stm32_fsdev_private.h"

/* Number of endpoints */
#define ENDPOINT_COUNTS 8

static struct stm32_fsdev_private_data private_data;
static struct usbd_device _usbd_dev;

static usbd_device *stm32f103_usbd_init(void);
static void copy_from_pm(void *buf, const volatile void *vPM, uint16_t len);
static void copy_to_pm(volatile void *vPM, const void *buf, uint16_t len);

const struct usbd_backend usbd_stm32_fsdev_v1 = {
	.init = stm32f103_usbd_init,
	.set_address = stm32_fsdev_set_address,
	.ep_setup = stm32_fsdev_ep_setup,
	.set_ep_type = stm32_fsdev_set_ep_type,
	.set_ep_size = stm32_fsdev_set_ep_size,
	.ep_reset = stm32_fsdev_endpoints_reset,
	.set_ep_stall = stm32_fsdev_set_ep_stall,
	.get_ep_stall = stm32_fsdev_get_ep_stall,
	.set_ep_nak = stm32_fsdev_set_ep_nak,
	.ep_write_packet = stm32_fsdev_ep_write_packet,
	.ep_read_packet = stm32_fsdev_ep_read_packet,
	.enable_sof = stm32_fsdev_enable_sof,
	.poll = stm32_fsdev_poll,
	.get_ep_dtog = stm32_fsdev_get_ep_dtog,
	.set_ep_dtog = stm32_fsdev_set_ep_dtog,
	.ep_flush = stm32_fsdev_ep_flush,
	.frame_number = stm32_fsdev_frame_number
};

/** Initialize the USB device controller hardware of the STM32. */
static usbd_device *stm32f103_usbd_init(void)
{
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USBEN);

	private_data.ep_count = ENDPOINT_COUNTS;
	private_data.copy_from_pm = copy_from_pm;
	private_data.copy_to_pm = copy_to_pm;
	_usbd_dev.backend_data = &private_data;

	stm32_fsdev_init(&_usbd_dev);

	return &_usbd_dev;
}

/* copy_to_pm() and copy_from_pm() do not check alignment of memory
 *  and handle unaligned access seperatly because the
 *  the core (Cortex M3) used on the f1 support unaligned access.
 */

static void copy_to_pm(volatile void *vPM, const void *buf, uint16_t len)
{
	const uint16_t *lbuf = buf;
	volatile uint16_t *PM = vPM;

	for (len = (len + 1) >> 1; len; PM += 2, lbuf++, len--) {
		*PM = *lbuf;
	}
}

static void copy_from_pm(void *buf, const volatile void *vPM, uint16_t len)
{
	uint16_t *lbuf = buf;
	const volatile uint16_t *PM = vPM;
	uint8_t odd = len & 1;

	for (len >>= 1; len; PM += 2, lbuf++, len--) {
		*lbuf = *PM;
	}

	if (odd) {
		*(uint8_t *) lbuf = *(uint8_t *) PM;
	}
}
