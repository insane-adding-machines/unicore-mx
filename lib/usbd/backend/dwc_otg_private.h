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

/*
 * This file is intended to be included by either otg_hs.h or otg_fs.h
 * Contains common definitions of Command and Status Registers (CSR) and their
 * bit definitions.
 */

#ifndef UNICOREMX_USBD_DWC_OTG_PRIVATE_H
#define UNICOREMX_USBD_DWC_OTG_PRIVATE_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/common/dwc_otg.h>

BEGIN_DECLS

/*
 * Hardware quirk
 * ==============
 *
 * The FIFO depth can be found in GHWCFG3 (heigher 16bit).
 * for STM32F407 FS, GHWCFG3 = 0x20001e8
 *     (0x200 = 512 words = 512 * 4 bytes = 2KB)
 *
 * But, ST reference manual (STM32F407, OTG_FS) says the FIFO depth is 1.25KB.
 *
 * When endpoints are used with 2KB layout.
 * It cause communication problem with endpoint.
 * 10bytes were written (for endpoint) to beyond 1.25KB,
 *    host was receiving garbage (mostly same).
 *
 * So, a compile time configuration value was provided to override
 *  the GHWCFG3 FIFO value.
 */

#define USBD_BACKEND_EXTRA													\
	uint32_t base_address;

struct dwc_otg_private_data {
	/* FIFO unused (in terms of 32-bit words).
	 * This field is decremented one every dedicate TX FIFO allocated.
	 *  (ie on every IN endpoint) */
	uint16_t fifo_remaining;

	/* FIFO RX size overall requirement.
	 * 1. incremented to accomodate for control endpoint setup packet
	 * 2. incremented to accomodate for OUT endpoint status
	 *
	 * (control endpoint = OUT + IN endpoint
	 *   so, for control both need to be calculated) */
	uint16_t fifo_rx_usage_overall;

	/* FIFO Largest OUT endpoint that need to be allocated (+ 1)
	 *  (in terms of 32-bit words).
	 * If double buffer flag is passed, everything is multiplied with 2 */
	uint16_t fifo_rx_usage_packet;

	/* The DIEP0TSIZ and DOEP0TSIZ have pktcnt field small.
	 *  so, to compensate, we will the values in these variables.
	 *  and decrement them as use them with the periph.
	 * Note: Instead of making two, kept once because data flow in
	 *   one direction at a time */
	uint16_t ep0tsiz_pktcnt;

	/* FIXME: used for all endpoint setup_data. */
	struct usb_setup_data setup_data;
};

#define USBD_DEVICE_EXTRA												\
	struct dwc_otg_private_data private_data;

//#include "../usbd_private.h"
void dwc_otg_init(usbd_device *dev);

void dwc_otg_set_address(usbd_device *dev, uint8_t addr);
uint8_t dwc_otg_get_address(usbd_device *dev);
void dwc_otg_ep_prepare_start(usbd_device *dev);
void dwc_otg_ep_prepare(usbd_device *dev, uint8_t addr, usbd_ep_type type,
					uint16_t max_size, uint16_t internval,
					usbd_ep_flags flags);
void dwc_otg_ep_prepare_end(usbd_device *dev);
void dwc_otg_set_ep_dtog(usbd_device *dev, uint8_t addr, bool dtog);
bool dwc_otg_get_ep_dtog(usbd_device *dev, uint8_t addr);
void dwc_otg_set_ep_stall(usbd_device *dev, uint8_t addr, bool stall);
bool dwc_otg_get_ep_stall(usbd_device *dev, uint8_t addr);
void dwc_otg_poll(usbd_device *dev);
void dwc_otg_disconnect(usbd_device *dev, bool disconnected);
void dwc_otg_enable_sof(usbd_device *dev, bool enable);
usbd_speed dwc_otg_get_speed(usbd_device *dev);

uint16_t dwc_otg_frame_number(usbd_device *dev);

typedef struct usbd_urb usbd_urb;
void dwc_otg_urb_submit(usbd_device *dev, usbd_urb *urb);
void dwc_otg_urb_cancel(usbd_device *dev, usbd_urb *urb);

END_DECLS

#endif
