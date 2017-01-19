/*
 * This file is part of the unicore-mx project.
 *
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

#include <string.h>
#include <unicore-mx/usbd/usbd.h>
#include "usbd_private.h"

/*
 * The Transfer design is such that application code submit transfer.
 * Transfer is encapsulated in to an URB.
 * Based on the availibility of the endpoint, the URB is submitted to backend.
 *   If endpoint available:
 *     - endpoint is marked as not-available (anymore)
 *     - append to active list
 *     - submit to backend
 *   if endpoint not available:
 *      - append to waiting list
 *
 * Later, when the endpoint is freed
 *    (transfer succesfully finished, cancellled, timeout etc..)
 * Waiting URB are schedule and as per availibility of endpoint they are submitted.
 *
 * At reset, all transfer are cancelled
 *    because the life time of the transfer has ended.
 *
 * At SET_CONFIGURATION, all non-EP0 transfer are cancelled
 *    because the life time of the transfer has ended.
 *
 * In the design, while the endpoint is being prepared (in SET_CONFIGURATION)
 *   All transfer transfer are force to be added to Waiting list.
 *  and right after endpoint preperation has completed,
 *    scheduling is done in the order they were added.
 *
 *
 * endpoint preperation is a method in which the backend is told about the
 *  resource requirement the endpoint.
 *  based on the description, it allocate resource specific to the periph.
 * endpoint preperation is just an hint for the stack, they can be ignored!
 *
 * There are 3 queue [active, waiting, unused].
 *
 * When a URB is done (or at init or reset), the object is moved to "unused".
 * When a new transfer is submitted, a "unused" URB object is poped and
 *  used for the transfer.
 *
 * In between usbd_ep_prepare_start() and usbd_ep_prepare_end() backend should
 * clear DTOG bit for the endpoints. (except EP0)
 */

#if defined(USBD_DEBUG)
static const char *stringify_transfer_status(usbd_transfer_status status) {
	switch (status) {
	case USBD_ONE_PACKET_DATA: return "USBD_ONE_PACKET_DATA";
	case USBD_SUCCESS: return "USBD_SUCCESS";
	case USBD_ERR_TIMEOUT: return "USBD_ERR_TIMEOUT";
	case USBD_ERR_IO: return "USBD_ERR_IO";
	case USBD_ERR_SIZE: return "USBD_ERR_SIZE";
	case USBD_ERR_CONFIG_CHANGE: return "USBD_ERR_CONFIG_CHANGE";
	case USBD_ERR_RES_UNAVAIL: return "USBD_ERR_RES_UNAVAIL";
	case USBD_ERR_CONN: return "USBD_ERR_CONN";
	case USBD_ERR_BABBLE: return "USBD_ERR_BABBLE";
	case USBD_ERR_DTOG: return "USBD_ERR_DTOG";
	case USBD_ERR_SHORT_PACKET: return "USBD_ERR_SHORT_PACKET";
	case USBD_ERR_INVALID: return "USBD_ERR_INVALID";
	case USBD_ERR_CANCEL: return "USBD_ERR_CANCEL";
	case USBD_ERR_OVERFLOW: return "USBD_ERR_OVERFLOW";
	default: return "**UNKNOWN**";
	}
}
#endif

/**
 * Get for a free URB
 * @param[in] dev USB Device
 * @return pointer to URB (success)
 * @return NULL (failure)
 */
static inline usbd_urb *unused_pop(usbd_device *dev)
{
	if (dev->urbs.unused == NULL) {
		LOG_LN("WARN: all urb in use");
		return NULL;
	}

	usbd_urb *tmp = dev->urbs.unused;
	dev->urbs.unused = tmp->next;
	return tmp;
}

/**
 * Add the URB to unused list
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 */
static inline void unused_push(usbd_device *dev, usbd_urb *urb)
{
	urb->next = dev->urbs.unused;
	dev->urbs.unused = urb;
}

