/**
 * @defgroup usbd_control_file Generic USB Control Requests
 *
 * @ingroup USBD
 *
 * @brief <b>Generic USB Control Requests</b>
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
#include <stdlib.h>
#include <unicore-mx/usbd/usbd.h>
#include "usbd_private.h"

enum usbd_control_result {
	USBD_REQ_STALL		= 0, /**< Stall the Request */
	USBD_REQ_HANDLED	= 1, /**< Request has been handled */
	USBD_REQ_NEXT		= 2, /**< Request to be served to next handler */
};

/**
 * Control request arguments.
 */
struct usbd_control_arg {
	const struct usb_setup_data *setup; /**< setup data */
	/** Complete callback (after status stage) */
	usbd_control_transfer_callback complete;
	void *buf; /** Buffer */
	size_t len; /**< @a buf length */
};

/**
 * Build usb configuration descriptor on the fly. \n
 * This will use @a buf to store the generate descriptor.
 * @param[in] dev USB Device
 * @param[in] cfg configuration descriptor
 * @param[in] buf Buffer
 * @param[in] len Maximum length of buffer
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

static inline uint8_t descriptor_type(uint16_t wValue)
{
	return wValue >> 8;
}

static inline uint8_t descriptor_index(uint16_t wValue)
{
	return wValue & 0xFF;
}

/**
 * Convert @a utf8 to @a utf16
 * @param[in] utf8 UTF-8 String ('\0' ended)
 * @param[out] utf16 UTF-16 String
 * @param[in] max_count Maximum number of "character" that can be store in @a utf16
 * @return Number of "character" actually stored in @a utf16
 * @return -1 on failure
 */
static int utf8_to_utf16(const uint8_t *utf8, uint16_t *utf16,
								size_t max_count)
{
	unsigned used = 0;
	unsigned i = 0;

	for (;;) {
		uint32_t utf32_ch;

		/* Read input string (utf8) till \0 is not reached */
		if (utf8[i] == '\0') {
			break;
		}

		/* Starting by assuming ASCII */
		utf32_ch = utf8[i++]; /* 0xxxxxxx */

		/* Reference: https://en.wikipedia.org/wiki/UTF-16 */

		/* utf-8? */
		if ((utf32_ch & 0x80) != 0) {
			/* Decoding header */
			uint8_t trailing_bytes_utf8;
			if ((utf32_ch & 0xe0) == 0xc0) {
				/* 110xxxxx 10xxxxxx */
				trailing_bytes_utf8 = 1;
				utf32_ch &= 0x1f;
			} else if ((utf32_ch & 0xf0) == 0xe0) {
				/* 1110xxxx 10xxxxxx 10xxxxxx */
				trailing_bytes_utf8 = 2;
				utf32_ch &= 0x0f;
			} else if ((utf32_ch & 0xf8) == 0xf0) {
				/* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
				trailing_bytes_utf8 = 3;
				utf32_ch &= 0x07;
			} else {
				/* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 5- and 6-byte sequences are not valid in utf-8 */
				return -1;
			}

			if (!utf32_ch) {
				/* The standard specifies that the correct encoding of a
				 * code point use only the minimum number of bytes required
				 * to hold the significant bits of the code point */
				return -1;
			}

			/* trailing bytes payload */
			while (trailing_bytes_utf8-- > 0) {
				utf32_ch = (utf32_ch << 6) | (utf8[i++] & 0x3f);
			}

			/* UTF-8 was restricted by RFC 3629 to end at U+10FFFF, in order
			 * to match the constraints of the UTF-16 character encoding */
			if (utf32_ch > 0x10FFFF) {
				return -1;
			}
		}

		/* Reference: https://en.wikipedia.org/wiki/UTF-16 */

		/* Converting the utf-32 character to utf-16 character */
		if ((utf32_ch <= 0xD7FF) || ((utf32_ch >= 0xE000) && (utf32_ch <= 0xFFFF))) {
			/* normal 16bit data */

			/* Out of memory? */
			if (!((used + 1) < max_count)) {
				break;
			}

			utf16[used++] = (uint16_t) utf32_ch;
		} else if ((utf32_ch >= 0x10000) && (utf32_ch <= 0x10FFFF)) {
			/* surrogate pairs */
			uint16_t surrogate_high, surrogate_low;

			/* memory available to store utf-16 character */
			if (!((used + 2) < max_count)) {
				break;
			}

			/* encode */
			utf32_ch -= 0x010000;
			surrogate_low = 0xDC00 + (utf32_ch & 0x3FF);
			surrogate_high = 0xD800 + (utf32_ch >> 10);

			/* store */
			utf16[used++] = surrogate_high;
			utf16[used++] = surrogate_low;
		} else {
			/* Cannot convert to utf-16 */
			return -1;
		}
	}

	return used;
}

