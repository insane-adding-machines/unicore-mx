/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
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
 * This file is intended to be included by either DWC_OTG_hs.h or DWC_OTG_fs.h
 * Contains common definitions of Command and Status Registers (CSR) and their
 * bit definitions.
 */

#ifndef UNICOREMX_USBD_BACKEND_DWC_OTG_H
#define UNICOREMX_USBD_BACKEND_DWC_OTG_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/usbd/usbd.h>

/* Core Global Control and Status Registers */
#define DWC_OTG_GOTGCTL(base)		MMIO32((base) + 0x000)
#define DWC_OTG_GOTGINT(base)		MMIO32((base) + 0x004)
#define DWC_OTG_GAHBCFG(base)		MMIO32((base) + 0x008)
#define DWC_OTG_GUSBCFG(base)		MMIO32((base) + 0x00C)
#define DWC_OTG_GRSTCTL(base)		MMIO32((base) + 0x010)
#define DWC_OTG_GINTSTS(base)		MMIO32((base) + 0x014)
#define DWC_OTG_GINTMSK(base)		MMIO32((base) + 0x018)
#define DWC_OTG_GRXSTSR(base)		MMIO32((base) + 0x01C)
#define DWC_OTG_GRXSTSP(base)		MMIO32((base) + 0x020)
#define DWC_OTG_GRXFSIZ(base)		MMIO32((base) + 0x024)
#define DWC_OTG_GNPTXFSIZ(base)		MMIO32((base) + 0x028)
#define DWC_OTG_GNPTXSTS(base)		MMIO32((base) + 0x02C)
#define DWC_OTG_GCCFG(base)			MMIO32((base) + 0x038)
#define DWC_OTG_CID(base)			MMIO32((base) + 0x03C)
#define DWC_OTG_HPTXFSIZ(base)		MMIO32((base) + 0x100)
#define DWC_OTG_DIEPxTXF(base, x)	MMIO32((base) + 0x104 + 4*((x)-1))

/* Host-mode Control and Status Registers */
#define DWC_OTG_HCFG(base)				MMIO32((base) + 0x400)
#define DWC_OTG_HFIR(base)				MMIO32((base) + 0x404)
#define DWC_OTG_HFNUM(base)				MMIO32((base) + 0x408)
#define DWC_OTG_HPTXSTS(base)			MMIO32((base) + 0x410)
#define DWC_OTG_HAINT(base)				MMIO32((base) + 0x414)
#define DWC_OTG_HAINTMSK(base)			MMIO32((base) + 0x418)
#define DWC_OTG_HPRT(base)				MMIO32((base) + 0x440)
#define DWC_OTG_HCxCHAR(base, x)		MMIO32((base) + 0x500 + 0x20*(x))
#define DWC_OTG_HCxSPLT(base, x)		MMIO32((base) + 0x504 + 0x20*(x))
#define DWC_OTG_HCxINT(base, x)			MMIO32((base) + 0x508 + 0x20*(x))
#define DWC_OTG_HCxINTMSK(base, x)		MMIO32((base) + 0x50C + 0x20*(x))
#define DWC_OTG_HCxTSIZ(base, x)		MMIO32((base) + 0x510 + 0x20*(x))
#define DWC_OTG_HCxDMA(base, x)			MMIO32((base) + 0x514 + 0x20*(x))


/* Device-mode Control and Status Registers */
#define DWC_OTG_DCFG(base)				MMIO32((base) + 0x800)
#define DWC_OTG_DCTL(base)				MMIO32((base) + 0x804)
#define DWC_OTG_DSTS(base)				MMIO32((base) + 0x808)
#define DWC_OTG_DIEPMSK(base)			MMIO32((base) + 0x810)
#define DWC_OTG_DOEPMSK(base)			MMIO32((base) + 0x814)
#define DWC_OTG_DAINT(base)				MMIO32((base) + 0x818)
#define DWC_OTG_DAINTMSK(base)			MMIO32((base) + 0x81C)
#define DWC_OTG_DVBUSDIS(base)			MMIO32((base) + 0x828)
#define DWC_OTG_DVBUSPULSE(base)		MMIO32((base) + 0x82C)
#define DWC_OTG_DIEPEMPMSK(base)		MMIO32((base) + 0x834)
#define DWC_OTG_DEACHHINT				MMIO32((base) + 0x838)
#define DWC_OTG_DEACHHINTMSK			MMIO32((base) + 0x83C)
#define DWC_OTG_DIEPEACHMSK1			MMIO32((base) + 0x844)
#define DWC_OTG_DOEPEACHMSK1			MMIO32((base) + 0x884)

#define DWC_OTG_DIEPxCTL(base, x)			MMIO32((base) + 0x900 + 0x20*(x))
#define DWC_OTG_DIEPxINT(base, x)			MMIO32((base) + 0x908 + 0x20*(x))
#define DWC_OTG_DIEPxTSIZ(base, x)			MMIO32((base) + 0x910 + 0x20*(x))
#define DWC_OTG_DIEPxDMA(x)					MMIO32((base) + 0x914 + 0x20*(x))
#define DWC_OTG_DIEPxTXFSTS(base, x)		MMIO32((base) + 0x918 + 0x20*(x))

#define DWC_OTG_DOEPxCTL(base, x)			MMIO32((base) + 0xB00 + 0x20*(x))
#define DWC_OTG_DOEPxINT(base, x)			MMIO32((base) + 0xB08 + 0x20*(x))
#define DWC_OTG_DOEPxTSIZ(base, x)			MMIO32((base) + 0xB10 + 0x20*(x))
#define DWC_OTG_DOEPxDMA(x)					MMIO32((base) + 0xB14 + 0x20*(x))

/* Endpoint0 (a special endpoint)
 * same as other endpoint but:
 *  - Type can only be Control
 *  - Size can be {8, 16, 32, 64}
 */
