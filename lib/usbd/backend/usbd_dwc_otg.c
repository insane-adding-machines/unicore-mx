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
#include <unicore-mx/usbd/usbd.h>

/* FIXME: write code that handle back to back packet */
/* FIXME: stalled control transfer remove the transfer */

/*
 * Initialization on SET_ADDRESS Command
 * ======================================
 *  1. Set the DCFG register (DAD bits) with the address
 *      received in the SET_ADDRESS control transfer.
 *  2. Send out a STATUS_IN packet.
 */

/* OTG_EN_DED_TX_FIFO (Dedicate FIFO mode)
 * ==========================================
 * This mode is enable/disabled at instantiation.
 * The code assume that this mode is enabled. (ie OTG_EN_DED_TX_FIFO = 1)
 */

/* OTG_DFIFO_DYNAMIC (Dynamic FIFO sizing)
 * ========================================
 * In this mode, FIFO depth can be changed by software.
 * The code assume that this mode is enabled. (ie OTG_DFIFO_DYNAMIC=1)
 *
 * Care need to be taken:
 * Application should never pass prepare() with options that cause backend
 *  to allocate more than allowed FIFO size.
 * Usually, generally - application will not do so, but documented for edge
 *  cases.
 * Usually, the upper limit is 512 words. (ie 2KB)
 * This limit can be managed by
 */

/* __VA_ARGS__ are the optional argument for the register
 *   ex: endpoint number */
#define REBASE(REG, ...)	REG(dev->backend->base_address, ##__VA_ARGS__)

static void fifo_to_memory(volatile uint32_t *fifo, void *mem,
		unsigned bytes);
static void memory_to_fifo(const void *mem, volatile uint32_t *fifo,
		unsigned bytes);

/**
 * Get the number of device endpoint the periph support (including ep0)
 * @param[in] dev USB Device
 * @return endpoint count
 * @note if Config do not provide the information, GHWCFG2 is used
 */
static inline uint8_t get_ep_count(usbd_device *dev)
{
	uint8_t value = dev->config->ep_count;

	if (!value) {
		/* see comment on dwc_otg_private.h */
		value = DWC_OTG_GHWCFG2_NUMDEVEPS_GET(REBASE(DWC_OTG_GHWCFG2));
		value += 1;
	}

	return value;
}

/**
 * Get the FIFO depth
 * @param[in] dev USB Device
 * @return fifo depth
 * @note if Config do not provide the value, GHWCFG3 is used
 */
static inline uint16_t get_fifo_depth(usbd_device *dev)
{
	uint16_t value = dev->config->priv_mem / 4;

	if (!value) {
		/* see comment on dwc_otg_private.h */
		value = DWC_OTG_GHWCFG3_DFIFODEPTH_GET(REBASE(DWC_OTG_GHWCFG3));
	}

	return value;
}

/**
 * Flush FIFO
 * Perform a complete flush of RX and TX
 * @param[in] dev USB Device
 */
static void flush_fifo(usbd_device *dev)
{
	/* Flush RX, TX FIFO */
	REBASE(DWC_OTG_GRSTCTL) = DWC_OTG_GRSTCTL_RXFFLSH |
		(DWC_OTG_GRSTCTL_TXFFLSH | DWC_OTG_GRSTCTL_TXFNUM_ALL);

	while (REBASE(DWC_OTG_GRSTCTL) &
				(DWC_OTG_GRSTCTL_RXFFLSH | DWC_OTG_GRSTCTL_TXFFLSH));
}

void dwc_otg_init(usbd_device *dev)
{
	/* Wait for AHB idle. */
	while (!(REBASE(DWC_OTG_GRSTCTL) & DWC_OTG_GRSTCTL_AHBIDL));

	/* Do core soft reset. */
	REBASE(DWC_OTG_GRSTCTL) |= DWC_OTG_GRSTCTL_CSRST;
	while (REBASE(DWC_OTG_GRSTCTL) & DWC_OTG_GRSTCTL_CSRST);

	/* Clear SDIS because newer version have set by default */
	REBASE(DWC_OTG_DCTL) &= ~DWC_OTG_DCTL_SDIS;

	/* Force peripheral only mode. */
	REBASE(DWC_OTG_GUSBCFG) |= DWC_OTG_GUSBCFG_FDMOD | DWC_OTG_GUSBCFG_TRDT_MASK;

	REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_MMIS;

	/* Restart the PHY clock. */
	REBASE(DWC_OTG_PCGCCTL) = 0;

	/* Unmask interrupts for TX and RX. */
	REBASE(DWC_OTG_GAHBCFG) |= DWC_OTG_GAHBCFG_GINT;
	REBASE(DWC_OTG_GINTMSK) = DWC_OTG_GINTMSK_ENUMDNEM |
					DWC_OTG_GINTMSK_RXFLVLM |
					DWC_OTG_GINTMSK_IEPINT |
					DWC_OTG_GINTMSK_USBSUSPM |
					DWC_OTG_GINTMSK_WUIM;

	REBASE(DWC_OTG_DAINTMSK) = 0;
	REBASE(DWC_OTG_DIEPMSK) = DWC_OTG_DIEPMSK_XFRCM | DWC_OTG_DIEPMSK_EPDM;
	REBASE(DWC_OTG_DOEPMSK) = DWC_OTG_DOEPMSK_XFRCM | DWC_OTG_DOEPMSK_BBLERR |
								DWC_OTG_DOEPMSK_STUPM | DWC_OTG_DOEPMSK_EPDM;

	LOGF_LN("FIFO Depth: %"PRIu16, get_fifo_depth(dev));
	LOGF_LN("Endpoint count (including EP0): %"PRIu16, get_ep_count(dev));
}

void dwc_otg_set_address(usbd_device *dev, uint8_t addr)
{
	REBASE(DWC_OTG_DCFG) = (REBASE(DWC_OTG_DCFG) & ~DWC_OTG_DCFG_DAD_MASK) |
							DWC_OTG_DCFG_DAD(addr);
}

