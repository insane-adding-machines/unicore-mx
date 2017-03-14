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

static inline void ep_set_stat(uint8_t num, bool rx, uint16_t stat);
static inline void ep_clear_ctr(uint8_t num, bool rx);
static inline void ep_set_type(uint8_t num, uint16_t eptype);

/**
 * Set the endpoint @a num status to @a status
 * @param num Endpoint number (not including direction)
 * @param rx true for RX, false for TX
 * @param stat Status
 */
static inline void ep_set_stat(uint8_t num, bool rx, uint16_t stat)
{
	uint16_t ep = USB_EP(num);
	ep &= USB_EP_TYPE_MASK | USB_EP_KIND | USB_EP_EA_MASK |
				(rx ? USB_EP_STAT_RX_MASK : USB_EP_STAT_TX_MASK);
	ep ^= stat;
	ep |= USB_EP_CTR_RX | USB_EP_CTR_TX;
	USB_EP(num) = ep;
}

/**
 * Clear the endpoint CTR bit
 * @param num Endpoint number (not including direction)
 * @param rx true for CTR_RX, false for CTR_TX
 */
static inline void ep_clear_ctr(uint8_t num, bool rx)
{
	uint16_t ep = USB_EP(num);
	ep &= USB_EP_TYPE_MASK | USB_EP_KIND | USB_EP_EA_MASK;
	ep |= (rx ? USB_EP_CTR_TX : USB_EP_CTR_RX);
	USB_EP(num) = ep;
}

/**
 * Set the endpoint @a num type to @a eptype
 * @param num Endpoint number (not including direction)
 * @param eptype New endpoint type
 */
static inline void ep_set_type(uint8_t num, uint16_t eptype)
{
	uint16_t ep = USB_EP(num);
	ep &= USB_EP_KIND | USB_EP_EA_MASK;
	ep |= USB_EP_CTR_RX | USB_EP_CTR_TX | eptype;
	USB_EP(num) = ep;
}

void stm32_fsdev_init(usbd_device *dev)
{
	USB_BTABLE = 0;
	USB_ISTR = 0;

	/* Enable RESET, SUSPEND, RESUME and CTR interrupts. */
	USB_CNTR = USB_CNTR_RESETM | USB_CNTR_CTRM | USB_CNTR_SUSPM | USB_CNTR_WKUPM;

	dev->private_data.pma_used = 0;

	LOGF_LN("endpoint count: %"PRIu8, dev->config->ep_count);
}

void stm32_fsdev_set_address(usbd_device *dev, uint8_t addr)
{
	(void) dev;

	LOGF_LN("New device address  = %"PRIu8, addr);

	/* Set device address and enable. */
	USB_DADDR = USB_DADDR_ADD(addr) | USB_DADDR_EF;
}

uint8_t stm32_fsdev_get_address(usbd_device *dev)
{
	(void) dev;

	return USB_DADDR_ADD_GET(USB_DADDR);
}

/**
 * Disable all non 0 endpoint
 * @param dev USB Device
 */
static void disable_non_ep0(usbd_device *dev)
{
	unsigned  i;

	for (i = 1; i < dev->config->ep_count; i++) {
		USB_EP(i) = USB_EP(i) & (USB_EP_TYPE_MASK | USB_EP_EA_MASK |
						USB_EP_STAT_RX_MASK | USB_EP_STAT_TX_MASK);
	}
}

/**
 * Calculate the endpoint 0 COUNT_RX register value from @a ep0_size
 * @param ep0_size Endpoint 0 size
 * @return COUNT_RX register
 */
static uint16_t ep0_count_rx(uint16_t ep0_size)
{
	if (ep0_size >= 64) {
		return USB_EP_COUNT_RX_BL_SIZE | USB_EP_COUNT_RX_NUM_BLOCK(1);
	} else if (ep0_size >= 32) {
		return USB_EP_COUNT_RX_NUM_BLOCK(16);
	} else if (ep0_size >= 16) {
		return USB_EP_COUNT_RX_NUM_BLOCK(8);
	} else {
		return USB_EP_COUNT_RX_NUM_BLOCK(4);
	}
}

/**
 * Allocate buffer to Endpoint 0
 * @param dev USB Device
 */