#define DWC_OTG_DIEP0CTL(base)		DWC_OTG_DIEPxCTL(base, 0)
#define DWC_OTG_DIEP0TSIZ(base)		DWC_OTG_DIEPxTSIZ(base, 0)
#define DWC_OTG_DOEP0CTL(base)		DWC_OTG_DOEPxCTL(base, 0)
#define DWC_OTG_DOEP0TSIZ(base)		DWC_OTG_DOEPxCTL(base, 0)

/* Power and clock gating control and status register */
#define DWC_OTG_PCGCCTL(base)		MMIO32((base) + 0xE00)

/* Data FIFO */
#define DWC_OTG_FIFO(base, x)		(&MMIO32((base) + (((x) + 1) << 12)))


/* Global CSRs */
/* OTG USB control registers (DWC_OTG_GOTGCTL) */
#define DWC_OTG_GOTGCTL_BSVLD		(1 << 19)
#define DWC_OTG_GOTGCTL_ASVLD		(1 << 18)
#define DWC_OTG_GOTGCTL_DBCT		(1 << 17)
#define DWC_OTG_GOTGCTL_CIDSTS		(1 << 16)
#define DWC_OTG_GOTGCTL_DHNPEN		(1 << 11)
#define DWC_OTG_GOTGCTL_HSHNPEN		(1 << 10)
#define DWC_OTG_GOTGCTL_HNPRQ		(1 << 9)
#define DWC_OTG_GOTGCTL_HNGSCS		(1 << 8)
#define DWC_OTG_GOTGCTL_SRQ			(1 << 1)
#define DWC_OTG_GOTGCTL_SRQSCS		(1 << 0)

/* OTG AHB configuration register (DWC_OTG_GAHBCFG) */
#define DWC_OTG_GAHBCFG_GINT			0x0001
#define DWC_OTG_GAHBCFG_TXFELVL			0x0080
#define DWC_OTG_GAHBCFG_PTXFELVL			0x0100

/* OTG USB configuration register (DWC_OTG_GUSBCFG) */
#define DWC_OTG_GUSBCFG_TOCAL			0x00000003
#define DWC_OTG_GUSBCFG_SRPCAP			0x00000100
#define DWC_OTG_GUSBCFG_HNPCAP			0x00000200
#define DWC_OTG_GUSBCFG_TRDT_MASK		(0xf << 10)
#define DWC_OTG_GUSBCFG_TRDT_16BIT		(0x5 << 10)
#define DWC_OTG_GUSBCFG_TRDT_8BIT		(0x9 << 10)
#define DWC_OTG_GUSBCFG_NPTXRWEN		0x00004000
#define DWC_OTG_GUSBCFG_FHMOD			0x20000000
#define DWC_OTG_GUSBCFG_FDMOD			0x40000000
#define DWC_OTG_GUSBCFG_CTXPKT			0x80000000
#define DWC_OTG_GUSBCFG_PHYSEL			(1 << 6)

/* OTG reset register (DWC_OTG_GRSTCTL) */
#define DWC_OTG_GRSTCTL_AHBIDL			(1 << 31)
/* Bits 30:11 - Reserved */
#define DWC_OTG_GRSTCTL_TXFNUM_MASK		(0x1f << 6)
#define DWC_OTG_GRSTCTL_TXFFLSH			(1 << 5)
#define DWC_OTG_GRSTCTL_RXFFLSH			(1 << 4)
/* Bit 3 - Reserved */
#define DWC_OTG_GRSTCTL_FCRST			(1 << 2)
#define DWC_OTG_GRSTCTL_HSRST			(1 << 1)
#define DWC_OTG_GRSTCTL_CSRST			(1 << 0)

/* OTG interrupt status register (DWC_OTG_GINTSTS) */
#define DWC_OTG_GINTSTS_WKUPINT		(1 << 31)
#define DWC_OTG_GINTSTS_SRQINT		(1 << 30)
#define DWC_OTG_GINTSTS_DISCINT		(1 << 29)
#define DWC_OTG_GINTSTS_CIDSCHG		(1 << 28)
/* Bit 27 - Reserved */
#define DWC_OTG_GINTSTS_PTXFE		(1 << 26)
#define DWC_OTG_GINTSTS_HCINT		(1 << 25)
#define DWC_OTG_GINTSTS_HPRTINT		(1 << 24)
/* Bits 23:22 - Reserved */
#define DWC_OTG_GINTSTS_IPXFR			(1 << 21)
#define DWC_OTG_GINTSTS_INCOMPISOOUT	(1 << 21)
#define DWC_OTG_GINTSTS_IISOIXFR		(1 << 20)
#define DWC_OTG_GINTSTS_OEPINT			(1 << 19)
#define DWC_OTG_GINTSTS_IEPINT			(1 << 18)
/* Bits 17:16 - Reserved */
#define DWC_OTG_GINTSTS_EOPF			(1 << 15)
#define DWC_OTG_GINTSTS_ISOODRP			(1 << 14)
#define DWC_OTG_GINTSTS_ENUMDNE			(1 << 13)
#define DWC_OTG_GINTSTS_USBRST			(1 << 12)
#define DWC_OTG_GINTSTS_USBSUSP			(1 << 11)
#define DWC_OTG_GINTSTS_ESUSP			(1 << 10)
/* Bits 9:8 - Reserved */
#define DWC_OTG_GINTSTS_GONAKEFF		(1 << 7)
#define DWC_OTG_GINTSTS_GINAKEFF		(1 << 6)
#define DWC_OTG_GINTSTS_NPTXFE			(1 << 5)
#define DWC_OTG_GINTSTS_RXFLVL			(1 << 4)
#define DWC_OTG_GINTSTS_SOF				(1 << 3)
#define DWC_OTG_GINTSTS_OTGINT			(1 << 2)
#define DWC_OTG_GINTSTS_MMIS			(1 << 1)
#define DWC_OTG_GINTSTS_CMOD			(1 << 0)

