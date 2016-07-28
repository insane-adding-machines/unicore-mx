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

/*
 * This file is intended to be included by either otg_hs.h or otg_fs.h
 * Contains common definitions of Command and Status Registers (CSR) and their
 * bit definitions.
 */

#ifndef UNICOREMX_USBD_DWC_OTG_PRIVATE_H
#define UNICOREMX_USBD_DWC_OTG_PRIVATE_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usbd/backend/dwc_otg.h>

BEGIN_DECLS

struct dwc_otg_private_data {
	uint32_t base_address;
	uint16_t rx_fifo_size;

	/*
	 * force nak for endpoint out.
	 * bitX = epX
	 */
	uint16_t force_nak;

	/* number of endpoint in the instantiation */
	uint8_t ep_count;

	/*
	 * We keep a backup copy of the out endpoint size registers to restore
	 * them after a transaction.
	 * length: ep_count
	 */
	uint32_t *doeptsiz;

	/*
	 * Received packet size for each endpoint. This is assigned in
	 * dwc_otg_poll() which reads the packet status push register GRXSTSP
	 * for use in dwc_otg_ep_read_packet().
	 */
	uint16_t rxbcnt;

	uint16_t fifo_mem_top;
	uint16_t fifo_mem_top_ep0;
};

void dwc_otg_init(usbd_device *usbd_dev);

void dwc_otg_set_address(usbd_device *usbd_dev, uint8_t addr);
void dwc_otg_ep_setup(usbd_device *usbd_dev, uint8_t addr, uint8_t type,
			uint16_t max_size);
void dwc_otg_endpoints_reset(usbd_device *usbd_dev);
void dwc_otg_set_ep_stall(usbd_device *usbd_dev, uint8_t addr, bool stall);
bool dwc_otg_get_ep_stall(usbd_device *usbd_dev, uint8_t addr);
void dwc_otg_set_ep_nak(usbd_device *usbd_dev, uint8_t addr, bool nak);
uint16_t dwc_otg_ep_write_packet(usbd_device *usbd_dev, uint8_t addr,
				   const void *buf, uint16_t len);
uint16_t dwc_otg_ep_read_packet(usbd_device *usbd_dev, uint8_t addr,
				  void *buf, uint16_t len);
void dwc_otg_poll(usbd_device *usbd_dev);
void dwc_otg_disconnect(usbd_device *usbd_dev, bool disconnected);

void dwc_otg_set_ep_type(usbd_device *usbd_dev, uint8_t addr, uint8_t type);
void dwc_otg_enable_sof(usbd_device *usbd_dev, bool enable);

void dwc_otg_set_ep_size(usbd_device *usbd_dev, uint8_t addr,
			uint16_t max_size);

bool dwc_otg_get_ep_dtog(usbd_device *usbd_dev, uint8_t addr);
void dwc_otg_set_ep_dtog(usbd_device *usbd_dev, uint8_t addr, bool dtog);
void dwc_otg_ep_flush(usbd_device *usbd_dev, uint8_t addr);

uint16_t dwc_otg_frame_number(usbd_device *usbd_dev);

END_DECLS

#endif
