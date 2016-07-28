/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2011 Gareth McMullin <gareth@blacksphere.co.nz>
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

#include <string.h>
#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/tools.h>
#include <unicore-mx/usbd/usbd.h>
#include "../usbd_private.h"
#include "dwc_otg_private.h"

/* The FS core and the HS core have the same register layout.
 * As the code can be used on both cores, the registers offset is modified
 * according to the selected cores base address. */
#define private_data ((struct dwc_otg_private_data *) (usbd_dev->backend_data))
#define dev_base_address 	(private_data->base_address)

/* __VA_ARGS__ are the optional argument for the register
 *   ex: endpoint number */
#define REBASE(REG, ...)	REG(dev_base_address, ##__VA_ARGS__)
#define REBASE_FIFO(x)		DWC_OTG_FIFO(dev_base_address, x)

void dwc_otg_init(usbd_device *usbd_dev)
{
	unsigned i;
	uint16_t ep_msk;

	REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_MMIS;

	REBASE(DWC_OTG_GUSBCFG) |= DWC_OTG_GUSBCFG_PHYSEL;
	/* Enable VBUS sensing in device mode and power down the PHY. */
	REBASE(DWC_OTG_GCCFG) |= DWC_OTG_GCCFG_VBUSBSEN | DWC_OTG_GCCFG_PWRDWN;

	/* Wait for AHB idle. */
	while (!(REBASE(DWC_OTG_GRSTCTL) & DWC_OTG_GRSTCTL_AHBIDL));
	/* Do core soft reset. */
	REBASE(DWC_OTG_GRSTCTL) |= DWC_OTG_GRSTCTL_CSRST;
	while (REBASE(DWC_OTG_GRSTCTL) & DWC_OTG_GRSTCTL_CSRST);

	/* Force peripheral only mode. */
	REBASE(DWC_OTG_GUSBCFG) |= DWC_OTG_GUSBCFG_FDMOD | DWC_OTG_GUSBCFG_TRDT_MASK;

	/* Full speed device. */
	REBASE(DWC_OTG_DCFG) |= DWC_OTG_DCFG_DSPD;

	/* Restart the PHY clock. */
	REBASE(DWC_OTG_PCGCCTL) = 0;

	REBASE(DWC_OTG_GRXFSIZ) = private_data->rx_fifo_size;

	/* Unmask interrupts for TX and RX. */
	REBASE(DWC_OTG_GAHBCFG) |= DWC_OTG_GAHBCFG_GINT;
	REBASE(DWC_OTG_GINTMSK) = DWC_OTG_GINTMSK_ENUMDNEM |
					DWC_OTG_GINTMSK_RXFLVLM |
					DWC_OTG_GINTMSK_IEPINT |
					DWC_OTG_GINTMSK_USBSUSPM |
					DWC_OTG_GINTMSK_WUIM;

	/* generate mask for endpoints */
	ep_msk = 0x00;
	for (i = 0; i < private_data->ep_count; i++) {
		ep_msk |= (1 << i);
	}

	REBASE(DWC_OTG_DAINTMSK) = ep_msk;
	REBASE(DWC_OTG_DIEPMSK) = DWC_OTG_DIEPMSK_XFRCM;
}

void dwc_otg_set_address(usbd_device *usbd_dev, uint8_t addr)
{
	REBASE(DWC_OTG_DCFG) = (REBASE(DWC_OTG_DCFG) & ~DWC_OTG_DCFG_DAD_MASK) |
							DWC_OTG_DCFG_DAD(addr);
}