uint8_t dwc_otg_get_address(usbd_device *dev)
{
	return DWC_OTG_DCFG_DAD_GET(REBASE(DWC_OTG_DCFG));
}

static void disable_all_non_ep0(usbd_device *dev)
{
	unsigned i;

	for (i = 1; i < get_ep_count(dev); i++) {

		REBASE(DWC_OTG_DOEPxINT, i) = 0xFFFF;
		REBASE(DWC_OTG_DOEPxTSIZ, i) = 0;
		REBASE(DWC_OTG_DOEPxCTL, i) = DWC_OTG_DOEPCTL_SNAK;

		REBASE(DWC_OTG_DIEPxINT, i) = 0xFFFF;
		REBASE(DWC_OTG_DIEPxTSIZ, i) = 0;
		REBASE(DWC_OTG_DIEPxCTL, i) = DWC_OTG_DIEPCTL_SNAK;
	}

	REBASE(DWC_OTG_DAINTMSK) &= ~(DWC_OTG_DAINTMSK_OEPM(0) | DWC_OTG_DAINTMSK_IEPM(0));
	REBASE(DWC_OTG_DIEPEMPMSK) = 0;
}

/*
 * We are going to fill the FIFO from bottom to top.
 * Using this technique we can start allocating FIFO for dedicated TX FIFO.
 * and at the end (in dwc_otg_ep_prepare_end()) the remaining space will
 *  be used for RX FIFO
 *
 * RX FIFO (Slave Mode) =
 *   (5 * number of control endpoints + 8) +                             (e1)
 *   2 * ((largest USB packet used / 4) + 1 for status information)) +   (e2)
 *   (2 * number of OUT endpoints) + 1 for Global NAK                    (e3)
 *
 * e1: tracked by dev->private_data.fifo_rx_usage_overall
 * e2: tracked by dev->private_data.fifo_rx_usage_packet
 * e3: tracked by dev->private_data.fifo_rx_usage_overall
 */

void dwc_otg_ep_prepare_start(usbd_device *dev)
{
	uint16_t fifo_word = DIVIDE_AND_CEIL(dev->desc->bMaxPacketSize0, 4);
	fifo_word = MAX(fifo_word, 16);

	dev->private_data.fifo_rx_usage_overall = 16; /* by EP0 and constant */
	dev->private_data.fifo_rx_usage_packet = fifo_word + 1; /* EP0 */

	disable_all_non_ep0(dev);

	dev->private_data.fifo_remaining = get_fifo_depth(dev) - fifo_word;
	REBASE(DWC_OTG_DIEP0TXF) = DWC_OTG_DIEP0TXF_TX0FD(fifo_word) |
					DWC_OTG_DIEP0TXF_TX0FSA(dev->private_data.fifo_remaining);
}

/* layout of EPTYPE for DOEPxCTL and DIEPxCTL is same */
static const uint32_t eptyp_map[] = {
	[USBD_EP_CONTROL] = DWC_OTG_DOEPCTL_EPTYP_CONTROL,
	[USBD_EP_ISOCHRONOUS] = DWC_OTG_DOEPCTL_EPTYP_ISOCHRONOUS,
	[USBD_EP_BULK] = DWC_OTG_DOEPCTL_EPTYP_BULK,
	[USBD_EP_INTERRUPT] = DWC_OTG_DOEPCTL_EPTYP_INTERRUPT
};

void dwc_otg_ep_prepare(usbd_device *dev, uint8_t addr,
					usbd_ep_type type, uint16_t max_size, uint16_t interval,
					usbd_ep_flags flags)
{
	(void) interval;

	/* Allocate double the FIFO if requested */
	if (flags & USBD_EP_DOUBLE_BUFFER) {
		LOGF_LN("DOUBLE_BUFFER flag for endpoint 0x%"PRIx8
			", doubling max_size", addr);
		max_size *= 2;
	}

	uint16_t fifo_word = DIVIDE_AND_CEIL(max_size, 4);
	fifo_word = MAX(fifo_word, 16); /* required by periph */
	uint8_t num = ENDPOINT_NUMBER(addr);

	if (IS_IN_ENDPOINT(addr)) {
		/* FIXME: underflow */
		dev->private_data.fifo_remaining -= fifo_word; /* IN */

		REBASE(DWC_OTG_DIEPxTXF, num) = DWC_OTG_DIEPTXF_INEPTXFD(fifo_word) |
					DWC_OTG_DIEPTXF_INEPTXSA(dev->private_data.fifo_remaining);

		REBASE(DWC_OTG_DIEPxCTL, num) = DWC_OTG_DIEPCTL_SNAK |
						DWC_OTG_DIEPCTL_SD0PID | eptyp_map[type] |
						DWC_OTG_DIEPCTL_USBAEP | DWC_OTG_DIEPCTL_TXFNUM(num);
	} else {
		if (type == USBD_EP_CONTROL) {
			dev->private_data.fifo_rx_usage_overall += 13; /* Setup */
		}

		dev->private_data.fifo_rx_usage_overall += 2;

		uint16_t usage = fifo_word + ((flags & USBD_EP_DOUBLE_BUFFER) ? 2 : 1);
		dev->private_data.fifo_rx_usage_packet =
			MAX(dev->private_data.fifo_rx_usage_packet, usage); /* OUT */

		REBASE(DWC_OTG_DOEPxCTL, num) = DWC_OTG_DOEPCTL_SNAK |
							eptyp_map[type] | DWC_OTG_DOEPCTL_USBAEP |
							DWC_OTG_DOEPCTL_SD0PID;
	}
}

void dwc_otg_ep_prepare_end(usbd_device *dev)
{
	uint16_t fifo_rx = dev->private_data.fifo_rx_usage_overall +
							dev->private_data.fifo_rx_usage_packet;

	LOGF_LN("FIFO Available: %"PRIu16, get_fifo_depth(dev));
	LOGF_LN("FIFO remaining after assigning to FIFO TX: %"PRIu16,
						dev->private_data.fifo_remaining);
	LOGF_LN("FIFO RX (need): %"PRIu16, fifo_rx);

	if (fifo_rx > dev->private_data.fifo_remaining) {
		LOGF_LN("We need %"PRIu16" fifo words but %"PRIu16" is only left "
			"after memory has been allocated to dedicated TX FIFO. "
			"(Please reduce IN endpoints memory requirement)",
			fifo_rx, dev->private_data.fifo_remaining);
	}

	/* Assign what ever is left to FIFO RX */
	REBASE(DWC_OTG_GRXFSIZ) = dev->private_data.fifo_remaining;

	flush_fifo(dev);
}