/* OTG interrupt mask register (DWC_OTG_GINTMSK) */
#define DWC_OTG_GINTMSK_MMISM			0x00000002
#define DWC_OTG_GINTMSK_OTGINT			0x00000004
#define DWC_OTG_GINTMSK_SOFM			0x00000008
#define DWC_OTG_GINTMSK_RXFLVLM			0x00000010
#define DWC_OTG_GINTMSK_NPTXFEM			0x00000020
#define DWC_OTG_GINTMSK_GINAKEFFM		0x00000040
#define DWC_OTG_GINTMSK_GONAKEFFM		0x00000080
#define DWC_OTG_GINTMSK_ESUSPM			0x00000400
#define DWC_OTG_GINTMSK_USBSUSPM		0x00000800
#define DWC_OTG_GINTMSK_USBRST			0x00001000
#define DWC_OTG_GINTMSK_ENUMDNEM		0x00002000
#define DWC_OTG_GINTMSK_ISOODRPM		0x00004000
#define DWC_OTG_GINTMSK_EOPFM			0x00008000
#define DWC_OTG_GINTMSK_EPMISM			0x00020000
#define DWC_OTG_GINTMSK_IEPINT			0x00040000
#define DWC_OTG_GINTMSK_OEPINT			0x00080000
#define DWC_OTG_GINTMSK_IISOIXFRM		0x00100000
#define DWC_OTG_GINTMSK_IISOOXFRM		0x00200000
#define DWC_OTG_GINTMSK_IPXFRM			0x00200000
#define DWC_OTG_GINTMSK_PRTIM			0x01000000
#define DWC_OTG_GINTMSK_HCIM			0x02000000
#define DWC_OTG_GINTMSK_PTXFEM			0x04000000
#define DWC_OTG_GINTMSK_CIDSCHGM		0x10000000
#define DWC_OTG_GINTMSK_DISCINT			0x20000000
#define DWC_OTG_GINTMSK_SRQIM			0x40000000
#define DWC_OTG_GINTMSK_WUIM			0x80000000

/* OTG Receive Status Pop Register (DWC_OTG_GRXSTSP) */
/* Bits 31:25 - Reserved */
#define DWC_OTG_GRXSTSP_FRMNUM_MASK				(0xf << 21)
#define DWC_OTG_GRXSTSP_PKTSTS_MASK				(0xf << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_GOUTNAK			(0x1 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_OUT				(0x2 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_IN				(0x2 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_OUT_COMP			(0x3 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_IN_COMP			(0x3 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_SETUP_COMP		(0x4 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_DTERR			(0x5 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_SETUP			(0x6 << 17)
#define DWC_OTG_GRXSTSP_PKTSTS_CHH				(0x7 << 17)
#define DWC_OTG_GRXSTSP_DPID_MASK				(0x3 << 15)
#define DWC_OTG_GRXSTSP_DPID_DATA0				(0x0 << 15)
#define DWC_OTG_GRXSTSP_DPID_DATA1				(0x2 << 15)
#define DWC_OTG_GRXSTSP_DPID_DATA2				(0x1 << 15)
#define DWC_OTG_GRXSTSP_DPID_MDATA				(0x3 << 15)
#define DWC_OTG_GRXSTSP_BCNT_MASK				(0x7ff << 4)
#define DWC_OTG_GRXSTSP_EPNUM_MASK				(0xf << 0)

/* OTG general core configuration register (DWC_OTG_GCCFG) */
/* Bits 31:22 - Reserved */
#define DWC_OTG_GCCFG_NOVBUSSENS		(1 << 21)
#define DWC_OTG_GCCFG_SOFOUTEN			(1 << 20)
#define DWC_OTG_GCCFG_VBUSBSEN			(1 << 19)
#define DWC_OTG_GCCFG_VBUSASEN			(1 << 18)
/* Bit 17 - Reserved */
#define DWC_OTG_GCCFG_PWRDWN			(1 << 16)
/* Bits 15:0 - Reserved */

/* Device-mode CSRs */
/* OTG device control register (DWC_OTG_DCTL) */
/* Bits 31:12 - Reserved */
#define DWC_OTG_DCTL_POPRGDNE	(1 << 11)
#define DWC_OTG_DCTL_CGONAK		(1 << 10)
#define DWC_OTG_DCTL_SGONAK		(1 << 9)
#define DWC_OTG_DCTL_SGINAK		(1 << 8)
#define DWC_OTG_DCTL_TCTL_MASK	(7 << 4)
#define DWC_OTG_DCTL_GONSTS		(1 << 3)
#define DWC_OTG_DCTL_GINSTS		(1 << 2)
#define DWC_OTG_DCTL_SDIS		(1 << 1)
#define DWC_OTG_DCTL_RWUSIG		(1 << 0)

/* OTG device configuration register (DWC_OTG_DCFG) */
#define DWC_OTG_DCFG_DSPD			0x0003
#define DWC_OTG_DCFG_NZLSOHSK		0x0004

#define DWC_OTG_DCFG_DAD_SHIFT		(4)
#define DWC_OTG_DCFG_DAD_MASK		(0x7f << DWC_OTG_DCFG_DAD_SHIFT)
#define DWC_OTG_DCFG_DAD(v)			(((v) << DWC_OTG_DCFG_DAD_SHIFT) & \
										DWC_OTG_DCFG_DAD_MASK)

#define DWC_OTG_DCFG_PFIVL			0x1800