static void alloc_ep0_buf(usbd_device *dev)
{
	/* 1 endpoint (IN+OUT  or  Double buffer) require 8byte */
	dev->private_data.pma_used = dev->config->ep_count * 8;

	/* ADDR=0, type=CONTROL, STATUS_OUT=0
	 * CTR_RX=0, STAT_RX=NAK, DTOG_RX=1
	 * CTR_TX=0, STAT_TX=NAK, DTOG_TX=1
	 */
	uint16_t reg16 = USB_EP(0) & (USB_EP_STAT_RX_MASK | USB_EP_STAT_TX_MASK |
									USB_EP_DTOG_RX | USB_EP_DTOG_TX);
	reg16 ^= USB_EP_STAT_RX_NAK | USB_EP_STAT_TX_NAK |
				USB_EP_DTOG_RX | USB_EP_DTOG_TX;
	USB_EP(0) = reg16 | USB_EP_TYPE_CONTROL;

	USB_EP_ADDR_TX(0) = dev->private_data.pma_used;
	dev->private_data.pma_used += dev->desc->bMaxPacketSize0;

	USB_EP_ADDR_RX(0) = dev->private_data.pma_used;
	dev->private_data.pma_used += dev->desc->bMaxPacketSize0;

	USB_EP_COUNT_RX(0) = ep0_count_rx(dev->desc->bMaxPacketSize0);
}

/**
 * Set the receive buffer size for a given USB endpoint.
 *
 * @param[in] ep Index of endpoint to configure.
 * @param[in,out] size Size in bytes of the RX buffer (requested).
 *     Write back the number of memory actually allocated.
 *     Legal sizes : {2,4,6...62}; {64,96,128...992}.
 * @returns (uint16) COUNT_RX register value
 */
static uint16_t calc_ep_count_rx(uint16_t *size)
{
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
	uint16_t size_value = *size, reg;
	if (size_value > 62) { /* Course */
		uint16_t blocks_32byte = DIVIDE_AND_CEIL(size_value, 32) & 0x1F;
		size_value = blocks_32byte * 32;
		reg = USB_EP_COUNT_RX_BL_SIZE |
				USB_EP_COUNT_RX_NUM_BLOCK(blocks_32byte - 1);
	} else { /* Fine */
		uint16_t blocks_2byte = DIVIDE_AND_CEIL(size_value, 2) & 0x1F;
		size_value = blocks_2byte * 2;
		reg = USB_EP_COUNT_RX_NUM_BLOCK(blocks_2byte);
	}

	*size = size_value;
	return reg;
}

void stm32_fsdev_ep_prepare_start(usbd_device *dev)
{
	disable_non_ep0(dev);
	alloc_ep0_buf(dev);
}

static const uint16_t eptype_map[] = {
	[USBD_EP_CONTROL] = USB_EP_TYPE_CONTROL,
	[USBD_EP_ISOCHRONOUS] = USB_EP_TYPE_ISO,
	[USBD_EP_BULK] = USB_EP_TYPE_BULK,
	[USBD_EP_INTERRUPT] = USB_EP_TYPE_INTERRUPT
};

void stm32_fsdev_ep_prepare(usbd_device *dev, uint8_t addr, usbd_ep_type type,
				uint16_t max_size, uint16_t interval, usbd_ep_flags flags)
{
	LOG_CALL

	(void) interval;
	(void) flags;

	uint8_t num = ENDPOINT_NUMBER(addr);
	uint16_t reg16 = USB_EP(num) & ~(USB_EP_EA_MASK | USB_EP_TYPE_MASK);

	if (IS_IN_ENDPOINT(addr)) {
		reg16 &= ~(USB_EP_STAT_RX_MASK | USB_EP_CTR_TX);
		reg16 |= USB_EP_CTR_RX;
		reg16 ^= USB_EP_STAT_TX_NAK;
	} else {
		reg16 &= ~(USB_EP_STAT_TX_MASK | USB_EP_CTR_RX);
		reg16 |= USB_EP_CTR_TX;
		reg16 ^= USB_EP_STAT_RX_NAK;
	}

	USB_EP(num) = reg16 | eptype_map[type] | num;

	if (IS_IN_ENDPOINT(addr)) {
		/* convert max_size multiple of 2 (up) */
		if (max_size & 1) {
			max_size += 1;
		}

		USB_EP_ADDR_TX(num) = dev->private_data.pma_used;
	} else {
		USB_EP_ADDR_RX(num) = dev->private_data.pma_used;
		USB_EP_COUNT_RX(num) = calc_ep_count_rx(&max_size);
	}

	dev->private_data.pma_used += max_size;

	if (dev->private_data.pma_used > dev->config->priv_mem) {
		LOG_LN(">>> WARNING: PMA overflow. "
			"Application is requesting more memory than available!");
	}
}