/**
 * Find the string for @a lang_id and @a index for @a dev
 * @param[in] usbd_dev USB device
 * @param[in] lang_id Language ID
 * @param[in] index String index
 * @return non-NULL on success (UTF-8)
 * @return NULL on failure
 */
static const uint8_t *search_utf8_string(usbd_device *dev, uint16_t lang_id,
			size_t index)
{
	const struct usb_string_utf8_data *string =
		(dev->current_config != NULL) ? dev->current_config->string :
			dev->desc->string;

	if (string == NULL) {
		return NULL;
	}

	for (; string->data != NULL; string++) {
		if (string->lang_id != lang_id) {
			continue;
		}

		if (index >= string->count) {
			break;
		}

		return string->data[index];
	}

	return NULL;
}

/**
 * Build a list of Language ID that are available
 * @param[in] dev USB Device
 * @param[out] res Result to store
 * @param[in] max_count Maximum number of entries that can be stored in @a res
 * @return Number of entries stored in @a
 * @return -1 on failure
 */
static int build_available_lang(usbd_device *dev, uint16_t *res,
						size_t max_count)
{
	size_t i;
	const struct usb_string_utf8_data *string =
		(dev->current_config != NULL) ? dev->current_config->string :
			dev->desc->string;

	if (string == NULL) {
		return -1;
	}

	for (i = 0; i < max_count; i++) {
		const struct usb_string_utf8_data *str = &string[i];
		if (str->data == NULL) {
			break;
		}

		res[i] = str->lang_id;
	}

	return i;
}

/**
 * Handle GET_DESCRIPTOR(string) request from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 */
static enum usbd_control_result
standard_get_descriptor_string(usbd_device *dev,
						struct usbd_control_arg *arg)
{
	struct usb_string_descriptor *sd = dev->preserve.buf;
	const unsigned header_size = sizeof(sd->bLength) +
						sizeof(sd->bDescriptorType);
	size_t max_count = (dev->preserve.len - header_size) / sizeof(sd->wData[0]);
	int used;
	uint8_t index = descriptor_index(arg->setup->wValue);

	if (!index) {
		/* Build the list of available langauge strings */
		used = build_available_lang(dev, (uint16_t*)sd->wData, max_count);
	} else {
		/* Search for the UTF-8 string and convert it to UTF-16 */
		const uint8_t *utf8_str;
		utf8_str = search_utf8_string(dev, arg->setup->wIndex, index - 1);
		used = (utf8_str == NULL) ? -1 :
			utf8_to_utf16(utf8_str, (uint16_t*)sd->wData, max_count);
	}

	if (used < 0) {
		return USBD_REQ_STALL;
	}

	/* Copy back data */
	sd->bLength = header_size + (used * sizeof(sd->wData[0]));
	sd->bDescriptorType = USB_DT_STRING;

	arg->buf = sd;
	arg->len = MIN(arg->setup->wLength, sd->bLength);

	/* we have done it! */
	return USBD_REQ_HANDLED;
}

/**
 * Handle GET_DESCRIPTOR request from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 */
static enum usbd_control_result
standard_get_descriptor(usbd_device *dev, struct usbd_control_arg *arg)
{
	uint8_t type = descriptor_type(arg->setup->wValue);
	uint8_t index = descriptor_index(arg->setup->wValue);

	LOGF_LN("GET_DESCRIPTOR: type = %"PRIu8", index = %"PRIu8,
					type, index);

	switch (type) {
	case USB_DT_DEVICE:
		arg->buf = (void *) dev->desc;
		arg->len = MIN(arg->setup->wLength, dev->desc->bLength);
	return USBD_REQ_HANDLED;
	case USB_DT_CONFIGURATION:
		if (index < dev->desc->bNumConfigurations) {
			arg->buf = dev->preserve.buf;
			arg->len = build_config_descriptor(
							&dev->desc->config[index],
							dev->preserve.buf,
							MIN(arg->setup->wLength, dev->preserve.len));
			return USBD_REQ_HANDLED;
		}
	case USB_DT_STRING:
		return standard_get_descriptor_string(dev, arg);
	}
	return USBD_REQ_STALL;
}

