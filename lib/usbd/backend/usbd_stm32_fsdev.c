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

#define private_data ((struct stm32_fsdev_private_data *) (usbd_dev->backend_data))

void stm32_fsdev_init(usbd_device *usbd_dev)
{
	USB_CNTR = 0;
	USB_BTABLE = 0;
	USB_ISTR = 0;

	/* Enable RESET, SUSPEND, RESUME and CTR interrupts. */
	USB_CNTR = USB_CNTR_RESETM | USB_CNTR_CTRM | USB_CNTR_SUSPM | USB_CNTR_WKUPM;

	/* initalize private data */
	private_data->pm_used = 0;
	private_data->force_nak = 0;
}

void stm32_fsdev_set_address(usbd_device *usbd_dev, uint8_t addr)
{
	(void)usbd_dev;
	/* Set device address and enable. */
	USB_DADDR = (addr & USB_DADDR_ADDR) | USB_DADDR_EF;
}

/**
 * Set the receive buffer size for a given USB endpoint.
 *
 * @param ep Index of endpoint to configure.
 * @param[in,out] size Size in bytes of the RX buffer (requested).
 *     Write back the number of memory actually allocated.
 *     Legal sizes : {2,4,6...62}; {64,96,128...992}.
 * @returns (uint16) Count register value
 */
static uint16_t stm32_fsdev_calc_ep_rx_bufsize(uint16_t *ptr_size)
{
	uint16_t size = *ptr_size;
	uint16_t alloc_size;
	/*
	 * Set Block size and number of blocks to hold buffer of the indicated size
	 * There are a maximum of 31 blocks allocated to a buffer, if BL_SIZE is 0
	 * those blocks are 2 bytes long, if BL_SIZE is 1 they are 32 bytes long.
	 * For buffer sizes < 63 bytes you can use 2 byte blocks, for buffer sizes > 62
	 * bytes you have to use 32 byte blocks.
	 *
	 * So for a buffer size < 63 bytes:
	 *    block_size = 2
	 *     number of blocks is (buf_size + 1) / 2 (round up to nearest number
	 *     of blocks
	 * For a buffer size > 62:
	 *    block_size = 32
	 *    number of blocks is (buf_size + 31) / 32 (round up to nearest integer
	 *     number of blocks.
	 */
	/*
	 * USB_COUNTn_RX reg fields : bits <14:10> are NUM_BLOCK; bit 15 is BL_SIZE
	 * - When (size <= 62), BL_SIZE is set to 0 and NUM_BLOCK set to (size / 2).
	 * - When (size > 62), BL_SIZE is set to 1 and NUM_BLOCK=((size / 32) - 1).
	 *
	 * This algo rounds to the next largest legal buffer size, except 0. Examples:
	 *	size =>	BL_SIZE, NUM_BLOCK	=> Actual bufsize
	 *	0		0		0			??? "Not allowed" according to RM0091, RM0008
	 *	1		0		1			2
	 *	61		0		31			62
	 *	63		1		1			64
	 */
	if (size > 62) {
		/* Round up, div by 32 and sub 1 == (size + 31)/32 - 1 == (size-1)/32)*/
		size = ((size - 1) >> 5) & 0x1F;
		alloc_size = (size + 1) << 5;
		/* Set BL_SIZE bit (no macro for this) */
		size |= (1<<5);
	} else {
		/* round up and div by 2 */
		size = (size + 1) >> 1;
		alloc_size = size << 1;
	}

	/* write back the actual memory allocated */
	*ptr_size = alloc_size;

	/* return the BL_SIZE and NUM_BLOCK fields */
	return size << 10;
}

void stm32_fsdev_ep_setup(usbd_device *usbd_dev, uint8_t addr,
			uint8_t type, uint16_t max_size)
{
	/* Translate USB standard type codes to STM32. */
	const uint16_t typelookup[] = {
		[USB_ENDPOINT_ATTR_CONTROL] = USB_EP_TYPE_CONTROL,
		[USB_ENDPOINT_ATTR_ISOCHRONOUS] = USB_EP_TYPE_ISO,
		[USB_ENDPOINT_ATTR_BULK] = USB_EP_TYPE_BULK,
		[USB_ENDPOINT_ATTR_INTERRUPT] = USB_EP_TYPE_INTERRUPT,
	};
	uint8_t dir = addr & 0x80;
	addr &= 0x7f;

	/* Assign address. */
	USB_SET_EP_ADDR(addr, addr);
	USB_SET_EP_TYPE(addr, typelookup[type]);

	/* note: for TX, their is no method to store max_size.
	 *   it is assumed that application will never send buffer
	 *    larger than max_size of TX.
	 *   so, usb_f103 do not have any internal checking for buffer write */

	if (dir || (addr == 0)) {
		/* convert max_size multiple of 2 (up) */
		if (max_size & 1) {
			max_size += 1;
		}

		USB_SET_EP_TX_ADDR(addr, private_data->pm_used);
		USB_CLR_EP_TX_DTOG(addr);
		USB_SET_EP_TX_STAT(addr, USB_EP_TX_STAT_NAK);
		private_data->pm_used += max_size;
	}

	if (!dir) {
		uint16_t count_reg = stm32_fsdev_calc_ep_rx_bufsize(&max_size);
		USB_SET_EP_RX_ADDR(addr, private_data->pm_used);
		USB_SET_EP_RX_COUNT(addr, count_reg);
		USB_CLR_EP_RX_DTOG(addr);
		USB_SET_EP_RX_STAT(addr, USB_EP_RX_STAT_VALID);
		private_data->pm_used += max_size;
	}
}