void stm32_fsdev_set_ep_stall(usbd_device *dev, uint8_t addr, bool stall)
{
	(void) dev;

	uint8_t num = ENDPOINT_NUMBER(addr);
	uint8_t stat_index; /* 0 = STALL, 1 = NAK, 2 = VALID */

	if (stall) {
		/* STALL the endpoint */
		stat_index = 0;
	} else {
		if (is_ep_free(dev, addr)) {
			/* No transfer in process */
			stat_index = 1;
		} else {
			/* Transfer in process */
			stat_index = 2;
		}
	}

	if (IS_IN_ENDPOINT(addr)) {
		static const uint16_t stat_list[3] = {
			USB_EP_STAT_TX_STALL,
			USB_EP_STAT_TX_NAK,
			USB_EP_STAT_TX_VALID
		};

		ep_set_stat(num, false, stat_list[stat_index]);
	} else {
		static const uint16_t stat_list[3] = {
			USB_EP_STAT_RX_STALL,
			USB_EP_STAT_RX_NAK,
			USB_EP_STAT_RX_VALID
		};

		ep_set_stat(num, true, stat_list[stat_index]);
	}
}

bool stm32_fsdev_get_ep_stall(usbd_device *dev, uint8_t addr)
{
	(void) dev;

	uint8_t num = ENDPOINT_NUMBER(addr);

	if (IS_IN_ENDPOINT(addr)) {
		return (USB_EP(num) & USB_EP_STAT_TX_MASK) == USB_EP_STAT_TX_STALL;
	} else {
		return (USB_EP(num) & USB_EP_STAT_RX_MASK) == USB_EP_STAT_RX_STALL;
	}
}

void stm32_fsdev_set_ep_dtog(usbd_device *dev, uint8_t addr, bool dtog)
{
	(void) dev;

	uint8_t num = ENDPOINT_NUMBER(addr);
	uint16_t dtog_mask = IS_IN_ENDPOINT(addr) ? USB_EP_DTOG_TX : USB_EP_DTOG_RX;
	uint16_t reg16 = USB_EP(num);
	bool dtog_cur = !!(reg16 & dtog_mask);
	if (dtog_cur != dtog) {
		reg16 &= USB_EP_EA_MASK | USB_EP_KIND | USB_EP_TYPE_MASK;
		reg16 |= USB_EP_CTR_RX | USB_EP_CTR_TX;
		USB_EP(num) = reg16 | dtog_mask;
	}
}

bool stm32_fsdev_get_ep_dtog(usbd_device *dev, uint8_t addr)
{
	(void) dev;

	uint8_t num = ENDPOINT_NUMBER(addr);
	uint16_t dtog_mask = IS_IN_ENDPOINT(addr) ? USB_EP_DTOG_TX : USB_EP_DTOG_RX;
	return !!(USB_EP(num) & dtog_mask);
}

/**
 * Process endpoint SETUP interrupt
 * @param dev USB Device
 * @param num Endpoint number
 */
static void process_setup_interrupt(usbd_device *dev, uint8_t num)
{
	LOG_CALL

	/* Clear CTR_RX and STATUS_OUT bit */
	uint16_t reg16 = USB_EP(num);
	reg16 &= USB_EP_EA_MASK | USB_EP_TYPE_MASK;
	reg16 |= USB_EP_CTR_TX;
	USB_EP(num) = reg16;

	struct usb_setup_data setup_data __attribute__((aligned(2)));

	uint16_t len = USB_EP_COUNT_RX_COUNT_GET(USB_EP_COUNT_RX(num));

	if (len != 8) {
		LOGF("SETUP packet not equal to length 8");
	}

	dev->backend->copy_from_pm(&setup_data, USB_EP_RX_PMA_BUF(num), 8);

	LOGF_LN("bmRequestType: 0x%02"PRIx8, setup_data.bmRequestType);
	LOGF_LN("bRequest: 0x%02"PRIx8, setup_data.bRequest);
	LOGF_LN("wValue: 0x%04"PRIx16, setup_data.wValue);
	LOGF_LN("wIndex: 0x%04"PRIx16, setup_data.wIndex);
	LOGF_LN("wLength: %"PRIu16, setup_data.wLength);

	usbd_handle_setup(dev, num, &setup_data);
}

