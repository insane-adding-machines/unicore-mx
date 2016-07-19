/*
 * Copyright (C) 2011 Gareth McMullin <gareth@blacksphere.co.nz>
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

#ifndef LM3S_MEMORYMAP_H
#define LM3S_MEMORYMAP_H

#include <unicore-mx/cm3/common.h>

/* --- LM3S specific peripheral definitions ----------------------------- */

#define GPIOA_APB_BASE			(0x40004000U)
#define GPIOB_APB_BASE			(0x40005000U)
#define GPIOC_APB_BASE			(0x40006000U)
#define GPIOD_APB_BASE			(0x40007000U)
#define GPIOE_APB_BASE			(0x40024000U)
#define GPIOF_APB_BASE			(0x40025000U)
#define GPIOG_APB_BASE			(0x40026000U)
#define GPIOH_APB_BASE			(0x40027000U)

#define GPIOA_BASE			(0x40058000U)
#define GPIOB_BASE			(0x40059000U)
#define GPIOC_BASE			(0x4005A000U)
#define GPIOD_BASE			(0x4005B000U)
#define GPIOE_BASE			(0x4005C000U)
#define GPIOF_BASE			(0x4005D000U)
#define GPIOG_BASE			(0x4005E000U)
#define GPIOH_BASE			(0x4005F000U)

#define SYSTEMCONTROL_BASE		(0x400FE000U)


/* Ethernet Registries 			*/
#define ETHERNET_BASE			(0x40048000U)
#define ETH_MAC_RISACK			MMIO32(ETHERNET_BASE)
#define ETH_MAC_IM			MMIO32(ETHERNET_BASE + 0x0004)
#define ETH_MAC_RCTL			MMIO32(ETHERNET_BASE + 0x0008)
#define ETH_MAC_TCTL			MMIO32(ETHERNET_BASE + 0x000C)
#define ETH_MAC_DATA			MMIO32(ETHERNET_BASE + 0x0010)
#define ETH_MAC_ADDR0			MMIO32(ETHERNET_BASE + 0x0014)
#define ETH_MAC_ADDR1			MMIO32(ETHERNET_BASE + 0x0018)
#define ETH_MAC_THR			MMIO32(ETHERNET_BASE + 0x001C)
#define ETH_MAC_MCTL			MMIO32(ETHERNET_BASE + 0x0020)
#define ETH_MAC_MDV			MMIO32(ETHERNET_BASE + 0x0024)
#define ETH_MAC_MTXD			MMIO32(ETHERNET_BASE + 0x002C)
#define ETH_MAC_MRXD			MMIO32(ETHERNET_BASE + 0x0030)
#define ETH_MAC_NP			MMIO32(ETHERNET_BASE + 0x0034)
#define ETH_MAC_TR			MMIO32(ETHERNET_BASE + 0x0038)
#define ETH_MAC_TS			MMIO32(ETHERNET_BASE + 0x003C)


#endif