/**
 * Free the endpoint from the URB.
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 */
static void free_ep_from_urb(usbd_device *dev, usbd_urb *urb)
{
	uint8_t addr = urb->transfer.ep_addr;

	if (!is_ep_free(dev, addr)) {
		/* Endpoint is not free. Tell backend to cancel it */
		dev->backend->urb_cancel(dev, urb);
		mark_ep_as_free(dev, addr, true);
	}
}

/**
 * Detach the item from the Queue
 * @param[in] queue Queue
 * @param[in] prev Previous item to @a item
 * @param[in] item Item to be detached
 * @return the next item in the queue (NULL if not available)
 */
static usbd_urb *queue_item_detach(struct usbd_urb_queue *queue,
				usbd_urb *prev, usbd_urb *item)
{
	if (queue->head == queue->tail) {
		/* two case:
		 *  - the queue is fully empty (not possible)
		 *  - single item (possible)
		 */
		queue->head = queue->tail = NULL;
		return NULL;
	}

	/* Item at head? */
	if (queue->head == item) {
		queue->head = item->next;
		return queue->head;
	}

	/* Item at tail? */
	if (queue->tail == item) {
		queue->tail = prev;
		prev->next = NULL;
		return NULL;
	}

	prev->next = item->next;
	return prev->next;
}

static void queue_item_append(struct usbd_urb_queue *queue, usbd_urb *urb)
{
	urb->next = NULL;

	if (queue->head == NULL && queue->tail == NULL) {
		queue->head = queue->tail = urb;
	} else if (queue->head != NULL && queue->tail != NULL) {
		queue->tail->next = urb;
		queue->tail = urb;
	} else {
		/* Problem! */
		LOGF_LN("URB Queue %p corrupt", queue);
		queue->head = queue->tail = urb;
	}
}

#if defined(USBD_ENABLE_TIMEOUT)
/**
 * Has the URB timeout out compared to @a now
 * @param[in] urb USB Request Block
 * @param[in] now Current time reference
 * @return false No
 * @return true Yes
 */
static inline bool is_urb_timed_out(usbd_urb *urb, uint64_t now)
{
	if (urb->transfer->timeout == USBD_TIMEOUT_NEVER) {
		return false;
	}

	if (urb->timeout_on >= now) {
		return false;
	}

	return true;
}

/**
 * Check for timeout of URB in @a queue.
 * @param dev USB Device
 * @param now Current time reference
 * @param queue Queue
 * @return true if any URB timeout
 * @return false if no URB timeout
 */
static bool queue_timeout_check(usbd_device *dev, uint64_t now,
					usbd_urb_queue *queue)
{
	bool any_urb_timedout = false;
	usbd_urb *urb = queue->head, *prev = NULL, tmp;

	while (urb != NULL) {
		if (!is_urb_timed_out(urb, now)) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		tmp = urb;
		urb = queue_item_detach(queue, prev, tmp);
		free_ep_from_urb(dev, tmp);
		urb_callback(dev, tmp, USBD_ERR_TIMEOUT);
		unused_push(dev, tmp);
		any_urb_timedout = true;
	}

	return any_urb_timedout;
}

/**
 * Check if any URB has timeout out, it yes remove then with
 *  status = USBD_ERR_TIMEOUT
 * @param[in] dev USB Device
 * @param[in] now Current time reference
 */
static void usbd_timeout_checkup(usbd_device *dev, uint64_t now)
{
	/* Check the Waiting Queue */
	queue_timeout_check(dev, now, &dev->urbs.waiting);

	/* Check the Active Queue */
	if (queue_timeout_check(dev, now, &dev->urbs.active)) {
		usbd_urb_schedule(dev);
	}
}
#endif

/**
 * Check if the Control endpoint @a size is acceptable or not
 * @param[in] size Control endpoint size
 * @return true on Yes
 * @return false on No
 */
static inline bool ctrl_ep0_size_acceptable(uint16_t size)
{
	switch (size) {
	case 8:
	case 16:
	case 32:
	case 64:
	return true;
	default:
	return false;
	}
}

/**
 * Try to assign allocate the endpoint for the URB
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @return true if the endpoint has been assigned to @a urb
 * @return false if the endpoint cannot be assigned to @a urb
 */
