/**
 * @defgroup usbd_driver_defines USB Drivers
 *
 * @brief <b>Defined Constants and Types for the USB Drivers</b>
 *
 * @ingroup USBD_defines
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
 * Copyright (C) 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

/**
 * Unlike the old stack in which application code had to handle packet level
 * details, the new stack design will let application request a transfer
 * in a abstract design without having to deal with packet level details.
 *
 * In the new design, the application code need to initalize the device object
 *  using usbd_init() (like the old stack).
 *
 * Application can register SET_CONFIGURATION and SET_INTERFACE callback.
 *
 * On SET_CONFIGURATION,
 *  application code need to pass on the requirement of memory different endpoints.
 *  because different hardware have different design which the stack abstract out.
 *  for example, some hardware have FIFO (dynamically) OR
 *  packet area that need to be divided among endpoints.
 *  one such feature is DOUBLE_BUFFER.
 *
 *  DOUBLE_BUFFER allow stack to make better use of hardware.
 *  the periph can send one buffer while the stack prepare the second buffer.
 *  and when done, switches the buffer.
 *
 *  usbd_ep_prepare(dev, ep, size, flag)
 *    ep = endpoint number (7bit = dir)
 *    size = maximum endpoint size
 *    flag = {DOUBLE_BUFFER}
 *  if the flag is not supported, it is simply ignored.
 *  (as in case of st fs dev) double buffering eat up 2 endpoint
 *  so, if you are using double buffer, do not setup the reverse direction endpoint.
 *
 *
 * On SET_INTERFACE,
 *    dtog need to be cleared for the endpoint inside the interface.
 *
 * Application code need to pass the endpoint size,type via transfer so that
 *  stack do not have to lookup (require computation) or cache (require memory)
 *  for the endpoint. also, application code can easily manintain the values.
 *
 * for application code, it can fill up the transfer object and submit it.
 * When the transfer is done, a callback is done (if requested)
 *
 * In transfer, endpoint_type=Control are a bit special.
 *  (application code will rarely have to deal with them, dont worry - ignore)
 * in order to receive SETUP packet, need to register callback for that.
 *  atm the callback is used for ep0 only and the specific api isnt public.
 *  in future, if someone come up with a usecase, we can think over it better and make it public
 *
 * when a SETUP packet callback is done, code need to handle the setup and do a
 *  IN/OUT transfer for data stage. or do the status stage.
 *
 * a special exception is, in order to stall the control transaction,
 *  only the callback shall be used for stalling.
 * also, stall on control endpoint are automatically cleared after sending to host.
 *
 * because double buffer is very different for different backends.
 *
 * Some tips:
 *  - Dont not call the api with more than endpoint supported by backend.
 *  - flags are backend/implementation specific and may be ignored.
 *  - SET/CLEAR stall does not necessarily result in Data toggle bit to be reset.
 *     this is very specifically dependent on the peripherial
 *  - when a STALL is set, all transfer all paused till the STALL is not cleared
 *    also, STALL override the NAK status
 */

typedef struct usbd_backend usbd_backend;
typedef struct usbd_device usbd_device;

extern const usbd_backend usbd_stm32_fsdev_v1;
extern const usbd_backend usbd_stm32_fsdev_v2;
extern const usbd_backend usbd_stm32_otg_fs;
extern const usbd_backend usbd_stm32_otg_hs;
extern const usbd_backend usbd_efm32lg;

#define USBD_STM32_FSDEV_V1	(&usbd_stm32_fsdev_v1)
#define USBD_STM32_FSDEV_V2	(&usbd_stm32_fsdev_v2)
#define USBD_STM32_OTG_FS	(&usbd_stm32_otg_fs)
#define USBD_STM32_OTG_HS	(&usbd_stm32_otg_hs)
#define USBD_EFM32LG		(&usbd_efm32lg)

enum usbd_speed {
	USBD_SPEED_UNKNOWN = 0,
	USBD_SPEED_LOW = 1,
	USBD_SPEED_FULL = 2,
	USBD_SPEED_HIGH = 3,
	USBD_SPEED_SUPER = 4 /* cost nothing ;) */
};

typedef enum usbd_speed usbd_speed;

	/** Optional features */
enum usbd_backend_features{
		USBD_FEATURE_NONE = 0,
		USBD_PHY_EXT = (1 << 0),
		USBD_VBUS_SENSE = (1 << 1),
		USBD_VBUS_EXT = (1 << 2)
		//* provide usb-core auto power-up/down on connect/disconnect
		, USBD_USE_POWERDOWN		= (1<<3)
};

