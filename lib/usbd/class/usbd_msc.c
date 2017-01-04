/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2013 Weston Schmidt <weston_schmidt@alumni.purdue.edu>
 * Copyright (C) 2013 Pavol Rusnak <stick@gk2.sk>
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unicore-mx/cm3/common.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usbd/class/msc.h>
#include "../usbd_private.h"
#include <unicore-mx/usb/byteorder.h>

/*
 * TODO:
 * - Removable media support
 * - Other design too (Bulk only atm)
 */

/* Definitions of Mass Storage Class from:
 *
 * (A) "Universal Serial Bus Mass Storage Class Bulk-Only Transport
 *      Revision 1.0"
 *
 * (B) "Universal Serial Bus Mass Storage Class Specification Overview
 *      Revision 1.0"
 */

/* The sense codes */
enum sbc_sense_key {
	SBC_SENSE_KEY_NO_SENSE			= 0x00,
	SBC_SENSE_KEY_RECOVERED_ERROR		= 0x01,
	SBC_SENSE_KEY_NOT_READY			= 0x02,
	SBC_SENSE_KEY_MEDIUM_ERROR		= 0x03,
	SBC_SENSE_KEY_HARDWARE_ERROR		= 0x04,
	SBC_SENSE_KEY_ILLEGAL_REQUEST		= 0x05,
	SBC_SENSE_KEY_UNIT_ATTENTION		= 0x06,
	SBC_SENSE_KEY_DATA_PROTECT		= 0x07,
	SBC_SENSE_KEY_BLANK_CHECK		= 0x08,
	SBC_SENSE_KEY_VENDOR_SPECIFIC		= 0x09,
	SBC_SENSE_KEY_COPY_ABORTED		= 0x0A,
	SBC_SENSE_KEY_ABORTED_COMMAND		= 0x0B,
	SBC_SENSE_KEY_VOLUME_OVERFLOW		= 0x0D,
	SBC_SENSE_KEY_MISCOMPARE		= 0x0E
};

enum sbc_asc {
	SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION	= 0x00,
	SBC_ASC_PERIPHERAL_DEVICE_WRITE_FAULT	= 0x03,
	SBC_ASC_LOGICAL_UNIT_NOT_READY		= 0x04,
	SBC_ASC_UNRECOVERED_READ_ERROR		= 0x11,
	SBC_ASC_INVALID_COMMAND_OPERATION_CODE	= 0x20,
	SBC_ASC_LBA_OUT_OF_RANGE		= 0x21,
	SBC_ASC_INVALID_FIELD_IN_CDB		= 0x24,
	SBC_ASC_WRITE_PROTECTED			= 0x27,
	SBC_ASC_NOT_READY_TO_READY_CHANGE	= 0x28,
	SBC_ASC_FORMAT_ERROR			= 0x31,
	SBC_ASC_MEDIUM_NOT_PRESENT		= 0x3A
};

enum sbc_ascq {
	SBC_ASCQ_NA				= 0x00,
	SBC_ASCQ_FORMAT_COMMAND_FAILED		= 0x01,
	SBC_ASCQ_INITIALIZING_COMMAND_REQUIRED	= 0x02,
	SBC_ASCQ_OPERATION_IN_PROGRESS		= 0x07
};

enum trans_event {
	EVENT_CBW_VALID,
	EVENT_NEED_STATUS
};

struct sbc_sense_info {
	uint8_t key;
	uint8_t asc;
	uint8_t ascq;
};


#define USBD_MSC_SEC_SIZE	512
struct usb_msc_trans {
	struct usb_msc_cbw cbw;

	uint32_t bytes_to_recv;
	uint32_t bytes_to_send;
	uint32_t byte_count;		/* Either read until equal to bytes_to_recv or
					   write until equal to bytes_to_send. */
	uint32_t lba_start;
	uint32_t block_count;
	uint32_t current_block;

	uint8_t msd_buf[USBD_MSC_SEC_SIZE];

	struct usb_msc_csw csw;
};

