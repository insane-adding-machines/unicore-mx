/*
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
 * This file is intended to be included by either otg_hs.h or otg_fs.h
 * Contains common definitions of Command and Status Registers (CSR) and their
 * bit definitions.
 */

#ifndef UNICOREMX_OTG_COMMON_H
#define UNICOREMX_OTG_COMMON_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/memorymap.h>
#include <unicore-mx/common/dwc_otg.h>

#define STM32_OTG_GCCFG(base)	DWC_OTG_GGPIO(base)
#define STM32_OTG_CID(base)		DWC_OTG_GUID(base)

/* OTG general core configuration register (OTG_GCCFG) */
/* Bits 31:22 - Reserved */
#define OTG_GCCFG_VBDEN			(1 << 21)
#define OTG_GCCFG_NOVBUSSENS	(1 << 21)
#define OTG_GCCFG_SOFOUTEN		(1 << 20)
#define OTG_GCCFG_VBUSBSEN		(1 << 19)
#define OTG_GCCFG_VBUSASEN		(1 << 18)
/* Bit 17 - Reserved */
#define OTG_GCCFG_PWRDWN		(1 << 16)
/* Bits 15:0 - Reserved */

#endif
