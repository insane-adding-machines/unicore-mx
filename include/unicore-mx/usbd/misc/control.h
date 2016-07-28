/*
 * This file is part of the unicore-mx project.
 *
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

#ifndef USBD_MISC_CONTROL_H
#define USBD_MISC_CONTROL_H

#include <unicore-mx/usbd/usbd.h>

struct usbd_control_handler {
	usbd_control_callback callback;

	/* Usage: (bmRequestType & type_mask) == type_value */
	uint8_t type_mask;
	uint8_t type_value;
};

/**
 * Route control callback to @a handlers.
 * @param usbd_dev USB Device
 * @param arg Control argument
 * @param handlers Handlers (last entry callback need to be NULL)
 * @return USBD_REQ_NEXT if no handler found
 * @note if a handler type matched and returned value (other than USBD_REQ_NEXT) then
 *   the function immediately return that value.
 */
enum usbd_control_result
usbd_control_route(usbd_device *usbd_dev, usbd_control_arg *arg,
			const struct usbd_control_handler *handlers);

/**
 * Simplify the process of STALL.
 * Instead of assert() use this for control callback function.
 */
#define USBD_STALL_ON_ASSERT_FAIL(to_assert)			\
	if (!(to_assert)) {							\
		return USBD_REQ_STALL;					\
	}

/** A smaller name version of USBD_STALL_ON_ASSERT_FAIL() macro */
#define USBD_STALL_ASSERT(to_assert) USBD_STALL_ON_ASSERT_FAIL(to_assert)

#endif
