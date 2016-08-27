/*
 * This file is part of the unicore-mx project.
 *
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

#include "dwc_otg_private.h"
#include "../usbd_private.h"

#include <unicore-mx/efm32/memorymap.h>
#include <unicore-mx/efm32/cmu.h>
#include <unicore-mx/efm32/usb.h>
#include <unicore-mx/usbd/usbd.h>

static struct usbd_device _usbd_dev;

static const usbd_backend_config _config = {
	.ep_count = 6,
	.priv_mem = 2048, /* 2KB * 1024 */
	.speed = USBD_SPEED_FULL,
	.feature = USBD_FEATURE_NONE
};

#define REBASE(REG, ...)	REG(usbd_efm32lg.base_address, ##__VA_ARGS__)

static usbd_device *init(const usbd_backend_config *config)
{
	/* Enable clock */
	CMU_HFCORECLKEN0 |= CMU_HFCORECLKEN0_USB | CMU_HFCORECLKEN0_USBC;
	CMU_CMD = CMU_CMD_USBCCLKSEL_HFCLKNODIV;

	/* wait till clock not selected */
	while (!(CMU_STATUS & CMU_STATUS_USBCHFCLKSEL));

	USB_ROUTE = USB_ROUTE_DMPUPEN | USB_ROUTE_PHYPEN;

	if (config == NULL) {
		config = &_config;
	}

	if (!(config->feature & USBD_VBUS_SENSE)) {
		REBASE(DWC_OTG_GOTGCTL) |= DWC_OTG_GOTGCTL_BVALOEN |
										DWC_OTG_GOTGCTL_BVALOVAL;
	}

	/* Internal PHY */
	REBASE(DWC_OTG_GUSBCFG) |= DWC_OTG_GUSBCFG_PHYSEL;

	/* Full speed device. */
	REBASE(DWC_OTG_DCFG) |= (REBASE(DWC_OTG_DCFG) & ~DWC_OTG_DCFG_DSPD_MASK) |
								DWC_OTG_DCFG_DSPD_FULL_1_1;

	_usbd_dev.backend = &usbd_efm32lg;
	_usbd_dev.config = config;

	dwc_otg_init(&_usbd_dev);

	return &_usbd_dev;
}

const struct usbd_backend usbd_efm32lg = {
	.init = init,
	.set_address = dwc_otg_set_address,
	.get_address = dwc_otg_get_address,
	.ep_prepare_start = dwc_otg_ep_prepare_start,
	.ep_prepare = dwc_otg_ep_prepare,
	.ep_prepare_end = dwc_otg_ep_prepare_end,
	.set_ep_dtog = dwc_otg_set_ep_dtog,
	.get_ep_dtog = dwc_otg_get_ep_dtog,
	.set_ep_stall = dwc_otg_set_ep_stall,
	.get_ep_stall = dwc_otg_get_ep_stall,
	.urb_submit = dwc_otg_urb_submit,
	.urb_cancel = dwc_otg_urb_cancel,
	.poll = dwc_otg_poll,
	.enable_sof = dwc_otg_enable_sof,
	.disconnect = dwc_otg_disconnect,
	.frame_number  = dwc_otg_frame_number,
	.get_speed = dwc_otg_get_speed,
	.set_address_before_status = true,
	.base_address = USB_OTG_BASE,
};
