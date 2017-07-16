/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2015, 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

/*
 * This file is intended to declare internal debug log printing port
 *  It can be copyed into target project folder, and modifyed to link with target system
 */

#ifndef UNICOREMX_USBD_LOG_PRIVATE_H
#define UNICOREMX_USBD_LOG_PRIVATE_H

#include <unicore-mx/cm3/common.h>

BEGIN_DECLS



#define USB_VALL   0
#define USB_VFAIL  1
#define USB_VINFO  2
#define USB_VNOTE  3
#define USB_VDEBUG 4
#define USB_VTRACE 5

#define USB_VCALL  USB_VTRACE
#define USB_VIO    USB_VDEBUG
#define USB_VIO2   USB_VTRACE
#define USB_VIO_INIT   USB_VDEBUG
#define USB_VSETUP USB_VDEBUG
#define USB_VSETUP_MSC USB_VDEBUG
#define USB_VURB       USB_VDEBUG
#define USB_VURBQUE    USB_VTRACE
#define USB_VURBFAIL   USB_VDEBUG
#define USB_VIO_MSC    USB_VDEBUG
#define USB_MSC        USB_VDEBUG

#if !defined(USBD_DEBUG)
//# define USBD_DEBUG USB_VDEBUG
#endif

#define NEW_LINE "\n"
#define PRIurb PRIu64
#define view_urbid(uid) uid

#if defined(USBD_DEBUG)
extern void usbd_log_puts(const char *arg);
extern void usbd_log_printf(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));
extern void usbd_log_call(const char *fname);

# include <inttypes.h>
# define USBD_STR(s) s
# define USBD_LOG(level, str)      if (level <= USBD_DEBUG) usbd_log_puts(str)
# define USBD_LOGF(level, fmt,...) if (level <= USBD_DEBUG) usbd_log_printf(fmt, ##__VA_ARGS__)

# define USBD_LOG_LN(level, str) USBD_LOG(level, USBD_STR(fmt)NEW_LINE);
//# define USBD_LOGF_LN(level, fmt,...) USBD_LOGF(level, fmt, __VA_ARGS__); USBD_LOG(level, NEW_LINE );
# define USBD_LOGF_LN(level, fmt,...) USBD_LOGF(level, USBD_STR(fmt)NEW_LINE, __VA_ARGS__)

# if USBD_DEBUG >= USB_VCALL
# define USBD_LOG_CALL usbd_log_call(__func__);
# else
# define USBD_LOG_CALL
# endif

//temps for backport
# define LOG(str)      USBD_LOG(USB_VALL, str)
# define LOGF(fmt,...) USBD_LOGF(USB_VALL, fmt, __VA_ARGS__)

# define LOG_LN(str)   USBD_LOG_LN(USB_VALL, str)
# define LOGF_LN(fmt,...) USBD_LOGF_LN(USB_VALL, fmt, __VA_ARGS__)

# define LOG_CALL      USBD_LOG_CALL

#else
# define USBD_LOG(level, str)
# define USBD_LOGF(level, fmt,...)
# define USBD_LOG_LN(level, str)
# define USBD_LOGF_LN(level, fmt,...)
# define USBD_LOG_CALL

# define LOG(str)
# define LOGF(fmt,...)
# define LOG_LN(str)
# define LOGF_LN(fmt,...)
# define LOG_CALL
#endif



END_DECLS

#endif
