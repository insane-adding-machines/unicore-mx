/** @defgroup usb_driver_defines USB Drivers

@brief <b>Defined Constants and Types for the USB Drivers</b>

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

#ifndef UNICOREMX_USBD_H
#define UNICOREMX_USBD_H

#include <stddef.h>
#include <unicore-mx/usb/usbstd.h>

BEGIN_DECLS

enum usbd_control_result {
	USBD_REQ_STALL		= 0, /**< Stall the Request */
	USBD_REQ_HANDLED	= 1, /**< Request has been handled */
	USBD_REQ_NEXT		= 2, /**< Request to be served to next handler */
};

typedef struct usbd_backend usbd_backend;
typedef struct usbd_device usbd_device;

extern const usbd_backend usbd_stm32_fsdev_v1;
extern const usbd_backend usbd_stm32_fsdev_v2;
extern const usbd_backend usbd_stm32_otg_fs;
extern const usbd_backend usbd_stm32_otg_hs;
extern const usbd_backend usbd_lm4f;
extern const usbd_backend usbd_efm32lg;

#define USBD_STM32_FSDEV_V1	(&usbd_stm32_fsdev_v1)
#define USBD_STM32_FSDEV_V2	(&usbd_stm32_fsdev_v2)
#define USBD_STM32_OTG_FS	(&usbd_stm32_otg_fs)
#define USBD_STM32_OTG_HS	(&usbd_stm32_otg_hs)
#define USBD_LM4F			(&usbd_lm4f)
#define USBD_EFM32LG		(&usbd_efm32lg)

typedef struct usbd_control_arg usbd_control_arg;

/**
 * Complete callback (invoked after status stage)
 * @param usbd_dev USB Device
 * @param arg Control callback argument
 */
typedef void (*usbd_control_complete_callback)(usbd_device *usbd_dev,
						usbd_control_arg *arg);

/**
 * @param usbd_dev USB Device
 * @param arg Control callback argument
 */
typedef enum usbd_control_result (*usbd_control_callback)(
		usbd_device *usbd_dev, struct usbd_control_arg *arg);

/**
 * @param usbd_dev USB Device
 * @param cfg Configuration
 */
typedef void (*usbd_set_config_callback)(usbd_device *usbd_dev,
				const struct usb_config_descriptor *cfg);

/**
 * @param usbd_dev USB Device
 * @param iface Interface
 * @param altsetting Alternate Setting
 */
typedef void (*usbd_set_interface_callback)(usbd_device *usbd_dev,
		const struct usb_interface *iface,
		const struct usb_interface_descriptor *altsetting);

/**
 * @param usbd_dev USB Device
 * @param ep Endpoint
 */
typedef void (* usbd_endpoint_callback)(usbd_device *usbd_dev, uint8_t ep);

/* Application code SHOULD NOT modify any member */
struct usbd_get_string_arg {
	uint8_t index;
	uint16_t lang_id;
	uint16_t *buf;
	uint16_t len;
};

/**
 * @param usbd_dev USB Device
 * @param index String index
 * @param lang_id Language ID (see USB_LANGID_* or USB_LANGID() macro)
 * @param[out] utf16 utf-16 string memory
 * @param max_count Maximum number of character (16bit) can be written to @a utf16
 * @return less than 0 to return error
 * @return 0 or greater than to return size
 * @note Application code should not exceed max_count number of character.
 * @note Application code should not return error (-ve value)
 *      in case it could not fit the whole string (it has).
 *     Instead it copy upto the available memory and return the filled memory.
 * @note if index is equal to zero,
 *   Application code need to copy Language (languages Identifier)
 *    that it support (ignore the lang_id paramter)
 */
typedef int (* usbd_get_string_callback)(usbd_device *usbd_dev,
					struct usbd_get_string_arg *arg);

/**
 * Control request arguments.
 */
struct usbd_control_arg {
	struct usb_setup_data setup __attribute__((aligned(4))); /**< setup data */
	usbd_control_complete_callback complete; /**< Complete callback (after status stage) */
	uint8_t *buf; /** Buffer */
	uint16_t len; /**< @a buf length */
};