static bool try_alloc_ep_for_urb(usbd_device *dev, usbd_urb *urb)
{
	uint8_t addr = urb->transfer.ep_addr;

	if (is_ep_free(dev, addr)) {
		mark_ep_as_free(dev, addr, false);
		return true;
	}

	return false;
}

usbd_urb_id usbd_transfer_submit(usbd_device *dev,
					const usbd_transfer *transfer)
{
	/* Various Checks */
	if (transfer->ep_type == USBD_EP_CONTROL) {
		if (!ENDPOINT_NUMBER(transfer->ep_addr) &&
				!ctrl_ep0_size_acceptable(transfer->ep_size)) {
			LOGF_LN("Invalid control endpoint size %"PRIu16" for EP0",
					transfer->ep_size);
			TRANSFER_INVALID(dev, transfer);
			return USBD_INVALID_URB_ID;
		}
	}

	/* check if got any URB free */
	usbd_urb *urb = unused_pop(dev);
	if (urb == NULL) {
		TRANSFER_NO_RES(dev, transfer);
		return USBD_INVALID_URB_ID;
	}

	/* store the information in URB */
	urb->id = dev->urbs.next_id++;
	urb->transfer = *transfer;
	urb->transfer.transferred = 0;
#if defined(USBD_ENABLE_TIMEOUT)
	urb->timeout_on = transfer->timeout ?
		(dev->last_poll + MS2US(transfer->timeout)) : 0;
#endif


#if defined(USBD_DEBUG)
	const char *ep_type_map_str[] = {
		[USBD_EP_CONTROL] = "Control",
		[USBD_EP_INTERRUPT] = "Interrupt",
		[USBD_EP_ISOCHRONOUS] = "Isochronous",
		[USBD_EP_BULK] = "Bulk",
	};

	LOGF_LN("Created URB %"PRIu64": %s %s %"PRIu8": data=%p, length=%u",
				urb->id, ep_type_map_str[urb->transfer.ep_type],
				IS_IN_ENDPOINT(urb->transfer.ep_addr) ? "IN" : "OUT",
				ENDPOINT_NUMBER(urb->transfer.ep_addr),
				urb->transfer.buffer, urb->transfer.length);
#endif

	bool to_active = !dev->urbs.force_all_new_urb_to_waiting &&
						try_alloc_ep_for_urb(dev, urb);

	LOGF_LN("[new] URB id=%"PRIu64" is %s", urb->id,
					to_active ? "active" : "waiting");

	if (to_active) {
		mark_ep_as_free(dev, urb->transfer.ep_addr, false);
		queue_item_append(&dev->urbs.active, urb);
		dev->backend->urb_submit(dev, urb);
	} else {
		queue_item_append(&dev->urbs.waiting, urb);
	}

	return urb->id;
}

/**
 * Do the callback for the @a urb
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param[in] status @a urb Status
 */
static void urb_callback(usbd_device *dev, usbd_urb *urb,
			usbd_transfer_status status)
{
	LOG_CALL

	LOGF_LN("URB %"PRIu64" transfer status = %s", urb->id,
		stringify_transfer_status(status));

	/* callback provided */
	if (urb->transfer.callback == NULL) {
		return;
	}

	/* Explicitly mentioned not to do success callback */
	if (urb->transfer.flags & USBD_FLAG_NO_SUCCESS_CALLBACK) {
		if (status == USBD_SUCCESS) {
			return;
		}
	}

	urb->transfer.callback(dev, &urb->transfer, status, urb->id);
}

bool usbd_transfer_cancel(usbd_device *dev, usbd_urb_id urb_id)
{
	usbd_urb *urb, *prev;

	if (IS_URB_ID_INVALID(urb_id)) {
		LOG_LN("invalid urb id passed to transfer_cancel");
		return false;
	}

	/* Check the Active Queue */
	for (prev = NULL, urb = dev->urbs.active.head; urb != NULL; ) {
		if (urb->id != urb_id) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		queue_item_detach(&dev->urbs.active, prev, urb);
		free_ep_from_urb(dev, urb);
		urb_callback(dev, urb, USBD_ERR_CANCEL);
		unused_push(dev, urb);
		return true;
	}

	/* Check the Waiting Queue */
	for (prev = NULL, urb = dev->urbs.waiting.head; urb != NULL; ) {
		if (urb->id != urb_id) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		queue_item_detach(&dev->urbs.waiting, prev, urb);
		urb_callback(dev, urb, USBD_ERR_CANCEL);
		unused_push(dev, urb);
		return true;
	}

	LOGF_LN("WARN: urb with id = %"PRIu64" not found", urb_id);
	return false;
}