/**
 * Perform URB complete by releasing the endpoint
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param[in] status Transfer status
 */
static void perform_urb_complete(usbd_device *dev, usbd_urb *urb,
		usbd_transfer_status status)
{
	LOG_CALL

	usbd_transfer *transfer = &urb->transfer;
	uint8_t num = ENDPOINT_NUMBER(transfer->ep_addr);

	if (transfer->ep_type == USBD_EP_CONTROL) {
		/* Clear STATUS_OUT bit */
		uint16_t reg16 = USB_EP(num);
		reg16 &= USB_EP_EA_MASK | USB_EP_TYPE_MASK;
		reg16 |= USB_EP_CTR_TX | USB_EP_CTR_RX;
		USB_EP(num) = reg16;
	}

	if (IS_IN_ENDPOINT(transfer->ep_addr)) {
		if ((USB_EP(num) & USB_EP_STAT_TX_MASK) != USB_EP_STAT_TX_STALL) {
			ep_set_stat(num, false, USB_EP_STAT_TX_NAK);
		}
	} else {
		if ((USB_EP(num) & USB_EP_STAT_RX_MASK) != USB_EP_STAT_RX_STALL) {
			ep_set_stat(num, true, USB_EP_STAT_RX_NAK);
		}
	}

	usbd_urb_complete(dev, urb, status);
}

/**
 * Process endpoint OUT interrupt
 * @param dev USB Device
 * @param num Endpoint number
 */
static void process_out_interrupt(usbd_device *dev, uint8_t num)
{
	LOG_CALL

	uint16_t len = USB_EP_COUNT_RX_COUNT_GET(USB_EP_COUNT_RX(num));

	ep_clear_ctr(num, true);

	usbd_urb *urb = usbd_find_active_urb(dev, num | 0x00);
	if (urb == NULL) {
		return;
	}

	usbd_transfer *transfer = &urb->transfer;

	if (len > transfer->ep_size) {
		/* Packet with data more than endpoint size */
		perform_urb_complete(dev, urb, USBD_ERR_BABBLE);
		return;
	}

	/* Copy data from PMA to memory */
	size_t space_avail = transfer->length - transfer->transferred;
	size_t storable_len = MIN(len, space_avail);

	if (storable_len) {
		void *buffer = usbd_urb_get_buffer_pointer(dev, urb, storable_len);
		dev->backend->copy_from_pm(buffer, USB_EP_RX_PMA_BUF(num), storable_len);
		usbd_urb_inc_data_pointer(dev, urb, storable_len);
	}

	if (len > space_avail) {
		LOGF_LN("WARN: At maximum could accomodate %u bytes but host has"
			" sent %"PRIu16" bytes", space_avail, len);
		perform_urb_complete(dev, urb, USBD_ERR_OVERFLOW);
		return;
	}

	if (len < transfer->ep_size) {
		if (transfer->ep_type == USBD_EP_BULK) {
			LOGF_LN("Short packet received for Bulk endpoint 0x%"PRIx8,
						transfer->ep_addr);

			if (transfer->flags & USBD_FLAG_SHORT_PACKET) {
				/* Short packet received (usually marker of end of transfer) */
				perform_urb_complete(dev, urb, USBD_SUCCESS);
				return;
			} else if (transfer->flags & USBD_FLAG_NO_SHORT_PACKET) {
				/* Short packet received when it flagged
				 *  that short packet will cause transfer failure */
				perform_urb_complete(dev, urb, USBD_ERR_SHORT_PACKET);
				return;
			}
		}
	}

	/* All data has been transferred */
	if (transfer->transferred >= transfer->length) {
		perform_urb_complete(dev, urb, USBD_SUCCESS);
		return;
	}

	/* More data! */
	ep_set_stat(num, true, USB_EP_STAT_RX_VALID);
}

/**
 * Process endpoint IN interrupt
 * @param dev USB Device
 * @param num Endpoint number
 */