struct usbd_msc {
	usbd_device *dev;
	uint8_t ep_in;
	uint8_t ep_in_size;
	uint8_t ep_out;
	uint8_t ep_out_size;
	const usbd_msc_backend *backend;
	struct usb_msc_trans trans;
	struct sbc_sense_info sense;
};

static usbd_msc _mass_storage;

/*-- SCSI Base Responses -----------------------------------------------------*/

static const uint8_t _spc3_inquiry_response[36] = {
	0x00,	/* Byte 0: Peripheral Qualifier = 0, Peripheral Device Type = 0 */
	0x80,	/* Byte 1: RMB = 1, Reserved = 0 */
	0x04,	/* Byte 2: Version = 0 */
	0x02,	/* Byte 3: Obsolete = 0, NormACA = 0, HiSup = 0, Response Data Format = 2 */
	0x20,	/* Byte 4: Additional Length (n-4) = 31 + 4 */
	0x00,	/* Byte 5: SCCS = 0, ACC = 0, TPGS = 0, 3PC = 0, Reserved = 0, Protect = 0 */
	0x00,	/* Byte 6: BQue = 0, EncServ = 0, VS = 0, MultiP = 0, MChngr = 0, Obsolete = 0, Addr16 = 0 */
	0x00,	/* Byte 7: Obsolete = 0, Wbus16 = 0, Sync = 0, Linked = 0, CmdQue = 0, VS = 0 */
		/* Byte 8 - Byte 15: Vendor Identification */
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		/* Byte 16 - Byte 31: Product Identification */
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		/* Byte 32 - Byte 35: Product Revision Level */
	0x20, 0x20, 0x20, 0x20
};

static const uint8_t _spc3_request_sense[18] = {
	0x70,	/* Byte 0: VALID = 0, Response Code = 112 */
	0x00,	/* Byte 1: Obsolete = 0 */
	0x00,	/* Byte 2: Filemark = 0, EOM = 0, ILI = 0, Reserved = 0, Sense Key = 0 */
		/* Byte 3 - Byte 6: Information = 0 */
	0, 0, 0, 0,
	0x0a,	/* Byte 7: Additional Sense Length = 10 */
		/* Byte 8 - Byte 11: Command Specific Info = 0 */
	0, 0, 0, 0,
	0x00,	/* Byte 12: Additional Sense Code (ASC) = 0 */
	0x00,	/* Byte 13: Additional Sense Code Qualifier (ASCQ) = 0 */
	0x00,	/* Byte 14: Field Replaceable Unit Code (FRUC) = 0 */
	0x00,	/* Byte 15: SKSV = 0, SenseKeySpecific[0] = 0 */
	0x00,	/* Byte 16: SenseKeySpecific[0] = 0 */
	0x00	/* Byte 17: SenseKeySpecific[0] = 0 */
};

/*-- SCSI Layer --------------------------------------------------------------*/

static void set_sbc_status(usbd_msc *ms,
				enum sbc_sense_key key,
				enum sbc_asc asc,
				enum sbc_ascq ascq)
{
	ms->sense.key = (uint8_t) key;
	ms->sense.asc = (uint8_t) asc;
	ms->sense.ascq = (uint8_t) ascq;
}

static void set_sbc_status_good(usbd_msc *ms)
{
	set_sbc_status(ms,
				SBC_SENSE_KEY_NO_SENSE,
				SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION,
				SBC_ASCQ_NA);
}

static void scsi_read_6(usbd_msc *ms,
			struct usb_msc_trans *trans,
			enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		uint8_t *buf = trans->cbw.CBWCB;

		trans->lba_start = (buf[2] << 8) | buf[3];
		trans->block_count = buf[4];
		trans->current_block = 0;

		/* TODO: Check the lba & block_count for range. */

		/* both are in terms of 512 byte blocks, so shift by 9 */
		trans->bytes_to_send = trans->block_count << 9;

		set_sbc_status_good(ms);
	}
}

static void scsi_write_6(usbd_msc *ms,
			 struct usb_msc_trans *trans,
			 enum trans_event event)
{
	(void) ms;