void dwc_otg_set_ep_dtog(usbd_device *dev, uint8_t addr, bool dtog)
{
	uint8_t num = ENDPOINT_NUMBER(addr);

	if (!num) {
		/* No way to set DTOG bit for EP0 */
		return;
	}

	/* DIEPxCTL and DOEPxCTL have same layout for SD1PID and SD0PID */
	uint32_t mask = dtog ?
			DWC_OTG_DIEPCTL_SD1PID :
			DWC_OTG_DIEPCTL_SD0PID;

	volatile uint32_t *reg32_ptr = IS_IN_ENDPOINT(addr) ?
			&REBASE(DWC_OTG_DIEPxCTL, num) :
			&REBASE(DWC_OTG_DOEPxCTL, num);

	*reg32_ptr |= mask;
}

bool dwc_otg_get_ep_dtog(usbd_device *dev, uint8_t addr)
{
	uint8_t num = ENDPOINT_NUMBER(addr);

	if (!num) {
		/* No way to set DTOG bit for EP0 */
		return false;
	}

	uint32_t reg32 = IS_IN_ENDPOINT(addr) ?
			REBASE(DWC_OTG_DIEPxCTL, num) :
			REBASE(DWC_OTG_DOEPxCTL, num);

	/* DIEPxCTL and DOEPxCTL have same layout for DPID */
	return !!(reg32 & DWC_OTG_DIEPCTL_DPID);
}

void dwc_otg_set_ep_stall(usbd_device *dev, uint8_t addr, bool stall)
{
	uint8_t num = ENDPOINT_NUMBER(addr);

	LOGF_LN("STALL endpoint 0x%"PRIx8" = %s", addr, stall ? "Yes" : "No");

	/* DIEP0CTL, DIEPxCTL, DOEP0CTL, DOEPxCTL have same STALL layout */
	volatile uint32_t *reg_ptr = IS_IN_ENDPOINT(addr) ?
		&REBASE(DWC_OTG_DIEPxCTL, num) :
		&REBASE(DWC_OTG_DOEPxCTL, num);

	if (stall) {
		*reg_ptr |= DWC_OTG_DIEPCTL_STALL;
	} else {
		/* Note: not possible to un-stall control endpoints (as per doc) */
		*reg_ptr &= ~DWC_OTG_DIEPCTL_STALL;
	}
}

bool dwc_otg_get_ep_stall(usbd_device *dev, uint8_t addr)
{
	uint8_t num = ENDPOINT_NUMBER(addr);
	uint32_t epctl = IS_IN_ENDPOINT(addr) ?
				REBASE(DWC_OTG_DIEPxCTL, num) :
				REBASE(DWC_OTG_DOEPxCTL, num);

	/* DIEPxCTL and DOEPxCTL has same layout of STALL */
	return !!(epctl & DWC_OTG_DIEPCTL_STALL);
}

/**
 * Convert endpoint 0 size to endpoint 0 MPS
 * @param[in] size Size
 */
static inline uint32_t ep0_mps(uint16_t size)
{
	if (size >= 64) {
		return DWC_OTG_DOEP0CTL_MPSIZ_64;
	} else if (size >= 32) {
		return DWC_OTG_DOEP0CTL_MPSIZ_32;
	} else if (size >= 16) {
		return DWC_OTG_DOEP0CTL_MPSIZ_16;
	} else {
		return DWC_OTG_DOEP0CTL_MPSIZ_8;
	}
}

/**
 * Write more data
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block (IN endpoint only)
 */
static void urb_to_fifo_1pkt(usbd_device *dev, usbd_urb *urb)
{
	LOG_CALL

	usbd_transfer *transfer = &urb->transfer;
	uint8_t ep_num = ENDPOINT_NUMBER(transfer->ep_addr);
	size_t rem_len = transfer->length - transfer->transferred;

	if (!rem_len) {
		LOGF_LN("No more data to send URB %"PRIu64" (endpoint 0x%"PRIx8") "
			"(intending ZLP?)", urb->id, transfer->ep_addr);
		return;
	}

	size_t tx_len = MIN(transfer->ep_size, rem_len);

	/* TX FIFO has enough space to write "tx_len" of data */
	size_t tx_words = DIVIDE_AND_CEIL(tx_len, 4);
	uint32_t dieptxfsts = REBASE(DWC_OTG_DIEPxTXFSTS, ep_num);
	uint16_t fifo_words_avail = DWC_OTG_DIEPTXFSTS_INEPTFSAV_GET(dieptxfsts);
	if (tx_words > fifo_words_avail) {
		LOGF_LN("Need %u DWORDS but TX FIFO %"PRIu8" only has %u DWORDS",
			tx_words, ep_num, fifo_words_avail);
		return;
	}

	void *buffer = usbd_urb_get_buffer_pointer(dev, urb, tx_len);
	memory_to_fifo(buffer, &REBASE(DWC_OTG_FIFO, ep_num), tx_len);
	usbd_urb_inc_data_pointer(dev, urb, tx_len);

	if (transfer->transferred >= transfer->length) {
		/* Disabling TX Empty interrupt since we have no more data */
		REBASE(DWC_OTG_DIEPEMPMSK) &= ~DWC_OTG_DIEPEMPMSK_INEPTXFEM(ep_num);
	}
}

static inline uint16_t calc_pktcnt(size_t transfer_len, uint16_t ep_size)
{
	if (!transfer_len) {
		return 1;
	}

	return DIVIDE_AND_CEIL(transfer_len, ep_size);
}

