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
 * This file covers definitions for the USB OTG FS peripheral.
 * This is the USB core included in the F105, F107, F2, F4 devices
 */

#ifndef UNICOREMX_OTG_FS_H
#define UNICOREMX_OTG_FS_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/memorymap.h>
#include <unicore-mx/stm32/otg_common.h>

#define OTG_FS_GCCFG		STM32_OTG_GCCFG(USB_OTG_FS_BASE)
#define OTG_FS_CID			STM32_OTG_CID(USB_OTG_FS_BASE)

#endif
