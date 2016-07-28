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

#include <unicore-mx/usbd/misc/string.h>

int usbd_utf8_to_utf16(const uint8_t *utf8,
					uint16_t *utf16, unsigned max_count)
{
	unsigned used = 0;
	unsigned i = 0;

	/* argument check */
	if ((utf8 == NULL) || (utf16 == NULL)) {
		return -1;
	}

	/* do anything? */
	if (!max_count) {
		return 0;
	}

	for (;;) {
		uint32_t utf32_ch;

		/* Read input string (utf8) till \0 is not reached */
		if (utf8[i] == '\0') {
			break;
		}

		/* Starting by assuming ASCII */
		utf32_ch = utf8[i++]; /* 0xxxxxxx */

		/* Reference: https://en.wikipedia.org/wiki/UTF-16 */

		/* utf-8? */
		if (utf32_ch & 0b10000000) {
			/* Decoding header */
			uint8_t trailing_bytes_utf8;
			if ((utf32_ch & 0b11100000) == 0b11000000) {
				/* 110xxxxx 10xxxxxx */
				trailing_bytes_utf8 = 1;
				utf32_ch &= 0b00011111;
			} else if((utf32_ch & 0b11110000) == 0b11100000) {
				/* 1110xxxx 10xxxxxx 10xxxxxx */
				trailing_bytes_utf8 = 2;
				utf32_ch &= 0b00001111;
			} else if((utf32_ch & 0b11111000) == 0b11110000) {
				/* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
				trailing_bytes_utf8 = 3;
				utf32_ch &= 0b00000111;
			} else {
				/* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 5- and 6-byte sequences are not valid in utf-8 */
				return -1;
			}

			if (!utf32_ch) {
				/* The standard specifies that the correct encoding of a
				 * code point use only the minimum number of bytes required
				 * to hold the significant bits of the code point */
				return -1;
			}

			/* trailing bytes payload */
			while (trailing_bytes_utf8-- > 0) {
				utf32_ch = (utf32_ch << 6) | (utf8[i++] & 0b00111111);
			}

			/* UTF-8 was restricted by RFC 3629 to end at U+10FFFF, in order
			 * to match the constraints of the UTF-16 character encoding */
			if (utf32_ch > 0x10FFFF) {
				return -1;
			}
		}

		/* Reference: https://en.wikipedia.org/wiki/UTF-16 */

		/* Converting the utf-32 character to utf-16 character */
		if ((utf32_ch <= 0xD7FF) || ((utf32_ch >= 0xE000) && (utf32_ch <= 0xFFFF))) {
			/* normal 16bit data */

			/* Out of memory? */
			if (!((used + 1) < max_count)) {
				break;
			}

			utf16[used++] = (uint16_t) utf32_ch;
		} else if ((utf32_ch >= 0x10000) && (utf32_ch <= 0x10FFFF)) {
			/* surrogate pairs */
			uint16_t surrogate_high, surrogate_low;

			/* memory available to store utf-16 character */
			if (!((used + 2) < max_count)) {
				break;
			}

			/* encode */
			utf32_ch -= 0x010000;
			surrogate_low = 0xDC00 + (utf32_ch & 0x3FF);
			surrogate_high = 0xD800 + (utf32_ch >> 10);

			/* store */
			utf16[used++] = surrogate_high;
			utf16[used++] = surrogate_low;
		} else {
			/* Cannot convert to utf-16 */
			return -1;
		}
	}

	return used;
}

int usbd_handle_string_ascii(struct usbd_get_string_arg *arg,
								const char **strings, unsigned strings_count)
{
	/* supported languages */
	if (!arg->index) {
		uint16_t len = arg->len;
		if (len) {
			len = 1;
			arg->buf[0] = USB_LANGID_ENGLISH_UNITED_STATES;
		}
		return len;
	}

	/* Only support English */
	if (arg->lang_id != USB_LANGID_ENGLISH_UNITED_STATES) {
		return -1;
	}

	/* we only have 3 strings */
	if (arg->index > strings_count) {
		return -1;
	}

	return usbd_utf8_to_utf16((const uint8_t *)strings[arg->index - 1],
						arg->buf, arg->len);
}