	if (EVENT_CBW_VALID == event) {
		uint8_t *buf = trans->cbw.CBWCB;

		trans->lba_start = ((0x1f & buf[1]) << 16) | (buf[2] << 8) | buf[3];
		trans->block_count = buf[4];
		trans->current_block = 0;

		trans->bytes_to_recv = trans->block_count << 9;
	}
}

static void scsi_write_10(usbd_msc *ms,
			  struct usb_msc_trans *trans,
			  enum trans_event event)
{
	(void) ms;

	if (EVENT_CBW_VALID == event) {
		uint8_t *buf = trans->cbw.CBWCB;

		trans->lba_start = (buf[2] << 24) | (buf[3] << 16) |
					(buf[4] << 8) | buf[5];
		trans->block_count = (buf[7] << 8) | buf[8];
		trans->current_block = 0;

		trans->bytes_to_recv = trans->block_count << 9;
	}
}

static void scsi_read_10(usbd_msc *ms,
			 struct usb_msc_trans *trans,
			 enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		uint8_t *buf = trans->cbw.CBWCB;

		trans->lba_start = (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5];
		trans->block_count = (buf[7] << 8) | buf[8];

		/* TODO: Check the lba & block_count for range. */

		/* both are in terms of 512 byte blocks, so shift by 9 */
		trans->bytes_to_send = trans->block_count << 9;

		set_sbc_status_good(ms);
	}
}

static void scsi_read_capacity(usbd_msc *ms,
					struct usb_msc_trans *trans,
					enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		msc_lba_t last_logical_addr = usbd_msc_blocks(ms->backend)-1;
		trans->msd_buf[0] = last_logical_addr >> 24;
		trans->msd_buf[1] = 0xff & (last_logical_addr >> 16);
		trans->msd_buf[2] = 0xff & (last_logical_addr >> 8);
		trans->msd_buf[3] = 0xff & last_logical_addr;

		/* Block size: 512 */
		trans->msd_buf[4] = 0;
		trans->msd_buf[5] = 0;
		trans->msd_buf[6] = 2;
		trans->msd_buf[7] = 0;
		trans->bytes_to_send = 8;
		set_sbc_status_good(ms);
	}
}



static 
void scsi_read_format_capacities(usbd_msc *ms,
 			       struct usb_msc_trans *trans,
 			       enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		usb_msc_rfc_capacity_list_header*  h      = (usb_msc_rfc_capacity_list_header*)trans->msd_buf;
		usb_msc_rfc_capacity_descriptor*   descr  = (usb_msc_rfc_capacity_descriptor*) (trans->msd_buf+sizeof(*h));
		
		u24_assign(h->dummy, 0);
		h->list_len = sizeof(*descr);

		msc_lba_t size = usbd_msc_blocks(ms->backend);
		descr->blocks_count = HTONL(size);
		descr->code 		= rfc_dc_Fomatted;
		u24_assign(descr->block_size, USBD_MSC_SEC_SIZE);

		trans->bytes_to_send = sizeof(*h) + h->list_len;
		set_sbc_status_good(ms);
	}
}

static void fallback_format_unit(usbd_msc *ms, struct usb_msc_trans *trans)
{
	uint32_t i;

	memset(trans->msd_buf, 0, sizeof(trans->msd_buf));

	for (i = 0; i < ms->backend->block_count; i++) {
		if (ms->backend->write_block(ms->backend, i, trans->msd_buf) != 0) {
			/* Error */
		}
	}
}

static void scsi_format_unit(usbd_msc *ms,
					struct usb_msc_trans *trans,
					enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		if (ms->backend->format_unit != NULL) {
			if (ms->backend->format_unit(ms->backend) != 0) {
				/* Error */
			}
		} else {
			fallback_format_unit(ms, trans);
		}

		set_sbc_status_good(ms);
	}
}

static void scsi_request_sense(usbd_msc *ms,
					struct usb_msc_trans *trans,
					enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		uint8_t *buf = trans->cbw.CBWCB;

		trans->bytes_to_send = buf[4];	/* allocation length */
		memcpy(trans->msd_buf, _spc3_request_sense,
			sizeof(_spc3_request_sense));

		trans->msd_buf[2] = ms->sense.key;
		trans->msd_buf[12] = ms->sense.asc;
		trans->msd_buf[13] = ms->sense.ascq;
	}
}