void dwc_otg_ep_setup(usbd_device *usbd_dev, uint8_t addr, uint8_t type,
			uint16_t max_size)
{
	/*
	 * Configure endpoint address and type. Allocate FIFO memory for
	 * endpoint. Install callback funciton.
	 */
	uint8_t dir = addr & 0x80;
	addr &= 0x7f;

	if (!addr) { /* For the default control endpoint */
		/* Configure IN part. */
		if (max_size >= 64) {
			REBASE(DWC_OTG_DIEP0CTL) = DWC_OTG_DIEP0CTL_MPSIZ_64;
		} else if (max_size >= 32) {
			REBASE(DWC_OTG_DIEP0CTL) = DWC_OTG_DIEP0CTL_MPSIZ_32;
		} else if (max_size >= 16) {
			REBASE(DWC_OTG_DIEP0CTL) = DWC_OTG_DIEP0CTL_MPSIZ_16;
		} else {
			REBASE(DWC_OTG_DIEP0CTL) = DWC_OTG_DIEP0CTL_MPSIZ_8;
		}

		REBASE(DWC_OTG_DIEP0TSIZ) = (max_size & DWC_OTG_DIEP0SIZ_XFRSIZ_MASK);
		REBASE(DWC_OTG_DIEP0CTL) |= DWC_OTG_DIEP0CTL_EPENA | DWC_OTG_DIEP0CTL_SNAK;

		/* Configure OUT part. */
		private_data->doeptsiz[0] = DWC_OTG_DIEP0SIZ_STUPCNT_1 |
									DWC_OTG_DIEP0SIZ_PKTCNT |
									(max_size & DWC_OTG_DIEP0SIZ_XFRSIZ_MASK);
		REBASE(DWC_OTG_DOEP0TSIZ) = private_data->doeptsiz[0];
		REBASE(DWC_OTG_DOEP0CTL) |= DWC_OTG_DOEP0CTL_EPENA | DWC_OTG_DIEP0CTL_SNAK;

		REBASE(DWC_OTG_GNPTXFSIZ) = ((max_size / 4) << 16) |
								private_data->rx_fifo_size;
		private_data->fifo_mem_top += max_size / 4;
		private_data->fifo_mem_top_ep0 = private_data->fifo_mem_top;

		return;
	}

	if (dir) {
		REBASE(DWC_OTG_DIEPxTXF, addr) = ((max_size / 4) << 16) |
									private_data->fifo_mem_top;
		private_data->fifo_mem_top += max_size / 4;

		REBASE(DWC_OTG_DIEPxTSIZ, addr) = (max_size & DWC_OTG_DIEPSIZ_XFRSIZ_MASK);
		REBASE(DWC_OTG_DIEPxCTL, addr) |=  /* FIXME: confirm |= is correct? (should it be =) */
					DWC_OTG_DIEPCTL_EPENA | DWC_OTG_DIEPCTL_SNAK |
					DWC_OTG_DIEPCTL_EPTYP(type) | DWC_OTG_DIEPCTL_USBAEP |
					DWC_OTG_DIEPCTL_SD0PID | DWC_OTG_DIEPCTL_TXFNUM(addr) |
					DWC_OTG_DIEPCTL_MPSIZ(max_size);
	}

	if (!dir) {
		private_data->doeptsiz[addr] = DWC_OTG_DIEPSIZ_PKTCNT(1) |
									(max_size & DWC_OTG_DIEPSIZ_XFRSIZ_MASK);
		REBASE(DWC_OTG_DOEPxTSIZ, addr) = private_data->doeptsiz[addr];
		REBASE(DWC_OTG_DOEPxCTL, addr) |= /* FIXME: confirm |= is correct? (should it be =) */
					DWC_OTG_DOEPCTL_EPENA | DWC_OTG_DOEPCTL_USBAEP |
					DWC_OTG_DIEPCTL_CNAK | DWC_OTG_DOEPCTL_SD0PID |
					DWC_OTG_DOEPCTL_EPTYP(type) | DWC_OTG_DOEPCTL_MPSIZ(max_size);
	}
}

void dwc_otg_set_ep_type(usbd_device *usbd_dev, uint8_t addr, uint8_t type)
{
	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	if (!addr) {
		/* EP0 is hardwired to Control endpoint */
		return;
	}

	if (dir) {
		uint32_t diepctl = REBASE(DWC_OTG_DIEPxCTL, addr);
		diepctl &= ~DWC_OTG_DIEPCTL_EPTYP_MASK;
		diepctl |= DWC_OTG_DIEPCTL_EPTYP(type);
		REBASE(DWC_OTG_DIEPxCTL, addr) = diepctl;
	} else {
		uint32_t doepctl = REBASE(DWC_OTG_DOEPxCTL, addr);
		doepctl &= ~DWC_OTG_DOEPCTL_EPTYP_MASK;
		doepctl |= DWC_OTG_DOEPCTL_EPTYP(type);
		REBASE(DWC_OTG_DOEPxCTL, addr) = doepctl;
	}
}

