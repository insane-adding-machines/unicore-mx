/**
 * @defgroup usbd_private_defines USB Private Structures
 *
 * @brief <b>Defined Constants and Types for the USB Private Structures</b>
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

#ifndef UNICOREMX_USBD_PRIVATE_H
#define UNICOREMX_USBD_PRIVATE_H

#include <unicore-mx/usbd/usbd.h>

/**
 * Compile time configuration: \n
 * USBD_URB_COUNT: Number of URB Object to allocate (default: 20) \n
 * USBD_ENABLE_TIMEOUT: Define to enable timeout functionality (default: undefined)
 */

#if defined(USBD_URB_COUNT) && (USBD_URB_COUNT < 1)
# error "Sanity check failed!!! go get sleep."				\
	"URB count less than 1 is meaningless in our universe."
#endif

#if !defined(USBD_URB_COUNT)
# define USBD_URB_COUNT 20
#endif

#if defined(__DOXYGEN__)
# define USBD_ENABLE_TIMEOUT
#endif

/* define macro "USBD_ENABLE_TIMEOUT" to enable timeout functionality.
 * It can help save processing as well as RAM.
 * By default - disabled. */

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct usbd_urb {
	uint64_t id;
	usbd_transfer transfer;
#if defined(USBD_ENABLE_TIMEOUT)
	uint64_t timeout_on;
#endif
	struct usbd_urb *next;
};

typedef struct usbd_urb usbd_urb;

/** Internal collection of device information. */
struct usbd_device {
	/** Device descriptor. */
	const struct usb_device_descriptor *desc;

	/**
	 * Current configuration of the device.
	 * If NULL, then the device is in {default, address} state
	 * If not NULL, then device is in configured state.
	 */
	const struct usb_config_descriptor *current_config;

	struct {
		void *buf;
		size_t len;
	} preserve;

	struct {
		/** invoked on bus-reset */
		usbd_generic_callback reset;

		/** invoked on bus-suspend */
		usbd_generic_callback suspend;

		/** invoked on bus-resume */
		usbd_generic_callback resume;

		/** invoked when sof received */
		usbd_generic_callback sof;

		/** invoked on SETUP packet */
		usbd_setup_callback setup;

		/** invoked on SET_CONFIGURATION */
		usbd_set_config_callback set_config;

		/** invoked on SET_INTERFACE */
		usbd_set_interface_callback set_interface;
	} callback;

	/** Backend */
	const struct usbd_backend *backend;

	/** Backend configuration */
	const struct usbd_backend_config *config;

#if defined(USBD_ENABLE_TIMEOUT)
	uint64_t last_poll;
#endif

	/**
	 * @a active - URB that are being processed
	 * @a waiting - URB that are waiting to be added to @a active once
	 *                  the endpoint become free.
	 */
	struct {
		/**
		 * @a head - Head of the Queue
		 * @a tail - Tail of the Queue
		 */
		struct usbd_urb_queue {
			usbd_urb *head, *tail;
		} active, waiting;

		/**
		 * 1 Means the endpoint is free to be used.
		 * 0 Means the endpoint is being used.
		 *
		 * BITn (where n = 0-15) - Endpoint OUT n
		 * BIT(n+16) (where n = 0-15) - Endpoint IN n
		 *
		 * If the endpoint is bidirectional (control) then,
		 *  the IN and OUT need to be marked together.
		 */
		uint32_t ep_free;

		/** List of unused objects (invalid) and empty shell for transfer */
		usbd_urb *unused;

		/** Array of URB allocated at compile time */
		usbd_urb arr[USBD_URB_COUNT];

		uint64_t next_id;

		/** Only allow EP0 transfer.
		 *  main use case is, ep_prepare_start and ep_prepare_end block */
		bool force_all_new_urb_to_waiting;
	} urbs;

#if defined(USBD_DEVICE_EXTRA)
	USBD_DEVICE_EXTRA
#endif
};