static void urb_submit_ep0(usbd_device *dev, usbd_urb *urb)
{
	LOG_CALL

	usbd_transfer *transfer = &urb->transfer;

	/* Calculate MPS value
	 * Note: DIEP0CTL and DOEP0CTL for MPSIZ have same layout */
	uint32_t mps = ep0_mps(transfer->ep_size);

	/* Calculate the number of packet to transmit */
	uint16_t pktcnt = calc_pktcnt(transfer->length, transfer->ep_size);

	if (IS_IN_ENDPOINT(transfer->ep_addr)) {
		/* Clear Interrupts */
		REBASE(DWC_OTG_DIEPxINT, 0) = 0xFFFF;

		/* Short packet check */
		if (transfer->flags & USBD_FLAG_SHORT_PACKET) {
			if ((transfer->ep_size * pktcnt) == transfer->length) {
				pktcnt += 1;
			}
		}

		pktcnt--;
		uint32_t xfrsiz = MIN(transfer->ep_size, transfer->length);

		REBASE(DWC_OTG_DIEP0TSIZ) = DWC_OTG_DIEP0TSIZ_PKTCNT_1 |
					DWC_OTG_DIEP0TSIZ_XFRSIZ(xfrsiz);

		REBASE(DWC_OTG_DIEP0CTL) = DWC_OTG_DIEP0CTL_EPENA | mps |
						DWC_OTG_DIEP0CTL_EPTYP_CONTROL |
						DWC_OTG_DIEP0CTL_CNAK | DWC_OTG_DIEP0CTL_USBAEP;

		/* Push first packet to memory! */
		if (transfer->length) {
			urb_to_fifo_1pkt(dev, urb);
		}

		/* Enable Interrupt */
		REBASE(DWC_OTG_DAINTMSK) |= DWC_OTG_DAINTMSK_IEPM(0);
	} else { /* OUT */
		/* Clear Interrupts */
		REBASE(DWC_OTG_DOEPxINT, 0) = 0xFFFF;

		pktcnt--;

		/* "For OUT transfers, the Transfer Size field in the endpoint’s
		 *  Transfer Size register must be a multiple of the maximum packet
		 * size of the endpoint, adjusted to the DWORD boundary."
		 *  For control, the endpoint size can only be 8, 16, 32, 64 (no problem)
		 */
		REBASE(DWC_OTG_DOEP0TSIZ) = DWC_OTG_DOEP0TSIZ_STUPCNT_3 |
					DWC_OTG_DOEP0TSIZ_PKTCNT_1 |
					DWC_OTG_DOEP0TSIZ_XFRSIZ(transfer->ep_size);

		REBASE(DWC_OTG_DOEP0CTL) = DWC_OTG_DOEP0CTL_EPENA |
					DWC_OTG_DOEP0CTL_EPTYP_CONTROL | DWC_OTG_DOEP0CTL_CNAK |
					mps | DWC_OTG_DOEP0CTL_USBAEP;

		/* Enable Interrupt */
		REBASE(DWC_OTG_DAINTMSK) |= DWC_OTG_DAINTMSK_OEPM(0);
	}

	/* Store for complete callback */
	dev->private_data.ep0tsiz_pktcnt = pktcnt;
}

/**
 * Get DWC_OTG_DOEPxTSIZ MC flag from transfer @a flags
 * @param[in] flags Transfer flags
 * @return the MC equivalent
 */
static uint32_t mc_from_flags(usbd_transfer_flags flags)
{
	switch (flags & USBD_FLAG_PACKET_PER_FRAME_MASK) {
	case USBD_FLAG_PACKET_PER_FRAME_1:
	return DWC_OTG_DIEPTSIZ_MC_1;
	case USBD_FLAG_PACKET_PER_FRAME_2:
	return DWC_OTG_DIEPTSIZ_MC_2;
	case USBD_FLAG_PACKET_PER_FRAME_3:
	return DWC_OTG_DIEPTSIZ_MC_3;
	default:
	LOGF_LN("Invalid USBD_FLAG_PACKET_PER_FRAME_n flag in transfer flag %i", flags);
	return DWC_OTG_DIEPTSIZ_MC_1;
	}
}

static void urb_submit_non_ep0(usbd_device *dev, usbd_urb *urb)
{
	LOG_CALL

	usbd_transfer *transfer = &urb->transfer;
	uint8_t ep_num = ENDPOINT_NUMBER(transfer->ep_addr);

	/* Calculate the number of packet to transmit */
	uint16_t pktcnt = calc_pktcnt(transfer->length, transfer->ep_size);

	if (IS_IN_ENDPOINT(transfer->ep_addr)) {
		/* Clear Interrupts */
		REBASE(DWC_OTG_DIEPxINT, ep_num) = 0xFFFF;

		/* Short packet only make sense for Control IN and Bulk IN/OUT */
		if (transfer->flags & USBD_FLAG_SHORT_PACKET) {
			if (transfer->ep_type == USBD_EP_CONTROL ||
					transfer->ep_type == USBD_EP_BULK) {
				if ((transfer->ep_size * pktcnt) == transfer->length) {
					pktcnt += 1;
				}
			}
		}

		REBASE(DWC_OTG_DIEPxTSIZ, ep_num) =
					mc_from_flags(transfer->flags) |
					DWC_OTG_DIEPTSIZ_PKTCNT(pktcnt) |
					DWC_OTG_DIEPTSIZ_XFRSIZ(transfer->length);

		REBASE(DWC_OTG_DIEPxCTL, ep_num) = DWC_OTG_DIEPCTL_EPENA |
					DWC_OTG_DIEPCTL_MPSIZ(transfer->ep_size) |
					DWC_OTG_DIEPCTL_CNAK | DWC_OTG_DIEPCTL_TXFNUM(ep_num) |
					eptyp_map[transfer->ep_type] | DWC_OTG_DIEPCTL_USBAEP;

		/* Push first packet to memory! */
		if (transfer->length) {
			/* Enable empty interrupt mask */
			REBASE(DWC_OTG_DIEPEMPMSK) |= DWC_OTG_DIEPEMPMSK_INEPTXFEM(ep_num);

			urb_to_fifo_1pkt(dev, urb);
		}

		/* Enable Interrupt */
		REBASE(DWC_OTG_DAINTMSK) |= DWC_OTG_DAINTMSK_IEPM(ep_num);
	} else { /* OUT */
		/* Clear Interrupts */
		REBASE(DWC_OTG_DOEPxINT, ep_num) = 0xFFFF;

		/* "For OUT transfers, the Transfer Size field in the endpoint’s
		 *  Transfer Size register must be a multiple of the maximum packet
		 * size of the endpoint, adjusted to the DWORD boundary." */
		uint16_t ep_dwords = DIVIDE_AND_CEIL(transfer->ep_size, 4);
		uint32_t xfrsiz = pktcnt * ep_dwords * 4;

		REBASE(DWC_OTG_DOEPxTSIZ, ep_num) = DWC_OTG_DOEPTSIZ_STUPCNT_3 |
									DWC_OTG_DOEPTSIZ_PKTCNT(pktcnt) |
									DWC_OTG_DOEPTSIZ_XFRSIZ(xfrsiz);

		REBASE(DWC_OTG_DOEPxCTL, ep_num) = DWC_OTG_DOEPCTL_EPENA |
					DWC_OTG_DOEPCTL_CNAK |
					DWC_OTG_DOEPCTL_MPSIZ(transfer->ep_size) |
					eptyp_map[transfer->ep_type] | DWC_OTG_DOEPCTL_USBAEP;

		/* Enable Interrupt */
		REBASE(DWC_OTG_DAINTMSK) |= DWC_OTG_DAINTMSK_OEPM(ep_num);
	}
}