void dwc_otg_set_ep_size(usbd_device *usbd_dev, uint8_t addr, uint16_t max_size)
{
	/*
	 * Configure endpoint address and type. Allocate FIFO memory for
	 * endpoint. Install callback funciton.
	 */
	uint8_t dir = addr & 0x80;
	addr &= 0x7f;

	if (!addr) {
		/* EP0 should not be modified after initalized */
		return;
	}

	if (dir) {
		/* only update INEPTXFD */
		uint32_t dieptxf = REBASE(DWC_OTG_DIEPxTXF, addr);
		dieptxf &= 0x0000FFFF;
		dieptxf |= ((max_size / 4) << 16);
		REBASE(DWC_OTG_DIEPxTXF, addr) = dieptxf;

		/* update XFRSIZ */
		REBASE(DWC_OTG_DIEPxTSIZ, addr) = (max_size & DWC_OTG_DIEPSIZ_XFRSIZ_MASK);

		/* only update MPSIZ */
		uint32_t diepctl = REBASE(DWC_OTG_DIEPxCTL, addr);
		diepctl &= ~0x3FF;
		diepctl |= max_size;
		REBASE(DWC_OTG_DIEPxCTL, addr) = diepctl;
	} else {
		/* update XFRSIZ */
		uint32_t doeptsiz = private_data->doeptsiz[addr];
		doeptsiz &= DWC_OTG_DIEPSIZ_XFRSIZ_MASK;
		doeptsiz |= (max_size & DWC_OTG_DIEPSIZ_XFRSIZ_MASK);
		private_data->doeptsiz[addr] = doeptsiz;
		REBASE(DWC_OTG_DOEPxTSIZ, addr) = doeptsiz;

		/* update MPSIZ */
		uint32_t doepctl = REBASE(DWC_OTG_DOEPxCTL, addr);
		doepctl &= ~0x3FF;
		doepctl |= max_size;
	}
}

void dwc_otg_endpoints_reset(usbd_device *usbd_dev)
{
	/* The core resets the endpoints automatically on reset. */
	private_data->fifo_mem_top = private_data->fifo_mem_top_ep0;
}

void dwc_otg_set_ep_stall(usbd_device *usbd_dev, uint8_t addr, bool stall)
{
	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	if (!addr) {
		if (stall) {
			REBASE(DWC_OTG_DIEP0CTL) |= DWC_OTG_DIEP0CTL_STALL;
		} else {
			REBASE(DWC_OTG_DIEP0CTL) &= ~DWC_OTG_DIEP0CTL_STALL;
		}
		/* FIXME: return required here? */
	}

	if (dir) {
		if (stall) {
			REBASE(DWC_OTG_DIEPxCTL, addr) |= DWC_OTG_DIEPCTL_STALL;
		} else {
			REBASE(DWC_OTG_DIEPxCTL, addr) &= ~DWC_OTG_DIEPCTL_STALL;
			REBASE(DWC_OTG_DIEPxCTL, addr) |= DWC_OTG_DIEPCTL_SD0PID;
		}
	} else {
		if (stall) {
			REBASE(DWC_OTG_DOEPxCTL, addr) |= DWC_OTG_DOEPCTL_STALL;
		} else {
			REBASE(DWC_OTG_DOEPxCTL, addr) &= ~DWC_OTG_DOEPCTL_STALL;
			REBASE(DWC_OTG_DOEPxCTL, addr) |= DWC_OTG_DOEPCTL_SD0PID;
		}
	}
}

bool dwc_otg_get_ep_stall(usbd_device *usbd_dev, uint8_t addr)
{
	uint8_t dir = 0x80;
	addr &= 0x7F;

	/* Return non-zero if STALL set. */
	if (dir) {
		return !!(REBASE(DWC_OTG_DIEPxCTL, addr) & DWC_OTG_DIEPCTL_STALL);
	} else {
		return !!(REBASE(DWC_OTG_DOEPxCTL, addr) & DWC_OTG_DOEPCTL_STALL);
	}
}

void dwc_otg_set_ep_nak(usbd_device *usbd_dev, uint8_t addr, bool nak)
{
	/* It does not make sence to force NAK on IN endpoints. */
	if (addr & 0x80) {
		return;
	}

	if (nak) {
		private_data->force_nak |= 1 << addr;
		REBASE(DWC_OTG_DOEPxCTL, addr) |= DWC_OTG_DOEPCTL_SNAK;
	} else {
		private_data->force_nak &= ~(1 << addr);
		REBASE(DWC_OTG_DOEPxCTL, addr) |= DWC_OTG_DOEPCTL_CNAK;
	}
}

