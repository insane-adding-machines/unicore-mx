/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2015, 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

#include "stm32_fsdev_private.h"
#include "../usbd_private.h"

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/st_usbfs.h>
#include <unicore-mx/usbd/usbd.h>

static struct usbd_device _usbd_dev;

static usbd_device *init(const usbd_backend_config *config);
static void copy_from_pm(void *buf, const volatile void *vPM, uint16_t len);
static void copy_to_pm(volatile void *vPM, const void *buf, uint16_t len);

const struct usbd_backend usbd_stm32_fsdev_v1 = {
	.init = init,
	.set_address = stm32_fsdev_set_address,
	.get_address = stm32_fsdev_get_address,
	.ep_prepare_start = stm32_fsdev_ep_prepare_start,
	.ep_prepare = stm32_fsdev_ep_prepare,
	.get_ep_dtog = stm32_fsdev_get_ep_dtog,
	.set_ep_dtog = stm32_fsdev_set_ep_dtog,
	.set_ep_stall = stm32_fsdev_set_ep_stall,
	.get_ep_stall = stm32_fsdev_get_ep_stall,
	.poll = stm32_fsdev_poll,
	.enable_sof = stm32_fsdev_enable_sof,
	.get_speed = stm32_fsdev_get_speed,
	.urb_submit = stm32_fsdev_urb_submit,
	.urb_cancel = stm32_fsdev_urb_cancel,
	.frame_number = stm32_fsdev_frame_number,

	.copy_from_pm = copy_from_pm,
	.copy_to_pm = copy_to_pm
};

static const usbd_backend_config _config = {
	.ep_count = 8,
	.priv_mem = 512,
	.speed = USBD_SPEED_FULL,
	.feature = USBD_FEATURE_NONE
};

/** Initialize the USB device controller hardware of the STM32. */
static usbd_device *init(const usbd_backend_config *config)
{
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USBEN);

	if (config == NULL) {
		config = &_config;
	}

	_usbd_dev.backend = &usbd_stm32_fsdev_v1;
	_usbd_dev.config = config;
	stm32_fsdev_init(&_usbd_dev);

	return &_usbd_dev;
}

/* copy_to_pm() and copy_from_pm() do not check alignment of memory
 *  and handle unaligned access seperatly because the
 *  the core (Cortex M3) used on the f1 support unaligned access.
 */

static void copy_to_pm(volatile void *vPM, const void *vBuf, uint16_t len)
{
	const uint16_t *hBuf = vBuf;
	volatile uint16_t *hPM = vPM;

	len = DIVIDE_AND_CEIL(len, 2);

	while (len--) {
		*hPM = *hBuf++;
		hPM += 2; /* 32bit space */
	}
}

static void copy_from_pm(void *vBuf, const volatile void *vPM, uint16_t len)
{
	uint16_t *hBuf = vBuf;
	const volatile uint16_t *hPM = vPM;
	bool odd = !!(len & 1);

	len /= 2;
	while (len) {
		*hBuf++ = *hPM;
		hPM += 2; /* 32bit space */
	}

	if (odd) {
		*(uint8_t *) hBuf = *(uint8_t *) hPM;
	}
}