static void scsi_mode_sense_6(usbd_msc *ms,
					struct usb_msc_trans *trans,
					enum trans_event event)
{
	(void) ms;

	if (EVENT_CBW_VALID == event) {
#if 0
		uint8_t *buf = trans->cbw.CBWCB;
		uint8_t page_code = buf[2];
		uint8_t allocation_length = buf[4];

		if (0x1C == page_code) {	/* Informational Exceptions */
#endif
			trans->bytes_to_send = 4;

			trans->msd_buf[0] = 3;	/* Num bytes that follow */
			trans->msd_buf[1] = 0;	/* Medium Type */
			trans->msd_buf[2] = 0;	/* Device specific param */
			trans->csw.dCSWDataResidue = 4;
#if 0
		} else if (0x01 == page_code) {	/* Error recovery */
		} else if (0x3F == page_code) {	/* All */
		} else {
			/* Error */
			trans->csw.bCSWStatus = USB_MSC_CSW_STATUS_FAILED;
			set_sbc_status(ms,
						SBC_SENSE_KEY_ILLEGAL_REQUEST,
						SBC_ASC_INVALID_FIELD_IN_CDB,
						SBC_ASCQ_NA);
		}
#endif
	}
}

static void scsi_inquiry(usbd_msc *ms,
			 struct usb_msc_trans *trans,
			 enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		uint8_t *buf = trans->cbw.CBWCB;
		uint8_t evpd = 1 & buf[1];

		if (0 == evpd) {
			size_t len;
			trans->bytes_to_send = sizeof(_spc3_inquiry_response);
			memcpy(trans->msd_buf, _spc3_inquiry_response, sizeof(_spc3_inquiry_response));

			len = strlen(ms->backend->vendor_id);
			len = MIN(len, 8);
			memcpy(&trans->msd_buf[8], ms->backend->vendor_id, len);

			len = strlen(ms->backend->product_id);
			len = MIN(len, 16);
			memcpy(&trans->msd_buf[16], ms->backend->product_id, len);

			len = strlen(ms->backend->product_rev);
			len = MIN(len, 4);
			memcpy(&trans->msd_buf[32], ms->backend->product_rev, len);

			trans->csw.dCSWDataResidue = sizeof(_spc3_inquiry_response);

			set_sbc_status_good(ms);
		} else {
			/* TODO: Add VPD 0x83 support */
			/* TODO: Add VPD 0x00 support */
		}
	}
}

static void scsi_command(usbd_msc *ms,
			 struct usb_msc_trans *trans,
			 enum trans_event event)
{
	if (EVENT_CBW_VALID == event) {
		/* Setup the default success */
		trans->csw.dCSWSignature = USB_MSC_CSW_SIGNATURE;
		trans->csw.dCSWTag = trans->cbw.dCBWTag;
		trans->csw.dCSWDataResidue = 0;
		trans->csw.bCSWStatus = USB_MSC_CSW_STATUS_SUCCESS;

		trans->bytes_to_send = 0;
		trans->bytes_to_recv = 0;
		trans->byte_count = 0;
	}