struct usbd_backend_config {
	/** Number of endpoints. (Including control endpoint) */
	uint8_t ep_count;

	/** Number of bytes of private memory */
	uint16_t priv_mem;

	/** Device speed */
	usbd_speed speed;

	/** Optional features see usbd_backend_features*/
	unsigned int feature;
};

typedef struct usbd_backend_config usbd_backend_config;

enum usbd_ep_flags {
	/** No flags */
	USBD_EP_NONE = 0,

	/**
	 * Enable double buffering for the endpoint.
	 *  Not all backend support this flag,
	 *  use only when you know what you are doing.
	 * Usually you can expect 2x memory allocation.
	 * If not supported, will be ignored.
	 */
	USBD_EP_DOUBLE_BUFFER = (1 << 0),

	/**
	 * This endpoint will contineously transfer data to host.
	 *  ie. data transfer will be Periodic in nature.
	 * This let backend optimize (if any such facility is available)
	 * If not supported, will be ignored.
	 */
	USBD_EP_PERIODIC = (1 << 1),

	/**
	 * Number of packets that will be sent in a micro-frame of USB
	 * Only valid for Isochronous
	 */
	USBD_EP_PACKET_PER_FRAME_1 = (0x0 << 2),
	USBD_EP_PACKET_PER_FRAME_2 = (0x1 << 2),
	USBD_EP_PACKET_PER_FRAME_3 = (0x2 << 2),

	/* Mask for USBD_EP_PACKET_PER_FRAME_n */
	USBD_EP_PACKET_PER_FRAME_MASK = (0x3 << 2)
};

typedef enum usbd_ep_flags usbd_ep_flags;

/**
 * USB Transfer flags
 */
enum usbd_transfer_flags {
	/** A readable value for special 0 */
	USBD_FLAG_NONE = 0,

	/**
	 * Transfer always end with a short packet,
	 *  even if it means adding an extra zero length packet.
	 * Currently only applies for bulk, control IN
	 * Setting this flag on other transfer is NOP
	 * Should not be set when USBD_FLAG_NO_SHORT_PACKET flag is set
	 */
	USBD_FLAG_SHORT_PACKET = (1 << 0),

	/**
	 * Transfer should never end with a short packet (less than endpoint size).
	 * If it does, treat it as error.
	 * This flag only applies to OUT transfer (ie write).
	 * Should not be set when USBD_FLAG_SHORT_PACKET flag is set
	 */
	USBD_FLAG_NO_SHORT_PACKET = (1 << 1),

	/**
	 * Perform Per packet callback.
	 * When ever there is new data or library is going to transmit new data.
	 * Note: transfer will only proceed further when callback return.
	 * If callback want, it can make changes to the transfer data memory.
	 * transfer status = USBD_ONE_PACKET_DATA will be used.
	 *
	 * This feature is useful in case the transfer will require alot of memory.
	 *   (along with USBD_FLAG_NO_MEMORY_INCREMENT flag)
	 *  that is not fesible to allocate at runtime. but if the transfer is
	 *  done on packet to packet basis, it is possible to work in part.
	 *
	 * Also, when the callback will be done. "transferred" will contain:
	 *  for transmit, the number of bytes transmitted.
	 *  for received, the number of bytes already received
	 *
	 * Though this is an extension to the callback mechanism, but it allow
	 *  application to handle stuff on packet to packet basis.
	 *
	 * Do not assume that backend will call only when previous packet is required.
	 *  It could do call multiple time (in a row) to get multiple packets.
	 *
	 * If the transfer is setup, callback is performed for each data stage packet only.
	 */
	USBD_FLAG_PER_PACKET_CALLBACK = (1 << 2),

	/**
	 * Do not use incremented memory address [1] after packet receive or transmit.
	 * If this flag is set, it is assumed that the pointer provided can
	 *  atleast hold endpoint 1 max packet size data.
	 * Usually, if flag is used with flag USBD_FLAG_PER_PACKET_CALLBACK.
	 * so, that callback can update the data.
	 *
	 * [1] NOT talk of incrementing transfer::data.
	 *    transfer::data remain same as provided by user thoughout the transfer
	 *     life cycle.
	 *
	 * If flag effectively prevent `transfer->data + transfer->transferred`
	 *   and transfer everytime use `transfer->data`
	 *
	 * Setting this flag without USBD_FLAG_PER_PACKET_CALLBACK will
	 *  transmit all packet with same content.
	 *  receive the last packet content.
	 */
	USBD_FLAG_NO_MEMORY_INCREMENT = (1 << 3),

