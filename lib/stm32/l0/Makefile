##
## Copyright (C) 2013 Frantisek Burian <BuFran@seznam.cz>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

LIBNAME		= libucmx_stm32l0
SRCLIBDIR	?= ../..

PREFIX		?= arm-none-eabi
#PREFIX		?= arm-elf
CC		= $(PREFIX)-gcc
AR		= $(PREFIX)-ar
TGT_CFLAGS	= -Os \
		  -Wall -Wextra -Wimplicit-function-declaration \
		  -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes \
		  -Wundef -Wshadow \
		  -I../../../include -fno-common \
		  -mcpu=cortex-m0plus $(FP_FLAGS) -mthumb -Wstrict-prototypes \
		  -ffunction-sections -fdata-sections -MD -DSTM32L0
TGT_CFLAGS      += $(DEBUG_FLAGS)

ARFLAGS		= rcs

OBJS		= gpio.o rcc.o desig.o
OBJS		+= pwr_common_all.o pwr_common_l01.o timer_common_all.o

OBJS            += gpio_common_all.o gpio_common_f0234.o rcc_common_all.o
OBJS		+= adc_common_v2.o
OBJS		+= crs_common_all.o

OBJS		+= usbd.o usbd_control.o usbd_standard.o
OBJS		+= usbd_stm32_fsdev.o usbd_stm32_fsdev_v2.o
OBJS		+= usbd_msc.o
OBJS		+= usbd_control.o usbd_string.o

VPATH += ../:../../cm3:../common
VPATH += ../../usbd:../../usbd/class:../../usbd/backend:../../usbd/misc

include ../../Makefile.include