unsigned usbd_transfer_cancel_ep(usbd_device *dev, uint8_t ep_addr)
{
	unsigned result = 0;
	usbd_urb *urb, *prev;

	/* Check the Active Queue */
	if (!is_ep_free(dev, ep_addr)) {
		for (prev = NULL, urb = dev->urbs.active.head; urb != NULL; ) {
			if (urb->transfer.ep_addr != ep_addr) {
				prev = urb;
				urb = urb->next;
				continue;
			}

			queue_item_detach(&dev->urbs.active, prev, urb);
			free_ep_from_urb(dev, urb);
			urb_callback(dev, urb, USBD_ERR_CANCEL);
			unused_push(dev, urb);
			result++;
			break;
		}
	}

	/* Check the Waiting Queue */
	for (prev = NULL, urb = dev->urbs.waiting.head; urb != NULL; ) {
		if (urb->transfer.ep_addr != ep_addr) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		queue_item_detach(&dev->urbs.waiting, prev, urb);
		urb_callback(dev, urb, USBD_ERR_CANCEL);
		unused_push(dev, urb);
		result++;
	}

	return result;
}

/**
 * Schedule new URB from WAITING to ACTIVE if the endpoint is free
 * @param[in] dev USB Device
 */
void usbd_urb_schedule(usbd_device *dev)
{
	if (dev->urbs.force_all_new_urb_to_waiting) {
		LOG_LN("Could not schedule (force_all_new_urb_to_waiting = true)");
		return;
	}

	usbd_urb *urb = dev->urbs.waiting.head, *prev = NULL, *tmp;

	while (urb != NULL) {
		if (!try_alloc_ep_for_urb(dev, urb)) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		tmp = urb;
		urb = queue_item_detach(&dev->urbs.waiting, prev, tmp);
		queue_item_append(&dev->urbs.active, tmp);

		LOGF_LN("[waiting] URB id=%"PRIu64" is now active", tmp->id);
		dev->backend->urb_submit(dev, tmp);
	}
}

/**
 * Detach the URB from active list
 * Usage: backend to remove a URB from active list.
 * @param[in] dev USB Device
 * @param[in] item Item to detach
 */
static void detach_from_active(usbd_device *dev, usbd_urb *item)
{
	usbd_urb *urb = dev->urbs.active.head, *prev = NULL;

	while (urb != NULL) {
		if (urb != item) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		queue_item_detach(&dev->urbs.active, prev, urb);
		free_ep_from_urb(dev, urb);
		return;
	}

	LOGF_LN("WARNING: Found not find URB %"PRIu64" in active list to detach it",
		urb->id);
}

/**
 * Free the URB as it purpose has been served.
 * Usage: Backend use it to mark the transfer
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param[in] status Transfer status to provide to callback
 */
void usbd_urb_complete(usbd_device *dev, usbd_urb *urb,
			usbd_transfer_status status)
{
	detach_from_active(dev, urb);
	urb_callback(dev, urb, status);
	unused_push(dev, urb);
	usbd_urb_schedule(dev);
}

/**
 * Find the current processing URB
 * @param[in] dev USB Device
 * @param[in] ep_addr Endpoint (including direction)
 * @return Found URB
 * @return NULL (Not found - If this is the case, we are in problem)
 */
usbd_urb *usbd_find_active_urb(usbd_device *dev, uint8_t ep_addr)
{
	usbd_urb *urb = NULL;

	LOGF_LN("Searching for URB which is active and has "
		"ep_addr=0x%"PRIx8, ep_addr);

	for (urb = dev->urbs.active.head; urb != NULL; urb = urb->next) {
		if (urb->transfer.ep_addr == ep_addr) {
			return urb;
		}
	}

	LOGF_LN("Unable to find the current processing URB for "
		"endpoint 0x%"PRIx8, ep_addr);
	return NULL;
}