static void process_in_interrupt(usbd_device *dev, uint8_t num)
{
	LOG_CALL

	ep_clear_ctr(num, false);

	usbd_urb *urb = usbd_find_active_urb(dev, num | 0x80);
	if (urb == NULL) {
		return;
	}

	usbd_transfer *transfer = &urb->transfer;
	uint16_t old_len = USB_EP_COUNT_TX(num) & 0x3FF;
	usbd_urb_inc_data_pointer(dev, urb, old_len);
	size_t rem = transfer->length - transfer->transferred;

	if (!rem) {
		/* Send a zero length packet if all condition are met.
		 *  - control or bulk endpoint
		 *  - short flag set
		 *  - last packet sent was equal to endpoint size
		 */
		if (transfer->flags & USBD_FLAG_SHORT_PACKET) {
			switch (transfer->ep_type) {
			case USBD_EP_BULK:
			case USBD_EP_CONTROL:
				if (old_len < transfer->ep_size) {
					break;
				}

				USB_EP_COUNT_TX(num) = 0;

				if ((USB_EP(num) & USB_EP_STAT_TX_MASK) == USB_EP_STAT_TX_STALL) {
					if (transfer->ep_type != USBD_EP_CONTROL) {
						return;
					}
				}

				ep_set_stat(num, false, USB_EP_STAT_TX_VALID);
			return;
			default:
			break;
			}
		}

		perform_urb_complete(dev, urb, USBD_SUCCESS);
		return;
	}

	/* sending more data */
	size_t len = MIN(rem, transfer->ep_size);
	void *buffer = usbd_urb_get_buffer_pointer(dev, urb, len);
	dev->backend->copy_to_pm(USB_EP_TX_PMA_BUF(num), buffer, len);

	USB_EP_COUNT_TX(num) = len & 0x3FF;
	if ((USB_EP(num) & USB_EP_STAT_TX_MASK) != USB_EP_STAT_TX_STALL) {
		/* Endpoint not stalled, convert it to valid */
		ep_set_stat(num, false, USB_EP_STAT_TX_VALID);
	}
}

void stm32_fsdev_poll(usbd_device *dev)
{
	uint16_t istr;
	while((istr = USB_ISTR) & USB_ISTR_CTR) {
		uint8_t num = USB_ISTR_EP_ID_GET(istr);

		LOGF_LN("CTR interrupt process before: EP%"PRIu8" = 0x%"PRIx16,
					num, USB_EP(num));

		if (istr & USB_ISTR_DIR) {
			if (USB_EP(num) & USB_EP_SETUP) {
				process_setup_interrupt(dev, num);
			} else {
				process_out_interrupt(dev, num);
			}
		} else {
			process_in_interrupt(dev, num);
		}

		LOGF_LN("CTR interrupt process after: EP%"PRIu8" = 0x%"PRIx16,
					num, USB_EP(num));
	}

	if (USB_ISTR & USB_ISTR_RESET) {
		USB_ISTR = ~USB_ISTR_RESET;
		USB_DADDR = USB_DADDR_EF;
		disable_non_ep0(dev);
		alloc_ep0_buf(dev);
		usbd_handle_reset(dev);
		return;
	}

	if (USB_ISTR & USB_ISTR_SUSP) {
		USB_ISTR = ~USB_ISTR_SUSP;

		/* suspend USB peripheral. */
		USB_CNTR |= USB_CNTR_FSUSP;

		/* remove static power consumption in the analog USB transceivers
		 *  but keeping them able to detect resume activity. */
		USB_CNTR |= USB_CNTR_LP_MODE;

		usbd_handle_suspend(dev);
	}

	if (USB_ISTR & USB_ISTR_WKUP) {
		/* take out USB Peripherial from suspend mode */
		USB_CNTR &= ~USB_CNTR_FSUSP;

		usbd_handle_resume(dev);
		USB_ISTR = ~USB_ISTR_WKUP;
	}

	if (USB_ISTR & USB_ISTR_SOF) {
		USB_ISTR = ~USB_ISTR_SOF;
		usbd_handle_sof(dev);
	}

	if (USB_ISTR & USB_ISTR_ERR) {
		USB_ISTR = ~USB_ISTR_ERR;
		LOG_LN("Error detected");
	}

	if (USB_ISTR & USB_ISTR_PMAOVR) {
		USB_ISTR = ~USB_ISTR_PMAOVR;
		LOG_LN("PMA Overrun detected");
	}
}