/* OTG Device IN Endpoint Common Interrupt Mask Register (DWC_OTG_DIEPMSK) */
/* Bits 31:10 - Reserved */
#define DWC_OTG_DIEPMSK_BIM			(1 << 9)
#define DWC_OTG_DIEPMSK_TXFURM		(1 << 8)
/* Bit 7 - Reserved */
#define DWC_OTG_DIEPMSK_INEPNEM		(1 << 6)
#define DWC_OTG_DIEPMSK_INEPNMM		(1 << 5)
#define DWC_OTG_DIEPMSK_ITTXFEMSK	(1 << 4)
#define DWC_OTG_DIEPMSK_TOM			(1 << 3)
/* Bit 2 - Reserved */
#define DWC_OTG_DIEPMSK_EPDM		(1 << 1)
#define DWC_OTG_DIEPMSK_XFRCM		(1 << 0)

/* OTG Device OUT Endpoint Common Interrupt Mask Register (DWC_OTG_DOEPMSK) */
/* Bits 31:10 - Reserved */
#define DWC_OTG_DOEPMSK_BOIM		(1 << 9)
#define DWC_OTG_DOEPMSK_OPEM		(1 << 8)
/* Bit 7 - Reserved */
#define DWC_OTG_DOEPMSK_B2BSTUP		(1 << 6)
/* Bit 5 - Reserved */
#define DWC_OTG_DOEPMSK_OTEPDM		(1 << 4)
#define DWC_OTG_DOEPMSK_STUPM		(1 << 3)
/* Bit 2 - Reserved */
#define DWC_OTG_DOEPMSK_EPDM		(1 << 1)
#define DWC_OTG_DOEPMSK_XFRCM		(1 << 0)

/* OTG Device Control IN Endpoint x Control Register (DWC_OTG_DIEPxCTL) */
#define DWC_OTG_DIEPCTL_EPENA				(1 << 31)
#define DWC_OTG_DIEPCTL_EPDIS				(1 << 30)
#define DWC_OTG_DIEPCTL_SD1PID				(1 << 29)
#define DWC_OTG_DIEPCTL_SODDFRM				DWC_OTG_DIEPCTL_SD1PID
#define DWC_OTG_DIEPCTL_SD0PID				(1 << 28)
#define DWC_OTG_DIEPCTL_SEVNFRM				DWC_OTG_DIEPCTL_SD0PID
#define DWC_OTG_DIEPCTL_SNAK				(1 << 27)
#define DWC_OTG_DIEPCTL_CNAK				(1 << 26)
#define DWC_OTG_DIEPCTL_TXFNUM_SHIFT		(22)
#define DWC_OTG_DIEPCTL_TXFNUM_MASK			(0xf << DWC_OTG_DIEPCTL_TXFNUM_SHIFT)
#define DWC_OTG_DIEPCTL_TXFNUM(v)			(((v) << DWC_OTG_DIEPCTL_TXFNUM_SHIFT) &\
												DWC_OTG_DIEPCTL_TXFNUM_MASK)
#define DWC_OTG_DIEPCTL_STALL			(1 << 21)
/* Bit 20 - Reserved */
#define DWC_OTG_DIEPCTL_EPTYP_SHIFT		(18)
#define DWC_OTG_DIEPCTL_EPTYP_MASK		(0x3 << DWC_OTG_DIEPCTL_EPTYP_SHIFT)
#define DWC_OTG_DIEPCTL_EPTYP(v)		(((v) << DWC_OTG_DIEPCTL_EPTYP_SHIFT) &\
											DWC_OTG_DIEPCTL_EPTYP_MASK)
#define DWC_OTG_DIEPCTL_NAKSTS			(1 << 17)
#define DWC_OTG_DIEPCTL_DPID			(1 << 16)
#define DWC_OTG_DIEPCTL_EONUM			DWC_OTG_DIEPCTL_DPID
#define DWC_OTG_DIEPCTL_USBAEP			(1 << 15)
/* Bits 14:11 - Reserved */
#define DWC_OTG_DIEPCTL_MPSIZ_SHIFT		(0)
#define DWC_OTG_DIEPCTL_MPSIZ_MASK		(0x3ff << DWC_OTG_DIEPCTL_MPSIZ_SHIFT)
#define DWC_OTG_DIEPCTL_MPSIZ(v)		(((v) << DWC_OTG_DIEPCTL_MPSIZ_SHIFT) &\
											DWC_OTG_DIEPCTL_MPSIZ_MASK)

/* OTG Device Control IN Endpoint 0 Control Register (DWC_OTG_DIEP0CTL) */
#define DWC_OTG_DIEP0CTL_EPENA			DWC_OTG_DIEPCTL_EPENA
#define DWC_OTG_DIEP0CTL_EPDIS			DWC_OTG_DIEPCTL_EPDIS
/* Bits 29:28 - Reserved */
#define DWC_OTG_DIEP0CTL_SNAK			DWC_OTG_DIEPCTL_SNAK
#define DWC_OTG_DIEP0CTL_CNAK			DWC_OTG_DIEPCTL_CNAK

#define DWC_OTG_DIEP0CTL_TXFNUM_SHIFT		DWC_OTG_DIEPCTL_TXFNUM_SHIFT
#define DWC_OTG_DIEP0CTL_TXFNUM_MASK		DWC_OTG_DIEPCTL_TXFNUM_MASK
#define DWC_OTG_DIEP0CTL_TXFNUM(v)			DWC_OTG_DIEPCTL_TXFNUM(v)

#define DWC_OTG_DIEP0CTL_STALL			DWC_OTG_DIEPCTL_STALL
/* Bit 20 - Reserved */
#define DWC_OTG_DIEP0CTL_NAKSTS			DWC_OTG_DIEPCTL_NAKSTS
/* Bit 16 - Reserved */
#define DWC_OTG_DIEP0CTL_USBAEP			DWC_OTG_DIEPCTL_USBAEP
/* Bits 14:2 - Reserved */
#define DWC_OTG_DIEP0CTL_MPSIZ_SHIFT		DWC_OTG_DIEPCTL_MPSIZ_SHIFT
#define DWC_OTG_DIEP0CTL_MPSIZ_MASK			(0x3 << DWC_OTG_DIEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DIEP0CTL_MPSIZ_64			(0 << DWC_OTG_DIEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DIEP0CTL_MPSIZ_32			(1 << DWC_OTG_DIEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DIEP0CTL_MPSIZ_16			(2 << DWC_OTG_DIEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DIEP0CTL_MPSIZ_8			(3 << DWC_OTG_DIEP0CTL_MPSIZ_SHIFT)

