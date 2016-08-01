/*
 * This file is part of the unicore-mx project.
 *
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

#include <unicore-mx/efm32/memorymap.h>
#include <unicore-mx/efm32/cmu.h>
#include <unicore-mx/efm32/usb.h>
#include <unicore-mx/usbd/usbd.h>
#include "../usbd_private.h"
#include "dwc_otg_private.h"

/* Receive FIFO size in 32-bit words. */
#define RX_FIFO_SIZE 256

#define ENDPOINTS_COUNT 6

static uint32_t doeptsiz[ENDPOINTS_COUNT];
static struct dwc_otg_private_data private_data;
static struct usbd_device _usbd_dev;

/** Initialize the USB_FS device controller hardware of the STM32. */
static usbd_device *efm32lg_usbd_init(void)
{
	/* Enable clock */
	CMU_HFCORECLKEN0 |= CMU_HFCORECLKEN0_USB | CMU_HFCORECLKEN0_USBC;
	CMU_CMD = CMU_CMD_USBCCLKSEL_HFCLKNODIV;

	/* wait till clock not selected */
	while (!(CMU_STATUS & CMU_STATUS_USBCHFCLKSEL));

	private_data.base_address = USB_BASE;
	private_data.rx_fifo_size = RX_FIFO_SIZE;
	private_data.fifo_mem_top = RX_FIFO_SIZE;
	private_data.doeptsiz = doeptsiz;
	private_data.ep_count = ENDPOINTS_COUNT;
	_usbd_dev.backend_data = &private_data;

	dwc_otg_init(&_usbd_dev);

	return &_usbd_dev;
}

const struct usbd_backend usbd_efm32lg = {
	.init = efm32lg_usbd_init,
	.set_address = dwc_otg_set_address,
	.ep_setup = dwc_otg_ep_setup,
	.set_ep_type = dwc_otg_set_ep_type,
	.set_ep_size = dwc_otg_set_ep_size,
	.ep_reset = dwc_otg_endpoints_reset,
	.set_ep_stall = dwc_otg_set_ep_stall,
	.get_ep_stall = dwc_otg_get_ep_stall,
	.set_ep_nak = dwc_otg_set_ep_nak,
	.ep_write_packet = dwc_otg_ep_write_packet,
	.ep_read_packet = dwc_otg_ep_read_packet,
	.poll = dwc_otg_poll,
	.disconnect = dwc_otg_disconnect,
	.enable_sof = dwc_otg_enable_sof,
	.get_ep_dtog = dwc_otg_get_ep_dtog,
	.set_ep_dtog = dwc_otg_set_ep_dtog,
	.ep_flush = dwc_otg_ep_flush,
	.frame_number = dwc_otg_frame_number,
	.set_address_before_status = true,
};
