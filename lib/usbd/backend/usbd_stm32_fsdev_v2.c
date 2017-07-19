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

static usbd_device *init(const usbd_backend_config *config);
static void disconnect(usbd_device *dev, bool disconnect);
static void copy_from_pm(void *buf, const volatile void *vPM, uint16_t len);
static void copy_to_pm(volatile void *vPM, const void *buf, uint16_t len);

static struct usbd_device _usbd_dev;

const struct usbd_backend usbd_stm32_fsdev_v2 = {
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
	.disconnect = disconnect,
	.frame_number = stm32_fsdev_frame_number,

	.copy_from_pm = copy_from_pm,
	.copy_to_pm = copy_to_pm
};

static const usbd_backend_config _config = {
	.ep_count = 8,
	.priv_mem = 1024,
	.speed = USBD_SPEED_FULL,
	.feature = USBD_FEATURE_NONE
};

/** Initialize the USB device controller hardware of the STM32. */
static usbd_device *init(const usbd_backend_config *config)
{
	rcc_periph_clock_enable(RCC_USB);

	if (config == NULL) {
		config = &_config;
	}

	_usbd_dev.backend = &usbd_stm32_fsdev_v2;
	_usbd_dev.config = config;
	stm32_fsdev_init(&_usbd_dev);
	USB_BCDR = USB_BCDR_DPPU;

	return &_usbd_dev;
}

static void copy_to_pm(volatile void *vPM, const void *vBuf, uint16_t len)
{
	volatile uint16_t *hPM = vPM;

#if !defined(__ARM_FEATURE_UNALIGNED)
	if(((uintptr_t) vBuf) & 0x01) {
		const uint8_t *uBuf = vBuf;
		len /= 2;

		while (len--) {
			*hPM++ = (uBuf[1] << 8) | uBuf[0];
			uBuf += 2;
		}

		*(uint8_t *) hPM = *uBuf;
		return;
	}
#endif /* !defined(__ARM_FEATURE_UNALIGNED) */

	const uint16_t *hBuf = vBuf;
	len = DIVIDE_AND_CEIL(len, 2);

	while (len--) {
		*hPM++ = *hBuf++;
	}
}

static void copy_from_pm(void *vBuf, const volatile void *vPM, uint16_t len)
{
	const volatile uint16_t *hPM = vPM;
	uint8_t odd = len & 1;
	len /= 2;

#if !defined(__ARM_FEATURE_UNALIGNED)
	if(((uintptr_t) vBuf) & 0x01) {
		uint8_t *uBuf = vBuf;

		while (len--) {
			register uint16_t value = *hPM++;
			*uBuf++ = value;
			*uBuf++ = value >> 8;
		}

		if (odd) {
			*uBuf = *(uint8_t *) hPM;
		}

		return;
	}
#endif /* !defined(__ARM_FEATURE_UNALIGNED) */

	uint16_t *hBuf = vBuf;

	while (len--) {
		*hBuf++ = *hPM++;
	}

	if (odd) {
		*(uint8_t *) hBuf = *(uint8_t *) hPM;
	}
}

static void disconnect(usbd_device *dev, bool disconnect)
{
	(void) dev;

	if(disconnect) {
		USB_CNTR |= USB_CNTR_PDWN;
		USB_BCDR &= ~USB_BCDR_DPPU;
	} else {
		USB_CNTR &= ~USB_CNTR_PDWN;
		USB_BCDR |= USB_BCDR_DPPU;
	}
}