	/**
	 * Do not perform success callback.
	 * No callback with transfer status = USBD_SUCCESS will be performed.
	 */
	USBD_FLAG_NO_SUCCESS_CALLBACK = (1 << 4),

	/**
	 * Number of packets that will be sent in a micro-frame of USB.
	 * Only valid for Isochronous
	 */
	USBD_FLAG_PACKET_PER_FRAME_1 = (0x0 << 5),
	USBD_FLAG_PACKET_PER_FRAME_2 = (0x1 << 5),
	USBD_FLAG_PACKET_PER_FRAME_3 = (0x2 << 5),

	/* Mask for USBD_FLAG_PACKET_PER_FRAME_n */
	USBD_FLAG_PACKET_PER_FRAME_MASK = (0x3 << 5)
};

typedef unsigned char usbd_transfer_flags;

/**
 * USB Transfer status
 */
enum usbd_transfer_status {
	/** Transfer status used for USBD_FLAG_PER_PACKET_CALLBACK flag */
	USBD_ONE_PACKET_DATA = 1,

	/** Success */
	USBD_SUCCESS = 0,

	/** Transfer has time'd out */
	USBD_ERR_TIMEOUT = -1,

	/** Input/Output Error */
	USBD_ERR_IO = -2,

	/** Size invalid (Transfer too large) */
	USBD_ERR_SIZE = -3,

	/** Host has changed current configuration,
	 *  so all pending transfer will be stopped */
	USBD_ERR_CONFIG_CHANGE = -4,

	/** Resource unavailable, try again later. */
	USBD_ERR_RES_UNAVAIL = -5,

	/** device in disconnected state! */
	USBD_ERR_CONN = -6,

	/** More data received than endpoint size in one packet */
	USBD_ERR_BABBLE = -7,

	/** Data toggle mismatch */
	USBD_ERR_DTOG = -8,

	/** Packet smaller than endpoint size received */
	USBD_ERR_SHORT_PACKET = -9,

	/** Invalid transfer requested.
	 * @warning if this status is passed to callback,
	 *   Callback SHOULD NOT consider library generated values
	 *    (usbd_transfer::id and usbd_transfer::transferred) as valid!
	 *  This is because library will try to return the original invalid transfer.
	 */
	USBD_ERR_INVALID = -10,

	/** Transfer cancelled by application */
	USBD_ERR_CANCEL = -11,

	/** Got data more data than could accomodate in buffer.
	 *  This is the condition when host has sent a packet that could only be
	 *  partially stored in buffer. */
	USBD_ERR_OVERFLOW = -12
};

typedef enum usbd_transfer_status usbd_transfer_status;

typedef uint64_t usbd_urb_id;

typedef struct usbd_transfer usbd_transfer;

typedef void (*usbd_transfer_callback)(usbd_device *dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id);

/**
 * Type of endpoint
 */
enum usbd_ep_type {
	USBD_EP_CONTROL = 0, /**< Control endpoint */
	USBD_EP_ISOCHRONOUS = 1, /**< Isochronous endpoint */
	USBD_EP_BULK = 2, /**< Bulk endpoint */
	USBD_EP_INTERRUPT = 3 /**< Interrupt endpoint */
};

typedef enum usbd_ep_type usbd_ep_type;

/**
 * USB Transfer
 */
struct usbd_transfer {
	/** Endpoint type */
	usbd_ep_type ep_type;

	/** Endpoint Address.
	 *   bit 0-3: Endpoint number
	 *   bit 7: HIGH = in-endpoint, LOW = out-endpoint
	 */
	uint8_t ep_addr;

	/** Endpoint size */
	uint16_t ep_size;

	/** Interval at which host will read (in frame). (0 If not applicable).
	 *  This can be supplied if USBD_EP_PERIODIC flag was supplies for
	 *   backend reference to make better decision. */
	uint16_t ep_interval;

	/** Buffer to read/write */
	void *buffer;

	/** Number of bytes to transfer */
	size_t length;

	/** Number of bytes transferred to/from @a data
	 *  @warning Manipulated by library internally. not accepted from user
	 */
	size_t transferred;

