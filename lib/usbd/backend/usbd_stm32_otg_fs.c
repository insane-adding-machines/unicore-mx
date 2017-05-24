/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2011 Gareth McMullin <gareth@blacksphere.co.nz>
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

#include <string.h>
#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/otg_fs.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/usbd/usbd.h>

static usbd_device *init(const usbd_backend_config *config);
static bool otgfs_is_powered(usbd_device *dev);
static usbd_power_status otgfs_power_control(usbd_device *dev, usbd_power_action action);

static struct usbd_device _usbd_dev;

const struct usbd_backend usbd_stm32_otg_fs = {
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
	.base_address = USB_OTG_FS_BASE,
	  .is_vbus        = otgfs_is_powered
	, .power_control  = otgfs_power_control
};

#define REBASE(REG, ...)	REG(usbd_stm32_otg_fs.base_address, ##__VA_ARGS__)

static const usbd_backend_config _config = {
	.ep_count = 6,
	.priv_mem = 1280, /* 1.25KB * 1024 */
	.speed = USBD_SPEED_FULL,
	.feature = USBD_FEATURE_NONE
};

/** Initialize the USB device controller hardware of the STM32. */
static usbd_device *init(const usbd_backend_config *config)
{
	rcc_periph_clock_enable(RCC_OTGFS);

	if (config == NULL) {
		config = &_config;
	}

	_usbd_dev.backend = &usbd_stm32_otg_fs;
	_usbd_dev.config = config;

	//* stm32f4 use FS CID=0x1100, HS CID=0x02000600
	//* stm32f7            0x3000,    CID=0x00003100
	const unsigned otg_hs_cid_boundary = 0x3100;

	if (config->feature & USBD_VBUS_SENSE) {
		if (OTG_FS_CID >= otg_hs_cid_boundary) { /* 2.0 HS core*/
			/* Enable VBUS detection */
			OTG_FS_GCCFG |= OTG_GCCFG_VBDEN;
		} else { /* 1.x  FS core*/
			/* Enable VBUS sensing in device mode */
			OTG_FS_GCCFG |= OTG_GCCFG_VBUSBSEN;
		}
	} else {
		if (OTG_FS_CID >= otg_hs_cid_boundary) { /* 2.0 */
			/* Disable VBUS detection. */
			OTG_FS_GCCFG &= ~OTG_GCCFG_VBDEN;
			REBASE(DWC_OTG_GOTGCTL) |= DWC_OTG_GOTGCTL_BVALOEN |
											DWC_OTG_GOTGCTL_BVALOVAL;
		} else { /* 1.x */
			/* Disable VBUS sensing in device mode. */
			OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS | OTG_GCCFG_VBUSBSEN;
		}
	}

	/* Power up the PHY */
	OTG_FS_GCCFG |= OTG_GCCFG_PWRDWN;

	/* Internal PHY */
	REBASE(DWC_OTG_GUSBCFG) |= DWC_OTG_GUSBCFG_PHYSEL;

	/* Full speed device. */
	REBASE(DWC_OTG_DCFG) |= (REBASE(DWC_OTG_DCFG) & ~DWC_OTG_DCFG_DSPD_MASK) |
								DWC_OTG_DCFG_DSPD_FULL_1_1;

	dwc_otg_init(&_usbd_dev);

	return &_usbd_dev;
}

static
bool otgfs_is_powered(usbd_device *dev){
	uint32_t base = dev->backend->base_address;
	return (DWC_OTG_GOTGCTL(base) & DWC_OTG_GOTGCTL_BSVLD) != 0;
}

static
usbd_power_status otgfs_power_control(usbd_device *dev, usbd_power_action action){
	uint32_t base = dev->backend->base_address;
	switch (action){
		case usbd_paActivate:{
			DWC_OTG_PCGCCTL(base) = 0;
			OTG_FS_GCCFG |= OTG_GCCFG_PWRDWN;
			REBASE(DWC_OTG_GINTMSK) |= DWC_OTG_GINTMSK_ENUMDNEM | DWC_OTG_GINTSTS_USBSUSP
			                        | DWC_OTG_GINTMSK_RXFLVLM | DWC_OTG_GINTMSK_IEPINT ;
			break;
		}
		case usbd_paShutdown: {
		/* Wait for AHB idle. */
			while (( (DWC_OTG_GRSTCTL(base) & DWC_OTG_GRSTCTL_AHBIDL) == 0) );
			//* drop ISRs, cause stoped Core cant handle them
			REBASE(DWC_OTG_GINTSTS) = ~( DWC_OTG_GINTSTS_USBSUSP
			                           | DWC_OTG_GINTSTS_WKUPINT
			                           | DWC_OTG_GINTSTS_SRQINT
			                           );
			REBASE(DWC_OTG_GINTMSK) &= ~( DWC_OTG_GINTMSK_ENUMDNEM 
			                             | DWC_OTG_GINTMSK_RXFLVLM | DWC_OTG_GINTMSK_IEPINT
			                            );
			//poser down PHY
			OTG_FS_GCCFG &= ~OTG_GCCFG_PWRDWN;
			DWC_OTG_PCGCCTL(base) = DWC_OTG_PCGCCTL_STPPCLK; //| DWC_OTG_PCGCCTL_GATEHCLK ;
			break;
		}
	}
	usbd_power_status res = 0;
	if ( (DWC_OTG_PCGCCTL(base) & DWC_OTG_PCGCCTL_PHYSUSP) == 0)
		res |= usbd_psPHYOn;
	if ((OTG_FS_GCCFG & OTG_GCCFG_PWRDWN) != 0)
		res |= usbd_psCoreEnabled;
	return res;
}