/**
 * Callback when the SET_ADDRESS stage as completed
 * @param[in] dev USB Device
 */
static void _set_address_complete(usbd_device *dev)
{
	uint8_t *buf = dev->preserve.buf;
	dev->backend->set_address(dev, buf[0]);
}

/**
 * Handle SET_ADDRESS request from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 * @return Result
 */
static enum usbd_control_result
standard_set_address(usbd_device *dev, struct usbd_control_arg *arg)
{
	uint8_t new_addr = arg->setup->wValue;
	LOGF_LN("SET_ADDRESS: %"PRIu8, new_addr);

	/* Control Out should only be used */
	if (arg->setup->bmRequestType != 0x00) {
		return USBD_REQ_STALL;
	}

	/*
	 * Some parts require the address to be set before status stage.
	 */
	if (dev->backend->set_address_before_status) {
		dev->backend->set_address(dev, new_addr);
	} else {
		/* Store the new address in EP0 buffer for complete callback */
		uint8_t *buf = dev->preserve.buf;
		buf[0] = new_addr;
		arg->complete =
			(usbd_control_transfer_callback) _set_address_complete;
	}

	return USBD_REQ_HANDLED;
}

/**
 * Search for the configuration with bConfigurationValue == @a value
 * @return the found configuration (success)
 * @return NULL on failure (not found)
 */
static const struct usb_config_descriptor*
search_config(usbd_device *dev, uint8_t value)
{
	unsigned i;

	for (i = 0; i < dev->desc->bNumConfigurations; i++) {
		const struct usb_config_descriptor *cfg = &dev->desc->config[i];
		if (cfg->bConfigurationValue == value) {
			return cfg;
		}
	}

	return NULL;
}

/**
 * Handle SET_CONFIGURATION from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 * @return Result
 */
static enum usbd_control_result
standard_set_configuration(usbd_device *dev, struct usbd_control_arg *arg)
{
	unsigned i;
	const struct usb_config_descriptor *cfg = NULL;
	uint8_t bConfigValue = arg->setup->wValue;

	LOGF_LN("SET_CONFIGURATION: %"PRIu8, bConfigValue);

	if (bConfigValue > 0) {
		cfg = search_config(dev, bConfigValue);
		if (cfg == NULL) {
			return USBD_REQ_STALL;
		}
	}

	dev->current_config = cfg;

	if (cfg != NULL) {
		/* reset all alternate settings configuration */
		for (i = 0; i < cfg->bNumInterfaces; i++) {
			if (cfg->interface[i].cur_altsetting) {
				*cfg->interface[i].cur_altsetting = 0;
			}
		}
	}

	/* We have no use for old non EP0 transfer now */
	LOG_LN("SET_CONFIGURATION: Removing all non endpoint 0 transfer");
	usbd_purge_all_non_ep0_transfer(dev, USBD_ERR_CONFIG_CHANGE);

	/* Make sure that while backend is preparing the endpoint, no transfers
	 * are submitted to backend.
	 * When preperation is over, transfers will be schedule in the order they
	 *  were submitted!
	 */
	LOG_LN("SET_CONFIGURATION: force_all_new_urb_to_waiting = true");
	dev->urbs.force_all_new_urb_to_waiting = true;

	if (dev->backend->ep_prepare_start) {
		dev->backend->ep_prepare_start(dev);
	}

	if (dev->callback.set_config != NULL) {
		dev->callback.set_config(dev, cfg);
	}

	if (dev->backend->ep_prepare_end) {
		dev->backend->ep_prepare_end(dev);
	}

	/* Now all transfer are allowed */
	LOG_LN("SET_CONFIGURATION: force_all_new_urb_to_waiting = false");
	dev->urbs.force_all_new_urb_to_waiting = false;

	/* Schedule new transfer that have been submitted in set_config callback */
	usbd_urb_schedule(dev);

	return USBD_REQ_HANDLED;
}