	USBD_LOGF_LN(USB_VIO_MSC, "SCSI:cmd %x", trans->cbw.CBWCB[0]);
	switch (trans->cbw.CBWCB[0]) {
	case USB_MSC_SCSI_TEST_UNIT_READY:
	case USB_MSC_SCSI_SEND_DIAGNOSTIC:
		/* Do nothing, just send the success. */
		set_sbc_status_good(ms);
		break;
	case USB_MSC_SCSI_FORMAT_UNIT:
		scsi_format_unit(ms, trans, event);
		break;
	case USB_MSC_SCSI_REQUEST_SENSE:
		scsi_request_sense(ms, trans, event);
		break;
	case USB_MSC_SCSI_MODE_SENSE_6:
		scsi_mode_sense_6(ms, trans, event);
		break;
	case USB_MSC_SCSI_READ_6:
		scsi_read_6(ms, trans, event);
		break;
	case USB_MSC_SCSI_INQUIRY:
		scsi_inquiry(ms, trans, event);
		break;
	case USB_MSC_SCSI_READ_CAPACITY:
		scsi_read_capacity(ms, trans, event);
		break;
	case USB_MSC_SCSI_READ_10:
		scsi_read_10(ms, trans, event);
		break;
	case USB_MSC_SCSI_WRITE_6:
		scsi_write_6(ms, trans, event);
		break;
	case USB_MSC_SCSI_WRITE_10:
		scsi_write_10(ms, trans, event);
		break;
	case USB_MSC_SCSI_READ_FORMAT_CAPACITIES:
		scsi_read_format_capacities(ms, trans, event);
	 	break;
	default:
		USBD_LOGF_LN(USB_VIO_MSC, "SCSI:cmd %x uncknown", trans->cbw.CBWCB[0]);
		set_sbc_status(ms, SBC_SENSE_KEY_ILLEGAL_REQUEST,
					SBC_ASC_INVALID_COMMAND_OPERATION_CODE,
					SBC_ASCQ_NA);

		trans->bytes_to_send = 0;
		trans->bytes_to_recv = 0;
		trans->csw.bCSWStatus = USB_MSC_CSW_STATUS_FAILED;
		break;
	}
}

/*-- USB Mass Storage Layer --------------------------------------------------*/

static inline void lock(usbd_msc *ms)
{
	if (ms->backend->lock != NULL) {
		if (ms->backend->lock() != 0) {
			/* Error */
		}
	}
}

static inline void unlock(usbd_msc *ms)
{
	if (ms->backend->unlock != NULL) {
		if (ms->backend->unlock() != 0) {
			/* Error */
		}
	}
}

/*
 * set_config(): SET_CONFIGURATION callback
 * cbw_recv_from_host(): CBW transfer submit
 * cbw_recv_from_host_callback(): CBW transfer submit callback
 * buf_send_to_host(): Send buffer to host
 * buf_send_to_host_callback(): Send buffer to host callback
 * buf_recv_from_host(): Receive buffer from host
 * buf_recv_from_host_callback(): Receive buffer from host callback
 * csw_send_to_host(): CSW send to host
 * csw_send_to_host_callback(): CSW send to host callback
 *
 * set_config() -> cbw_recv_from_host()
 *
 * cbw_recv_from_host_callback()
 *          \-> buf_send_to_host()
 *          |-> buf_recv_from_host()
 *          |-> csw_send_to_host()
 *
 * buf_send_to_host_callback()
 *          \-> buf_send_to_host()
 *          |-> csw_send_to_host()
 *
 * buf_recv_from_host_callback()
 *          \-> buf_recv_from_host()
 *          |-> csw_send_to_host()
 *
 * csw_send_to_host_callback() -> cbw_recv_from_host()
 */

/**
 * Try to resubmit the transfer if it is possible
 */
static inline void try_resubmit(usbd_device *dev, const usbd_transfer *transfer,
			usbd_transfer_status status)
{
	switch (status) {
	case USBD_ERR_TIMEOUT:
	case USBD_ERR_IO:
	case USBD_ERR_BABBLE:
	case USBD_ERR_DTOG:
	case USBD_ERR_SHORT_PACKET:
	case USBD_ERR_OVERFLOW:
		/* Resubmit */
		usbd_transfer_submit(dev, transfer);
	break;

	case USBD_ERR_RES_UNAVAIL:
	case USBD_ERR_CANCEL:
	case USBD_SUCCESS:
	case USBD_ERR_SIZE:
	case USBD_ERR_CONN:
	case USBD_ERR_INVALID:
	case USBD_ERR_CONFIG_CHANGE:
	default:
	break;
	}
}

static void reset_trans(struct usb_msc_trans *trans)
{
	trans->lba_start = ~0;
	trans->block_count = 0;
	trans->current_block = 0;
	trans->bytes_to_recv = 0;
	trans->bytes_to_send = 0;
	trans->byte_count = 0;
}