void dwc_otg_urb_submit(usbd_device *dev, usbd_urb *urb)
{
	LOG_CALL

	if (ENDPOINT_NUMBER(urb->transfer.ep_addr)) {
		urb_submit_non_ep0(dev, urb);
	} else {
		urb_submit_ep0(dev, urb);
	}
}

void dwc_otg_urb_cancel(usbd_device *dev, usbd_urb *urb)
{
	(void) dev;
	(void) urb;
}

typedef struct {
	uint32_t	data[1];
} __attribute__((packed)) uint32_packed_array;

/**
 * Copy @a bytes count from FIFO ( @a fifo) to memory @a mem
 * @param[in] fifo FIFO pointer
 * @param[in] mem Memory pointer
 * @param[in] bytes Number of bytes to copy
 * @note @a fifo will only be accessed in 32bit
 * @note @a mem should be 8bit, 16bit and 32bit accessible
 */
static void fifo_to_memory(volatile uint32_t *fifo, void *mem,
			size_t bytes)
{
	LOG_CALL

	uint32_t *mem32 = mem;
	if (((uintptr_t)mem & 3) == 0) {
		//aligned memload
	while (bytes >= 4) {
		bytes -= 4;
		*mem32++ = *fifo;
	}
	}//if (((uintptr_t)mem & 3) == 0)
	else {
		//unaligned memload
		uint32_packed_array *src = (uint32_packed_array *)mem;
		while (bytes >= 4) {
			bytes -= 4;
			(src++)->data[0] = *fifo;
		}
		mem32 = (uint32_t*)src;
	}

	if (bytes) {
		/* remaining data (less than 4bytes) */
		uint32_t extra = *fifo;
		//optimisation on ARM compiler of memcpy call here, breaks 
		//    the stack frame and crush <extra> contents
		//memcpy(mem32, (void*)&extra, bytes);
		uint8_t* memc = (uint8_t*)mem32;
		for (; bytes > 0; bytes--, extra = extra >> 8)
			*memc++ = extra&0xff;
	}
}

/**
 * Copy @a bytes count from memory ( @a mem) to FIFO ( @a fifo)
 * @param[in] mem Memory pointer
 * @param[in] fifo FIFO pointer
 * @param[in] bytes Number of bytes to copy
 * @note @a fifo and @a mem will only be accessed in 32bit
 */
static void memory_to_fifo(const void *mem, volatile uint32_t *fifo,
			size_t bytes)
{
	LOG_CALL

	if (((uintptr_t)mem & 3) == 0) {
		//aligned memload
	const uint32_t *mem32 = (const uint32_t *)mem;
	unsigned j;
	for (j = 0; j < bytes; j += 4) {
		*fifo = *mem32++;
	}
	}//if (((uintptr_t)mem & 3) == 0)
	else {
		//unaligned memload
		const uint32_packed_array *mem32 = (const uint32_packed_array *)mem;
		unsigned j;
		for (j = 0; j < bytes; j += 4) {
			*fifo = (mem32++)->data[0];
		}
	}
}

/**
 * Read data from FIFO and thow it away
 * @param[in] dev USB Device
 * @param[in] bytes Number of bytes
 */
static inline void fifo_read_and_throw(usbd_device *dev, unsigned bytes)
{
	register uint32_t tmp __attribute__((unused));

	while (bytes >= 4) {
		tmp = REBASE(DWC_OTG_FIFO, 0);
		bytes -= 4;
	}

	if (bytes) {
		tmp = REBASE(DWC_OTG_FIFO, 0);
	}
}

/**
 * Perform a premature URB complete
 * This is useful when some kind of unexpected things happen in between
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param[in] status Transfer status
 */
static void premature_urb_complete(usbd_device *dev, usbd_urb *urb,
		usbd_transfer_status status)
{
	usbd_transfer *transfer = &urb->transfer;
	uint8_t ep_num = ENDPOINT_NUMBER(transfer->ep_addr);

	if (IS_IN_ENDPOINT(transfer->ep_addr)) {
		REBASE(DWC_OTG_DIEPxCTL, ep_num) |= DWC_OTG_DIEPCTL_SNAK;
		REBASE(DWC_OTG_DIEPxINT, ep_num) = 0xFFFF;
		REBASE(DWC_OTG_DAINTMSK) &= ~DWC_OTG_DAINTMSK_IEPM(ep_num);
	} else {
		REBASE(DWC_OTG_DOEPxCTL, ep_num) |= DWC_OTG_DOEPCTL_SNAK;
		REBASE(DWC_OTG_DOEPxINT, ep_num) = 0xFFFF ^ DWC_OTG_DOEPINT_STUP;
		REBASE(DWC_OTG_DAINTMSK) &= ~DWC_OTG_DAINTMSK_OEPM(ep_num);
	}

	usbd_urb_complete(dev, urb, status);
}