/**
 * Clear up the whole queue
 * @param[in] dev USB Device
 * @param[in] queue Queue to clear up
 * @param[in] status Status transfer to end with
 */
static void flush_queue(usbd_device *dev, struct usbd_urb_queue *queue,
		usbd_transfer_status status)
{
	usbd_urb *urb;

	for (urb = queue->head; urb != NULL; urb = urb->next) {
		urb_callback(dev, urb, status);
	}

	queue->head = queue->tail = NULL;
}

/**
 * Intalize @a dev->urbs->unused.
 * All arr entries are set to unused.
 * @param[in] dev USB Device
 */
void usbd_put_all_urb_into_unused(usbd_device *dev)
{
	unsigned i;
	usbd_urb *prev;

	dev->urbs.unused = prev = &dev->urbs.arr[0];

	for (i = 1; i < USBD_URB_COUNT; i++) {
		usbd_urb *urb = &dev->urbs.arr[i];
		prev->next = urb;
		prev = urb;
	}

	prev->next = NULL;
}

/**
 * Move every URB to unused list (a reset has occured)
 * @param[in] dev USB device
 * @param[in] status Transfer status
 */
void usbd_purge_all_transfer(usbd_device *dev, usbd_transfer_status status)
{
	flush_queue(dev, &dev->urbs.active, status);
	flush_queue(dev, &dev->urbs.waiting, status);
	usbd_put_all_urb_into_unused(dev);
	dev->urbs.ep_free = ~0;
}

/**
 * Purge any non zero endpoint transfer with @a status
 * @param dev USB Device
 * @param queue Queue
 * @param status Callback status
 */
static void purge_non_ep0_tranfer_from_queue(usbd_device *dev,
			struct usbd_urb_queue *queue, usbd_transfer_status status)
{
	usbd_urb *urb = queue->head, *prev = NULL, *tmp;

	while (urb != NULL) {
		if (!ENDPOINT_NUMBER(urb->transfer.ep_addr)) {
			prev = urb;
			urb = urb->next;
			continue;
		}

		tmp = urb;
		urb = queue_item_detach(queue, prev, tmp);
		urb_callback(dev, tmp, status);
		unused_push(dev, tmp);
	}
}

/**
 * Reset all non EP0 endpoint (and remove the transfers too)
 * Cancel all non EP0 transfer.
 * Usage: SET_CONFIGURATION
 * @param[in] dev USB Device
 * @param[in] status Transfer status
 */
void usbd_purge_all_non_ep0_transfer(usbd_device *dev,
				usbd_transfer_status status)
{
	purge_non_ep0_tranfer_from_queue(dev, &dev->urbs.active, status);
	purge_non_ep0_tranfer_from_queue(dev, &dev->urbs.waiting, status);

	/* Mark all endpoint as free (except EP0) */
	dev->urbs.ep_free |= ~0x00010001;
}

/**
 * Callback from the data stage transfer
 */
static void _control_status_callback(usbd_device *dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		LOGF_LN("[Control status stage] Transfer %"PRIu64" failed with "
			"status=%s", urb_id, stringify_transfer_status(status));
		return;
	}

	usbd_control_transfer_callback callback = (usbd_control_transfer_callback)transfer->user_data;
	if (callback != NULL) {
		callback(dev, NULL);
	}
}

/**
 * Perform status stage.
 * @param[in] ep_addr Endpoint address (bit7: high = status in, low = status out)
 * @param[in] ep_size Endpoint size
 * @param[in] callback Callback to be done when successful
 */
static void _control_status_stage(usbd_device *dev, uint8_t ep_addr,
		uint16_t ep_size, usbd_control_transfer_callback callback)
{
	const usbd_transfer transfer = {
		.ep_type = USBD_EP_CONTROL,
		.ep_addr = ep_addr,
		.ep_size = ep_size,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = NULL,
		.length = 0,
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = _control_status_callback,
		.user_data = callback
	};

	if (usbd_transfer_submit(dev, &transfer) == USBD_INVALID_URB_ID) {
		LOGF_LN("Failed to submit status transfer for control endpoint 0x%"PRIx8,
					ep_addr);
	}
}