void stm32_fsdev_set_ep_type(usbd_device *usbd_dev, uint8_t addr,
				uint8_t type)
{
	(void)usbd_dev;

	addr &= 0x7F;

	/* Translate USB standard type codes to STM32. */
	const uint16_t typelookup[] = {
		[USB_ENDPOINT_ATTR_CONTROL] = USB_EP_TYPE_CONTROL,
		[USB_ENDPOINT_ATTR_ISOCHRONOUS] = USB_EP_TYPE_ISO,
		[USB_ENDPOINT_ATTR_BULK] = USB_EP_TYPE_BULK,
		[USB_ENDPOINT_ATTR_INTERRUPT] = USB_EP_TYPE_INTERRUPT,
	};

	USB_SET_EP_TYPE(addr, typelookup[type]);
}

void stm32_fsdev_set_ep_size(usbd_device *usbd_dev, uint8_t addr,
			       uint16_t max_size)
{
	(void)usbd_dev;

	uint8_t dir = addr & 0x80;
	addr &= 0x7f;

	/* note: for TX, their is no method to store max_size.
	 *   it is assumed that application will never send buffer
	 *    larger than max_size of TX.
	 *   so, usb_f103 do not have any internal checking for buffer write */

	if (!dir) {
		USB_SET_EP_RX_COUNT(addr, stm32_fsdev_calc_ep_rx_bufsize(&max_size));
	}
}

void stm32_fsdev_endpoints_reset(usbd_device *usbd_dev)
{
	int i;

	/* Reset all endpoints. */
	for (i = 1; i < private_data->ep_count; i++) {
		USB_SET_EP_TX_STAT(i, USB_EP_TX_STAT_DISABLED);
		USB_SET_EP_RX_STAT(i, USB_EP_RX_STAT_DISABLED);
	}

	/* 1 endpoint (IN+OUT  or  Double buffer) require 8byte */
	private_data->pm_used =
					(private_data->ep_count * 8) +
					(2 * usbd_dev->desc->bMaxPacketSize0);
}

void stm32_fsdev_set_ep_stall(usbd_device *usbd_dev, uint8_t addr,
				   bool stall)
{
	(void)usbd_dev;
	if (addr == 0) {
		USB_SET_EP_TX_STAT(addr, stall ? USB_EP_TX_STAT_STALL :
				   USB_EP_TX_STAT_NAK);
	}

	if (addr & 0x80) {
		addr &= 0x7F;

		USB_SET_EP_TX_STAT(addr, stall ? USB_EP_TX_STAT_STALL :
				   USB_EP_TX_STAT_NAK);

		/* Reset to DATA0 if clearing stall condition. */
		if (!stall) {
			USB_CLR_EP_TX_DTOG(addr);
		}
	} else {
		/* Reset to DATA0 if clearing stall condition. */
		if (!stall) {
			USB_CLR_EP_RX_DTOG(addr);
		}

		USB_SET_EP_RX_STAT(addr, stall ? USB_EP_RX_STAT_STALL :
				   USB_EP_RX_STAT_VALID);
	}
}

bool stm32_fsdev_get_ep_stall(usbd_device *usbd_dev, uint8_t addr)
{
	(void)usbd_dev;
	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	if (dir) {
		return !!(USB_EP(addr) & USB_EP_TX_STAT);
	} else {
		return !!(USB_EP(addr) & USB_EP_RX_STAT);
	}
}

void stm32_fsdev_set_ep_nak(usbd_device *usbd_dev, uint8_t addr, bool nak)
{
	(void)usbd_dev;
	/* It does not make sence to force NAK on IN endpoints. */
	if (addr & 0x80) {
		return;
	}

	if (nak) {
		private_data->force_nak |= 1 << addr;
	} else {
		private_data->force_nak &= ~(1 << addr);
	}

	if (nak) {
		USB_SET_EP_RX_STAT(addr, USB_EP_RX_STAT_NAK);
	} else {
		USB_SET_EP_RX_STAT(addr, USB_EP_RX_STAT_VALID);
	}
}

