/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2016, 2017 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

#include "dwc_otg-private.h"
#include "../usbh-private.h"
#include <unicore-mx/stm32/otg_fs.h>
#include <unicore-mx/stm32/memorymap.h>
#include <unicore-mx/stm32/rcc.h>

static usbh_host host;

#if !defined(USBH_STM32_OTG_FS_CHANNEL_COUNT)
# define USBH_STM32_OTG_FS_CHANNEL_COUNT 8
#elif !(USBH_STM32_OTG_FS_CHANNEL_COUNT > 0)
# error "Got get some sleep, channel count need to be a greater than 0"
#endif

#define REBASE(REG, ...)	REG(usbh_stm32_otg_fs.base_address, ##__VA_ARGS__)

static usbh_dwc_otg_chan channels[USBH_STM32_OTG_FS_CHANNEL_COUNT];

static const usbh_backend_config _config = {
	.chan_count = USBH_STM32_OTG_FS_CHANNEL_COUNT,
	.priv_mem = 1280, /* 1.25KB * 1024 */
	.speed = USBH_SPEED_FULL,
	.feature = USBH_FEATURE_NONE
};

static usbh_host *init(const usbh_backend_config *config)
{
	rcc_periph_clock_enable(RCC_OTGFS);

	if (config == NULL) {
		config = &_config;
	}

	host.backend = &usbh_stm32_otg_fs;
	host.config = config;

	if (config->feature & USBH_VBUS_SENSE) {
		if (OTG_FS_CID >= 0x00002000) { /* 2.0 */
			/* Enable VBUS detection */
			OTG_FS_GCCFG |= OTG_GCCFG_VBDEN;
		} else { /* 1.x */
			/* Enable VBUS sensing in host mode */
			OTG_FS_GCCFG |= OTG_GCCFG_VBUSASEN;
		}
	} else {
		if (OTG_FS_CID >= 0x00002000) { /* 2.0 */
			/* Disable VBUS detection. */
			OTG_FS_GCCFG &= ~OTG_GCCFG_VBDEN;
			REBASE(DWC_OTG_GOTGCTL) |= DWC_OTG_GOTGCTL_AVALOEN |
											DWC_OTG_GOTGCTL_AVALOVAL;
		} else { /* 1.x */
			/* Disable VBUS sensing in host mode. */
			OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS | OTG_GCCFG_VBUSASEN;
		}
	}

	/* Power up the PHY */
	OTG_FS_GCCFG |= OTG_GCCFG_PWRDWN;

	/* Internal PHY */
	REBASE(DWC_OTG_GUSBCFG) |= DWC_OTG_GUSBCFG_PHYSEL;

	usbh_dwc_otg_init(&host);

	return &host;
}

usbh_backend usbh_stm32_otg_fs = {
	.init = init,
	.poll = usbh_dwc_otg_poll,
	.speed = usbh_dwc_otg_speed,
	.reset = usbh_dwc_otg_reset,
	.transfer_submit = usbh_dwc_otg_transfer_submit,
	.transfer_cancel = usbh_dwc_otg_transfer_cancel,

	.base_address = USB_OTG_FS_BASE,
	.channels_count = USBH_STM32_OTG_FS_CHANNEL_COUNT,
	.channels = channels
};