	/** Transfer related flags */
	usbd_transfer_flags flags;

	/** Timeout, in milliseconds (0 for never timeout) */
	uint32_t timeout;

	/** Callback to perform when complete or error occur */
	usbd_transfer_callback callback;

	/** User specific data */
	void *user_data;
};

/* A readable value for special 0 */
#define USBD_TIMEOUT_NEVER 0

/* Marker ID of URB as invalid */
#define USBD_INVALID_URB_ID 0

/* A readable value for not applicable (special 0) */
#define USBD_INTERVAL_NA 0

/**
 * @param[in] dev USB Device
 * @param[in] cfg Configuration
 */
typedef void (*usbd_set_config_callback)(usbd_device *dev,
					const struct usb_config_descriptor *cfg);

/**
 * @param[in] dev USB Device
 * @param[in] iface Interface
 * @param[in] altsetting Alternate Setting
 */
typedef void (*usbd_set_interface_callback)(usbd_device *dev,
					const struct usb_interface *iface,
					const struct usb_interface_descriptor *altsetting);

typedef void (*usbd_setup_callback)(usbd_device *dev, uint8_t addr,
					const struct usb_setup_data *setup_data);

typedef void (* usbd_session_callback)(usbd_device *dev, bool online);

typedef void (* usbd_generic_callback)(usbd_device *dev);

/**
 * Main initialization entry point.
 *
 * Initialize the USB firmware library to implement the USB device described
 * by the descriptors provided.
 *
 * @param[in] backend Backend
 * @param[in] config Backend configuration (NULL for None)
 * @param[in] dev Pointer to USB device descriptor. This must not be changed while
 *            the device is in use.
 * @param[in] ep0_buffer Pointer to array that would hold the data
 *                       received during control requests with DATA
 *                       stage
 * @param[in] ep0_buffer_size Size of control_buffer
 * @return the usb device initialized for use.
 */
usbd_device* usbd_init(const usbd_backend *backend,
					const usbd_backend_config *config,
					const struct usb_device_descriptor *dev,
					void *ep0_buffer, size_t ep0_buffer_size);

/**
 * Called when SETUP packet is received on control endpoint 0
 * @param[in] dev USB Device
 * @param[in] setup_data Setup Data
 */
void usbd_ep0_setup(usbd_device *dev, const struct usb_setup_data *setup_data);

/**
 * Register a setup callback to handle SETUP packet.
 * @param[in] dev USB Device
 * @param[in] callback Callback
 * @note if nothing is registered, control is passed to usbd_ep0_setup()
 * @note Application code can use this callback to take over any
 *  control endpoint (which includes EP0).
 *  it can do so by registering setup callback and then passing handle to
 *  usbd_ep0_setup() when it wishes.
 *  It is recommended to do so to handle vendor/class request only!
 *
 * @note Application code which want to send data to host make sure that:
 *  if there is less data than requested by host,
 *  application must make sure that the transfer flags USBD_FLAG_SHORT_PACKET is set.
 *  This will make sure that a zero packet is send to
 *  host to notify there is no more data,
 *  when the data to be transferred is multiple of endpoint size.
 *  OR application code can simply use usbd_transfer_control() to do the above.
 */
void usbd_register_setup_callback(usbd_device *dev,
					usbd_setup_callback callback);

/**
 * Register bus reset callback
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 */
void usbd_register_reset_callback(usbd_device *dev,
					usbd_generic_callback callback);

/**
 * Register suspend callback
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 */
void usbd_register_suspend_callback(usbd_device *dev,
					usbd_generic_callback callback);

/**
 * Register resume callback
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 */
void usbd_register_resume_callback(usbd_device *dev,
					usbd_generic_callback callback);

/**
 * Register start-of-frame callback
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 */
void usbd_register_sof_callback(usbd_device *dev,
					usbd_generic_callback callback);

/**
 * Register session connect/disconnect callback
 *   with USBD_VBUS_SENSE feature primary need for cable 
 *   plug detection
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 */
void usbd_register_session_callback(usbd_device *dev,
					usbd_session_callback callback);

/**
 * Register SET_CONFIGURATION callback \n
 * stack will invoke @a callback when ever SET_CONFIGURATION is received
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 * @warning Application should not submit any transfer from the callback.
 * @warning Application should make sure that all (except non-zero endpoint)
 *  transfer are placed after SET_CONFIGURATION.
 *  Application can use usbd_register_set_interface_callback() callback to
 *  start transfer for the endpoint that is inside the interface.
 */