uint16_t stm32_fsdev_ep_write_packet(usbd_device *usbd_dev, uint8_t addr,
				     const void *buf, uint16_t len)
{
	(void)usbd_dev;
	addr &= 0x7F;

	if ((USB_EP(addr) & USB_EP_TX_STAT) == USB_EP_TX_STAT_VALID) {
		return 0;
	}

	private_data->copy_to_pm(USB_GET_EP_TX_BUFF(addr), buf, len);
	USB_SET_EP_TX_COUNT(addr, len);
	USB_SET_EP_TX_STAT(addr, USB_EP_TX_STAT_VALID);

	return len;
}

uint16_t stm32_fsdev_ep_read_packet(usbd_device *usbd_dev, uint8_t addr,
					 void *buf, uint16_t len)
{
	(void)usbd_dev;
	if ((USB_EP(addr) & USB_EP_RX_STAT) == USB_EP_RX_STAT_VALID) {
		return 0;
	}

	len = MIN(USB_GET_EP_RX_COUNT(addr) & 0x3ff, len);
	private_data->copy_from_pm(buf, USB_GET_EP_RX_BUFF(addr), len);

	if (!(private_data->force_nak & (1 << addr))) {
		USB_SET_EP_RX_STAT(addr, USB_EP_RX_STAT_VALID);
	}

	return len;
}

void stm32_fsdev_poll(usbd_device *usbd_dev)
{
	uint16_t istr = USB_ISTR;

	/* process endpoint related interrupt */
	while(istr & USB_ISTR_CTR) {
		uint8_t addr = istr & USB_ISTR_EP_ID;
		uint8_t type;

		if (istr & USB_ISTR_DIR) {
			/* OUT or SETUP? */
			type = (USB_EP(addr) & USB_EP_SETUP) ?
						USB_CALLBACK_SETUP : USB_CALLBACK_OUT;
			USB_CLR_EP_RX_CTR(addr);
		} else {
			type = USB_CALLBACK_IN;
			USB_CLR_EP_TX_CTR(addr);
		}

		USBD_INVOKE_EP_CALLBACK(usbd_dev, type, addr)

		/* reload ISTR */
		istr = USB_ISTR;
	}

	if (istr & USB_ISTR_RESET) {
		istr &= ~USB_ISTR_RESET;
		private_data->pm_used = private_data->ep_count * 8;
		USBD_INVOKE_RESET_CALLBACK(usbd_dev)
		goto done;
	}

	if (istr & USB_ISTR_SUSP) {
		istr &= ~USB_ISTR_SUSP;
		USBD_INVOKE_SUSPEND_CALLBACK(usbd_dev)
	}

	if (istr & USB_ISTR_WKUP) {
		istr &= ~USB_ISTR_WKUP;
		USBD_INVOKE_RESUME_CALLBACK(usbd_dev)
	}

	if (istr & USB_ISTR_SOF) {
		istr &= ~USB_ISTR_SOF;
		USBD_INVOKE_SOF_CALLBACK(usbd_dev)
	}

	done:
	USB_ISTR = istr;
}

void stm32_fsdev_enable_sof(usbd_device *usbd_dev, bool enable)
{
	(void)usbd_dev;
	uint32_t reg32 = USB_CNTR;
	if (enable) {
		reg32 |= USB_CNTR_SOFM;
	} else {
		reg32 &= ~USB_CNTR_SOFM;
	}
	USB_CNTR = reg32;
}

void stm32_fsdev_ep_flush(usbd_device *usbd_dev, uint8_t addr)
{
	(void)usbd_dev;

	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	if (dir) {
		USB_CLR_EP_TX_CTR(addr);
		USB_SET_EP_TX_STAT(addr, USB_EP_TX_STAT_NAK);
	} else {
		USB_CLR_EP_RX_CTR(addr);
		uint16_t rx_stat = (private_data->force_nak & (1 << addr)) ?
								USB_EP_RX_STAT_NAK : USB_EP_RX_STAT_VALID;
		USB_SET_EP_RX_STAT(addr, rx_stat);
	}
}

void stm32_fsdev_set_ep_dtog(usbd_device *usbd_dev, uint8_t addr, bool dtog)
{
	(void)usbd_dev;
	uint16_t dtog_mask = (addr & 0x80) ? USB_EP_TX_DTOG : USB_EP_RX_DTOG;
	addr &= 0x7F;
	uint16_t reg16 = USB_EP(addr);
	bool dtog_cur = !!(reg16 & dtog_mask);
	if (dtog_cur != dtog) {
		USB_EP(addr) = (reg16 & USB_EP_NTOGGLE_MSK) | dtog_mask;
	}
}

bool stm32_fsdev_get_ep_dtog(usbd_device *usbd_dev, uint8_t addr)
{
	(void)usbd_dev;
	uint16_t dtog_mask = (addr & 0x80) ? USB_EP_TX_DTOG : USB_EP_RX_DTOG;
	addr &= 0x7F;
	return !!(USB_EP(addr) & dtog_mask);
}

uint16_t stm32_fsdev_frame_number(usbd_device *usbd_dev)
{
	(void)usbd_dev;
	return USB_FNR & 0x7FF;
}