void stm32_fsdev_enable_sof(usbd_device *dev, bool enable)
{
	(void) dev;

	if (enable) {
		USB_CNTR |= USB_CNTR_SOFM;
	} else {
		USB_CNTR &= ~USB_CNTR_SOFM;
	}
}

usbd_speed stm32_fsdev_get_speed(usbd_device *dev)
{
	(void) dev;

	return USBD_SPEED_FULL;
}

static void urb_submit_in(usbd_device *dev, usbd_urb *urb)
{
	LOG_CALL

	(void) dev;

	usbd_transfer *transfer = &urb->transfer;
	uint8_t num = ENDPOINT_NUMBER(transfer->ep_addr);

	size_t len = MIN(transfer->ep_size, transfer->length);

	/* WARN: IN, OUT endpoint have to be of same type for same endpoint number */
	ep_set_type(num, eptype_map[transfer->ep_type]);

	if (len) {
		void *buffer = usbd_urb_get_buffer_pointer(dev, urb, len);
		dev->backend->copy_to_pm(USB_EP_TX_PMA_BUF(num), buffer, len);
	}

	USB_EP_COUNT_TX(num) = len & 0x3FF;

	/* control endpoint will override stall stat */
	if (transfer->ep_type == USBD_EP_CONTROL ||
		(USB_EP(num) & USB_EP_STAT_TX_MASK) != USB_EP_STAT_TX_STALL) {
		/* Endpoint not stalled, convert it to valid */
		ep_set_stat(num, false, USB_EP_STAT_TX_VALID);
	}
}

static void urb_submit_out(usbd_device *dev, usbd_urb *urb)
{
	LOG_CALL

	(void) dev;

	usbd_transfer *transfer = &urb->transfer;
	uint8_t num = ENDPOINT_NUMBER(transfer->ep_addr);

	/* WARN: IN, OUT endpoint have to be of same type for same endpoint number */
	ep_set_type(num, eptype_map[transfer->ep_type]);

	if (transfer->ep_type == USBD_EP_CONTROL && !transfer->length) {
		/* set STATUS_OUT=1 */
		uint16_t reg16 = USB_EP(num);
		reg16 &= USB_EP_TYPE_MASK | USB_EP_EA_MASK;
		reg16 |= USB_EP_CTR_TX | USB_EP_CTR_RX | USB_EP_STATUS_OUT;
		USB_EP(num) = reg16;
	}

	/* control endpoint will override stall stat */
	if (transfer->ep_type == USBD_EP_CONTROL ||
		(USB_EP(num) & USB_EP_STAT_RX_MASK) != USB_EP_STAT_RX_STALL) {
		/* Endpoint not stalled, convert it to valid */
		ep_set_stat(num, true, USB_EP_STAT_RX_VALID);
	}
}

void stm32_fsdev_urb_submit(usbd_device *dev, usbd_urb *urb)
{
#if defined(USBD_DEBUG)
	uint8_t num = ENDPOINT_NUMBER(urb->transfer.ep_addr);
	LOGF_LN("before submit EP%"PRIu8" =  0x%"PRIx16, num, USB_EP(num));
#endif

	if (IS_IN_ENDPOINT(urb->transfer.ep_addr)) {
		urb_submit_in(dev, urb);
	} else {
		urb_submit_out(dev, urb);
	}

#if defined(USBD_DEBUG)
	LOGF_LN("after submit EP%"PRIu8" =  0x%"PRIx16, num, USB_EP(num));
#endif
}

void stm32_fsdev_urb_cancel(usbd_device *dev, usbd_urb *urb)
{
	(void) dev;
	uint8_t addr = urb->transfer.ep_addr;
	uint8_t num = ENDPOINT_NUMBER(addr);

	/* If any endpoint is valid, set it to NAK */
	if (IS_IN_ENDPOINT(addr)) {
		if ((USB_EP(num) & USB_EP_STAT_TX_MASK) == USB_EP_STAT_TX_VALID) {
			ep_set_stat(num, false, USB_EP_STAT_TX_NAK);
		}
	} else {
		if ((USB_EP(num) & USB_EP_STAT_RX_MASK) == USB_EP_STAT_RX_VALID) {
			ep_set_stat(num, true, USB_EP_STAT_RX_NAK);
		}
	}
}

uint16_t stm32_fsdev_frame_number(usbd_device *dev)
{
	(void) dev;

	return USB_FNR_FN_GET(USB_FNR);
}
