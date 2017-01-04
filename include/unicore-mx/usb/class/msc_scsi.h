/** @defgroup usb_msc_scsi_defines USB MSC SCSI API  Definitions

@brief <b>Defined Constants and Types for the USB MSC SCSI API Definitions</b>

@ingroup USB_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2013
alexrayne <alexraynepe196@gmail.com>
@date 30 ecember 2016

LGPL License Terms @ref lgpl_license
*/
/*
 * Copyright (C) 2016 alexrayne <alexraynepe196@gmail.com>
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

#ifndef UNICORE_USB_CLASS_MSC_SCSI_H
#define UNICORE_USB_CLASS_MSC_SCSI_H

#include <stdint.h>

/* Implemented SCSI Commands */
#define USB_MSC_SCSI_TEST_UNIT_READY		0x00
#define USB_MSC_SCSI_REQUEST_SENSE			0x03
#define USB_MSC_SCSI_FORMAT_UNIT			0x04
#define USB_MSC_SCSI_READ_6					0x08
#define USB_MSC_SCSI_WRITE_6				0x0A
#define USB_MSC_SCSI_INQUIRY				0x12
#define USB_MSC_SCSI_MODE_SENSE_6			0x1A
#define USB_MSC_SCSI_SEND_DIAGNOSTIC		0x1D
#define USB_MSC_SCSI_READ_FORMAT_CAPACITIES		0x23
#define USB_MSC_SCSI_READ_CAPACITY			0x25
#define USB_MSC_SCSI_READ_10				0x28
/* Required SCSI Commands */

/* Optional SCSI Commands */
#define USB_MSC_SCSI_REPORT_LUNS					0xA0
#define USB_MSC_SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define USB_MSC_SCSI_MODE_SELECT_6					0x15
#define USB_MSC_SCSI_MODE_SELECT_10					0x55
#define USB_MSC_SCSI_MODE_SENSE_10					0x5A
#define USB_MSC_SCSI_READ_12						0xA8
#define USB_MSC_SCSI_READ_TOC_PMA_ATIP				0x43
#define USB_MSC_SCSI_START_STOP_UNIT				0x1B
#define USB_MSC_SCSI_SYNCHRONIZE_CACHE				0x35
#define USB_MSC_SCSI_VERIFY							0x2F
#define USB_MSC_SCSI_WRITE_10						0x2A
#define USB_MSC_SCSI_WRITE_12						0xAA

#ifdef __cplusplus
  extern "C" {
#endif 

//* structires uses net order (big_endian)
typedef uint32_t	net_u32_t;
typedef uint16_t	net_u16_t;
typedef uint8_t		net_u24_t[3];

static inline
void u24_assign(net_u24_t dst, uint32_t x){
	dst[2] = x & 0xff; 
	x >>= 8;
	dst[1] = x & 0xff;
	x >>= 8;
	dst[0] = x & 0xff;
};

static inline 
uint32_t u24_asul(net_u24_t x)
{
	uint32_t res = (x[0] << 8) | x[1];
	res = (res << 8) | x[2];
	return res;
};

typedef enum{
	  scsi_vpd_Supported         = 0	//* < Supported Vital Product Data pages 
	, scsi_vpd_Serial            = 0x80 //* < Unit Serial Number page
	, scsi_vpd_DevIdentification = 0x83 //* < Device Identification
} scsi_VPD_page;

//* SCSI - INQUIRY structures
typedef struct {
	uint8_t		op_code;
	uint8_t		evpd;
	uint8_t		page_code;	//* \see scsi_VPD_page
	net_u16_t	alloc_len;
} __attribute__((packed)) usb_inquiry_cmd ;


typedef enum {
/**  A peripheral device having the specified peripheral device type is connected to this logical unit. If the 
device server is unable to determine whether or not a peripheral device is connected, it also shall use 
this peripheral qualifier. This peripheral qualifier does not mean that the peripheral device connected 
to the logical unit is ready for access.*/
	  scsi_pqAvail     = 0  
/** A peripheral device having the specified peripheral device type is not connected to this logical unit. 
However, the device server is capable of supporting the specified peripheral device type on this logi-
cal unit. */
	, scsi_pqRemoved   = 1
/** The device server is not capable of supporting a peripheral device on this logical unit. For this periph-
eral qualifier the peripheral device type shall be set to 1Fh. All other peripheral device type values are 
reserved for this peripheral qualifier */
	, scsi_pqNoSupport = 3
} scsi_periferial_qualify;

typedef enum {
	  scsi_dtDirectBlock = 0	//* < Direct access block device (e.g., magnetic disk)
	, scsi_dtSeqBlock   = 1	//* < Sequential-access device (e.g., magnetic tape)
	, scsi_dtWriteOnce  = 4	//* < Write-once device (e.g., some optical disks)
	, scsi_dtCDDVD      = 5	//* < CD/DVD device
	, scsi_dtOptical    = 7	//* <Optical memory device (e.g., some optical disks)
	, scsi_dtRAID       = 0xc	//* <Storage array controller device (e.g., RAID)
	, scsi_dtSimleDirect= 0xe	//* <Simplified direct-access device (e.g., magnetic disk)
} scsi_periferial_devtype;

#define SCSI_DEVICE_CODE(type, qual) ( (((qual) & 0x7)<<5) | ((type) & 0x1f) )

typedef struct {
	uint8_t   device_code;   //*  \see SCSI_DEVICE_CODE
	uint8_t   page_code;	   //* \see scsi_VPD_page
	uint8_t   page_len;
} __attribute__((packed)) usb_inquiry_ack ;



//* SCSI - READ FORMAT CAPACITIES structures
typedef struct {
	net_u24_t	dummy;
	uint8_t		list_len;
} __attribute__((packed)) usb_msc_rfc_capacity_list_header ;

typedef enum {
	  rfc_dc_Unfomatted	= 1	//* < Unformatted Media - Maximum formattable capacity for this cartridge
	, rfc_dc_Fomatted		= 2	//* < Formatted Media - Current media capacity
	, rfc_dc_NoMedia		= 3	//* < No Cartridge in Drive - Maximum formattable capacity for any cartridge
} usb_msc_rfc_capacity_descriptor_code;

typedef struct {
	net_u32_t	blocks_count;
	uint8_t		code;	//* < \see usb_msc_rfc_capacity_descriptor_code
	net_u24_t	block_size;
} __attribute__((packed)) usb_msc_rfc_capacity_descriptor;

#ifdef __cplusplus
  }
#endif 

#endif//UNICORE_USB_CLASS_MSC_SCSI_H