/**
 * Main initialization entry point.
 *
 * Initialize the USB firmware library to implement the USB device described
 * by the descriptors provided.
 *
 * @param backend Backend
 * @param dev Pointer to USB device descriptor. This must not be changed while
 *            the device is in use.
 * @param conf Pointer to array of USB configuration descriptors. These must
 *             not be changed while the device is in use. The length of this
 *             array is determined by the bNumConfigurations field in the
 *             device descriptor.
 * @param strings TODO
 * @param control_buffer Pointer to array that would hold the data
 *                       received during control requests with DATA
 *                       stage
 * @param control_buffer_size Size of control_buffer
 * @return the usb device initialized for use.
 */
usbd_device* usbd_init(const usbd_backend *backend,
					const struct usb_device_descriptor *dev,
					uint8_t *control_buffer, uint16_t control_buffer_size);

typedef void (* usbd_generic_callback)(usbd_device *usbd_dev);

/**
 * Register bus reset callback
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 */
void usbd_register_reset_callback(usbd_device *usbd_dev,
					usbd_generic_callback callback);

/**
 * Register suspend callback
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 */
void usbd_register_suspend_callback(usbd_device *usbd_dev,
					usbd_generic_callback callback);

/**
 * Register resume callback
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 */
void usbd_register_resume_callback(usbd_device *usbd_dev, usbd_generic_callback callback);

/**
 * Register start-of-frame callback
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 */
void usbd_register_sof_callback(usbd_device *usbd_dev, usbd_generic_callback callback);

/** Register application callback function for handling USB control requests. \n
 * @param[in] usbd_dev usb device
 * @param[in] callback control callback.
 * @note Internally the stack first pass the control request to application code. \n
 *  - If application code handle the control request, \n
 *     To proceed to status-stage: @a callback return @a USBD_REQ_HANDLED \n
 *     To STALL: @a callback return @a USBD_REQ_STALL \n
 *  - If application code want stack to handle the control request. \n
 *     (ex: standard control request), \n
 *     The @a callback need to return USBD_REQ_NEXT \n
 * @note If no callback has been assigned, USBD_REQ_NEXT is assumed.
 * @note Using @a callback, application code can override any control request.
 *   (obviously that include standard request)
 */
void usbd_register_control_callback(usbd_device *usbd_dev,
				usbd_control_callback callback);

/**
 * Register SET_CONFIGURATION callback \n
 * stack will invoke @a callback when ever SET_CONFIGURATION is received
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 */
void usbd_register_set_config_callback(usbd_device *usbd_dev,
				usbd_set_config_callback callback);

/**
 * Register set-interface callback \n
 * stack will invoke @a callback when ever SET_INTERFACE is received
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 */
void usbd_register_set_interface_callback(usbd_device *usbd_dev,
				usbd_set_interface_callback callback);

/**
 * Register get-descriptor (string) callback \n
 * stack will invoke @a callback when ever SET_DESCRIPTOR (string) is received
 * @param[in] usbd_dev usb device
 * @param[in] callback callback to be invoked
 * @sa usbd_utf8_to_utf16()
 * @sa usbd_handle_string_us_english()
 */
void usbd_register_get_string_callback(usbd_device *usbd_dev,
				usbd_get_string_callback callback);

/**
 * Poll the device.
 * @param[in] usbd_dev usb device
 */
void usbd_poll(usbd_device *usbd_dev);

/**
 * Force disconnect.
 * @param[in] usbd_dev usb device
 * @param[in] disconnect perform disconnection
 * @note not all backend support this facility.
 */
void usbd_disconnect(usbd_device *usbd_dev, bool disconnected);

/**
 * Initalize an endpoint
 * @param[in] usbd_dev usb device
 * @param[in] addr endpoint number including direction
 * @param[in] type endpoint type
 * @param[in] max_size maximum number of bytes the endpoint can hold
 * @param[in] callback function to invoke when data is receivied or transmitted.
 * @note do not pass addr = ep0 (0x00 or 0x80), as it is internally used.
 * @note the prefered location is set-configuration callback
 * @sa usbd_set_ep_type()
 * @sa usbd_set_ep_size()
 * @sa usbd_register_ep_callback()
 */
