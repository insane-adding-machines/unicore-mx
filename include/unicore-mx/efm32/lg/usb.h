/*
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

#ifndef UNICOREMX_EFM32_USB_H
#define UNICOREMX_EFM32_USB_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/usbd/usbd.h>

#define USB_CTRL			MMIO32(USB_BASE + 0x000)
#define USB_STATUS			MMIO32(USB_BASE + 0x004)
#define USB_IF				MMIO32(USB_BASE + 0x008)
#define USB_IFS				MMIO32(USB_BASE + 0x00C)
#define USB_IFC				MMIO32(USB_BASE + 0x010)
#define USB_IEN				MMIO32(USB_BASE + 0x014)
#define USB_ROUTE			MMIO32(USB_BASE + 0x018)

/* USB_CTRL */
#define USB_CTRL_DMPUAP			(1 << 1)

/* USB_ROUTE */
#define USB_ROUTE_DMPUPEN		(1 << 2)
#define USB_ROUTE_VBUSENPEN		(1 << 1)
#define USB_ROUTE_PHYPEN		(1 << 0)

#define USB_OTG_BASE			(USB_BASE + 0x3C000)

#endif

