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
 * Copyright (C) 2015-2017 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

static inline uint8_t descriptor_type(uint16_t wValue)
{
	return wValue >> 8;
}

static inline uint8_t descriptor_index(uint16_t wValue)
{
	return wValue & 0xFF;
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
	const struct usbd_info_string *string = NULL;
	const struct usb_string_descriptor *send = NULL;
	uint8_t index = descriptor_index(arg->setup->wValue);
	uint16_t lang_id = arg->setup->wIndex;
	unsigned i, avail_lang;

	/* Search for the string data */
	if (dev->current_config != NULL) {
		for (i = 0; i < dev->info->device.desc->bNumConfigurations; i++) {
			if (dev->current_config == dev->info->config[i].desc) {
				string = dev->info->config[i].string;
				break;
			}
		}
	} else {
		string = dev->info->device.string;
	}

	if (string == NULL) {
		/* No string descriptor */
		return USBD_REQ_STALL;
	}

	if (index > string->count) {
		/* Out of range */
		return USBD_REQ_STALL;
	}

	if (!index) {
		send = string->lang_list;
	} else {
		/* Search for the data for the specified language id */
		avail_lang = (string->lang_list->bLength - 2) / 2;
		for (i = 0; i < avail_lang; i++) {
			if (string->lang_list->wData[i] == lang_id) {
				send = string->data[i][index - 1];
				break;
			}
		}
	}

	if (send == NULL) {
		/* No string descriptor to send */
		return USBD_REQ_STALL;
	}

	arg->buf = (void *) send;
	arg->len = MIN(arg->setup->wLength, send->bLength);

	/* we have done it! */
	return USBD_REQ_HANDLED;
}

/**
 * Handle GET_DESCRIPTOR(configuration) request from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 */
static enum usbd_control_result
standard_get_descriptor_config(usbd_device *dev,
						struct usbd_control_arg *arg)
{
	uint8_t index = descriptor_index(arg->setup->wValue);

	if (index >= dev->info->device.desc->bNumConfigurations) {
		return USBD_REQ_STALL;
	}

	const struct usb_config_descriptor *cfg = dev->info->config[index].desc;

	arg->buf = (void *) cfg;
	arg->len = MIN(arg->setup->wLength, cfg->wTotalLength);

	return USBD_REQ_HANDLED;
}

/**
 * Handle GET_DESCRIPTOR(device) request from host
 * @param[in] dev USB Device
 * @param[in] arg Arguments
 */
static enum usbd_control_result
standard_get_descriptor_device(usbd_device *dev,
						struct usbd_control_arg *arg)
{
	const struct usb_device_descriptor *dd = dev->info->device.desc;

	arg->buf = (void *) dd;
	arg->len = MIN(arg->setup->wLength, dd->bLength);

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

	LOGF_LN("GET_DESCRIPTOR: type = %"PRIu8", index = %"PRIu8,
					type, descriptor_index(arg->setup->wValue));

	switch (type) {
	case USB_DT_DEVICE:
	return standard_get_descriptor_device(dev, arg);
	case USB_DT_CONFIGURATION:
	return standard_get_descriptor_config(dev, arg);
	case USB_DT_STRING:
	return standard_get_descriptor_string(dev, arg);
	}

	return USBD_REQ_STALL;
}

static uint8_t _set_addr_value;

/**
 * Callback when the SET_ADDRESS stage as completed
 * @param[in] dev USB Device
 */
