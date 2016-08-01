/** @defgroup usb_drivers_file Generic USB Drivers

@ingroup USB

@brief <b>Generic USB Drivers</b>

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2010
Gareth McMullin <gareth@blacksphere.co.nz>

@date 10 March 2013

LGPL License Terms @ref lgpl_license
*/

/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
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

/**@{*/

#include <string.h>
#include <unicore-mx/usbd/usbd.h>
#include "usbd_private.h"

usbd_device* usbd_init(const usbd_backend *backend,
				const struct usb_device_descriptor *dev_desc,
				uint8_t *control_buffer, uint16_t control_buffer_size)
{
	usbd_device *usbd_dev;

	usbd_dev = backend->init();
	if (usbd_dev == NULL) {
		return NULL;
	}

	usbd_dev->backend = backend;
	usbd_dev->desc = dev_desc;
	usbd_dev->ep0.preserve.buf = control_buffer;
	usbd_dev->ep0.preserve.len = control_buffer_size;

	usbd_dev->callback.ep[0][USB_CALLBACK_SETUP] = _usbd_control_setup;
	usbd_dev->callback.ep[0][USB_CALLBACK_OUT] = _usbd_control_out;
	usbd_dev->callback.ep[0][USB_CALLBACK_IN] = _usbd_control_in;

	usbd_dev->callback.reset = NULL;
	usbd_dev->callback.sof = NULL;
	usbd_dev->callback.suspend = NULL;
	usbd_dev->callback.resume = NULL;
	usbd_dev->callback.set_config = NULL;
	usbd_dev->callback.control = NULL;
	usbd_dev->callback.set_interface = NULL;
	usbd_dev->callback.get_string = NULL;

	return usbd_dev;
}

void usbd_register_control_callback(usbd_device *usbd_dev,
				usbd_control_callback callback)
{
	usbd_dev->callback.control = callback;
}

void usbd_register_set_config_callback(usbd_device *usbd_dev,
				usbd_set_config_callback callback)
{
	usbd_dev->callback.set_config = callback;
}

void usbd_register_set_interface_callback(usbd_device *usbd_dev,
				usbd_set_interface_callback callback)
{
	usbd_dev->callback.set_interface = callback;
}

void usbd_register_get_string_callback(usbd_device *usbd_dev,
				usbd_get_string_callback callback)
{
	usbd_dev->callback.get_string = callback;
}

void usbd_register_reset_callback(usbd_device *usbd_dev, usbd_generic_callback callback)
{
	usbd_dev->callback.reset = callback;
}

void usbd_register_suspend_callback(usbd_device *usbd_dev,
				usbd_generic_callback callback)
{
	usbd_dev->callback.suspend = callback;
}

void usbd_register_resume_callback(usbd_device *usbd_dev,
				usbd_generic_callback callback)
{
	usbd_dev->callback.resume = callback;
}

void usbd_register_sof_callback(usbd_device *usbd_dev,
				usbd_generic_callback callback)
{
	usbd_dev->callback.sof = callback;

	if (usbd_dev->backend->enable_sof) {
		usbd_dev->backend->enable_sof(usbd_dev, callback != NULL);
	}
}

/* Functions to be provided by the hardware abstraction layer */
void usbd_poll(usbd_device *usbd_dev)
{
	usbd_dev->backend->poll(usbd_dev);
}

void usbd_disconnect(usbd_device *usbd_dev, bool disconnect)
{
	if (usbd_dev->backend->disconnect) {
		usbd_dev->backend->disconnect(usbd_dev, disconnect);
	}
}

void usbd_ep_setup(usbd_device *usbd_dev, uint8_t addr, uint8_t type,
			uint16_t max_size, usbd_endpoint_callback callback)
{
	usbd_dev->backend->ep_setup(usbd_dev, addr, type, max_size);
	usbd_register_ep_callback(usbd_dev, addr, callback);
}

void usbd_set_ep_type(usbd_device *usbd_dev, uint8_t addr, uint8_t type)
{
	if(usbd_dev->backend->set_ep_type) {
		usbd_dev->backend->set_ep_type(usbd_dev, addr, type);
	}
}

void usbd_set_ep_size(usbd_device *usbd_dev, uint8_t addr, uint16_t max_size)
{
	if (usbd_dev->backend->set_ep_size) {
		usbd_dev->backend->set_ep_size(usbd_dev, addr, max_size);
	}
}

uint16_t usbd_ep_write_packet(usbd_device *usbd_dev, uint8_t addr,
			const void *buf, uint16_t len)
{
	addr &= 0x7F;
	return usbd_dev->backend->ep_write_packet(usbd_dev, addr, buf, len);
}

uint16_t usbd_ep_read_packet(usbd_device *usbd_dev, uint8_t addr,
			void *buf, uint16_t len)
{
	addr &= 0x7F;
	return usbd_dev->backend->ep_read_packet(usbd_dev, addr, buf, len);
}

void usbd_set_ep_stall(usbd_device *usbd_dev, uint8_t addr, bool stall)
{
	usbd_dev->backend->set_ep_stall(usbd_dev, addr, stall);
}

bool usbd_get_ep_stall(usbd_device *usbd_dev, uint8_t addr)
{
	return usbd_dev->backend->get_ep_stall(usbd_dev, addr);
}

void usbd_set_ep_nak(usbd_device *usbd_dev, uint8_t addr, bool nak)
{
	usbd_dev->backend->set_ep_nak(usbd_dev, addr, nak);
}

void usbd_set_ep_dtog(usbd_device *usbd_dev, uint8_t addr, bool dtog)
{
	usbd_dev->backend->set_ep_dtog(usbd_dev, addr, dtog);
}

bool usbd_get_ep_dtog(usbd_device *usbd_dev, uint8_t addr)
{
	return usbd_dev->backend->get_ep_dtog(usbd_dev, addr);
}

void usbd_ep_flush(usbd_device *usbd_dev, uint8_t addr)
{
	if (usbd_dev->backend->ep_flush) {
		usbd_dev->backend->ep_flush(usbd_dev, addr);
	}
}

void usbd_register_ep_callback(usbd_device *usbd_dev, uint8_t addr,
			usbd_endpoint_callback callback)
{
	unsigned dir = (addr & 0x80) ? USB_CALLBACK_IN : USB_CALLBACK_OUT;
	addr &= 0x7F;

	if (addr) {
		usbd_dev->callback.ep[addr][dir] = callback;
	}
}

uint16_t usbd_frame_number(usbd_device *usbd_dev)
{
	if (usbd_dev->backend->frame_number) {
		return usbd_dev->backend->frame_number(usbd_dev);
	}

	return 0;
}

const struct usb_config_descriptor *usbd_get_config(usbd_device *usbd_dev)
{
	return usbd_dev->current_config;
}

uint8_t usbd_get_address(usbd_device *usbd_dev)
{
	return usbd_dev->current_address;
}

/**@}*/