/* OTG Device Control OUT Endpoint x Control Register (DWC_OTG_DOEPxCTL) */
#define DWC_OTG_DOEPCTL_EPENA			(1 << 31)
#define DWC_OTG_DOEPCTL_EPDIS			(1 << 30)
#define DWC_OTG_DOEPCTL_SD1PID			(1 << 29)
#define DWC_OTG_DOEPCTL_SODDFRM			DWC_OTG_DOEPCTL_SD1PID
#define DWC_OTG_DOEPCTL_SD0PID			(1 << 28)
#define DWC_OTG_DOEPCTL_SEVNFRM			DWC_OTG_DOEPCTL_SD0PID
#define DWC_OTG_DOEPCTL_SNAK			(1 << 27)
#define DWC_OTG_DOEPCTL_CNAK			(1 << 26)
/* Bits 25:22 - Reserved */
#define DWC_OTG_DOEPCTL_STALL			(1 << 21)
#define DWC_OTG_DOEPCTL_SNPM			(1 << 20)

#define DWC_OTG_DOEPCTL_EPTYP_SHIFT		(18)
#define DWC_OTG_DOEPCTL_EPTYP_MASK		(0x3 << DWC_OTG_DOEPCTL_EPTYP_SHIFT)
#define DWC_OTG_DOEPCTL_EPTYP(v)		(((v) << DWC_OTG_DOEPCTL_EPTYP_SHIFT) &\
											DWC_OTG_DOEPCTL_EPTYP_MASK)

#define DWC_OTG_DOEPCTL_NAKSTS			(1 << 17)
#define DWC_OTG_DOEPCTL_DPID			(1 << 16)
#define DWC_OTG_DOEPCTL_EONUM			DWC_OTG_DOEPCTL_DPID
#define DWC_OTG_DOEPCTL_USBAEP			(1 << 15)
/* Bits 14:11 - Reserved */
#define DWC_OTG_DOEPCTL_MPSIZ_SHIFT		(0)
#define DWC_OTG_DOEPCTL_MPSIZ_MASK		(0x3ff << DWC_OTG_DOEPCTL_MPSIZ_SHIFT)
#define DWC_OTG_DOEPCTL_MPSIZ(v)		(((v) << DWC_OTG_DOEPCTL_MPSIZ_SHIFT) &\
											DWC_OTG_DOEPCTL_MPSIZ_MASK)

/* OTG Device Control OUT Endpoint 0 Control Register (DWC_OTG_DOEP0CTL) */
#define DWC_OTG_DOEP0CTL_EPENA			DWC_OTG_DOEPCTL_EPENA
#define DWC_OTG_DOEP0CTL_EPDIS			DWC_OTG_DOEPCTL_EPDIS
/* Bits 29:28 - Reserved */
#define DWC_OTG_DOEP0CTL_SNAK			DWC_OTG_DOEPCTL_SNAK
#define DWC_OTG_DOEP0CTL_CNAK			DWC_OTG_DOEPCTL_CNAK
/* Bits 25:22 - Reserved */
#define DWC_OTG_DOEP0CTL_STALL			DWC_OTG_DOEPCTL_STALL
#define DWC_OTG_DOEP0CTL_SNPM			DWC_OTG_DOEPCTL_SNPM

#define DWC_OTG_DOEP0CTL_EPTYP_SHIFT		DWC_OTG_DOEPCTL_EPTYP_SHIFT
#define DWC_OTG_DOEP0CTL_EPTYP_MASK			DWC_OTG_DOEPCTL_EPTYP_MASK
#define DWC_OTG_DOEP0CTL_EPTYP(v)			DWC_OTG_DOEPCTL_EPTYP(v)

#define DWC_OTG_DOEP0CTL_NAKSTS			DWC_OTG_DOEPCTL_NAKSTS
/* Bit 16 - Reserved */
#define DWC_OTG_DOEP0CTL_USBAEP			DWC_OTG_DOEPCTL_USBAEP
/* Bits 14:2 - Reserved */
#define DWC_OTG_DOEP0CTL_MPSIZ_SHIFT		DWC_OTG_DOEPCTL_MPSIZ_SHIFT
#define DWC_OTG_DOEP0CTL_MPSIZ_MASK			(0x3 << DWC_OTG_DOEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DOEP0CTL_MPSIZ_64			(0 << DWC_OTG_DOEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DOEP0CTL_MPSIZ_32			(1 << DWC_OTG_DOEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DOEP0CTL_MPSIZ_16			(2 << DWC_OTG_DOEP0CTL_MPSIZ_SHIFT)
#define DWC_OTG_DOEP0CTL_MPSIZ_8			(3 << DWC_OTG_DOEP0CTL_MPSIZ_SHIFT)

/* OTG Device IN Endpoint Interrupt Register (DWC_OTG_DIEPxINT) */
/* Bits 31:8 - Reserved */
#define DWC_OTG_DIEPINT_TXFE		(1 << 7)
#define DWC_OTG_DIEPINT_INEPNE		(1 << 6)
/* Bit 5 - Reserved */
#define DWC_OTG_DIEPINT_ITTXFE		(1 << 4)
#define DWC_OTG_DIEPINT_TOC			(1 << 3)
/* Bit 2 - Reserved */
#define DWC_OTG_DIEPINT_EPDISD		(1 << 1)
#define DWC_OTG_DIEPINT_XFRC		(1 << 0)

