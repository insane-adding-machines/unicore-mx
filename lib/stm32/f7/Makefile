##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2013 Alexandru Gagniuc <mr.nuke.me@gmail.com>
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

LIBNAME		= libucmx_stm32f7
SRCLIBDIR	?= ../..

PREFIX		?= arm-none-eabi

CC		= $(PREFIX)-gcc
AR		= $(PREFIX)-ar

# STM32F7 only supports single precision FPU
FP_FLAGS	?= -mfloat-abi=hard -mfpu=fpv5-sp-d16

TGT_CFLAGS	= -Os -g \
		  -Wall -Wextra -Wimplicit-function-declaration \
		  -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes \
		  -Wundef -Wshadow \
		  -I../../../include -fno-common \
		  -mcpu=cortex-m4 -mthumb $(FP_FLAGS) \
		  -Wstrict-prototypes \
		  -ffunction-sections -fdata-sections -MD -DSTM32F7
TGT_CFLAGS      += $(DEBUG_FLAGS)
ARFLAGS		= rcs

OBJS		= rcc.o gpio.o gpio_common_all.o gpio_common_f0234.o

OBJS		+= rcc_common_all.o flash_common_f234.o flash_common_f24.o usart_f7.o \
		    i2c_common_all.o dma_common_f247.o usart_common_all.o exti_common_all.o rng_common_f247.o

OBJS		+= timer_common_all.o timer_common_f2347.o timer_common_f247.o

OBJS		+= usbd.o usbd_ep0.o usbd_transfer.o
OBJS		+= usbd_dwc_otg.o usbd_stm32_otg_fs.o usbd_stm32_otg_hs.o
OBJS		+= usbd_msc.o


OBJS		+= ltdc_common_f47.o fmc_common_f47.o

OBJS		+= mac.o phy.o mac_stm32fxx7.o phy_ksz8051mll.o fmc_common_f47.o

OBJS		+= pwr.o pwr_common_all.o

# FIXME: USB device stack not being compiled

OBJS		+= usbh_dev_enum.o usbh_device.o usbh_host.o
OBJS		+= usbh_hub.o usbh_transfer.o usbh_urb.o
OBJS		+= usbh_dwc_otg.o usbh_stm32_otg_fs.o
OBJS		+= usbh_hid.o
OBJS		+= usbh_ctrlreq.o

VPATH += ../:../../cm3:../common
VPATH += ../../usbd:../../usbd/class:../../usbd/backend
VPATH += ../../ethernet
VPATH += ../../usbh:../../usbh/backend:../../usbh/helper:../../usbh/class

###############################################################################
# Checking CPU support in the toolchain
###############################################################################
# TODO: This check and silent skip of build should be removed, when it will be
# sure that first compatible toolchain (g-a-e 4.8 2014q3) will be sufficiently
# penetrated in majority of user stations.
define MISSING_CPU
Your toolchain doesn't support -mcpu=cortex-m7.
Please use gcc-arm-embedded 4.8 2014q3 or newer. Skipping this sub-library.
endef

ifneq ($(shell $(CC) --help=target | grep -q '\<cortex-m7\>'; echo $$?),0)
  $(warning $(MISSING_CPU))

all clean:
	@true

$(SRCLIBDIR)/$(LIBNAME).a:
	@true

else
  include ../../Makefile.include
endif
