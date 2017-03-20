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

#define NUM_OF_CHANNELS 8

static usbh_dwc_otg_chan channels[NUM_OF_CHANNELS];

static usbh_host *init(void)
{
	rcc_periph_clock_enable(RCC_OTGFS);

	host.backend = &usbh_stm32_otg_fs;

	if (OTG_FS_CID >= 0x00002000) { /* 2.0 */
		OTG_FS_GCCFG = OTG_GCCFG_VBDEN | OTG_GCCFG_PWRDWN;
		DWC_OTG_GOTGCTL(USB_OTG_FS_BASE) |= DWC_OTG_GOTGCTL_AVALOEN |
										DWC_OTG_GOTGCTL_AVALOVAL;
	} else { /* 1.x */
		OTG_FS_GCCFG = OTG_GCCFG_VBUSASEN | OTG_GCCFG_PWRDWN;
	}

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

	.fifo_size = {
		.rx = 160,
		.tx_np = 76,
		.tx_p = 76
	},

	.channels_count = NUM_OF_CHANNELS,
	.channels = channels
};