/* OTG Device IN Endpoint Interrupt Register (DWC_OTG_DOEPxINT) */
/* Bits 31:7 - Reserved */
#define DWC_OTG_DOEPINT_B2BSTUP		(1 << 6)
/* Bit 5 - Reserved */
#define DWC_OTG_DOEPINT_OTEPDIS		(1 << 4)
#define DWC_OTG_DOEPINT_STUP		(1 << 3)
/* Bit 2 - Reserved */
#define DWC_OTG_DOEPINT_EPDISD		(1 << 1)
#define DWC_OTG_DOEPINT_XFRC		(1 << 0)

/* OTG Device OUT Endpoint x Transfer Size Register (DWC_OTG_DOEPxTSIZ) */
/* Bit 31 - Reserved */
#define DWC_OTG_DIEPSIZ_STUPCNT_SHIFT	(29)
#define DWC_OTG_DIEPSIZ_STUPCNT_MASK	(0x3 << DWC_OTG_DIEPSIZ_STUPCNT_SHIFT)
#define DWC_OTG_DIEPSIZ_STUPCNT_1		(0x1 << DWC_OTG_DIEPSIZ_STUPCNT_SHIFT)
#define DWC_OTG_DIEPSIZ_STUPCNT_2		(0x2 << DWC_OTG_DIEPSIZ_STUPCNT_SHIFT)
#define DWC_OTG_DIEPSIZ_STUPCNT_3		(0x3 << DWC_OTG_DIEPSIZ_STUPCNT_SHIFT)

#define DWC_OTG_DIEPSIZ_PKTCNT_SHIFT	(19)
#define DWC_OTG_DIEPSIZ_PKTCNT_MASK		(0x3ff << DWC_OTG_DIEPSIZ_PKTCNT_SHIFT)
#define DWC_OTG_DIEPSIZ_PKTCNT(v)		(((v) << DWC_OTG_DIEPSIZ_PKTCNT_SHIFT) &\
											DWC_OTG_DIEPSIZ_PKTCNT_MASK)

#define DWC_OTG_DIEPSIZ_XFRSIZ_SHIFT	(0)
#define DWC_OTG_DIEPSIZ_XFRSIZ_MASK		(0x7ffff << 0)

/* OTG Device OUT Endpoint 0 Transfer Size Register (DWC_OTG_DOEP0TSIZ) */
/* Bit 31 - Reserved */
#define DWC_OTG_DIEP0SIZ_STUPCNT_SHIFT	(29)
#define DWC_OTG_DIEP0SIZ_STUPCNT_MASK	(0x3 << DWC_OTG_DIEP0SIZ_STUPCNT_SHIFT)
#define DWC_OTG_DIEP0SIZ_STUPCNT_1		(0x1 << DWC_OTG_DIEP0SIZ_STUPCNT_SHIFT)
#define DWC_OTG_DIEP0SIZ_STUPCNT_2		(0x2 << DWC_OTG_DIEP0SIZ_STUPCNT_SHIFT)
#define DWC_OTG_DIEP0SIZ_STUPCNT_3		(0x3 << DWC_OTG_DIEP0SIZ_STUPCNT_SHIFT)
/* Bits 28:20 - Reserved */
#define DWC_OTG_DIEP0SIZ_PKTCNT			(1 << 19)
/* Bits 18:7 - Reserved */
#define DWC_OTG_DIEP0SIZ_XFRSIZ_SHIFT	(0)
#define DWC_OTG_DIEP0SIZ_XFRSIZ_MASK	(0x7f << DWC_OTG_DIEP0SIZ_XFRSIZ_SHIFT)

/* Host-mode CSRs */
/* OTG Host non-periodic transmit FIFO size register
(DWC_OTG_HNPTXFSIZ)/Endpoint 0 Transmit FIFO size (DWC_OTG_DIEP0TXF) */
#define DWC_OTG_HNPTXFSIZ_PTXFD_MASK	(0xffff0000)
#define DWC_OTG_HNPTXFSIZ_PTXSA_MASK	(0x0000ffff)

/* OTG Host periodic transmit FIFO size register (DWC_OTG_HPTXFSIZ) */
#define DWC_OTG_HPTXFSIZ_PTXFD_MASK		(0xffff0000)
#define DWC_OTG_HPTXFSIZ_PTXSA_MASK		(0x0000ffff)

/* OTG Host Configuration Register (DWC_OTG_HCFG) */
/* Bits 31:3 - Reserved */
#define DWC_OTG_HCFG_FSLSS			(1 << 2)
#define DWC_OTG_HCFG_FSLSPCS_48MHz	(0x1 << 0)
#define DWC_OTG_HCFG_FSLSPCS_6MHz	(0x2 << 0)
#define DWC_OTG_HCFG_FSLSPCS_MASK	(0x3 << 0)

/* OTG Host Frame Interval Register (DWC_OTG_HFIR) */
/* Bits 31:16 - Reserved */
#define DWC_OTG_HFIR_FRIVL_MASK		(0x0000ffff)

/* OTG Host frame number/frame time remaining register (DWC_OTG_HFNUM) */
#define DWC_OTG_HFNUM_FTREM_MASK		(0xffff0000)
#define DWC_OTG_HFNUM_FRNUM_MASK		(0x0000ffff)

/* OTG Host periodic transmit FIFO/queue status register (DWC_OTG_HPTXSTS) */
#define DWC_OTG_HPTXSTS_PTXQTOP_MASK					(0xff000000)
#define DWC_OTG_HPTXSTS_PTXQTOP_ODDFRM					(1<<31)
#define DWC_OTG_HPTXSTS_PTXQTOP_EVENFRM					(0<<31)
#define DWC_OTG_HPTXSTS_PTXQTOP_CHANNEL_NUMBER_MASK		(0xf<<27)
#define DWC_OTG_HPTXSTS_PTXQTOP_ENDPOINT_NUMBER_MASK	(0xf<<27)
#define DWC_OTG_HPTXSTS_PTXQTOP_TYPE_INOUT				(0x00<<25)
#define DWC_OTG_HPTXSTS_PTXQTOP_TYPE_ZEROLENGTH			(0x01<<25)
#define DWC_OTG_HPTXSTS_PTXQTOP_TYPE_DISABLECMD			(0x11<<25)
#define DWC_OTG_HPTXSTS_PTXQTOP_TERMINATE				(1<<24)
#define DWC_OTG_HPTXSTS_PTXQSAV_MASK					(0x00ff0000)
#define DWC_OTG_HPTXSTS_PTXFSAVL_MASK					(0x0000ffff)