/**
 * Pop data from FIFO and store it in to URB
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param[in] bcnt Number of bytes to pop
 * @note @a bcnt is derived from RXFLVL interrupt
 */
static void fifo_to_urb_1pkt(usbd_device *dev, usbd_urb *urb, uint16_t bcnt)
{
	usbd_transfer *transfer = &urb->transfer;

	if (!transfer->length) {
		if (bcnt) {
			/* Got non-zero bytes from host, 0 bytes were expected */
			fifo_read_and_throw(dev, bcnt);
			premature_urb_complete(dev, urb, USBD_ERR_OVERFLOW);
		}

		/* for transfer->length = 0 and bcnt = 0, XFRC interrupt will handle */
		return;
	}

	/* Copy what ever is possible to buffer */
	size_t space_avail = transfer->length - transfer->transferred;
	size_t storable_len = MIN(bcnt, space_avail);
	void *buffer = usbd_urb_get_buffer_pointer(dev, urb, storable_len);
	fifo_to_memory(&REBASE(DWC_OTG_FIFO, 0), buffer, storable_len);
	usbd_urb_inc_data_pointer(dev, urb, storable_len);

	if (bcnt > space_avail) {
		LOGF_LN("WARN: At maximum could accomodate %u bytes but host has"
			"sent %"PRIu16" bytes", space_avail, bcnt);
		fifo_read_and_throw(dev, space_avail - storable_len);
		premature_urb_complete(dev, urb, USBD_ERR_OVERFLOW);
		return;
	}

	if (bcnt < transfer->ep_size) {
		if (transfer->ep_type == USBD_EP_BULK) {
			LOGF_LN("Short packet received for Bulk endpoint 0x%"PRIx8,
						transfer->ep_addr);

			if (transfer->flags & USBD_FLAG_SHORT_PACKET) {
				/* Short packet received (usually marker of end of transfer) */
				premature_urb_complete(dev, urb, USBD_SUCCESS);
			} else if (transfer->flags & USBD_FLAG_NO_SHORT_PACKET) {
				/* Short packet received when it flagged
				 *  that short packet will cause transfer failure */
				premature_urb_complete(dev, urb, USBD_ERR_SHORT_PACKET);
			}
		}
	}
}

/**
 * Handle the RXFLVL interrupt.
 * This interrupt is used to handle
 *   - DATA OUT packet
 *   - SETUP packet
 * @param[in] dev USB Device
 */
static void handle_rxflvl_interrupt(usbd_device *dev)
{
	uint32_t rxstsp = REBASE(DWC_OTG_GRXSTSP);
	uint8_t ep_num = DWC_OTG_GRXSTSP_EPNUM_GET(rxstsp);
	uint16_t bcnt = DWC_OTG_GRXSTSP_BCNT_GET(rxstsp);

#if defined(USBD_DEBUG)
	const char *map_pktsts[] = {
		[0] = "RESERVED_0",
		[1] = "GOUTNAK",
		[2] = "OUT",
		[3] = "OUT_COMP",
		[4] = "SETUP_COMP",
		[5] = "DTERR",
		[6] = "SETUP",
		[7] = "CHH",
		[8] = "RESERVED_8",
		[9] = "RESERVED_9",
		[10] = "RESERVED_10",
		[11] = "RESERVED_11",
		[12] = "RESERVED_12",
		[13] = "RESERVED_13",
		[14] = "RESERVED_14",
		[15] = "RESERVED_15"
	};

	LOGF_LN("GRXSTSP: rxstsp = %s, ep_num = %"PRIu8", bcnt = %"PRIu16,
		map_pktsts[DWC_OTG_GRXSTSP_PKTSTS_GET(rxstsp)], ep_num, bcnt);
#endif

	switch (rxstsp & DWC_OTG_GRXSTSP_PKTSTS_MASK) {
	case DWC_OTG_GRXSTSP_PKTSTS_OUT: {
		if (bcnt) {
			usbd_urb *urb = usbd_find_active_urb(dev, ep_num);
			if (urb != NULL) {
				fifo_to_urb_1pkt(dev, urb, bcnt);
			}
		}
	} break;
	case DWC_OTG_GRXSTSP_PKTSTS_SETUP: {
		if (bcnt != 8) {
			LOG_LN("SETUP packet in FIFO not equal to 8");
			break;
		}

		struct usb_setup_data *setup_data = &dev->private_data.setup_data;
		uint32_t *io = (void *) setup_data;
		io[0] = REBASE(DWC_OTG_FIFO, 0);
		io[1] = REBASE(DWC_OTG_FIFO, 0);
		LOGF_LN("bmRequestType: 0x%02"PRIx8, setup_data->bmRequestType);
		LOGF_LN("bRequest: 0x%02"PRIx8, setup_data->bRequest);
		LOGF_LN("wValue: 0x%04"PRIx16, setup_data->wValue);
		LOGF_LN("wIndex: 0x%04"PRIx16, setup_data->wIndex);
		LOGF_LN("wLength: %"PRIu16, setup_data->wLength);
	} break;
	case DWC_OTG_GRXSTSP_PKTSTS_SETUP_COMP: {
		/* Enable Interrupt to receive the SETUP packet */
		REBASE(DWC_OTG_DAINTMSK) |= DWC_OTG_DAINTMSK_OEPM(ep_num);
	} break;
	}
}

/**
 * Process IN endpoint interrupt
 * @param[in] dev USB Device
 * @param[in] ep_num Endpoint number (ie without direction bit)
 */