/**
 * Callback from the data stage transfer
 */
static void _control_data_callback(usbd_device *dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		LOGF_LN("[Control data stage] Transfer %"PRIu64" failed with "
			"status=%s, not going into status stage", urb_id,
			stringify_transfer_status(status));
		return;
	}

	usbd_control_transfer_callback callback = (usbd_control_transfer_callback)transfer->user_data;
	usbd_control_transfer_feedback feedback = USBD_CONTROL_TRANSFER_OK;

	if (callback != NULL) {
		usbd_control_transfer_callback_arg arg = {
			.buffer = transfer->buffer,
			.length = transfer->transferred
		};

		/* User provided callback, ask callback what to do next */
		feedback = callback(dev, &arg);
	}

	/* User want to stall the transfer */
	if (feedback & USBD_CONTROL_TRANSFER_STALL) {
		usbd_set_ep_stall(dev, ENDPOINT_NUMBER(transfer->ep_addr), true);
		usbd_set_ep_stall(dev, ENDPOINT_NUMBER(transfer->ep_addr) | 0x80, true);
		return;
	}

	/* User dont want anymore callback */
	if (feedback & USBD_CONTROL_TRANSFER_NO_STATUS_CALLBACK) {
		callback = NULL;
	}

	_control_status_stage(dev, transfer->ep_addr ^ 0x80, transfer->ep_size,
		callback);
}

/**
 * Perform DATA stage
 * @param[in] dev USB Device
 * @param[in] ep_addr Endpoint address (7bit: high = DATA IN, low = DATA OUT)
 * @param[in] ep_size Endpoint size
 * @param[in] wLength Number of actually data bytes requested by host
 * @param[in] buf Pointer to buffer to use
 * @param[in] len Number of bytes to actually send
 * @param[in] callback Callback to done on successful completition of transaction.
 */
static void _control_data_stage(usbd_device *dev, uint8_t ep_addr,
		uint8_t ep_size, uint16_t wLength, void *buf, size_t len,
		usbd_control_transfer_callback callback)
{
	if (IS_IN_ENDPOINT(ep_addr)) {
		if (len > wLength) {
			LOGF_LN("WARN: Try to send %u bytes when host only requested "
				"%"PRIu16" bytes (more than requested) on control endpoint "
				"0x%"PRIx8, len, wLength, ep_addr);
			len = wLength;
		}
	} else {
		if (len < wLength) {
			LOGF_LN("WARNING: Host will be sending %"PRIu16" bytes but "
				"buffer can only accomodated %u bytes on control endpoint "
				"0x%"PRIx8, wLength, len, ep_addr);
		}
	}

	usbd_transfer_flags flags = USBD_FLAG_NONE;

	/*
	 * Transfer should always end with a short packet tell the host that
	 *  because we have less data than the host is expecting.
	 * This flag come into play when the "len" is multiple of endpoint size.
	 */
	if (len != wLength) {
		flags = USBD_FLAG_SHORT_PACKET;
	}

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_CONTROL,
		.ep_addr = ep_addr,
		.ep_size = ep_size,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = buf,
		.length = len,
		.flags = flags,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = _control_data_callback,
		.user_data = callback
	};

	if (usbd_transfer_submit(dev, &transfer) == USBD_INVALID_URB_ID) {
		LOGF_LN("Failed to submit data transfer for control endpoint 0x%"PRIx8,
					ep_addr);
	}
}

