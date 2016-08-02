/** @defgroup usb_control_file Generic USB Control Requests

@ingroup USB

@brief <b>Generic USB Control Requests</b>

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

#include <stdlib.h>
#include <unicore-mx/usbd/usbd.h>
#include "usbd_private.h"

/**
 * According to the USB 2.0 specification, section 8.5.3, when a control \n
 * transfer is stalled, the pipe becomes idle. We provide one utility to stall \n
 * a transaction to reduce boilerplate code.
 * @param[in] usbd_dev usb device
 */
static void stall_transaction(usbd_device *usbd_dev)
{
	usbd_set_ep_stall(usbd_dev, 0, true);
	usbd_dev->ep0.state = IDLE;
}

/**
 * Send a subset of data from ctrl_buf to host, using EP0.
 * @param[in] usbd_dev usb device
 */
static void usb_control_send_chunk(usbd_device *usbd_dev)
{
	struct usb_control *ctrl = &(usbd_dev->ep0);
	uint16_t chunk_size = MIN(usbd_dev->desc->bMaxPacketSize0, ctrl->arg.len);

	/* Data stage, normal transmission */
	usbd_ep_write_packet(usbd_dev, 0, ctrl->arg.buf, chunk_size);
	ctrl->arg.buf += chunk_size;
	ctrl->arg.len -= chunk_size;

	/* decide next state */
	if (!ctrl->arg.len) {
		ctrl->state = ctrl->send_zlp_at_end ? DATA_IN : LAST_DATA_IN;
		ctrl->send_zlp_at_end = false;
	} else {
		ctrl->state = DATA_IN;
	}
}

/**
 * Receive a subset of data from host and store it into ctrl_buf, using EP0. \n
 * Here ctrl_buf contain the "Control OUT transfer data stage" data. \n
 * @param[in] usbd_dev usb device
 */
static int usb_control_recv_chunk(usbd_device *usbd_dev)
{
	usbd_control_arg *arg = &(usbd_dev->ep0.arg);
	uint16_t remaining = arg->setup.wLength - arg->len;
	uint16_t packet_size = MIN(usbd_dev->desc->bMaxPacketSize0, remaining);
	uint8_t *mem = arg->buf + arg->len;
	uint16_t readed_size = usbd_ep_read_packet(usbd_dev, 0, mem, packet_size);

	if (readed_size != packet_size) {
		stall_transaction(usbd_dev);
		return -1;
	}

	arg->len += readed_size;

	return packet_size;
}

/**
 * Dispach the control request to application code. \n
 * If control-callback is not set, stack try to handle it. \n
 * If control-transfer return USBD_REQ_NEXT, then stack try to handle it. \n
 * Else, application code handled the control request.
 * @param[in] usbd_dev usb device
 * @retval @a USBD_REQ_STALL to stall the control request
 * @retval @a USBD_REQ_HANDLED to proceed to data/status stage
 */
static enum usbd_control_result
usb_control_dispatch(usbd_device *usbd_dev)
{
	usbd_control_arg *arg = &(usbd_dev->ep0.arg);

	/* Call user command hook function. */
	if(usbd_dev->callback.control != NULL) {
		enum usbd_control_result result;
		result = usbd_dev->callback.control(usbd_dev, arg);

		if (result != USBD_REQ_NEXT) {
			return result;
		}
	}

	/* Try standard request if not already handled. */
	return _usbd_standard_request(usbd_dev, arg);
}

/**
 * Handle no-data (aka command) request
 * @param[in] usbd_dev usb device
 * @sa usb_control_setup_write()
 * @sa usb_control_setup_read()
 */
static void usb_control_setup_no_data(usbd_device *usbd_dev)
{
	struct usb_control *ctrl = &(usbd_dev->ep0);
	ctrl->arg.buf = NULL;
	ctrl->arg.len = 0;

	if (usb_control_dispatch(usbd_dev) == USBD_REQ_HANDLED) {
		/* Go to status stage if handled. */
		usbd_ep_write_packet(usbd_dev, 0, NULL, 0);
		ctrl->state = STATUS_IN;
	} else {
		/* Stall endpoint on failure. */
		stall_transaction(usbd_dev);
	}
}

/**
 * Handle read requests.
 * @param[in] usbd_dev usb device
 * @sa usb_control_setup_write()
 * @sa usb_control_setup_no_data()
 */
static void usb_control_setup_read(usbd_device *usbd_dev)
{
	struct usb_control *ctrl = &(usbd_dev->ep0);
	usbd_control_arg *arg = &(ctrl->arg);
	arg->buf = ctrl->preserve.buf;
	arg->len = arg->setup.wLength;

	if (usb_control_dispatch(usbd_dev) == USBD_REQ_HANDLED) {
		ctrl->send_zlp_at_end = false;
		/* application is sending less data than host is expecting.
		 * and if application send length is multiple of ep0 size,
		 * then we have to send a ZLP (Zero Length Packet) to host
		 * to notify that we are out of data (in the end) */
		if (arg->len < arg->setup.wLength) {
			if (arg->len) {
				if (!(arg->len % usbd_dev->desc->bMaxPacketSize0)) {
					ctrl->send_zlp_at_end = true;
				}
			}
		}
		/* Go to data out stage */
		usb_control_send_chunk(usbd_dev);
	} else {
		/* Stall endpoint on failure. */
		stall_transaction(usbd_dev);
	}
}

