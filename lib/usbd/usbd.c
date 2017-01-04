/**
 * @defgroup usbd_drivers_file Generic USB Drivers
 *
 * @ingroup USB
 *
 * @brief <b>Generic USB Drivers</b>
 *
 * @author @htmlonly &copy; @endhtmlonly 2016
 * Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 *
 * @author @htmlonly &copy; @endhtmlonly 2010
 * Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * @date 11 September 2016
 *
 * LGPL License Terms @ref lgpl_license
 */

/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
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

/**@{*/

#include <string.h>
#include <unicore-mx/usbd/usbd.h>
#include "usbd_private.h"

usbd_device* usbd_init(const usbd_backend *backend,
				const usbd_backend_config *config,
				const struct usb_device_descriptor *dev_desc,
				void *ep0_buffer, size_t ep0_buffer_size)
{
	usbd_device *dev = backend->init(config);
	if (dev == NULL) {
		return NULL;
	}

	dev->desc = dev_desc;
	dev->preserve.buf = ep0_buffer;
	dev->preserve.len = ep0_buffer_size;

	dev->callback.reset = NULL;
	dev->callback.sof = NULL;
	dev->callback.suspend = NULL;
	dev->callback.resume = NULL;
	dev->callback.set_config = NULL;
	dev->callback.set_interface = NULL;
	dev->callback.setup = NULL;

	dev->urbs.next_id = 1;

	dev->urbs.force_all_new_urb_to_waiting = false;

	/* all are free */
	dev->urbs.ep_free = ~0;

	dev->urbs.active.head = NULL;
	dev->urbs.active.tail = NULL;

	dev->urbs.waiting.head = NULL;
	dev->urbs.waiting.tail = NULL;

	usbd_put_all_urb_into_unused(dev);

	return dev;
}

void usbd_register_setup_callback(usbd_device *dev,
				usbd_setup_callback callback)
{
	dev->callback.setup = callback;
}

void usbd_register_set_config_callback(usbd_device *dev,
				usbd_set_config_callback callback)
{
	dev->callback.set_config = callback;
}

void usbd_register_set_interface_callback(usbd_device *dev,
				usbd_set_interface_callback callback)
{
	dev->callback.set_interface = callback;
}

void usbd_register_reset_callback(usbd_device *dev,
				usbd_generic_callback callback)
{
	dev->callback.reset = callback;
}

void usbd_register_suspend_callback(usbd_device *dev,
				usbd_generic_callback callback)
{
	dev->callback.suspend = callback;
}

void usbd_register_resume_callback(usbd_device *dev,
				usbd_generic_callback callback)
{
	dev->callback.resume = callback;
}

void usbd_register_sof_callback(usbd_device *dev,
				usbd_generic_callback callback)
{
	dev->callback.sof = callback;

	if (dev->backend->enable_sof) {
		dev->backend->enable_sof(dev, callback != NULL);
	}
}

/* Functions to be provided by the hardware abstraction layer */
void usbd_poll(usbd_device *dev, uint32_t us)
{
#if !defined(USBD_ENABLE_TIMEOUT)
	(void) us;
#endif
	dev->backend->poll(dev);

#if defined(USBD_ENABLE_TIMEOUT)
	uint64_t now = dev->last_poll + us;
	usbd_timeout_checkup(dev, now);
	dev->last_poll = now;
#endif
}

void usbd_disconnect(usbd_device *dev, bool disconnect)
{
	if (dev->backend->disconnect) {
		dev->backend->disconnect(dev, disconnect);
	}
}

void usbd_ep_prepare(usbd_device *dev, uint8_t addr, usbd_ep_type type,
			uint16_t max_size, uint16_t interval, usbd_ep_flags flags)
{
	if (!ENDPOINT_NUMBER(addr)) {
		LOGF_LN("EP0 is for internal use! do not call "
			"usbd_ep_prepare() with addr=%"PRIx8, addr);
		return;
	}

	if (dev->backend->ep_prepare) {
		dev->backend->ep_prepare(dev, addr, type, max_size, interval, flags);
	}
}

void usbd_set_ep_stall(usbd_device *dev, uint8_t addr, bool stall)
{
	dev->backend->set_ep_stall(dev, addr, stall);
}

bool usbd_get_ep_stall(usbd_device *dev, uint8_t addr)
{
	return dev->backend->get_ep_stall(dev, addr);
}

void usbd_set_ep_dtog(usbd_device *dev, uint8_t addr, bool dtog)
{
	dev->backend->set_ep_dtog(dev, addr, dtog);
}

bool usbd_get_ep_dtog(usbd_device *dev, uint8_t addr)
{
	return dev->backend->get_ep_dtog(dev, addr);
}

uint16_t usbd_frame_number(usbd_device *dev)
{
	if (dev->backend->frame_number) {
		return dev->backend->frame_number(dev);
	}

	return 0;
}

const struct usb_config_descriptor *usbd_get_config(usbd_device *dev)
{
	return dev->current_config;
}

uint8_t usbd_get_address(usbd_device *dev)
{
	return dev->backend->get_address(dev);
}

usbd_speed usbd_get_speed(usbd_device *dev)
{
	return dev->backend->get_speed(dev);
}

/**@}*/

#if defined(USBD_DEBUG)
void usbd_log_call(const char *fname){
	USBD_LOG(USB_VALL, "inside ");
	USBD_LOG_LN(USB_VALL, fname);
}
#endif

