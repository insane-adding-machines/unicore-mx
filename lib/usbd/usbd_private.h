/** @defgroup usb_private_defines USB Private Structures

@brief <b>Defined Constants and Types for the USB Private Structures</b>

@ingroup USB_defines

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

#ifndef __USB_PRIVATE_H
#define __USB_PRIVATE_H

#include <unicore-mx/usbd/usbd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
 * unicore-mx usb stack do not support non-zero control endpoints.
 * non-zero control endpoints are very-very rare thing.
 * so, It is not worth the headache.
 * see: http://stackoverflow.com/q/22417493/1500988
 * see: "10.1.2 Control Mechanisms" usb 2.0 specification p278 for more
 */

/** Internal collection of device information. */
struct usbd_device {
	/** Device descriptor. */
	const struct usb_device_descriptor *desc;

	/**
	 * Address of Device on bus
	 * If 0, device in default state.
	 * If not 0, device in {address, configured} state
	 */
	uint8_t current_address;

	/**
	 * Current configuration of the device.
	 * If NULL, then the device is in {default, address} state
	 * If not NULL, then device is in configured state.
	 */
	const struct usb_config_descriptor *current_config;

	/* this is where all the control transfer state information is kept */
	struct usb_control {
		enum {
			IDLE, /**< Currently idle */
			DATA_IN, /**< Send data packet to host (on next callback) */
			LAST_DATA_IN, /**< Send data (last) packet to host (on next callback) */
			STATUS_OUT, /** Status Out (on next callback) */

			DATA_OUT, /**< Receive data packet from host (in next callback) */
			LAST_DATA_OUT, /**< Receive data (last) packet from host (in next callback) */
			STATUS_IN, /**< Status In (in next callback) */
		} state;

		/* if true, send a ZLP (Zero Length packet) to host */
		bool send_zlp_at_end;

		/* provided in usbd_init(). */
		struct {
			uint8_t *buf;
			uint16_t len;
		} preserve;

		struct usbd_control_arg arg;
	} ep0;

	struct {
		/** invoked on bus-reset */
		usbd_generic_callback reset;

		/** invoked on bus-suspend */
		usbd_generic_callback suspend;

		/** invoked on bus-resume */
		usbd_generic_callback resume;

		/** invoked when sof received */
		usbd_generic_callback sof;

		/** invoked on control request */
		usbd_control_callback control;

		/** invoked on SET_CONFIGURATION */
		usbd_set_config_callback set_config;

		/** invoked on SET_INTERFACE */
		usbd_set_interface_callback set_interface;

		/** invoked on GET_DESCRIPTOR (string) */
		usbd_get_string_callback get_string;

		/** invoked on endpoint activity */
		usbd_endpoint_callback ep[8][3];
	} callback;

	/** Backend */
	const struct usbd_backend *backend;
	void *backend_data;
};

#define USB_CALLBACK_IN 0
#define USB_CALLBACK_OUT 1
#define USB_CALLBACK_SETUP 2

/* Do not appear to belong to the API, so are omitted from docs */
/**@}*/

void _usbd_control_setup(usbd_device *usbd_dev, uint8_t ea);
void _usbd_control_in(usbd_device *usbd_dev, uint8_t ea);
void _usbd_control_out(usbd_device *usbd_dev, uint8_t ea);

enum usbd_control_result
_usbd_standard_request_device(usbd_device *usbd_dev, struct usbd_control_arg *arg);

enum usbd_control_result
_usbd_standard_request_interface(usbd_device *usbd_dev, struct usbd_control_arg *arg);

enum usbd_control_result
_usbd_standard_request_endpoint(usbd_device *usbd_dev, struct usbd_control_arg *arg);

enum usbd_control_result
_usbd_standard_request(usbd_device *usbd_dev, struct usbd_control_arg *arg);

void _usbd_reset(usbd_device *usbd_dev);

/* Functions provided by the hardware abstraction. */
struct usbd_backend {
	usbd_device * (*init)(void);
	void (*set_address)(usbd_device *usbd_dev, uint8_t addr);
	void (*ep_setup)(usbd_device *usbd_dev, uint8_t addr, uint8_t type,
					uint16_t max_size);
	void (*set_ep_type)(usbd_device *usbd_dev, uint8_t addr, uint8_t type);
	void (*set_ep_size)(usbd_device *usbd_dev, uint8_t addr, uint16_t max_size);
	void (*ep_reset)(usbd_device *usbd_dev);
	void (*set_ep_stall)(usbd_device *usbd_dev, uint8_t addr,  bool stall);
	void (*set_ep_nak)(usbd_device *usbd_dev, uint8_t addr, bool nak);
	bool (*get_ep_stall)(usbd_device *usbd_dev, uint8_t addr);
	uint16_t (*ep_write_packet)(usbd_device *usbd_dev, uint8_t addr,
					const void *buf, uint16_t len);
	uint16_t (*ep_read_packet)(usbd_device *usbd_dev, uint8_t addr, void *buf,
					uint16_t len);
	void (*poll)(usbd_device *usbd_dev);
	void (*disconnect)(usbd_device *usbd_dev, bool disconnected);
	void (* enable_sof)(usbd_device *usbd_dev, bool enable);

	/* clear dtog bit */
	void (* set_ep_dtog)(usbd_device *usbd_dev, uint8_t addr, bool dtog);
	bool (* get_ep_dtog)(usbd_device *usbd_dev, uint8_t addr);

	/* flush any pending data */
	void (*ep_flush)(usbd_device *usbd_dev, uint8_t addr);

	/* Frame number */
	uint16_t (*frame_number)(usbd_device *usbd_dev);

	/*
	 * this is to tell usb generic code
	 *  that address need to be set before status-stage.
	 */
	bool set_address_before_status;

	/*
	 * Peripherial base address.
	 * Many peripherial share code.
	 * This is used to pass as parameter to code
	 */
	uint32_t base_address;
};

#define USBD_INVOKE_RESET_CALLBACK(usbd_dev) \
	_usbd_reset(usbd_dev);

#define USBD_INVOKE_SUSPEND_CALLBACK(usbd_dev)	\
	if (usbd_dev->callback.suspend != NULL) {	\
		usbd_dev->callback.suspend(usbd_dev);	\
	}

#define USBD_INVOKE_RESUME_CALLBACK(usbd_dev)	\
	if (usbd_dev->callback.resume != NULL) {	\
		usbd_dev->callback.suspend(usbd_dev);	\
	}

#define USBD_INVOKE_SOF_CALLBACK(usbd_dev)	\
	if (usbd_dev->callback.sof != NULL) {	\
		usbd_dev->callback.sof(usbd_dev);	\
	}

#define USBD_INVOKE_EP_CALLBACK(usbd_dev, type, ep_val)							\
	if (usbd_dev->callback.ep[ep_val & 0x7F][type] != NULL) {					\
		usbd_endpoint_callback cb;												\
		cb = usbd_dev->callback.ep[ep_val & 0x7F][type];						\
		if (type == USB_CALLBACK_IN) { cb(usbd_dev, ep_val | 0x80); }			\
		else if (type == USB_CALLBACK_OUT) { cb(usbd_dev, ep_val & 0x7F); }		\
		else if (type == USB_CALLBACK_SETUP) {cb(usbd_dev, ep_val & 0x7F); }	\
	}

#endif
