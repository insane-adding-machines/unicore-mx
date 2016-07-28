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
	uint16_t pm_used;

	/** Force NAK on endpoint endpoint (OUT).
	 *  @note Used by backend **/
	uint16_t force_nak;

	/** Number of available endpoints.
	 *  This will be used to count the pm_top value. */
	uint8_t ep_count;

	/**
	 * Copy a data buffer to packet memory.
	 *
	 * @param vPM Destination pointer into packet memory.
	 * @param buf Source pointer to data buffer.
	 * @param len Number of bytes to copy.
	 */
	void (*copy_to_pm)(volatile void *vPM, const void *buf, uint16_t len);

	/**
	 * Copy a data buffer from packet memory.
	 *
	 * @param buf Source pointer to data buffer.
	 * @param vPM Destination pointer into packet memory.
	 * @param len Number of bytes to copy.
	 */
	void (*copy_from_pm)(void *buf, const volatile void *vPM, uint16_t len);
};

void stm32_fsdev_init(struct usbd_device *usbd_dev);
void stm32_fsdev_set_address(usbd_device *usbd_dev, uint8_t addr);
void stm32_fsdev_set_ep_rx_bufsize(usbd_device *usbd_dev, uint8_t ep, uint32_t size);
void stm32_fsdev_ep_setup(usbd_device *usbd_dev, uint8_t addr, uint8_t type,
				uint16_t max_size);
void stm32_fsdev_set_ep_type(usbd_device *usbd_dev, uint8_t addr, uint8_t type);
void stm32_fsdev_set_ep_size(usbd_device *usbd_dev, uint8_t addr, uint16_t max_size);
void stm32_fsdev_endpoints_reset(usbd_device *usbd_dev);
void stm32_fsdev_set_ep_stall(usbd_device *usbd_dev, uint8_t addr, bool stall);
bool stm32_fsdev_get_ep_stall(usbd_device *usbd_dev, uint8_t addr);
void stm32_fsdev_set_ep_nak(usbd_device *usbd_dev, uint8_t addr, bool nak);
uint16_t stm32_fsdev_ep_write_packet(usbd_device *usbd_dev, uint8_t addr,
				     const void *buf, uint16_t len);
uint16_t stm32_fsdev_ep_read_packet(usbd_device *usbd_dev, uint8_t addr,
					 void *buf, uint16_t len);
void stm32_fsdev_poll(usbd_device *usbd_dev);
void stm32_fsdev_enable_sof(usbd_device *usbd_dev, bool enable);
void stm32_fsdev_ep_flush(usbd_device *usbd_dev, uint8_t addr);
void stm32_fsdev_set_ep_dtog(usbd_device *usbd_dev, uint8_t addr, bool dtog);
bool stm32_fsdev_get_ep_dtog(usbd_device *usbd_dev, uint8_t addr);
uint16_t stm32_fsdev_frame_number(usbd_device *usbd_dev);

END_DECLS

#endif