uint16_t dwc_otg_ep_write_packet(usbd_device *usbd_dev, uint8_t addr,
								const void *buf, uint16_t len)
{
	const uint32_t *buf32 = buf;
	int i;

	/* Return if endpoint is already enabled. */
	if (REBASE(DWC_OTG_DIEPxTSIZ, addr) & DWC_OTG_DIEPSIZ_PKTCNT_MASK) {
		return 0;
	}

	/* Enable endpoint for transmission. */
	REBASE(DWC_OTG_DIEPxTSIZ, addr) = DWC_OTG_DIEPSIZ_PKTCNT(1) | len;
	REBASE(DWC_OTG_DIEPxCTL, addr) |= DWC_OTG_DIEPCTL_EPENA | DWC_OTG_DIEPCTL_CNAK;
	volatile uint32_t *fifo = REBASE_FIFO(addr);

	/* Copy buffer to endpoint FIFO, note - memcpy does not work */
	for (i = len; i > 0; i -= 4) {
		*fifo++ = *buf32++;
	}

	return len;
}

uint16_t dwc_otg_ep_read_packet(usbd_device *usbd_dev, uint8_t addr,
				  void *buf, uint16_t len)
{
	int i;
	uint32_t *buf32 = buf;
	uint32_t extra;

	len = MIN(len, private_data->rxbcnt);
	private_data->rxbcnt -= len;

	volatile uint32_t *fifo = REBASE_FIFO(addr);
	for (i = len; i >= 4; i -= 4) {
		*buf32++ = *fifo++;
	}

	if (i) {
		extra = *fifo++;
		memcpy(buf32, &extra, i);
	}

	REBASE(DWC_OTG_DOEPxTSIZ, addr) = private_data->doeptsiz[addr];
	REBASE(DWC_OTG_DOEPxCTL, addr) |= DWC_OTG_DOEPCTL_EPENA |
								((private_data->force_nak & (1 << addr)) ?
										DWC_OTG_DOEPCTL_SNAK : DWC_OTG_DOEPCTL_CNAK);

	return len;
}

void dwc_otg_poll(usbd_device *usbd_dev)
{
	/* Read interrupt status register. */
	uint32_t intsts = REBASE(DWC_OTG_GINTSTS);
	int i;

	if (intsts & DWC_OTG_GINTSTS_ENUMDNE) {
		/* Handle USB RESET condition. */
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_ENUMDNE;
		private_data->fifo_mem_top = private_data->rx_fifo_size;
		USBD_INVOKE_RESET_CALLBACK(usbd_dev)
		return;
	}

	/* Note: RX and TX handled differently in this device. */
	if (intsts & DWC_OTG_GINTSTS_RXFLVL) {
		/* Receive FIFO non-empty. */
		uint32_t rxstsp = REBASE(DWC_OTG_GRXSTSP);
		uint32_t pktsts = rxstsp & DWC_OTG_GRXSTSP_PKTSTS_MASK;
		if ((pktsts != DWC_OTG_GRXSTSP_PKTSTS_OUT) &&
			(pktsts != DWC_OTG_GRXSTSP_PKTSTS_SETUP)) {
			return;
		}

		uint8_t ep = rxstsp & DWC_OTG_GRXSTSP_EPNUM_MASK;
		uint8_t type;
		if (pktsts == DWC_OTG_GRXSTSP_PKTSTS_SETUP) {
			type = USB_CALLBACK_SETUP;
		} else {
			type = USB_CALLBACK_OUT;
		}

		/* Save packet size for dwc_otg_ep_read_packet(). */
		private_data->rxbcnt = (rxstsp & DWC_OTG_GRXSTSP_BCNT_MASK) >> 4;

		/*
		 * FIXME: Why is a delay needed here?
		 * This appears to fix a problem where the first 4 bytes
		 * of the DATA OUT stage of a control transaction are lost.
		 */
		for (i = 0; i < 1000; i++) {
			__asm__("nop");
		}

		USBD_INVOKE_EP_CALLBACK(usbd_dev, type, ep)

		/* Discard unread packet data. */
		for (i = 0; i < private_data->rxbcnt; i += 4) {
			(void)*REBASE_FIFO(ep);
		}

		private_data->rxbcnt = 0;
	}

	/*
	 * There is no global interrupt flag for transmit complete.
	 * The XFRC bit must be checked in each DWC_OTG_DIEPxINT(x).
	 */
	for (i = 0; i < private_data->ep_count; i++) { /* Iterate over endpoints. */
		if (REBASE(DWC_OTG_DIEPxINT, i) & DWC_OTG_DIEPINT_XFRC) {
			/* Transfer complete. */
			USBD_INVOKE_EP_CALLBACK(usbd_dev, USB_CALLBACK_IN, i)

			REBASE(DWC_OTG_DIEPxINT, i) = DWC_OTG_DIEPINT_XFRC;
		}
	}

	if (intsts & DWC_OTG_GINTSTS_USBSUSP) {
		USBD_INVOKE_SUSPEND_CALLBACK(usbd_dev)
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_USBSUSP;
	}

	if (intsts & DWC_OTG_GINTSTS_WKUPINT) {
		USBD_INVOKE_RESUME_CALLBACK(usbd_dev)
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_WKUPINT;
	}

	if (intsts & DWC_OTG_GINTSTS_SOF) {
		USBD_INVOKE_SOF_CALLBACK(usbd_dev)
		REBASE(DWC_OTG_GINTSTS) = DWC_OTG_GINTSTS_SOF;
	}
}