static void cbw_recv_from_host(usbd_msc *ms,
								struct usb_msc_trans *trans);

/**
 * Callback for CSW transfer.
 * @sa csw_send_to_host()
 */
static void csw_send_to_host_callback(usbd_device *dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		try_resubmit(dev, transfer, status);
		return;
	}

	usbd_msc *ms = transfer->user_data;
	struct usb_msc_trans *trans = &ms->trans;

	reset_trans(trans); /* End of transaction */
	cbw_recv_from_host(ms, trans); /* Restart! */
}

/**
 * Send CSW to host
 * @sa csw_send_to_host_callback()
 */
static void csw_send_to_host(usbd_msc *ms,
								struct usb_msc_trans *trans)
{
	scsi_command(ms, trans, EVENT_NEED_STATUS);

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = ms->ep_in,
		.ep_size = ms->ep_in_size,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = &trans->csw,
		.length = sizeof(trans->csw),
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = csw_send_to_host_callback,
		.user_data = ms
	};

	usbd_transfer_submit(ms->dev, &transfer);
}

static void buf_recv_from_host(usbd_msc *ms,
								struct usb_msc_trans *trans);

static void buf_recv_from_host_callback(usbd_device *dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		try_resubmit(dev, transfer, status);
		return;
	}

	usbd_msc *ms = transfer->user_data;
	struct usb_msc_trans *trans = &ms->trans;

	if (trans->block_count) {
		uint32_t lba = trans->lba_start + trans->current_block;
		if (ms->backend->write_block(ms->backend, lba, trans->msd_buf) != 0) {
			/* Error */
		}
		trans->current_block++;
	}

	trans->byte_count += transfer->transferred;

	if (trans->byte_count < trans->bytes_to_recv) {
		buf_recv_from_host(ms, trans);
		return;
	}

	if (trans->block_count) {
		if (trans->current_block == trans->block_count) {
			trans->current_block = 0;
		}

		unlock(ms);
	}

	csw_send_to_host(ms, trans);
}

/**
 * Receive @a trans buffer content from host
 */
static void buf_recv_from_host(usbd_msc *ms,
									struct usb_msc_trans *trans)
{
	uint32_t rem = trans->bytes_to_recv - trans->byte_count;

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = ms->ep_out,
		.ep_size = ms->ep_out_size,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = trans->msd_buf,
		.length = MIN(rem, sizeof(trans->msd_buf)),
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = buf_recv_from_host_callback,
		.user_data = ms
	};

	usbd_transfer_submit(ms->dev, &transfer);
}

static void buf_send_to_host(usbd_msc *ms,
								struct usb_msc_trans *trans);

static void buf_send_to_host_callback(usbd_device *dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		try_resubmit(dev, transfer, status);
		return;
	}

	usbd_msc *ms = transfer->user_data;
	struct usb_msc_trans *trans = &ms->trans;

	trans->byte_count += transfer->transferred;

	if (trans->byte_count < trans->bytes_to_send) {
		buf_send_to_host(ms, trans); /* Send more */
		return;
	}

	if (trans->block_count) {
		if (trans->current_block == trans->block_count) {
			trans->current_block = 0;
		}
	}

	csw_send_to_host(ms, trans);
}

/**
 * Send @a trans buffer content to host
 */
static void buf_send_to_host(usbd_msc *ms,
							struct usb_msc_trans *trans)
{
	if (trans->block_count) {
		uint32_t lba = trans->lba_start + trans->current_block;
		if (ms->backend->read_block(ms->backend, lba, trans->msd_buf) != 0) {
			/* Error */
		}
		trans->current_block++;
	}

	uint32_t rem = trans->bytes_to_send - trans->byte_count;

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = ms->ep_in,
		.ep_size = ms->ep_in_size,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = trans->msd_buf,
		.length = MIN(rem, sizeof(trans->msd_buf)),
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = buf_send_to_host_callback,
		.user_data = ms
	};

	usbd_transfer_submit(ms->dev, &transfer);
}

/**
 * CBW read from host callback
 * @sa cbw_recv_from_host()
 */
