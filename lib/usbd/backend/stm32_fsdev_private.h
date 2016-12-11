/*
 * This file is part of the unicore-mx project.
 *
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

#ifndef UNICOREMX_USBD_STM32_FSDEV_PRIVATE_H
#define UNICOREMX_USBD_STM32_FSDEV_PRIVATE_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/usbd/usbd.h>

BEGIN_DECLS

struct stm32_fsdev_private_data {
	/** The number of bytes of endpoint buffer memory used.
	 *  @note used by backend */
	uint16_t pma_used;
};

#define USBD_DEVICE_EXTRA \
	struct stm32_fsdev_private_data private_data;

/*
 * ep_count:
 * Number of available endpoints.
 *  This will be used to count the pm_top value.
 *
 * pma_size: Number of bytes (PMA)
 *
 * copy_to_pm:
 * Copy a data buffer to packet memory.
 * @param[in] vPM Destination pointer into packet memory.
 * @param[in] vBuf Source pointer to data buffer.
 * @param[in] len Number of bytes to copy.
 *
 * copy_from_pm:
 * Copy a data buffer from packet memory.
 *
 * @param[in] vBuf Source pointer to data buffer.
 * @param[in] vPM Destination pointer into packet memory.
 * @param[in] len Number of bytes to copy.
 */
#define USBD_BACKEND_EXTRA													\
	void (*copy_to_pm)(volatile void *vPM, const void *vBuf, uint16_t len); \
	void (*copy_from_pm)(void *vBuf, const volatile void *vPM, uint16_t len);

void stm32_fsdev_init(struct usbd_device *dev);
void stm32_fsdev_set_address(usbd_device *dev, uint8_t addr);
uint8_t stm32_fsdev_get_address(usbd_device *dev);
void stm32_fsdev_ep_prepare_start(usbd_device *dev);
void stm32_fsdev_ep_prepare(usbd_device *dev, uint8_t addr, usbd_ep_type type,
				uint16_t max_size, uint16_t interval, usbd_ep_flags flags);
void stm32_fsdev_set_ep_stall(usbd_device *dev, uint8_t addr, bool stall);
bool stm32_fsdev_get_ep_stall(usbd_device *dev, uint8_t addr);
void stm32_fsdev_set_ep_dtog(usbd_device *dev, uint8_t addr, bool dtog);
bool stm32_fsdev_get_ep_dtog(usbd_device *dev, uint8_t addr);
void stm32_fsdev_poll(usbd_device *dev);
void stm32_fsdev_enable_sof(usbd_device *dev, bool enable);
usbd_speed stm32_fsdev_get_speed(usbd_device *dev);

typedef struct usbd_urb usbd_urb;
void stm32_fsdev_urb_submit(usbd_device *dev, usbd_urb *urb);
void stm32_fsdev_urb_cancel(usbd_device *dev, usbd_urb *urb);

uint16_t stm32_fsdev_frame_number(usbd_device *dev);

END_DECLS

#endif
