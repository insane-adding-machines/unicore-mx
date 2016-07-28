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

static void stm32f072_usbd_disconnect(usbd_device *usbd_dev, bool disconnect);
static usbd_device *stm32f072_usbd_init(void);
static void copy_from_pm(void *buf, const volatile void *vPM, uint16_t len);
static void copy_to_pm(volatile void *vPM, const void *buf, uint16_t len);

static struct stm32_fsdev_private_data private_data;

static struct usbd_device _usbd_dev;

const struct usbd_backend usbd_stm32_fsdev_v2 = {
	.init = stm32f072_usbd_init,
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
	.disconnect = stm32f072_usbd_disconnect,
	.frame_number = stm32_fsdev_frame_number,
};

/** Initialize the USB device controller hardware of the STM32. */
static usbd_device *stm32f072_usbd_init(void)
{
	rcc_periph_clock_enable(RCC_USB);

	private_data.ep_count = ENDPOINT_COUNTS;
	private_data.copy_from_pm = copy_from_pm;
	private_data.copy_to_pm = copy_to_pm;
	_usbd_dev.backend_data = &private_data;

	stm32_fsdev_init(&_usbd_dev);
	USB_BCDR = USB_BCDR_DPPU;

	return &_usbd_dev;
}

/* copy_to_pm() and copy_from_pm() check alignment of memory
 *  and handle unaligned access seperatly because the core used
 *  on the {STM32F0, STM32L0} which do not support unaligned access.
 * STM32F0 - Cortex M0
 * STM32F1 - Cortex M0+
 *
 * TODO: this backend is also used for STM32L1.
 *  but since STM32L1 has a Cortex M3 core, it support unaligned access.
 */

void copy_to_pm(volatile void *vPM, const void *buf, uint16_t len)
{
	volatile uint16_t *PM = vPM;

	if(((uintptr_t) buf) & 0x01) {
		const uint8_t *lbuf = buf;
		for (len >>= 1; len; PM++, lbuf += 2, len--) {
			*PM = ((*(lbuf + 1)) << 8) | (*lbuf);
		}

		*(uint8_t *) PM = *(uint8_t *) lbuf;
	} else {
		const uint16_t *lbuf = buf;
		for (len = (len + 1) >> 1; len; PM++, lbuf++, len--) {
			*PM = *lbuf;
		}
	}
}

static void copy_from_pm(void *buf, const volatile void *vPM, uint16_t len)
{
	const volatile uint16_t *PM = vPM;
	uint8_t odd = len & 1;
	len >>= 1;

	if(((uintptr_t) buf) & 0x01) {
		for (; len; PM++, len--) {
			register uint16_t value = *PM;
			*(uint8_t *) buf++ = value;
			*(uint8_t *) buf++ = value >> 8;
		}
	} else {
		for (; len; PM++, buf += 2, len--) {
			*(uint16_t *) buf = *PM;
		}
	}

	if (odd) {
		*(uint8_t *) buf = *(uint8_t *) PM;
	}
}

static void stm32f072_usbd_disconnect(usbd_device *usbd_dev, bool disconnect)
{
	(void) usbd_dev;

	if(disconnect) {
		USB_CNTR |= USB_CNTR_PWDN;
		USB_BCDR &= ~USB_BCDR_DPPU;
	} else {
		USB_CNTR &= ~USB_CNTR_PWDN;
		USB_BCDR |= USB_BCDR_DPPU;
	}
}