/**
 * Handle write requests.
 * @param[in] usbd_dev usb device
 * @sa usb_control_setup_read()
 * @sa usb_control_setup_no_data()
 */
static void usb_control_setup_write(usbd_device *usbd_dev)
{
	struct usb_control *ctrl = &(usbd_dev->ep0);
	usbd_control_arg *arg = &(ctrl->arg);

	/* Host want to write data from than available buffer? */
	if (arg->setup.wLength > ctrl->preserve.len) {
		stall_transaction(usbd_dev);
		return;
	}

	/* use (internal) control buffer */
	arg->buf = ctrl->preserve.buf;
	arg->len = 0;

	/* Wait for DATA OUT stage. */
	if (arg->setup.wLength > usbd_dev->desc->bMaxPacketSize0) {
		ctrl->state = DATA_OUT;
	} else {
		ctrl->state = LAST_DATA_OUT;
	}
}

/* Do not appear to belong to the API, so are omitted from docs */
/**@}*/

/**
 * Internal handler for setup callback on EP0.
 * Backend will call this function whenever a SETUP is received.
 * @param[in] usbd_dev usb device
 * @param[in] ea Endpoint address (ignored)
 * @note
 *  Limitation: \n
 *  As per USB specification, there can be multiple control callback. \n
 *  To keep things simple, stack only support EP0 address as control callback. \n
 *  non-zero control endpoints are *very* *very* rare and have limited application.
 */
void _usbd_control_setup(usbd_device *usbd_dev, uint8_t ea)
{
	(void)ea;
	struct usb_control *ctrl = &(usbd_dev->ep0);
	usbd_control_arg *arg = &(ctrl->arg);
	struct usb_setup_data *setup = &(arg->setup);

	/* read setup data */
	if (usbd_ep_read_packet(usbd_dev, 0, setup, 8) != 8) {
		stall_transaction(usbd_dev);
		return;
	}

	/* prepare for handling */
	arg->complete = NULL;

	if (setup->wLength == 0) {
		usb_control_setup_no_data(usbd_dev);
	} else if (setup->bmRequestType & 0x80) {
		usb_control_setup_read(usbd_dev);
	} else {
		usb_control_setup_write(usbd_dev);
	}
}

/* for readability */
static void complete_transaction(usbd_device *usbd_dev)
{
	struct usb_control *ctrl = &(usbd_dev->ep0);
	usbd_control_arg *arg = &(ctrl->arg);
	if (arg->complete) {
		arg->complete(usbd_dev, arg);
	}

	/* clear all (internal) states */
	ctrl->state = IDLE;
	arg->len = 0;
	arg->buf = NULL;
	arg->complete = NULL;
}

/**
 * Internal handler for data-out callback on EP0.
 * Backend will call this function whenever data is received.
 * @param[in] usbd_dev usb device
 * @param[in] ea Endpoint address (ignored)
 * @note see @a _usbd_control_setup() for limitation
 * @sa _usbd_control_setup()
 */
void _usbd_control_out(usbd_device *usbd_dev, uint8_t ea)
{
	(void)ea;
	struct usb_control *ctrl = &(usbd_dev->ep0);

	switch (ctrl->state) {
	case DATA_OUT:
		if (usb_control_recv_chunk(usbd_dev) < 0) {
			break;
		}
		/* next state? */
		if ((ctrl->arg.setup.wLength - ctrl->arg.len) <= usbd_dev->desc->bMaxPacketSize0) {
			ctrl->state = LAST_DATA_OUT;
		}
		break;
	case LAST_DATA_OUT:
		if (usb_control_recv_chunk(usbd_dev) < 0) {
			break;
		}
		/*
		 * We have now received the full data payload.
		 * Invoke callback to process.
		 */
		if (usb_control_dispatch(usbd_dev) == USBD_REQ_HANDLED) {
			/* Got to status stage on success. */
			usbd_ep_write_packet(usbd_dev, 0, NULL, 0);
			ctrl->state = STATUS_IN;
		} else {
			stall_transaction(usbd_dev);
		}
		break;
	case STATUS_OUT:
		usbd_ep_read_packet(usbd_dev, 0, NULL, 0);
		complete_transaction(usbd_dev);
		break;
	default:
		stall_transaction(usbd_dev);
	}
}

/**
 * Internal handler for data-in callback on EP0. \n
 * Backend will call this function whenever data is send from endpoint.
 * @param[in] usbd_dev usb device
 * @param[in] ea Endpoint address (ignored)
 * @note see @a _usbd_control_setup() for limitation
 * @sa _usbd_control_setup()
 */
void _usbd_control_in(usbd_device *usbd_dev, uint8_t ea)
{
	(void)ea;

	switch (usbd_dev->ep0.state) {
	case DATA_IN:
		usb_control_send_chunk(usbd_dev);
		break;
	case LAST_DATA_IN:
		/*  At this stage, all data to host has been sent.
		 *  Now, we are just waiting for the Status-Out */
		usbd_dev->ep0.state = STATUS_OUT;
		break;
	case STATUS_IN:
		complete_transaction(usbd_dev);
		break;
	default:
		stall_transaction(usbd_dev);
	}
}