void dwc_otg_disconnect(usbd_device *usbd_dev, bool disconnected)
{
	if (disconnected) {
		REBASE(DWC_OTG_DCTL) |= DWC_OTG_DCTL_SDIS;
	} else {
		REBASE(DWC_OTG_DCTL) &= ~DWC_OTG_DCTL_SDIS;
	}
}

void dwc_otg_enable_sof(usbd_device *usbd_dev, bool enable)
{
	if (enable) {
		REBASE(DWC_OTG_GINTMSK) |= DWC_OTG_GINTMSK_SOFM;
	} else {
		REBASE(DWC_OTG_GINTMSK) &= ~DWC_OTG_GINTMSK_SOFM;
	}
}

void dwc_otg_ep_flush(usbd_device *usbd_dev, uint8_t addr)
{
	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	if (!addr) {
		/* operation is not supported (since we cannot disable the endpoint) */
		return;
	}

	if (dir) {
		if (REBASE(DWC_OTG_DIEPxCTL, addr) & DWC_OTG_DIEPCTL_EPENA) {
			REBASE(DWC_OTG_DIEPxCTL, addr) |= DWC_OTG_DIEPCTL_EPDIS;

			/* wait till the endpoint is not disabled
			 *  FIXME: busy loop */
			while (!(REBASE(DWC_OTG_DIEPxINT, addr) & DWC_OTG_DIEPINT_EPDISD));
		}

		REBASE(DWC_OTG_DIEPxCTL, addr) |= DWC_OTG_DIEPCTL_SNAK;
		REBASE(DWC_OTG_DIEPxTSIZ, addr) = 0;
	} else {
		/* not possible due to the design of FIFO */
	}
}

void dwc_otg_set_ep_dtog(usbd_device *usbd_dev, uint8_t addr, bool dtog)
{
	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	if (!addr) {
		/* invalid request!
		 * ep0 dtog should not be messed up. */
		return;
	}

	if (dir) {
		REBASE(DWC_OTG_DIEPxCTL, addr) |=
			dtog ? DWC_OTG_DIEPCTL_SD1PID : DWC_OTG_DIEPCTL_SD0PID;
	} else {
		REBASE(DWC_OTG_DOEPxCTL, addr) |=
			dtog ? DWC_OTG_DOEPCTL_SD1PID : DWC_OTG_DOEPCTL_SD0PID;
	}
}

bool dwc_otg_get_ep_dtog(usbd_device *usbd_dev, uint8_t addr)
{
	uint8_t dir = addr & 0x80;
	addr &= 0x7F;

	/* for endpoint0, OUT DPID is reserved.
	 *   so instead returning IN DPID for both IN,OUT for ep0 */
	if (!addr) {
		dir = 0x80;
	}

	if (dir) {
		return !!(REBASE(DWC_OTG_DIEPxCTL, addr) & DWC_OTG_DIEPCTL_DPID);
	} else {
		return !!(REBASE(DWC_OTG_DOEPxCTL, addr) & DWC_OTG_DOEPCTL_DPID);
	}
}

uint16_t dwc_otg_frame_number(usbd_device *usbd_dev)
{
	(void)usbd_dev;
	return (REBASE(DWC_OTG_DSTS) & DWC_OTG_DSTS_FNSOF_MASK)
				>> DWC_OTG_DSTS_FNSOF_SHIFT;
}