void usbd_ep_setup(usbd_device *usbd_dev, uint8_t addr, uint8_t type,
					uint16_t max_size, usbd_endpoint_callback callback);

/**
 * Reconfigure type of endpoint after @a usbd_ep_setup() has been callback.
 * @param[in] usbd_dev usb device
 * @param[in] addr endpoint number including direction
 * @param[in] type endpoint type
 * @sa usbd_ep_setup()
 * @note not all backend support this facility.
 */
void usbd_set_ep_type(usbd_device *usbd_dev, uint8_t addr, uint8_t type);

/**
 * Reconfigure size of endpoint after endpoint has be setup'd.
 * @param usbd_dev USB Device
 * @param addr endpoint number including direction
 * @param max_size the new size to be configured.
 * @note max_size should not be greater
 *        than what was passed at the time of setup.
 * @see usbd_ep_setup()
 * @note not all backend support this facility.
 */
void usbd_set_ep_size(usbd_device *usbd_dev, uint8_t addr,
						uint16_t max_size);

/**
 * Write data to IN endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (assuming IN)
 * @param buf Buffer
 * @param len Number of bytes
 * @note memory alignment of @a buf can help in copying quicker,
 *        check backend requirement.
 * @return the number of bytes actually copied.
 */
uint16_t usbd_ep_write_packet(usbd_device *usbd_dev, uint8_t addr,
				const void *buf, uint16_t len);

/**
 * Read data from IN endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (assuming OUT)
 * @param buf Buffer
 * @param len Number of bytes
 * @note memory alignment of @a buf can help in copying quicker,
 *        check backend requirement.
 * @return the number of bytes actually readed.
 * @internal
 * @note this is also used to read setup data
 * @endinternal
 */
uint16_t usbd_ep_read_packet(usbd_device *usbd_dev, uint8_t addr,
						void *buf, uint16_t len);

/**
 * STALL the endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 * @param stall true to stall the endpoint
 */
void usbd_set_ep_stall(usbd_device *usbd_dev, uint8_t addr, bool stall);

/**
 * Get STALL status of endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 * @retval true if endpoint in STALL
 */
bool usbd_get_ep_stall(usbd_device *usbd_dev, uint8_t addr);

/**
 * NAK the endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 * @param nak true to NAK the endpoint
 */
void usbd_set_ep_nak(usbd_device *usbd_dev, uint8_t addr, bool nak);

/**
 * Get NAK status of endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 * @retval true if endpoint NAK
 * @note IN callback will never fail to write data to the same endpoint
 * @warning OUT callback should read the data or
 *    else it will be dropped after the callback return
 */
void usbd_register_ep_callback(usbd_device *usbd_dev, uint8_t addr,
				usbd_endpoint_callback callback);

/**
 * Set the endpoint data-toggle
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 * @param dtog New data toggle value
 */
void usbd_set_ep_dtog(usbd_device *usbd_dev, uint8_t addr, bool dtog);

/**
 * Get the endpoint data-toggle
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 * @return true if data-toggle = 1
 */
bool usbd_get_ep_dtog(usbd_device *usbd_dev, uint8_t addr);

/**
 * Flush pending data from endpoint
 * @param usbd_dev USB Device
 * @param addr Endpoint number (including direction)
 */
void usbd_ep_flush(usbd_device *usbd_dev, uint8_t addr);

/**
 * Get frame-number of current Frame
 * @param usbd_dev USB Device
 * @return 11-bit frame-number
 */
uint16_t usbd_frame_number(usbd_device *usbd_dev);

/**
 * Get device current configuration.
 * @param usbd_dev USB Device
 * @return NULL if device is not assigned a configuration by host.
 * @return Current configuration (set by host)
 */
const struct usb_config_descriptor *usbd_get_config(usbd_device *usbd_dev);

/**
 * Get device current address
 * @param usbd_dev USB Device
 * @return 0 if device is not assigned a address by host.
 * @return Current device address
 */
uint8_t usbd_get_address(usbd_device *usbd_dev);

END_DECLS

#endif

/**@}*/