static void process_in_endpoint_interrupt(usbd_device *dev, uint8_t ep_num)
{
	LOG_CALL

	const uint8_t ep_addr = ep_num | 0x80;

	if (REBASE(DWC_OTG_DIEPxINT, ep_num) & DWC_OTG_DIEPINT_EPDISD) {
		REBASE(DWC_OTG_DIEPxINT, ep_num) = DWC_OTG_DIEPINT_EPDISD;
		LOGF_LN("Endpoint disabled 0x%"PRIx8, ep_addr);
	}

	usbd_urb *urb = usbd_find_active_urb(dev, ep_addr);

	if (REBASE(DWC_OTG_DIEPxINT, ep_num) & DWC_OTG_DIEPINT_XFRC) {
		REBASE(DWC_OTG_DIEPxINT, ep_num) = DWC_OTG_DIEPINT_XFRC;

		LOGF_LN("Transfer Complete: endpoint 0x%"PRIx8, ep_addr);

		if (!ep_num && urb != NULL && dev->private_data.ep0tsiz_pktcnt) {
			/* We are still sending data! */

			size_t rem = urb->transfer.length - urb->transfer.transferred;
			uint32_t xfrsiz = MIN(urb->transfer.ep_size, rem);
			dev->private_data.ep0tsiz_pktcnt--;

			REBASE(DWC_OTG_DIEP0TSIZ) = DWC_OTG_DIEP0TSIZ_PKTCNT_1 |
				DWC_OTG_DIEP0TSIZ_XFRSIZ(xfrsiz);

			REBASE(DWC_OTG_DIEP0CTL) |= DWC_OTG_DIEP0CTL_EPENA |
											DWC_OTG_DIEP0CTL_CNAK;

			if (xfrsiz) {
				urb_to_fifo_1pkt(dev, urb);
			}
		} else {
			/* Set NAK on the endpoint */
			REBASE(DWC_OTG_DIEPxCTL, ep_num) |= DWC_OTG_DIEPCTL_SNAK;

			/* Disabling TX Empty interrupt */
			REBASE(DWC_OTG_DIEPEMPMSK) &= ~DWC_OTG_DIEPEMPMSK_INEPTXFEM(ep_num);

			/* Disable Interrupt */
			REBASE(DWC_OTG_DAINTMSK) &= ~DWC_OTG_DAINTMSK_IEPM(ep_num);

			/* The URB has been processed, do the callback */
			if (urb != NULL) {
				usbd_urb_complete(dev, urb, USBD_SUCCESS);
			}
		}

		return;
	}

	if (REBASE(DWC_OTG_DIEPxINT, ep_num) & DWC_OTG_DIEPINT_TXFE) {
		/* Send more data */
		LOGF_LN("Sending more data for endpoint 0x%"PRIx8, ep_addr);

		if (urb != NULL) {
			/* As per doc, before writing to FIFO, we need to write to CTL register.
			 *  In this case, clearing NAK is probably a NOP */
			REBASE(DWC_OTG_DIEPxCTL, ep_num) |= DWC_OTG_DIEPCTL_CNAK;

			urb_to_fifo_1pkt(dev, urb);
		}
	}

	if (REBASE(DWC_OTG_DIEPxINT, ep_num) & DWC_OTG_DIEPINT_ITTXFE) {
		LOGF_LN("Data IN Token received when endpoint 0x%"PRIx8" FIFO was empty",
			ep_addr);
		REBASE(DWC_OTG_DIEPxINT, ep_num) = DWC_OTG_DIEPINT_ITTXFE;
	}
}

/**
 * Process OUT endpoint interrupt
 * @param[in] dev USB Device
 * @param[in] ep_num Endpoint number (ie without direction bit)
 */
static void process_out_endpoint_interrupt(usbd_device *dev, uint8_t ep_num)
{
	LOG_CALL

	const uint8_t ep_addr = ep_num;

	if (REBASE(DWC_OTG_DOEPxINT, ep_num) & DWC_OTG_DOEPINT_EPDISD) {
		LOGF_LN("Endpoint disabled 0x%"PRIx8, ep_addr);
		REBASE(DWC_OTG_DOEPxINT, ep_num) = DWC_OTG_DOEPINT_EPDISD;
	}

	if (REBASE(DWC_OTG_DOEPxINT, ep_num) & DWC_OTG_DOEPINT_BBLERR) {
		LOGF_LN("Received more data than expected on endpoint 0x%"PRIx8,
			ep_addr);
		usbd_urb *urb = usbd_find_active_urb(dev, ep_addr);
		REBASE(DWC_OTG_DOEPxINT, ep_num) = DWC_OTG_DOEPINT_BBLERR;
		premature_urb_complete(dev, urb, USBD_ERR_BABBLE);
	}

	if (REBASE(DWC_OTG_DOEPxINT, ep_num) & DWC_OTG_DOEPINT_XFRC) {
		REBASE(DWC_OTG_DOEPxINT, ep_num) = DWC_OTG_DOEPINT_XFRC;

		LOGF_LN("Transfer Complete: endpoint 0x%"PRIx8, ep_addr);
		usbd_urb *urb = usbd_find_active_urb(dev, ep_addr);

		if (!ep_num && urb != NULL && dev->private_data.ep0tsiz_pktcnt) {
			/* We are still expecting data! */

			dev->private_data.ep0tsiz_pktcnt--;

			REBASE(DWC_OTG_DOEP0TSIZ) = DWC_OTG_DOEP0TSIZ_STUPCNT_3 |
						DWC_OTG_DOEP0TSIZ_PKTCNT_1 |
						DWC_OTG_DOEP0TSIZ_XFRSIZ(urb->transfer.ep_size);

			REBASE(DWC_OTG_DOEP0CTL) |= DWC_OTG_DOEP0CTL_EPENA;
		} else {
			/* Set NAK on the endpoint */
			REBASE(DWC_OTG_DOEPxCTL, ep_num) |= DWC_OTG_DOEPCTL_SNAK;

			/* Disable Interrupt (if has no setup packet) */
			if (!(REBASE(DWC_OTG_DOEPxINT, ep_num) & DWC_OTG_DOEPINT_STUP)) {
				REBASE(DWC_OTG_DAINTMSK) &= ~DWC_OTG_DAINTMSK_OEPM(ep_num);
			}

			/* The URB has been processed, do the callback */
			if (urb != NULL) {
				usbd_urb_complete(dev, urb, USBD_SUCCESS);
			}
		}
	}

	if (REBASE(DWC_OTG_DOEPxINT, ep_num) & DWC_OTG_DOEPINT_STUP) {
		LOGF_LN("Setup phase done for endpoint 0x%"PRIx8, ep_addr);
		REBASE(DWC_OTG_DOEPxINT, ep_num) = DWC_OTG_DOEPINT_STUP;

		REBASE(DWC_OTG_DOEPxTSIZ, ep_num) |= DWC_OTG_DOEPTSIZ_STUPCNT_3;
		usbd_handle_setup(dev, ep_num, &dev->private_data.setup_data);
	}

	if (REBASE(DWC_OTG_DOEPxINT, ep_num) & DWC_OTG_DOEPINT_OTEPDIS) {
		REBASE(DWC_OTG_DOEPxINT, ep_num) = DWC_OTG_DOEPINT_OTEPDIS;
		LOGF_LN("Data OUT Token received when endpoint 0x%"PRIx8" was disable",
			ep_addr);
	}
}