void usbd_register_set_config_callback(usbd_device *dev,
					usbd_set_config_callback callback);

/**
 * Register set-interface callback \n
 * stack will invoke @a callback when ever SET_INTERFACE is received
 * @param[in] dev USB Device
 * @param[in] callback callback to be invoked
 */
void usbd_register_set_interface_callback(usbd_device *dev,
					usbd_set_interface_callback callback);

/**
 * Poll the device.
 * @param[in] dev USB Device
 * @param[in] us Number of microseconds passed since the last poll or init
 * @note it is recommended to poll every millisecond
 * @note it is recommended to provide atleast 0.1ms resolution in @a us
 * @note if timeout is not required, @a us should be 0 for all calls
 */
void usbd_poll(usbd_device *dev, uint32_t us);

/**
 * Force disconnect.
 * @param[in] dev USB Device
 * @param[in] disconnect perform disconnection
 * @note not all backend support this facility.
 */
void usbd_disconnect(usbd_device *dev, bool disconnected);

/**
 * Describe the stack about the endpoint usage.
 * @param[in] dev USB Device
 * @param[in] addr endpoint number including direction
 * @param[in] type Expected endpoint type
 * @param[in] max_size maximum number of bytes the endpoint could hold
 * @param[in] interval Interval at which host may poll (0 = none or can vary)
 * @param[in] flag Flags
 * @note do not pass addr = ep0 (0x00 or 0x80), as it is internally used.
 * @note the prefered location is SET_CONFIGURATION callback
 * @note @a type is used to determine the best possible strategy for the endpoint.
 *  For example, if @a type = USBD_EP_CONTROL is passed, the endpoint will accept
 *   SETUP packets.
 * @note max_size is used for memory allocation in
 *  worst case senario for the configuration.
 * @note @a interval value is just for reference
 * @note for bidirectional endpoint (current CONTROL is the only), this
 *  function is only called once. (without any direction bit)
 */
void usbd_ep_prepare(usbd_device *dev, uint8_t addr, usbd_ep_type type,
				uint16_t max_size, uint16_t interval, usbd_ep_flags flags);

/**
 * STALL the endpoint
 * @param[in] dev USB Device
 * @param[in] addr Endpoint number (including direction)
 * @param[in] stall true to stall the endpoint
 * @note Stall automatically clear when on control endpoint
 *  after STALL is send to host
 */
void usbd_set_ep_stall(usbd_device *dev, uint8_t addr, bool stall);

/**
 * Get STALL status of endpoint
 * @param[in] dev USB Device
 * @param[in] addr Endpoint number (including direction)
 * @retval true if endpoint in STALL
 */
bool usbd_get_ep_stall(usbd_device *dev, uint8_t addr);

/**
 * Set the endpoint data-toggle
 * @param[in] dev USB Device
 * @param[in] addr Endpoint number (including direction)
 * @param[in] dtog New data toggle value
 */
void usbd_set_ep_dtog(usbd_device *dev, uint8_t addr, bool dtog);

/**
 * Get the endpoint data-toggle
 * @param[in] dev USB Device
 * @param[in] addr Endpoint number (including direction)
 * @return true if data-toggle = 1
 * @note the DTOG value is for the last transfer that was successfuly.
 */
bool usbd_get_ep_dtog(usbd_device *dev, uint8_t addr);

/**
 * Get frame-number of current Frame
 * @param[in] dev USB Device
 * @return 11-bit frame-number
 */
uint16_t usbd_frame_number(usbd_device *dev);

/**
 * Get device current configuration.
 * @param[in] dev USB Device
 * @return NULL if device is not assigned a configuration by host.
 * @return Current configuration (set by host)
 */
const struct usb_config_descriptor *usbd_get_config(usbd_device *dev);

/**
 * Get device current address
 * @param[in] dev USB Device
 * @return 0 if device is not assigned a address by host.
 * @return Current device address
 */
uint8_t usbd_get_address(usbd_device *dev);

/**
 * Get the enumeration speed of device
 * @param[in] dev USB Device
 * @return Current enumeration speed
 */
usbd_speed usbd_get_speed(usbd_device *dev);


/**
 * Get status of connected cable.
 * @param[in] dev USB Device
 * @return true - cable connected, and Vbus active
 */
bool usbd_is_vbus(usbd_device *dev);