static void _set_address_complete(usbd_device *dev)
{
	dev->backend->set_address(dev, _set_addr_value);
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
		_set_addr_value = new_addr;
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

	for (i = 0; i < dev->info->device.desc->bNumConfigurations; i++) {
		const struct usb_config_descriptor *cfg = dev->info->config[i].desc;
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

#if (USBD_INTERFACE_MAX > 0)
	if (cfg != NULL) {
		/* Make all pointer NULL */
		memset(dev->current_iface, 0, sizeof(dev->current_iface));
	}
#endif

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
	const uint8_t *value;
	static const uint8_t zero = 0;
	value = (dev->current_config != NULL) ?
		&(dev->current_config)->bConfigurationValue /* configured state */
		: &zero /* address stage */;

	arg->buf = (void *) value;
	arg->len = MIN(arg->setup->wLength, 1);

	return USBD_REQ_HANDLED;
}

/**
 * Search for Interface with @a bInterfaceNumber and
 *  @a bAlternateSetting in @a cfg
 * @param[in] cfg Configuration descriptor
 * @param[in] bInterfaceNumber Interface number
 * @param[in] bAlternateSetting Alternate setting number
 */
static const struct usb_interface_descriptor *
search_iface(const struct usb_config_descriptor *cfg,
				uint8_t bInterfaceNumber, uint8_t bAlternateSetting)
{
	unsigned i = cfg->bLength;

	while (i < cfg->wTotalLength) {
		const struct usb_interface_descriptor *iface;
		iface = ((const void *) cfg) + i;

		if (iface->bDescriptorType == USB_DT_INTERFACE &&
			iface->bInterfaceNumber == bInterfaceNumber &&
			iface->bAlternateSetting == bAlternateSetting) {
			return iface;
		}

		i += iface->bLength;
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
	const struct usb_interface_descriptor *iface;
	uint8_t index = arg->setup->wIndex;
	uint8_t alt_set = arg->setup->wValue;

	LOGF_LN("SET_INTERFACE: num = %"PRIu8", alt-set = %"PRIu8,
		index,  alt_set);

	/* STALL since the device is in address stage (or unconfigured) */
	if (dev->current_config == NULL) {
		return USBD_REQ_STALL;
	}

#if (USBD_INTERFACE_MAX == 0)
	if (alt_set) {
		/* SET_INTERFACE for non-zero alternate setting not supported
		 * because we do not have backing storage to store the information
		 */
		return USBD_REQ_STALL;
	}
#else
	if (index >= USBD_INTERFACE_MAX) {
		/* SET_INTERFACE for the interface is not possible because
		 * the array is not sufficiently big to store the value
		 */
		return USBD_REQ_STALL;
	}
#endif

	/* search for the interface alternate setting */
	iface = search_iface(dev->current_config, index, alt_set);
	if (iface == NULL) {
		/* interface alternate setting not found */
		return USBD_REQ_STALL;
	}

#if (USBD_INTERFACE_MAX > 0)
	dev->current_iface[index] = iface;
#endif

	if (dev->callback.set_interface != NULL) {
		dev->callback.set_interface(dev, iface);
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
	uint8_t index = arg->setup->wIndex;

	/* STALL since the device is in address stage (or unconfigured) */
	if (dev->current_config == NULL) {
		return USBD_REQ_STALL;
	}

#if (USBD_INTERFACE_MAX == 0)
	/* We have no backing information to provide to host */
	return USBD_REQ_STALL;
#else
	/* We have no backing information to provide to host */
	if (index >= USBD_INTERFACE_MAX) {
		/* SET_INTERFACE for the interface is not possible because
		 * the array is not sufficiently big to store the value
		 */
		return USBD_REQ_STALL;
	}
#endif

	if (dev->current_iface[index] == NULL) {
		/* SET_INTERFACE not called yet.
		 * USB specs not documented the behaviour. resorting to STALL
		 */
		return USBD_REQ_STALL;
	}

	arg->buf = (void *) &dev->current_iface[index]->bAlternateSetting;
	arg->len = MIN(arg->setup->wLength, 1);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_device_get_status(usbd_device *dev, struct usbd_control_arg *arg)
{
	(void)dev;

	/* bit 0: self powered */
	/* bit 1: remote wakeup */

	static const uint8_t buf[2] = {0, 0};

	arg->buf = (void *) buf;
	arg->len = MIN(arg->setup->wLength, 2);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_interface_get_status(usbd_device *dev, struct usbd_control_arg *arg)
{
	(void)dev;
	/* not defined */

	static const uint8_t buf[2] = {0, 0};

	arg->buf = (void *) buf;
	arg->len = MIN(arg->setup->wLength, 2);

	return USBD_REQ_HANDLED;
}

static enum usbd_control_result
standard_endpoint_get_status(usbd_device *dev, struct usbd_control_arg *arg)
{
	static uint8_t not_halted[2] = {0, 0};
	static uint8_t halted[2] = {1, 0};

	arg->buf = usbd_get_ep_stall(dev, arg->setup->wIndex) ? halted : not_halted;
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
			USBD_LOGF_LN(USB_VSETUP,"USBD:CLEAR_FEATURE - halt EP%x", ep_addr);
			usbd_set_ep_dtog(dev, ep_addr, false);
			usbd_set_ep_stall(dev, ep_addr, false);
			return USBD_REQ_HANDLED;
		}
	break;
	case USB_REQ_SET_FEATURE:
		if (arg->setup->wValue == USB_FEAT_ENDPOINT_HALT) {
			uint8_t ep_addr = arg->setup->wIndex;
			USBD_LOGF_LN(USB_VSETUP,"USBD:SET_FEATURE - halt EP%x", ep_addr);
			usbd_set_ep_stall(dev, ep_addr, true);
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
	if ((setup_data->bmRequestType & USB_REQ_TYPE_TYPE) != USB_REQ_TYPE_STANDARD) {
		goto stall;
	}

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

	stall:
	usbd_ep0_stall(dev);
}

/* Do not appear to belong to the API, so are omitted from docs */
/**@}*/