/**
 * Allocate the (fully) FIFO for EP0 only.
 * This will divide the FIFO into two equal part.
 *  half for for EP0 IN and other half for EP0 OUT
 * @param[in] dev USB Device
 * @note this could have been done by calling usbd_ep_prepare_start() and
 *   then usbd_ep_prepare_end(). This simply does that!
 */
static inline void alloc_fifo_for_ep0_only(usbd_device *dev)
{
	uint16_t fifo_depth = get_fifo_depth(dev);
	uint16_t fifo_depth_2 = fifo_depth / 2;
	REBASE(DWC_OTG_GRXFSIZ) = fifo_depth_2;
	REBASE(DWC_OTG_DIEP0TXF) = DWC_OTG_DIEP0TXF_TX0FD(fifo_depth_2) |
								DWC_OTG_DIEP0TXF_TX0FSA(fifo_depth_2);
}

void dwc_otg_poll(usbd_device *dev)
{
	if (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_ENUMDNE) {
		REBASE(DWC_OTG_DCFG) &= ~DWC_OTG_DCFG_DAD_MASK;
		disable_all_non_ep0(dev);
		alloc_fifo_for_ep0_only(dev);
		flush_fifo(dev);
		REBASE(DWC_OTG_DAINTMSK) = DWC_OTG_DAINTMSK_OEPM(0);
		REBASE(DWC_OTG_DOEPxINT, 0) = 0xFFFF;
		REBASE(DWC_OTG_DOEP0TSIZ) = DWC_OTG_DOEP0TSIZ_STUPCNT_3;
		REBASE(DWC_OTG_DIEPxINT, 0) = 0xFFFF;
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_ENUMDNE;
		usbd_handle_reset(dev);
		return;
	}

	/* process endpoint RX data */
	while (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_RXFLVL) {
		handle_rxflvl_interrupt(dev);
	}

	if (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_OEPINT) {
		unsigned i;
		uint32_t daint = REBASE(DWC_OTG_DAINT);

		/* DAINT set bits are not cleared
		 *  when crossponding DAINTMSK bit is cleared */
		daint &= REBASE(DWC_OTG_DAINTMSK);

		for (i = 0; i < get_ep_count(dev); i++) {
			if (daint & DWC_OTG_DAINT_OEPINT(i)) {
				process_out_endpoint_interrupt(dev, i);
			}
		}
	}

	if (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_IEPINT) {
		unsigned i;
		uint32_t daint = REBASE(DWC_OTG_DAINT);

		/* DAINT set bits are not cleared
		 *  when crossponding DAINTMSK bit is cleared */
		daint &= REBASE(DWC_OTG_DAINTMSK);

		for (i = 0; i < get_ep_count(dev); i++) {
			if (daint & DWC_OTG_DAINT_IEPINT(i)) {
				process_in_endpoint_interrupt(dev, i);
			}
		}
	}

	if (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_USBSUSP) {
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_USBSUSP;
		usbd_handle_suspend(dev);
	}

	if (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_WKUPINT) {
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_WKUPINT;
		usbd_handle_resume(dev);
	}

	if (REBASE(DWC_OTG_GINTSTS) & DWC_OTG_GINTSTS_SOF) {
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_SOF;
		usbd_handle_sof(dev);
	}
}

void dwc_otg_disconnect(usbd_device *dev, bool disconnected)
{
	if (disconnected) {
		REBASE(DWC_OTG_DCTL) |= DWC_OTG_DCTL_SDIS;
	} else {
		REBASE(DWC_OTG_DCTL) &= ~DWC_OTG_DCTL_SDIS;
	}
}

void dwc_otg_enable_sof(usbd_device *dev, bool enable)
{
	if (enable) {
		REBASE(DWC_OTG_GINTMSK) |= DWC_OTG_GINTMSK_SOFM;
	} else {
		REBASE(DWC_OTG_GINTMSK) &= ~DWC_OTG_GINTMSK_SOFM;
	}
}

uint16_t dwc_otg_frame_number(usbd_device *dev)
{
	return DWC_OTG_DSTS_FNSOF_GET(REBASE(DWC_OTG_DSTS));
}

usbd_speed dwc_otg_get_speed(usbd_device *dev)
{
	/* These values are only valid after enumeration done */
	switch (REBASE(DWC_OTG_DSTS) & DWC_OTG_DSTS_ENUMSPD_MASK) {
	case DWC_OTG_DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ:
	return USBD_SPEED_HIGH;
	case DWC_OTG_DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ:
	case DWC_OTG_DSTS_ENUMSPD_FS_PHY_48MHZ:
	return USBD_SPEED_FULL;
	case DWC_OTG_DSTS_ENUMSPD_LS_PHY_6MHZ:
	return USBD_SPEED_LOW;
	}

	/* Keep compiler happy! */
	return USBD_SPEED_UNKNOWN;
}
