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

#include <unicore-mx/usbd/misc/control.h>

enum usbd_control_result
usbd_control_route(usbd_device *dev, usbd_control_arg *arg,
			const struct usbd_control_handler *handlers)
{
	for (;;) {
		/* EOF list? */
		if (handlers->callback == NULL) {
			break;
		}

		/* Continue until some handler handle the request  */
		if ((arg->setup.bmRequestType & handlers->type_mask) == handlers->type_value) {
			enum usbd_control_result result = handlers->callback(dev, arg);
			if (result != USBD_REQ_NEXT) {
				return result;
			}
		}

		handlers++;
	}

	/* No handler found */
	return USBD_REQ_NEXT;
}