/* OTG Host all channels interrupt mask register (DWC_OTG_HAINT) */
/* Bits 31:16 - Reserved */
#define DWC_OTG_HAINTMSK_HAINT_MASK		(0x0000ffff)

/* OTG Host all channels interrupt mask register (DWC_OTG_HAINTMSK) */
/* Bits 31:16 - Reserved */
#define DWC_OTG_HAINTMSK_HAINTM_MASK	(0x0000ffff)

/* OTG Host port control and status register (DWC_OTG_HPRT) */
/* Bits 31:19 - Reserved */
#define DWC_OTG_HPRT_PSPD_HIGH				(0x0 << 17)
#define DWC_OTG_HPRT_PSPD_FULL				(0x1 << 17)
#define DWC_OTG_HPRT_PSPD_LOW				(0x2 << 17)
#define DWC_OTG_HPRT_PSPD_MASK				(0x3 << 17)
#define DWC_OTG_HPRT_PTCTL_DISABLED			(0x0 << 13)
#define DWC_OTG_HPRT_PTCTL_J				(0x1 << 13)
#define DWC_OTG_HPRT_PTCTL_K				(0x2 << 13)
#define DWC_OTG_HPRT_PTCTL_SE0_NAK			(0x3 << 13)
#define DWC_OTG_HPRT_PTCTL_PACKET			(0x4 << 13)
#define DWC_OTG_HPRT_PTCTL_FORCE_ENABLE		(0x5 << 13)
#define DWC_OTG_HPRT_PPWR					(1 << 12)
#define DWC_OTG_HPRT_PLSTS_DM				(1 << 11)
#define DWC_OTG_HPRT_PLSTS_DP				(1 << 10)
/* Bit 9 - Reserved */
#define DWC_OTG_HPRT_PRST			(1 << 8)
#define DWC_OTG_HPRT_PSUSP			(1 << 7)
#define DWC_OTG_HPRT_PRES			(1 << 6)
#define DWC_OTG_HPRT_POCCHNG		(1 << 5)
#define DWC_OTG_HPRT_POCA			(1 << 4)
#define DWC_OTG_HPRT_PENCHNG		(1 << 3)
#define DWC_OTG_HPRT_PENA			(1 << 2)
#define DWC_OTG_HPRT_PCDET			(1 << 1)
#define DWC_OTG_HPRT_PCSTS			(1 << 0)

/* OTG Host channel-x characteristics register (DWC_OTG_HCxCHAR) */
#define DWC_OTG_HCCHAR_CHENA					(1 << 31)
#define DWC_OTG_HCCHAR_CHDIS					(1 << 30)
#define DWC_OTG_HCCHAR_ODDFRM					(1 << 29)
#define DWC_OTG_HCCHAR_DAD_MASK					(0x7f << 22)
#define DWC_OTG_HCCHAR_MCNT_1					(0x1 << 20)
#define DWC_OTG_HCCHAR_MCNT_2					(0x2 << 20)
#define DWC_OTG_HCCHAR_MCNT_3					(0x3 << 20)
#define DWC_OTG_HCCHAR_MCNT_MASK				(0x3 << 20)
#define DWC_OTG_HCCHAR_EPTYP_CONTROL			(0 << 18)
#define DWC_OTG_HCCHAR_EPTYP_ISOCHRONOUS		(1 << 18)
#define DWC_OTG_HCCHAR_EPTYP_BULK				(2 << 18)
#define DWC_OTG_HCCHAR_EPTYP_INTERRUPT			(3 << 18)
#define DWC_OTG_HCCHAR_EPTYP_MASK				(3 << 18)
#define DWC_OTG_HCCHAR_LSDEV					(1 << 17)
/* Bit 16 - Reserved */
#define DWC_OTG_HCCHAR_EPDIR_OUT			(0 << 15)
#define DWC_OTG_HCCHAR_EPDIR_IN				(1 << 15)
#define DWC_OTG_HCCHAR_EPDIR_MASK			(1 << 15)
#define DWC_OTG_HCCHAR_EPNUM_MASK			(0xf << 11)
#define DWC_OTG_HCCHAR_MPSIZ_MASK			(0x7ff << 0)

/* OTG Host channel-x interrupt register (DWC_OTG_HCxINT) */
/* Bits 31:11 - Reserved */
#define DWC_OTG_HCINT_DTERR		(1 << 10)
#define DWC_OTG_HCINT_FRMOR		(1 << 9)
#define DWC_OTG_HCINT_BBERR		(1 << 8)
#define DWC_OTG_HCINT_TXERR		(1 << 7)
// Note: DWC_OTG_HCINT_NYET: Only in High Speed
#define DWC_OTG_HCINT_NYET		(1 << 6)
#define DWC_OTG_HCINT_ACK		(1 << 5)
#define DWC_OTG_HCINT_NAK		(1 << 4)
#define DWC_OTG_HCINT_STALL		(1 << 3)
// Note: DWC_OTG_HCINT_AHBERR: Only in High Speed
#define DWC_OTG_HCINT_AHBERR		(1 << 2)
#define DWC_OTG_HCINT_CHH		(1 << 1)
#define DWC_OTG_HCINT_XFRC		(1 << 0)