/* Functions provided by the hardware abstraction. */
struct usbd_backend {
	usbd_device * (*init)(const usbd_backend_config *config);
	void (*set_address)(usbd_device *dev, uint8_t addr);
	uint8_t (*get_address)(usbd_device *dev);
	void (*ep_prepare_start)(usbd_device *dev);
	void (*ep_prepare)(usbd_device *dev, uint8_t addr, usbd_ep_type type,
					uint16_t max_size, uint16_t interval, usbd_ep_flags flags);
	void (*ep_prepare_end)(usbd_device *dev);
	void (*set_ep_dtog)(usbd_device *dev, uint8_t addr,  bool dtog);
	bool (*get_ep_dtog)(usbd_device *dev, uint8_t addr);
	void (*set_ep_stall)(usbd_device *dev, uint8_t addr,  bool stall);
	bool (*get_ep_stall)(usbd_device *dev, uint8_t addr);
	void (*poll)(usbd_device *dev);
	void (*disconnect)(usbd_device *dev, bool disconnected);
	void (*enable_sof)(usbd_device *dev, bool enable);
	usbd_speed (*get_speed)(usbd_device *dev);
	void (*urb_submit)(usbd_device *dev, usbd_urb *urb);
	void (*urb_cancel)(usbd_device *dev, usbd_urb *urb);

	/* Frame number */
	uint16_t (*frame_number)(usbd_device *dev);

	/*
	 * this is to tell usb generic code
	 *  that address need to be set before status-stage.
	 */
	bool set_address_before_status;

	/* somewhere in backend header or code.
	 * WARNING: this will make the struct size variable.
	 *  (cannot be used for array - anyway no one will be used it for array) */
#if defined(USBD_BACKEND_EXTRA)
	USBD_BACKEND_EXTRA
#endif
};

#if defined(USBD_DEBUG)
extern void usbd_log_puts(const char *arg);
extern void usbd_log_printf(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));
# include <inttypes.h>
# define LOG(str) usbd_log_puts(str)
# define LOGF(fmt,...) usbd_log_printf(fmt, ##__VA_ARGS__)
#else
# define LOG(str)
# define LOGF(fmt,...)
#endif

#define NEW_LINE "\n"
#define LOG_LN(str) LOG(str); LOG(NEW_LINE)
#define LOGF_LN(fmt,...) LOGF(fmt, __VA_ARGS__); LOG(NEW_LINE)
#define LOG_CALL LOG("inside "); LOG_LN(__func__);

#define IS_URB_ID_INVALID(urb_id) ((urb_id) == USBD_INVALID_URB_ID)
#define IS_URB_INVALID(urb) IS_URB_ID_INVALID((urb)->id)

/** Convert milliseconds to microseconds */
#define MS2US(ms) ((ms) * 1000)

#define DIVIDE_AND_CEIL(divident, divisor) \
	((divident) + (divisor) - 1) / (divisor)

/**
 * Perform a callback for transfer
 * @param[in] t Transfer
 * @param[in] status Status of transfer
 * @param[in] transferred Length of data transferred
 */
#define TRANSFER_CALLBACK(dev, transfer, status, urb_id)				\
	if ((transfer)->callback != NULL) {									\
		(transfer)->callback((dev), (transfer), (status), (urb_id));	\
	}

/**
 * Perform callback when no endpoint is available for transfer
 * @param[in] t Transfer
 */
#define TRANSFER_NO_RES(dev, t)		\
	TRANSFER_CALLBACK((dev), (t), USBD_ERR_RES_UNAVAIL, USBD_INVALID_URB_ID)

/**
 * Perform callback when transfer is invalid
 * @param[in] t Transfer
 */
#define TRANSFER_INVALID(dev, t)		\
	TRANSFER_CALLBACK((dev), (t), USBD_ERR_INVALID, USBD_INVALID_URB_ID)

#define URB_CALLBACK(dev, urb, status)		\
	TRANSFER_CALLBACK((dev), &(urb)->transfer, status, (urb)->id)

/* Just for readability, nothing special */
#define IS_OUT_ENDPOINT(ep_addr) (!((ep_addr) & 0x80))
#define IS_IN_ENDPOINT(ep_addr) (!!((ep_addr) & 0x80))
#define ENDPOINT_NUMBER(ep_addr) ((ep_addr) & 0x7F)


void usbd_urb_complete(usbd_device *dev, usbd_urb *urb,
						usbd_transfer_status status);

void usbd_urb_detach_from_active(usbd_device *dev, usbd_urb *urb);

void usbd_urb_schedule(usbd_device *dev);

usbd_urb *usbd_find_active_urb(usbd_device *dev, uint8_t ep);

void *usbd_urb_get_buffer_pointer(usbd_device *dev, usbd_urb *urb, size_t len);
void usbd_urb_inc_data_pointer(usbd_device *dev, usbd_urb *urb, size_t len);

#if defined(USBD_ENABLE_TIMEOUT)
void usbd_timeout_checkup(usbd_device *dev, uint64_t now);
#endif

void usbd_purge_all_transfer(usbd_device *dev, usbd_transfer_status status);

