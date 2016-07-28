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

#ifndef USBD_MISC_STRING_H
#define USBD_MISC_STRING_H

#include <unicore-mx/usbd/usbd.h>

BEGIN_DECLS

/**
 * This will try to convert as much as much utf-8 character till they are available and
 *  there is space left in utf-16 buffer.
 * @param[in] utf8 utf-8 input string
 * @param[out] utf16 utf-16 output string
 * @param[in] max_count Maximum number of
 * 				utf-16 character than can be written to @a utf16
 * @return number of bytes used in @a utf16 array
 * @retval -1 if invalid utf-8 character
 * @retval -1 if cannot be character cannot be converted to utf-16
 * @note @a utf8 is assumed to be NULL terminated
 * @note @a utf16 is *not* NULL terminated
 */
int usbd_utf8_to_utf16(const uint8_t *utf8, uint16_t *utf16, unsigned max_count);

/**
 * Common code to handle ASCII (as US English) string only.
 * @copydoc usbd_get_string_callback
 * @param[in] strings ASCII strings
 * @param strings_count Number of ASCII string in @a strings
 */
int usbd_handle_string_ascii(struct usbd_get_string_arg *arg,
			const char **strings, unsigned strings_count);

END_DECLS

#endif