/* OTG Host channel-x interrupt mask register (DWC_OTG_HCxINTMSK) */
/* Bits 31:11 - Reserved */
#define DWC_OTG_HCINTMSK_DTERRM		(1 << 10)
#define DWC_OTG_HCINTMSK_FRMORM		(1 << 9)
#define DWC_OTG_HCINTMSK_BBERRM		(1 << 8)
#define DWC_OTG_HCINTMSK_TXERRM		(1 << 7)
// Note: DWC_OTG_HCINTMSK_NYET: Only in High Speed
#define DWC_OTG_HCINTMSK_NYET		(1 << 6)
#define DWC_OTG_HCINTMSK_ACKM		(1 << 5)
#define DWC_OTG_HCINTMSK_NAKM		(1 << 4)
#define DWC_OTG_HCINTMSK_STALLM		(1 << 3)
// Note: DWC_OTG_HCINTMSK_AHBERR: Only in High Speed
#define DWC_OTG_HCINTMSK_AHBERR		(1 << 2)
#define DWC_OTG_HCINTMSK_CHHM		(1 << 1)
#define DWC_OTG_HCINTMSK_XFRCM		(1 << 0)

/* OTG Host channel-x transfer size register (DWC_OTG_HCxTSIZ) */
// Note: DWC_OTG_HCTSIZ_DOPING: Only in High Speed
#define DWC_OTG_HCTSIZ_DOPING			(1 << 31)
#define DWC_OTG_HCTSIZ_DPID_DATA0		(0x0 << 29)
#define DWC_OTG_HCTSIZ_DPID_DATA1		(0x2 << 29)
#define DWC_OTG_HCTSIZ_DPID_DATA2		(0x1 << 29)
#define DWC_OTG_HCTSIZ_DPID_MDATA		(0x3 << 29)
#define DWC_OTG_HCTSIZ_DPID_MASK		(0x3 << 29)
#define DWC_OTG_HCTSIZ_PKTCNT_MASK		(0x3ff << 19)
#define DWC_OTG_HCTSIZ_XFRSIZ_MASK		(0x7ffff << 0)

/* Device-mode CSRs*/
/* OTG device each endpoint interrupt register (DWC_OTG_DEACHHINT) */
/* Bits 31:18 - Reserved */
#define DWC_OTG_DEACHHINT_OEP1INT		(1 << 17)
/* Bits 16:2 - Reserved */
#define DWC_OTG_DEACHHINT_IEP1INT		(1 << 1)
/* Bit 0 - Reserved */

/* OTG device each in endpoint-1 interrupt register (DWC_OTG_DIEPEACHMSK1) */
/* Bits 31:14 - Reserved */
#define DWC_OTG_DIEPEACHMSK1_NAKM 			(1 << 13)
/* Bits 12:10 - Reserved */
#define DWC_OTG_DIEPEACHMSK1_BIM			(1 << 9)
#define DWC_OTG_DIEPEACHMSK1_TXFURM			(1 << 8)
/* Bit 7 - Reserved */
#define DWC_OTG_DIEPEACHMSK1_INEPNEM		(1 << 6)
#define DWC_OTG_DIEPEACHMSK1_INEPNMM		(1 << 5)
#define DWC_OTG_DIEPEACHMSK1_ITTXFEMSK		(1 << 4)
#define DWC_OTG_DIEPEACHMSK1_TOM			(1 << 3)
/* Bit 2 - Reserved */
#define DWC_OTG_DIEPEACHMSK1_EPDM			(1 << 1)
#define DWC_OTG_DIEPEACHMSK1_XFRCM			(1 << 0)

/* OTG device each OUT endpoint-1 interrupt register (DWC_OTG_DOEPEACHMSK1) */
/* Bits 31:15 - Reserved */
#define DWC_OTG_DOEPEACHMSK1_NYETM			(1 << 14)
#define DWC_OTG_DOEPEACHMSK1_NAKM			(1 << 13)
#define DWC_OTG_DOEPEACHMSK1_BERRM			(1 << 12)
/* Bits 11:10 - Reserved */
#define DWC_OTG_DOEPEACHMSK1_BIM			(1 << 9)
#define DWC_OTG_DOEPEACHMSK1_OPEM			(1 << 8)
/* Bits 7:3 - Reserved */
#define DWC_OTG_DOEPEACHMSK1_AHBERRM		(1 << 2)
#define DWC_OTG_DOEPEACHMSK1_EPDM			(1 << 1)
#define DWC_OTG_DOEPEACHMSK1_XFRCM			(1 << 0)

/* Host-mode CSRs */
/* OTG host channel-x split control register (DWC_OTG_HCxSPLT) */
#define DWC_OTG_HCSPLT_SPLITEN				(1 << 31)
/* Bits 30:17 - Reserved */
#define DWC_OTG_HCSPLT_COMPLSPLT			(1 << 16)
#define DWC_OTG_HCSPLT_XACTPOS_ALL			(0x3 << 14)
#define DWC_OTG_HCSPLT_XACTPOS_BEGIN		(0x2 << 14)
#define DWC_OTG_HCSPLT_XACTPOS_MID			(0x0 << 14)
#define DWC_OTG_HCSPLT_XACTPOS_END			(0x1 << 14)
#define DWC_OTG_HCSPLT_HUBADDR_MASK			(0x7f << 7)
#define DWC_OTG_HCSPLT_PORTADDR_MASK		(0x7f << 0)

/* Device mode status register DWC_OTG_DSTS */
#define DWC_OTG_DSTS_FNSOF_SHIFT	8
#define DWC_OTG_DSTS_FNSOF_MASK		(0x3FFF << DWC_OTG_DSTS_FNSOF_SHIFT)

#define DWC_OTG_DSTS_EERR			(1 << 2)
#define DWC_OTG_DSTS_ENUMSPD		(1 << 1)
#define DWC_OTG_DSTS_SUSPSTS		(1 << 0)

#endif