void usbd_control_transfer(usbd_device *dev, uint8_t ep_addr,
	uint16_t ep_size, const struct usb_setup_data *setup_data,
	void *buf, size_t len, usbd_control_transfer_callback callback)
{
	/* Data stage and status stage both have DTOG = 1 */
	dev->backend->set_ep_dtog(dev, ep_addr | 0x80, true);
	dev->backend->set_ep_dtog(dev, ep_addr & 0x7F, true);

	if (!setup_data->wLength) {
		/* wLength is zero, directly proceed to status IN stage! */
		_control_status_stage(dev, ep_addr | 0x80, ep_size, callback);
		return;
	}

	if (!(setup_data->bmRequestType & 0x80)) {
		if (setup_data->wLength > len) {
			/* User is providing less buffer to store than the host is
			 *  going to send for data stage.
			 * If our safest bet is to STALL the transaction.
			 */
			LOGF_LN("STALL: User provide less buffer than "
						"the host is going to send");
			usbd_set_ep_stall(dev, ep_addr | 0x80, true);
			usbd_set_ep_stall(dev, ep_addr & 0x7F, true);
			return;
		}
	}

	if (setup_data->bmRequestType & 0x80) {
		ep_addr |= 0x80;
	} else {
		ep_addr &= 0x7F;
	}

	_control_data_stage(dev, ep_addr, ep_size, setup_data->wLength, buf, len,
				callback);
}

void usbd_ep0_transfer(usbd_device *dev,
	const struct usb_setup_data *setup_data, void *buf, size_t len,
	usbd_control_transfer_callback callback)
{
	usbd_control_transfer(dev, 0x00, dev->desc->bMaxPacketSize0,
		setup_data, buf, len, callback);
}

void usbd_ep0_stall(usbd_device *dev)
{
	LOG_LN("Stalling EP0");
	usbd_set_ep_stall(dev, 0x00, true);
	usbd_set_ep_stall(dev, 0x80, true);
}

/**
 * Called by backend to get a pointer to receive/transmit 1 packet
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param len The number of bytes it should be able to hold/provide
 * @return pointer to data
 */
void *usbd_urb_get_buffer_pointer(usbd_device *dev, usbd_urb *urb, size_t len)
{
	usbd_transfer *transfer = &urb->transfer;
	bool out = IS_OUT_ENDPOINT(transfer->ep_addr);

#if defined(USBD_DEBUG)
	if ((transfer->transferred + len) > transfer->length) {
		LOGF_LN("URB %"PRIu64" buffer overflow detected! "
			"(backend want to %s %"PRIu16" bytes to buffer)", urb->id,
			out ? "write" : "read", len);
		LOGF_LN("transfer->length: %"PRIu16, transfer->length);
		LOGF_LN("transfer->transferred: %"PRIu16, transfer->transferred);
	}
#else
	(void) len;
#endif

	if (transfer->flags & USBD_FLAG_PER_PACKET_CALLBACK) {
		if (!out) {
			/* IN endpoint, get data from user */
			URB_CALLBACK(dev, urb, USBD_ONE_PACKET_DATA);
		}
	}

	if (transfer->flags & USBD_FLAG_NO_MEMORY_INCREMENT) {
		/* User said to reuse same buffer location everytime. */
		return transfer->buffer;
	}

	return (void*)((uintptr_t)transfer->buffer + transfer->transferred);
}

/**
 * Called by backend when it get data
 * @param[in] dev USB Device
 * @param[in] urb USB Request Block
 * @param[in] len
 */
void usbd_urb_inc_data_pointer(usbd_device *dev, usbd_urb *urb, size_t len)
{
	usbd_transfer *transfer = &urb->transfer;
	bool out = IS_OUT_ENDPOINT(transfer->ep_addr);

#if defined(USBD_DEBUG)
	if ((transfer->transferred + len) > transfer->length) {
		LOGF_LN("URB %"PRIu64" buffer overflow detected! "
			"(backend is reporting that it has %s %"PRIu16" bytes)", urb->id,
			out ? "written" : "readed", len);
		LOGF_LN("transfer->length: %"PRIu16, transfer->length);
		LOGF_LN("transfer->transferred: %"PRIu16, transfer->transferred);
	}
#endif

	transfer->transferred += len;

	if (transfer->flags & USBD_FLAG_PER_PACKET_CALLBACK) {
		if (out) {
			/* OUT endpoint, give data to user */
			URB_CALLBACK(dev, urb, USBD_ONE_PACKET_DATA);
		}
	}
}