void usbd_put_all_urb_into_unused(usbd_device *dev);

void usbd_purge_all_non_ep0_transfer(usbd_device *dev,
			usbd_transfer_status status);

inline uint32_t ep_free_mask(uint8_t ep_addr);
inline void usbd_handle_suspend(usbd_device *dev);
inline void usbd_handle_resume(usbd_device *dev);
inline void usbd_handle_sof(usbd_device *dev);
inline void usbd_handle_setup(usbd_device *dev, uint8_t ep,
					const struct usb_setup_data *setup_data);
inline void usbd_handle_reset(usbd_device *dev);
inline bool is_ep_free(usbd_device *dev, uint8_t ep_addr);
inline void mark_ep_as_free(usbd_device *dev, uint8_t ep_addr, bool yes);

/**
 * Get the DTOG bit mask for @a ep_addr
 * @param[in] ep_addr Endpoint address (including direction)
 * @return mask
 */
static inline 
uint32_t ep_free_mask(uint8_t ep_addr)
{
	uint32_t num = ENDPOINT_NUMBER(ep_addr);

	if (IS_IN_ENDPOINT(ep_addr)) {
		num += 16;
	}

	return 1 << num;
}

/**
 * SUSPEND detected on bus
 * @param[in] dev USB Device
 */
inline void usbd_handle_suspend(usbd_device *dev)
{
	LOG_LN("SUSPEND detected!");

	if (dev->callback.suspend != NULL) {
		dev->callback.suspend(dev);
	}
}

/**
 * RESUME detected on bus
 * @param[in] dev USB Device
 */
inline void usbd_handle_resume(usbd_device *dev)
{
	LOG_LN("RESUME detected!");

	if (dev->callback.resume != NULL) {
		dev->callback.resume(dev);
	}
}

/**
 * SOF detected on bus
 * @param[in] dev USB Device
 */
inline void usbd_handle_sof(usbd_device *dev)
{
	/* Oh no! dont print anything here... or else... */

	if (dev->callback.sof != NULL) {
		dev->callback.sof(dev);
	}
}

/**
 * Called by backend to pass the setup data when a SETUP packet is recevied
 *  on control endpoint.
 * @param[in] dev USB Device
 * @param[in] ep Endpoint on which SETUP was received
 * @param[in] setup_data Setup Data
 * @note setup_data is only expected to be valid till this function do not return.
 */
inline void usbd_handle_setup(usbd_device *dev, uint8_t ep,
					const struct usb_setup_data *setup_data)
{
	if (dev->callback.setup != NULL) {
		dev->callback.setup(dev, ep, setup_data);
	} else if (!ep) {
		/* No callback registered, route to ep0 by default */
		usbd_ep0_setup(dev, setup_data);
	} else {
		LOGF_LN("WARNING: Application code need to handle SETUP packet on "
			"0x%"PRIx8", (stalling...)", ep);

		usbd_set_ep_stall(dev, ENDPOINT_NUMBER(ep) | 0x80, true);
		usbd_set_ep_stall(dev, ENDPOINT_NUMBER(ep), true);
	}
}

/**
 * RESET detected on bus
 * @param[in] dev USB Device
 */
inline void usbd_handle_reset(usbd_device *dev)
{
	LOG_LN("RESET detected");

	usbd_purge_all_transfer(dev, USBD_ERR_CONN);

	dev->urbs.force_all_new_urb_to_waiting = false;

	dev->current_config = NULL;

	if (dev->callback.reset != NULL) {
		dev->callback.reset(dev);
	}
}

/**
 * Check if the endpoint is free
 * @param[in] dev USB Device
 * @param[in] ep_addr Endpoint (including direction)
 */
static inline 
bool is_ep_free(usbd_device *dev, uint8_t ep_addr)
{
	return !!(dev->urbs.ep_free & ep_free_mask(ep_addr));
}

/**
 * Mark the endpoint as unused on @a yes
 * @param[in] dev USB Device
 * @param[in] ep_addr Endpoint address (including direction)
 * @param[in] yes Yes (if true, mark it as unused)
 */
static inline 
void mark_ep_as_free(usbd_device *dev, uint8_t ep_addr, bool yes)
{
	uint32_t mask = ep_free_mask(ep_addr);
	if (yes) {
		dev->urbs.ep_free |= mask;
	} else {
		dev->urbs.ep_free &= ~mask;
	}
}

/* Do not appear to belong to the API, so are omitted from docs */
/**@}*/

#endif