/**
 * Controls power state of usb-core and PHY
 * @param[in] dev USB Device
 * @param[in] onoff  - true turn on device in action, and power PHY
 *                     false disable PHY and stops usb-core
 */
void usbd_enable(usbd_device *dev, bool onoff);

/**
 * Perform a transfer
 *
 * Data OUT:
 *  - application provide a buffer to write data to
 *  - stack fill the buffer
 *  - it does a callback to application when full
 *
 * Data IN:
 *  - device provide a buffer to read from
 *  - stack send the buffer
 *  - when the buffer is empty, callback
 *
 * @param[in] usbd_dev USB Device
 * @param[in] transfer USB device transfer
 * @return Transfer ID
 * @note @a transfer pointer need to be only valid till callee return.
 *    (go allocate @a transfer on stack!)
 * @note @a transfer::data need to be valid till callback is performed.
 */
usbd_urb_id usbd_transfer_submit(usbd_device *dev,
					const usbd_transfer *transfer);

/**
 * Cancel a transfer
 * @param[in] usbd_dev USB Device
 * @param[in] id Transfer ID
 * @return true on success
 * @return false on failure
 */
bool usbd_transfer_cancel(usbd_device *dev, usbd_urb_id urb_id);

/**
 * Cancel all transfer for the endpoint @a addr
 * @param[in] usbd_dev USB Device
 * @param[in] ep_addr Endpoint number (including direction)
 * @return Number of transfers cancelled
 */
unsigned usbd_transfer_cancel_ep(usbd_device *dev, uint8_t ep_addr);

enum usbd_control_transfer_feedback {
	/* Goto to status stage and
	 * perform callback when status stage is complete.
	 * (And Pass this for status stage callback) */
	USBD_CONTROL_TRANSFER_OK = 0,

	/* Stall status stage
	 * Note: "NO_STATUS_CALLBACK" flag dont make sense if "STALL" flag is set */
	USBD_CONTROL_TRANSFER_STALL = (1 << 1),

	/* Do not perform callback after status stage complete */
	USBD_CONTROL_TRANSFER_NO_STATUS_CALLBACK = (1 << 2)
};

typedef enum usbd_control_transfer_feedback usbd_control_transfer_feedback;

struct usbd_control_transfer_callback_arg {
	void *buffer; /**< Pointer to data stage buffer */
	size_t length; /**< Number of bytes actually transferred */
};

typedef struct usbd_control_transfer_callback_arg
					usbd_control_transfer_callback_arg;

/**
 * USB Transfer control handle callback
 * @param[in] dev USB Device
 * @param[in] arg Callback argument
 * @return feedback argument
 * @note Callback after data stage is finished, @a arg will not be NULL.
 *    (Containing the data that was provided originally by application)
 * @note Callback after status stage is finished, @a arg will be NULL.
 */
typedef usbd_control_transfer_feedback (*usbd_control_transfer_callback)
			(usbd_device *dev, const usbd_control_transfer_callback_arg *arg);

/**
 * Instead of directly handling control transfer states,
 * Perform data and status stage for you.
 * Make decision based on argument and @a setup_data content.
 * @param[in] dev USB Device
 * @param[in] ep_addr Endpoint address (without direction)
 * @param[in] ep_size Endpoint size
 * @param[in] setup_data Setup Data (cannot be NULL)
 * @param[in] buf Buffer to send (can be NULL)
 * @param[in] len Length of @a buf (0 if @a buf is NULL)
 * @param[in] callback Callback to perform on successful transaction (status stage)
 * @note @a setup_data should be valid till the function do not return.
 *  (ie @a setup_data can reside on stack)
 * @note if the @a setup_data->wLength is zero, same as calling directly
 *  usbd_tranfer_control_status()
 */
void usbd_control_transfer(usbd_device *dev, uint8_t ep_addr,
	uint16_t ep_size, const struct usb_setup_data *setup_data, void *buf,
	size_t len, usbd_control_transfer_callback callback);

/** Same as usbd_transfer_control_data() but for endpoint 0 */
void usbd_ep0_transfer(usbd_device *dev,
	const struct usb_setup_data *setup_data, void *buf, size_t len,
	usbd_control_transfer_callback callback);

/**
 * Ease STALL'ing of EP0 for public code.
 * @param dev USB Device
 */
void usbd_ep0_stall(usbd_device *dev);

END_DECLS

#endif

/**@}*/