static void cbw_recv_from_host_callback(usbd_device *dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		try_resubmit(dev, transfer, status);
		return;
	}

	usbd_msc *ms = transfer->user_data;
	struct usb_msc_trans *trans = &ms->trans;

	scsi_command(ms, trans, EVENT_CBW_VALID);

	if (trans->block_count) {
		lock(ms);
	}

	if (trans->bytes_to_recv) {
		buf_recv_from_host(ms, trans);
	} else if (trans->bytes_to_send) {
		buf_send_to_host(ms, trans);
	} else {
		csw_send_to_host(ms, trans);
	}
}

/**
 * CBW read from host
 * @sa cbw_recv_from_host_callback()
 */
static void cbw_recv_from_host(usbd_msc *ms,
							struct usb_msc_trans *trans)
{
	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = ms->ep_out,
		.ep_size = ms->ep_out_size,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = &trans->cbw,
		.length = sizeof(trans->cbw),
		.flags = USBD_FLAG_SHORT_PACKET,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = cbw_recv_from_host_callback,
		.user_data = ms
	};

	usbd_transfer_submit(ms->dev, &transfer);
}

/** @brief Handle various control requests related to the msc storage
 *	   interface.
 * @return true on handled
 * @return false when ignored
 */
bool usbd_msc_setup_ep0(usbd_msc *ms,
					const struct usb_setup_data *setup_data)
{
	usbd_device *dev = ms->dev;

	const uint8_t mask = USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT;
	const uint8_t value = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE;

	if ((setup_data->bmRequestType & mask) == value) {
		USBD_LOGF(USB_VSETUP, "USB:MSC:Req %x ", (int)setup_data->bRequest);
		switch (setup_data->bRequest) {
		case USB_MSC_REQ_BULK_ONLY_RESET:
			USBD_LOG(USB_VSETUP,"BULK_ONLY_RESET\n");
			/* Do any special reset code here. */
			usbd_ep0_transfer(dev, setup_data, NULL, 0, NULL);
		return true;
		case USB_MSC_REQ_GET_MAX_LUN: {
			USBD_LOG(USB_VSETUP,"MAX_LUN 0\n");
			/* Return the number of LUNs.  We use 0. */
			static const uint8_t res = 0;
			usbd_ep0_transfer(dev, setup_data, (void *) &res,
							sizeof(res), NULL);
		return true;
		}
		}//switch (setup_data->bRequest)
 	}

	return false;
}

/**
 * Start the MSC and accept command from host
 * @param ms Mass Storage
 * @note Before calling this function, application should prepare the endpoint.
 */
void usbd_msc_start(usbd_msc *ms)
{
	cbw_recv_from_host(ms, &ms->trans);
}

/** @addtogroup usb_msc */
/** @{ */

/**
 * @brief Initializes the USB Mass Storage subsystem.
 *
 * @note Currently you can only have this profile active.
 *
 * @param[in] dev The USB device to associate the Mass Storage with.
 * @param[in] ep_in The USB 'IN' endpoint.
 * @param[in] ep_in_size The maximum endpoint size.  Valid values: 8, 16, 32 or 64
 * @param[in] ep_out The USB 'OUT' endpoint.
 * @param[in] ep_out_size The maximum endpoint size.  Valid values: 8, 16, 32 or 64
 * @param[in] backend Backend (Cannot be NULL)
 * @return Pointer to the usbd_msc struct.
 * @note @a backend should be valid till the returned object is valid
*/
usbd_msc *usbd_msc_init(usbd_device *dev,
				uint8_t ep_in, uint8_t ep_in_size,
				uint8_t ep_out, uint8_t ep_out_size,
				const usbd_msc_backend *backend)
{
	usbd_msc *ms = &_mass_storage;

	ms->dev = dev;
	ms->ep_in = ep_in;
	ms->ep_in_size = ep_in_size;
	ms->ep_out = ep_out;
	ms->ep_out_size = ep_out_size;
	ms->backend = backend;

	reset_trans(&ms->trans);

	set_sbc_status_good(ms);

	return ms;
}

/** @} */
