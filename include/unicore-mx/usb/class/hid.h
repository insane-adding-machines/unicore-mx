/** @defgroup usb_hid_defines USB HID Type Definitions

@brief <b>Defined Constants and Types for the USB HID Type Definitions</b>

@ingroup USB_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2010
Gareth McMullin <gareth@blacksphere.co.nz>

@date 10 March 2013

LGPL License Terms @ref lgpl_license
*/

/*
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

#ifndef UNICORE_USB_CLASS_HID_H
#define UNICORE_USB_CLASS_HID_H

#include <stdint.h>

#define USB_CLASS_HID	3

#define USB_DT_HID	0x21
#define USB_DT_REPORT	0x22

#define USB_REQ_HID_SET_IDLE 0x0A
#define USB_REQ_HID_SET_PROTOCOL 0x0B
#define USB_REQ_HID_PROTOCOL_BOOT 0x00
#define USB_REQ_HID_PROTOCOL_REPORT 0x01

#define USB_REQ_HID_GET_REPORT 0x01
#define USB_REQ_HID_REPORT_TYPE_INPUT 0x01
#define USB_REQ_HID_REPORT_TYPE_OUTPUT 0x02
#define USB_REQ_HID_REPORT_TYPE_FEATURE 0x03

struct usb_hid_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
} __attribute__((packed));

#endif

/**@}*/

