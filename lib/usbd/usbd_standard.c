/** @defgroup usb_standard_file Generic USB Standard Request Interface

@ingroup USB

@brief <b>Generic USB Standard Request Interface</b>

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

/**
 * Build usb configuration descriptor on the fly. \n
 * This will use @a buf to store the generate descriptor.
 * @param[in] usbd_dev usb device
 * @param cfg configuration descriptor
 * @param buf Buffer
 * @param len Maximum length of buffer
 * @retval total number of bytes used out of @a len
 */
static uint16_t build_config_descriptor(const struct usb_config_descriptor *cfg,
					uint8_t *buf, uint16_t len)
{
	struct usb_config_descriptor *output;
	uint16_t count, total = 0, total_len = 0;
	uint16_t i, j, k;

	/* preserved for later to write wLength */
	output = (struct usb_config_descriptor *) buf;

	memcpy(buf, cfg, count = MIN(len, cfg->bLength));
	buf += count;
	len -= count;
	total += count;
	total_len += cfg->bLength;

	/* For each interface... */
	for (i = 0; i < cfg->bNumInterfaces; i++) {
		/* Interface Association Descriptor, if any */
		if (cfg->interface[i].iface_assoc) {
			const struct usb_iface_assoc_descriptor *assoc =
					cfg->interface[i].iface_assoc;
			memcpy(buf, assoc, count = MIN(len, assoc->bLength));
			buf += count;
			len -= count;
			total += count;
			total_len += assoc->bLength;
		}
		/* For each alternate setting... */
		for (j = 0; j < cfg->interface[i].num_altsetting; j++) {
			const struct usb_interface_descriptor *iface =
					&cfg->interface[i].altsetting[j];
			/* Copy interface descriptor. */
			memcpy(buf, iface, count = MIN(len, iface->bLength));
			buf += count;
			len -= count;
			total += count;
			total_len += iface->bLength;
			/* Copy extra bytes (function descriptors). */
			if (iface->extra) {
				memcpy(buf, iface->extra, count = MIN(len, iface->extra_len));
				buf += count;
				len -= count;
				total += count;
				total_len += iface->extra_len;
			}
			/* For each endpoint... */
			for (k = 0; k < iface->bNumEndpoints; k++) {
				const struct usb_endpoint_descriptor *ep = &iface->endpoint[k];
				memcpy(buf, ep, count = MIN(len, ep->bLength));
				buf += count;
				len -= count;
				total += count;
				total_len += ep->bLength;
				/* Copy extra bytes (class specific). */
				if (ep->extra) {
					memcpy(buf, ep->extra, count = MIN(len, ep->extra_len));
					buf += count;
					len -= count;
					total += count;
					total_len += ep->extra_len;
				}
			}
		}
	}


	output->wTotalLength = total_len;
	return total;
}

static uint8_t usb_descriptor_type(uint16_t wValue)
{
	return wValue >> 8;
}

static uint8_t usb_descriptor_index(uint16_t wValue)
{
	return wValue & 0xFF;
}