/**
 * Handle GET_CONFIGURATION from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 * @return Result
 */
static enum usbd_control_result
standard_get_configuration(usbd_device *dev, struct usbd_control_arg *arg)
{
	uint8_t *buf = dev->preserve.buf;
	buf[0] = (dev->current_config != NULL) ?
		dev->current_config->bConfigurationValue /* configured state */
		: 0 /* address stage */;

	arg->buf = buf;
	arg->len = MIN(arg->setup->wLength, 1);

	return USBD_REQ_HANDLED;
}

/**
 * Search for Interface with bInterfaceNumber = @a bInterfaceNumber in @a cfg
 * @param[in] cfg Configuration
 * @param[in] bInterfaceNumber Interface number
 * @return Interface on success
 * @return NULL on failure
 */
static const struct usb_interface * search_iface(
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

/**
 * Search for Alternate setting bAlternateSetting == @a bAlternateSetting
 *  from @a iface
 * @param[in] iface Interface
 * @param[in] bAlternateSetting Alternate setting number
 */
static const struct usb_interface_descriptor *
search_iface_altset(const struct usb_interface *iface,
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

/**
 * Handle SET_INTERFACE from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 * @return Result
 */
static enum usbd_control_result
standard_set_interface(usbd_device *dev, struct usbd_control_arg *arg)
{
	const struct usb_interface *iface;
	const struct usb_interface_descriptor *iface_alt;

	uint8_t index = arg->setup->wIndex;
	uint8_t alt_set = arg->setup->wValue;

	LOGF_LN("SET_INTERFACE: num = %"PRIu8", alt-set = %"PRIu8,
		index,  alt_set);

	/* STALL since the device is in address stage */
	if (dev->current_config == NULL) {
		return USBD_REQ_STALL;
	}

	/* search for the interface */
	iface = search_iface(dev->current_config, index);
	if (iface == NULL) {
		/* interface not found */
		return USBD_REQ_STALL;
	}

	/* search for the interface alternate setting */
	iface_alt = search_iface_altset(iface, alt_set);
	if (iface_alt == NULL) {
		/* interface alternate setting not found */
		return USBD_REQ_STALL;
	}

	if (iface->cur_altsetting != NULL) {
		*iface->cur_altsetting = alt_set;
	} else if (alt_set > 0) {
		return USBD_REQ_STALL;
	}

	if (dev->callback.set_interface != NULL) {
		dev->callback.set_interface(dev, iface, iface_alt);
	}

	return USBD_REQ_HANDLED;
}

/**
 * Handle GET_INTERFACE from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 * @return Result
 */
static enum usbd_control_result
standard_get_interface(usbd_device *dev, struct usbd_control_arg *arg)
{
	const struct usb_interface *iface;

	if (dev->current_config == NULL) {
		return USBD_REQ_STALL;
	}

	iface = search_iface(dev->current_config, arg->setup->wIndex);
	if (iface == NULL) {
		return USBD_REQ_STALL;
	}

	uint8_t *buf = dev->preserve.buf;
	buf[0] = (iface->cur_altsetting) ? *iface->cur_altsetting : 0;

	arg->buf = buf;
	arg->len = MIN(arg->setup->wLength, 1);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_device_get_status(usbd_device *dev, struct usbd_control_arg *arg)
{
	(void)dev;

	/* bit 0: self powered */
	/* bit 1: remote wakeup */

	uint8_t *buf = dev->preserve.buf;
	buf[0] = 0;
	buf[1] = 0;

	arg->buf = buf;
	arg->len = MIN(arg->setup->wLength, 2);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_interface_get_status(usbd_device *dev, struct usbd_control_arg *arg)
{
	(void)dev;
	/* not defined */

	uint8_t *buf = dev->preserve.buf;
	buf[0] = 0;
	buf[1] = 0;

	arg->buf = buf;
	arg->len = MIN(arg->setup->wLength, 2);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_endpoint_get_status(usbd_device *dev, struct usbd_control_arg *arg)
{
	uint8_t *buf = dev->preserve.buf;
	buf[0] = usbd_get_ep_stall(dev, arg->setup->wIndex) ? 1 : 0;
	buf[1] = 0;

	arg->buf = buf;
	arg->len = MIN(arg->setup->wLength, 2);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_request_device(usbd_device *dev, struct usbd_control_arg *arg)
{
	switch (arg->setup->bRequest) {
	case USB_REQ_CLEAR_FEATURE:
	case USB_REQ_SET_FEATURE:
		if (arg->setup->wValue == USB_FEAT_DEVICE_REMOTE_WAKEUP) {
			/* Device wakeup code goes here. */
		}

		if (arg->setup->wValue == USB_FEAT_TEST_MODE) {
			/* Test mode code goes here. */
		}

	break;
	case USB_REQ_SET_ADDRESS:
	return standard_set_address(dev, arg);
	case USB_REQ_SET_CONFIGURATION:
	return standard_set_configuration(dev, arg);
	case USB_REQ_GET_CONFIGURATION:
	return standard_get_configuration(dev, arg);
	case USB_REQ_GET_DESCRIPTOR:
	return standard_get_descriptor(dev, arg);
	case USB_REQ_GET_STATUS:
		/*
		 * GET_STATUS always responds with zero reply.
		 * The application may override this behaviour.
		 */
	return standard_device_get_status(dev, arg);
	case USB_REQ_SET_DESCRIPTOR:
		/* SET_DESCRIPTOR is optional and not implemented. */
	break;
	}

	return USBD_REQ_STALL;
}

static enum usbd_control_result
standard_request_interface(usbd_device *dev, struct usbd_control_arg *arg)
{
	switch (arg->setup->bRequest) {
	case USB_REQ_CLEAR_FEATURE:
	case USB_REQ_SET_FEATURE:
		/* not defined */
	break;
	case USB_REQ_GET_INTERFACE:
	return standard_get_interface(dev, arg);
	case USB_REQ_SET_INTERFACE:
	return standard_set_interface(dev, arg);
	case USB_REQ_GET_STATUS:
	return standard_interface_get_status(dev, arg);
	}

	return USBD_REQ_STALL;
}

static enum usbd_control_result
standard_request_endpoint(usbd_device *dev, struct usbd_control_arg *arg)
{
	switch (arg->setup->bRequest) {
	case USB_REQ_CLEAR_FEATURE:
		if (arg->setup->wValue == USB_FEAT_ENDPOINT_HALT) {
			uint8_t ep_addr = arg->setup->wIndex;
			usbd_set_ep_dtog(dev, ep_addr, false);
			usbd_set_ep_stall(dev, ep_addr, false);
			return USBD_REQ_HANDLED;
		}
	break;
	case USB_REQ_SET_FEATURE:
		if (arg->setup->wValue == USB_FEAT_ENDPOINT_HALT) {
			usbd_set_ep_stall(dev, arg->setup->wIndex, true);
			return USBD_REQ_HANDLED;
		}
	break;
	case USB_REQ_GET_STATUS:
	return standard_endpoint_get_status(dev, arg);
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

static enum usbd_control_result
standard_request(usbd_device *dev, struct usbd_control_arg *arg)
{
	switch (arg->setup->bmRequestType & USB_REQ_TYPE_RECIPIENT) {
	case USB_REQ_TYPE_DEVICE:
	return standard_request_device(dev, arg);
	case USB_REQ_TYPE_INTERFACE:
	return standard_request_interface(dev, arg);
	case USB_REQ_TYPE_ENDPOINT:
	return standard_request_endpoint(dev, arg);
	default:
	return USBD_REQ_STALL;
	}
}

void usbd_ep0_setup(usbd_device *dev, const struct usb_setup_data *setup_data)
{
	/* Only handle Standard request. */
	if ((setup_data->bmRequestType & USB_REQ_TYPE_TYPE) == USB_REQ_TYPE_STANDARD) {

	struct usbd_control_arg arg = {
		.setup = setup_data,
		.complete = NULL,
		.buf = NULL,
		.len = 0
	};

	if (standard_request(dev, &arg) == USBD_REQ_HANDLED) {
		usbd_ep0_transfer(dev, setup_data, arg.buf, arg.len,
			arg.complete);
		return;
	}
	}//if ( ...== USB_REQ_TYPE_STANDARD)

	//stall:
	usbd_ep0_stall(dev);
}

/* Do not appear to belong to the API, so are omitted from docs */
/**@}*/