static enum usbd_control_result
usb_standard_get_descriptor_string(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	struct usb_string_descriptor *sd;
	int used;
	struct usbd_get_string_arg get_string_arg;
	const unsigned header_size = sizeof(sd->bLength) +
						sizeof(sd->bDescriptorType);

	/* no user callback to get string */
	if (usbd_dev->callback.get_string == NULL) {
		return USBD_REQ_STALL;
	}

	sd = (struct usb_string_descriptor *) arg->buf;
	get_string_arg.index = usb_descriptor_index(arg->setup.wValue);
	get_string_arg.lang_id = arg->setup.wIndex;
	get_string_arg.buf = sd->wData;
	get_string_arg.len = (arg->len - header_size) / sizeof(*sd->wData);
	used = usbd_dev->callback.get_string(usbd_dev, &get_string_arg);

	/* return error? */
	if (used < 0) {
		return USBD_REQ_STALL;
	}

	/* copy back data */
	sd->bLength = header_size + (used * sizeof(*sd->wData));
	arg->len = MIN(arg->len, sd->bLength);
	sd->bDescriptorType = USB_DT_STRING;

	/* we have done it! */
	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_get_descriptor(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	uint8_t descr_idx;

	descr_idx = usb_descriptor_index(arg->setup.wValue);

	switch (usb_descriptor_type(arg->setup.wValue)) {
	case USB_DT_DEVICE:
		arg->buf = (uint8_t *) usbd_dev->desc;
		arg->len = MIN(arg->len, usbd_dev->desc->bLength);
		return USBD_REQ_HANDLED;
	case USB_DT_CONFIGURATION:
		if (descr_idx < usbd_dev->desc->bNumConfigurations) {
			arg->len = build_config_descriptor(
							&usbd_dev->desc->config[descr_idx],
							arg->buf, arg->len);
			return USBD_REQ_HANDLED;
		}
	case USB_DT_STRING:
		return usb_standard_get_descriptor_string(usbd_dev, arg);
	}
	return USBD_REQ_STALL;
}

static void usb_standard_set_address_complete(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	usbd_dev->backend->set_address(usbd_dev, arg->setup.wValue);
}

static enum usbd_control_result
usb_standard_set_address(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	/* Control Out should only be used */
	if (arg->setup.bmRequestType != 0x00) {
		return USBD_REQ_STALL;
	}

	usbd_dev->current_address = arg->setup.wValue;

	/*
	 * Special workaround for STM32F10[57] that require the address
	 * to be set here before status stage. This is undocumented!
	 */
	if (usbd_dev->backend->set_address_before_status) {
		usbd_dev->backend->set_address(usbd_dev, arg->setup.wValue);
	} else {
		arg->complete = usb_standard_set_address_complete;
	}

	return USBD_REQ_HANDLED;
}

static const struct usb_config_descriptor*
usbd_search_config(usbd_device *usbd_dev, uint8_t value)
{
	unsigned i;

	for (i = 0; i < usbd_dev->desc->bNumConfigurations; i++) {
		const struct usb_config_descriptor *cfg = &usbd_dev->desc->config[i];
		if (cfg->bConfigurationValue == value) {
			return cfg;
		}
	}

	return NULL;
}

static enum usbd_control_result
usb_standard_set_configuration(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	unsigned i;
	const struct usb_config_descriptor *cfg = NULL;

	if(arg->setup.wValue > 0) {
		cfg = usbd_search_config(usbd_dev, arg->setup.wValue);
		if (cfg == NULL) {
			return USBD_REQ_STALL;
		}
	}

	usbd_dev->current_config = cfg;

	if (cfg != NULL) {
		/* reset all alternate settings configuration */
		for (i = 0; i < cfg->bNumInterfaces; i++) {
			if (cfg->interface[i].cur_altsetting) {
				*cfg->interface[i].cur_altsetting = 0;
			}
		}
	}

	/* Reset all endpoints. */
	usbd_dev->backend->ep_reset(usbd_dev);

	if(usbd_dev->callback.set_config != NULL) {
		usbd_dev->callback.set_config(usbd_dev, cfg);
	}

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_get_configuration(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	if (arg->len > 1) {
		arg->len = 1;
	}

	arg->buf[0] =  (usbd_dev->current_config != NULL) ?
		usbd_dev->current_config->bConfigurationValue /* configured state */
		: 0 /* address stage */;

	return USBD_REQ_HANDLED;
}

static const struct usb_interface * usbd_search_iface(
				const struct usb_config_descriptor *cfg,
				uint8_t bInterfaceNumber)
{
	unsigned i;

	for (i = 0; i < cfg->bNumInterfaces; i++) {
		const struct usb_interface *iface = &cfg->interface[i];

		/* interface contain any alternate setting */
		if (!iface->num_altsetting) {
			continue;
		}

		/* match with first alternate setting to check if bInterface exists. */
		const struct usb_interface_descriptor *alt = &iface->altsetting[0];
		if (alt->bInterfaceNumber == bInterfaceNumber) {
			return iface;
		}
	}

	return NULL;
}

static const struct usb_interface_descriptor *
usbd_search_iface_altset(const struct usb_interface *iface,
							uint8_t bAlternateSetting)
{
	unsigned i;

	for (i = 0; i < iface->num_altsetting; i++) {
		const struct usb_interface_descriptor *alt = &iface->altsetting[i];
		if (alt->bAlternateSetting == bAlternateSetting) {
			return alt;
		}
	}

	return NULL;
}

static enum usbd_control_result
usb_standard_set_interface(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	const struct usb_interface *iface;
	const struct usb_interface_descriptor *iface_alt;

	/* STALL since the device is in address stage */
	if (usbd_dev->current_config == NULL) {
		return USBD_REQ_STALL;
	}

	/* search for the interface */
	iface = usbd_search_iface(usbd_dev->current_config, arg->setup.wIndex);
	if (iface == NULL) {
		/* interface not found */
		return USBD_REQ_STALL;
	}

	/* search for the interface alternate setting */
	iface_alt = usbd_search_iface_altset(iface, arg->setup.wValue);
	if (iface_alt == NULL) {
		/* interface alternate setting not found */
		return USBD_REQ_STALL;
	}

	if (iface->cur_altsetting != NULL) {
		*iface->cur_altsetting = arg->setup.wValue;
	} else if (arg->setup.wValue > 0) {
		return USBD_REQ_STALL;
	}

	if (usbd_dev->callback.set_interface != NULL) {
		usbd_dev->callback.set_interface(usbd_dev, iface, iface_alt);
	}

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_get_interface(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	const struct usb_interface *iface;

	if (usbd_dev->current_config == NULL) {
		return USBD_REQ_STALL;
	}

	iface = usbd_search_iface(usbd_dev->current_config, arg->setup.wIndex);
	if (iface == NULL) {
		return USBD_REQ_STALL;
	}

	arg->len = 1;
	arg->buf[0] = (iface->cur_altsetting) ? *iface->cur_altsetting : 0;

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_device_get_status(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	(void)usbd_dev;

	/* bit 0: self powered */
	/* bit 1: remote wakeup */
	if (arg->len > 2) {
		arg->len = 2;
	}
	arg->buf[0] = 0;
	arg->buf[1] = 0;

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_interface_get_status(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	(void)usbd_dev;
	/* not defined */

	if (arg->len > 2) {
		arg->len = 2;
	}
	arg->buf[0] = 0;
	arg->buf[1] = 0;

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_endpoint_get_status(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	if (arg->len > 2) {
		arg->len = 2;
	}
	arg->buf[0] = usbd_get_ep_stall(usbd_dev, arg->setup.wIndex) ? 1 : 0;
	arg->buf[1] = 0;

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_endpoint_stall(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	usbd_set_ep_stall(usbd_dev, arg->setup.wIndex, true);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
usb_standard_endpoint_unstall(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	usbd_set_ep_stall(usbd_dev, arg->setup.wIndex, false);

	return USBD_REQ_HANDLED;
}

/* Do not appear to belong to the API, so are omitted from docs */
/**@}*/

enum usbd_control_result
_usbd_standard_request_device(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	switch (arg->setup.bRequest) {
	case USB_REQ_CLEAR_FEATURE:
	case USB_REQ_SET_FEATURE:
		if (arg->setup.wValue == USB_FEAT_DEVICE_REMOTE_WAKEUP) {
			/* Device wakeup code goes here. */
		}

		if (arg->setup.wValue == USB_FEAT_TEST_MODE) {
			/* Test mode code goes here. */
		}

		break;
	case USB_REQ_SET_ADDRESS:
		return usb_standard_set_address(usbd_dev, arg);
	case USB_REQ_SET_CONFIGURATION:
		return usb_standard_set_configuration(usbd_dev, arg);
	case USB_REQ_GET_CONFIGURATION:
		return usb_standard_get_configuration(usbd_dev, arg);
	case USB_REQ_GET_DESCRIPTOR:
		return usb_standard_get_descriptor(usbd_dev, arg);
	case USB_REQ_GET_STATUS:
		/*
		 * GET_STATUS always responds with zero reply.
		 * The application may override this behaviour.
		 */
		return usb_standard_device_get_status(usbd_dev, arg);
	case USB_REQ_SET_DESCRIPTOR:
		/* SET_DESCRIPTOR is optional and not implemented. */
		break;
	}

	return USBD_REQ_STALL;
}

enum usbd_control_result
_usbd_standard_request_interface(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	switch (arg->setup.bRequest) {
	case USB_REQ_CLEAR_FEATURE:
	case USB_REQ_SET_FEATURE:
		/* not defined */
		break;
	case USB_REQ_GET_INTERFACE:
		return usb_standard_get_interface(usbd_dev, arg);
	case USB_REQ_SET_INTERFACE:
		return usb_standard_set_interface(usbd_dev, arg);
	case USB_REQ_GET_STATUS:
		return usb_standard_interface_get_status(usbd_dev, arg);
	}

	return USBD_REQ_STALL;
}

enum usbd_control_result
_usbd_standard_request_endpoint(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	switch (arg->setup.bRequest) {
	case USB_REQ_CLEAR_FEATURE:
		if (arg->setup.wValue == USB_FEAT_ENDPOINT_HALT) {
			return usb_standard_endpoint_unstall(usbd_dev, arg);
		}
		break;
	case USB_REQ_SET_FEATURE:
		if (arg->setup.wValue == USB_FEAT_ENDPOINT_HALT) {
			return usb_standard_endpoint_stall(usbd_dev, arg);
		}
		break;
	case USB_REQ_GET_STATUS:
		return usb_standard_endpoint_get_status(usbd_dev, arg);
	case USB_REQ_SET_SYNCH_FRAME:
		/* FIXME: SYNCH_FRAME is not implemented. */
		/*
		 * SYNCH_FRAME is used for synchronization of isochronous
		 * endpoints which are not yet implemented.
		 */
		break;
	}

	return USBD_REQ_STALL;
}

enum usbd_control_result
_usbd_standard_request(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	/* Only handle Standard request. */
	if ((arg->setup.bmRequestType & USB_REQ_TYPE_TYPE) != USB_REQ_TYPE_STANDARD) {
		return USBD_REQ_STALL;
	}

	switch (arg->setup.bmRequestType & USB_REQ_TYPE_RECIPIENT) {
	case USB_REQ_TYPE_DEVICE:
		return _usbd_standard_request_device(usbd_dev, arg);
	case USB_REQ_TYPE_INTERFACE:
		return _usbd_standard_request_interface(usbd_dev, arg);
	case USB_REQ_TYPE_ENDPOINT:
		return _usbd_standard_request_endpoint(usbd_dev, arg);
	default:
		return USBD_REQ_STALL;
	}
}

/**
 * invoked (by backend) when usb device has been reset.
 * @param[in] usbd_dev usb device
 */
void _usbd_reset(usbd_device *usbd_dev)
{
	usbd_dev->current_address = 0;
	usbd_dev->current_config = NULL;
	usbd_ep_setup(usbd_dev, 0, USB_ENDPOINT_ATTR_CONTROL,
						usbd_dev->desc->bMaxPacketSize0, NULL);
	usbd_dev->backend->set_address(usbd_dev, 0);

	if (usbd_dev->callback.reset != NULL) {
		usbd_dev->callback.reset(usbd_dev);
	}
}
