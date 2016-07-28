/** @defgroup usb_type_defines USB Standard Structure Definitions

@brief <b>Defined Constants and Types for the USB Standard Structure
Definitions</b>

@ingroup USB_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2010
Gareth McMullin <gareth@blacksphere.co.nz>

@date 10 March 2013

A set of structure definitions for the USB control structures
defined in chapter 9 of the "Universal Serial Bus Specification Revision 2.0"
Available from the USB Implementers Forum - http://www.usb.org/

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

#ifndef UNICOREMX_USB_USBSTD_H
#define UNICOREMX_USB_USBSTD_H

#include <stdint.h>
#include <unicore-mx/cm3/common.h>

/*
 * This file contains structure definitions for the USB control structures
 * defined in chapter 9 of the "Universal Serial Bus Specification Revision 2.0"
 * Available from the USB Implementers Forum - http://www.usb.org/
 */

/* USB Setup Data structure - Table 9-2 */
struct usb_setup_data {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__((packed));

/* Class Definition */
#define USB_CLASS_VENDOR			0xFF

/* bmRequestType bit definitions */
/* bit 7 : direction */
#define USB_REQ_TYPE_DIRECTION			0x80
#define USB_REQ_TYPE_OUT			0x00
#define USB_REQ_TYPE_IN				0x80
/* bits 6..5 : type */
#define USB_REQ_TYPE_TYPE			0x60
#define USB_REQ_TYPE_STANDARD			0x00
#define USB_REQ_TYPE_CLASS			0x20
#define USB_REQ_TYPE_VENDOR			0x40
/* bits 4..0 : recipient */
#define USB_REQ_TYPE_RECIPIENT			0x1F
#define USB_REQ_TYPE_DEVICE			0x00
#define USB_REQ_TYPE_INTERFACE			0x01
#define USB_REQ_TYPE_ENDPOINT			0x02
#define USB_REQ_TYPE_OTHER			0x03

/* USB Standard Request Codes - Table 9-4 */
#define USB_REQ_GET_STATUS			0
#define USB_REQ_CLEAR_FEATURE			1
/* Reserved for future use: 2 */
#define USB_REQ_SET_FEATURE			3
/* Reserved for future use: 3 */
#define USB_REQ_SET_ADDRESS			5
#define USB_REQ_GET_DESCRIPTOR			6
#define USB_REQ_SET_DESCRIPTOR			7
#define USB_REQ_GET_CONFIGURATION		8
#define USB_REQ_SET_CONFIGURATION		9
#define USB_REQ_GET_INTERFACE			10
#define USB_REQ_SET_INTERFACE			11
#define USB_REQ_SET_SYNCH_FRAME			12

/* USB Descriptor Types - Table 9-5 */
#define USB_DT_DEVICE				1
#define USB_DT_CONFIGURATION			2
#define USB_DT_STRING				3
#define USB_DT_INTERFACE			4
#define USB_DT_ENDPOINT				5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_DT_OTHER_SPEED_CONFIGURATION	7
#define USB_DT_INTERFACE_POWER			8
/* From ECNs */
#define USB_DT_OTG				9
#define USB_DT_DEBUG				10
#define USB_DT_INTERFACE_ASSOCIATION		11

/* USB Standard Feature Selectors - Table 9-6 */
#define USB_FEAT_ENDPOINT_HALT			0
#define USB_FEAT_DEVICE_REMOTE_WAKEUP		1
#define USB_FEAT_TEST_MODE			2

/* Information Returned by a GetStatus() Request to a Device - Figure 9-4 */
#define USB_DEV_STATUS_SELF_POWERED		0x01
#define USB_DEV_STATUS_REMOTE_WAKEUP		0x02

/* USB Standard Device Descriptor - Table 9-8 */
struct usb_device_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;

	const struct usb_config_descriptor *config;
} __attribute__((packed));

#define USB_DT_DEVICE_SIZE 18

/* USB Device_Qualifier Descriptor - Table 9-9
 * Not used in this implementation.
 */
struct usb_device_qualifier_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint8_t bNumConfigurations;
	uint8_t bReserved;
} __attribute__((packed));

/* USB Standard Configuration Descriptor - Table 9-10 */
struct usb_config_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;

	/* Descriptor ends here.  The following are used internally: */
	const struct usb_interface {
		uint8_t *cur_altsetting;
		uint8_t num_altsetting;
		const struct usb_iface_assoc_descriptor *iface_assoc;
		const struct usb_interface_descriptor *altsetting;
	} *interface;
} __attribute__((packed));
#define USB_DT_CONFIGURATION_SIZE		9

/* USB Configuration Descriptor bmAttributes bit definitions */
#define USB_CONFIG_ATTR_DEFAULT			0x80	/** always required (USB2.0 table 9-10) */
#define USB_CONFIG_ATTR_SELF_POWERED		0x40
#define USB_CONFIG_ATTR_REMOTE_WAKEUP		0x20

/* Other Speed Configuration is the same as Configuration Descriptor.
 *  - Table 9-11
 */

/* USB Standard Interface Descriptor - Table 9-12 */
struct usb_interface_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;

	/* Descriptor ends here.  The following are used internally: */
	const struct usb_endpoint_descriptor *endpoint;
	const void *extra;
	size_t extra_len;
} __attribute__((packed));
#define USB_DT_INTERFACE_SIZE			9

/* USB Standard Endpoint Descriptor - Table 9-13 */
struct usb_endpoint_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;

	/* Descriptor ends here.  The following are used internally: */
	const void *extra;
	size_t extra_len;
} __attribute__((packed));
#define USB_DT_ENDPOINT_SIZE		7

/* USB bEndpointAddress helper macros */
#define USB_ENDPOINT_ADDR_OUT(x) (x)
#define USB_ENDPOINT_ADDR_IN(x) (0x80 | (x))

/* USB Endpoint Descriptor bmAttributes bit definitions - Table 9-13 */
/* bits 1..0 : transfer type */
#define USB_ENDPOINT_ATTR_CONTROL		0x00
#define USB_ENDPOINT_ATTR_ISOCHRONOUS		0x01
#define USB_ENDPOINT_ATTR_BULK			0x02
#define USB_ENDPOINT_ATTR_INTERRUPT		0x03
#define USB_ENDPOINT_ATTR_TYPE		0x03
/* bits 3..2 : Sync type (only if ISOCHRONOUS) */
#define USB_ENDPOINT_ATTR_NOSYNC		0x00
#define USB_ENDPOINT_ATTR_ASYNC			0x04
#define USB_ENDPOINT_ATTR_ADAPTIVE		0x08
#define USB_ENDPOINT_ATTR_SYNC			0x0C
#define USB_ENDPOINT_ATTR_SYNCTYPE		0x0C
/* bits 5..4 : usage type (only if ISOCHRONOUS) */
#define USB_ENDPOINT_ATTR_DATA			0x00
#define USB_ENDPOINT_ATTR_FEEDBACK		0x10
#define USB_ENDPOINT_ATTR_IMPLICIT_FEEDBACK_DATA 0x20
#define USB_ENDPOINT_ATTR_USAGETYPE		0x30

/* Table 9-15 specifies String Descriptor Zero.
 * Table 9-16 specified UNICODE String Descriptor.
 */
struct usb_string_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wData[];
} __attribute__((packed));

/* From ECN: Interface Association Descriptors, Table 9-Z */
struct usb_iface_assoc_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bFirstInterface;
	uint8_t bInterfaceCount;
	uint8_t bFunctionClass;
	uint8_t bFunctionSubClass;
	uint8_t bFunctionProtocol;
	uint8_t iFunction;
} __attribute__((packed));
#define USB_DT_INTERFACE_ASSOCIATION_SIZE 8

/**
 * As per the specification:
 * 16bit Identifier:
 *  bit[9:0] - Language
 *  bit[15:10] - Sub Language
 *
 * @param lang use USB_LANG_*
 * @param usblang use USB_SUBLANG_*
 */
#define USB_LANGID(lang, sublang) ((lang & 0x3FF) | ((sublang & 0x3F) << 10))

/* Language */
#define USB_LANG_ARABIC 0x01
#define USB_LANG_BULGARIAN 0x02
#define USB_LANG_CATALAN 0x03
#define USB_LANG_CHINESE 0x04
#define USB_LANG_CZECH 0x05
#define USB_LANG_DANISH 0x06
#define USB_LANG_GERMAN 0x07
#define USB_LANG_GREEK 0x08
#define USB_LANG_ENGLISH 0x09
#define USB_LANG_SPANISH 0x0a
#define USB_LANG_FINNISH 0x0b
#define USB_LANG_FRENCH 0x0c
#define USB_LANG_HEBREW 0x0d
#define USB_LANG_HUNGARIAN 0x0e
#define USB_LANG_ICELANDIC 0x0f
#define USB_LANG_ITALIAN 0x10
#define USB_LANG_JAPANESE 0x11
#define USB_LANG_KOREAN 0x12
#define USB_LANG_DUTCH 0x13
#define USB_LANG_NORWEGIAN 0x14
#define USB_LANG_POLISH 0x15
#define USB_LANG_PORTUGUESE 0x16
#define USB_LANG_ROMANIAN 0x18
#define USB_LANG_RUSSIAN 0x19
#define USB_LANG_CROATIAN 0x1a
#define USB_LANG_SERBIAN 0x1a
#define USB_LANG_SLOVAK 0x1b
#define USB_LANG_ALBANIAN 0x1c
#define USB_LANG_SWEDISH 0x1d
#define USB_LANG_THAI 0x1e
#define USB_LANG_TURKISH 0x1f
#define USB_LANG_URDU 0x20
#define USB_LANG_INDONESIAN 0x21
#define USB_LANG_UKRANIAN 0x22
#define USB_LANG_BELARUSIAN 0x23
#define USB_LANG_SLOVENIAN 0x24
#define USB_LANG_ESTONIAN 0x25
#define USB_LANG_LATVIAN 0x26
#define USB_LANG_LITHUANIAN 0x27
#define USB_LANG_FARSI 0x29
#define USB_LANG_VIETNAMESE 0x2a
#define USB_LANG_ARMENIAN 0x2b
#define USB_LANG_AZERI 0x2c
#define USB_LANG_BASQUE 0x2d
#define USB_LANG_MACEDONIAN 0x2f
#define USB_LANG_AFRIKAANS 0x36
#define USB_LANG_GEORGIAN 0x37
#define USB_LANG_FAEROESE 0x38
#define USB_LANG_HINDI 0x39
#define USB_LANG_MALAY 0x3e
#define USB_LANG_KAZAK 0x3f
#define USB_LANG_SWAHILI 0x41
#define USB_LANG_UZBEK 0x43
#define USB_LANG_TATAR 0x44
#define USB_LANG_BENGALI 0x45
#define USB_LANG_PUNJABI 0x46
#define USB_LANG_GUJARATI 0x47
#define USB_LANG_ORIYA 0x48
#define USB_LANG_TAMIL 0x49
#define USB_LANG_TELUGU 0x4a
#define USB_LANG_KANNADA 0x4b
#define USB_LANG_MALAYALAM 0x4c
#define USB_LANG_ASSAMESE 0x4d
#define USB_LANG_MARATHI 0x4e
#define USB_LANG_SANSKRIT 0x4f
#define USB_LANG_KONKANI 0x57
#define USB_LANG_MANIPURI 0x58
#define USB_LANG_SINDHI 0x59
#define USB_LANG_KASHMIRI 0x60
#define USB_LANG_NEPALI 0x61

/* Sub Language */
#define USB_SUBLANG_ARABIC_SAUDI_ARABIA 0x01
#define USB_SUBLANG_ARABIC_IRAQ 0x02
#define USB_SUBLANG_ARABIC_EGYPT 0x03
#define USB_SUBLANG_ARABIC_LIBYA 0x04
#define USB_SUBLANG_ARABIC_ALGERIA 0x05
#define USB_SUBLANG_ARABIC_MOROCCO 0x06
#define USB_SUBLANG_ARABIC_TUNISIA 0x07
#define USB_SUBLANG_ARABIC_OMAN 0x08
#define USB_SUBLANG_ARABIC_YEMEN 0x09
#define USB_SUBLANG_ARABIC_SYRIA 0x10
#define USB_SUBLANG_ARABIC_JORDAN 0x11
#define USB_SUBLANG_ARABIC_LEBANON 0x12
#define USB_SUBLANG_ARABIC_KUWAIT 0x13
#define USB_SUBLANG_ARABIC_UAE 0x14
#define USB_SUBLANG_ARABIC_BAHRAIN 0x15
#define USB_SUBLANG_ARABIC_QATAR 0x16
#define USB_SUBLANG_AZERI_CYRILLIC 0x01
#define USB_SUBLANG_AZERI_LATIN 0x02
#define USB_SUBLANG_CHINESE_TRADITIONAL 0x01
#define USB_SUBLANG_CHINESE_SIMPLIFIED 0x02
#define USB_SUBLANG_CHINESE_HONGKONG 0x03
#define USB_SUBLANG_CHINESE_SINGAPORE 0x04
#define USB_SUBLANG_CHINESE_MACAU 0x05
#define USB_SUBLANG_DUTCH 0x01
#define USB_SUBLANG_DUTCH_BELGIAN 0x02
#define USB_SUBLANG_ENGLISH_US 0x01
#define USB_SUBLANG_ENGLISH_UK 0x02
#define USB_SUBLANG_ENGLISH_AUS 0x03
#define USB_SUBLANG_ENGLISH_CAN 0x04
#define USB_SUBLANG_ENGLISH_NZ 0x05
#define USB_SUBLANG_ENGLISH_EIRE 0x06
#define USB_SUBLANG_ENGLISH_SOUTH_AFRICA 0x07
#define USB_SUBLANG_ENGLISH_JAMAICA 0x08
#define USB_SUBLANG_ENGLISH_CARIBBEAN 0x09
#define USB_SUBLANG_ENGLISH_BELIZE 0x0a
#define USB_SUBLANG_ENGLISH_TRINIDAD 0x0b
#define USB_SUBLANG_ENGLISH_PHILIPPINES 0x0c
#define USB_SUBLANG_ENGLISH_ZIMBABWE 0x0d
#define USB_SUBLANG_FRENCH 0x01
#define USB_SUBLANG_FRENCH_BELGIAN 0x02
#define USB_SUBLANG_FRENCH_CANADIAN 0x03
#define USB_SUBLANG_FRENCH_SWISS 0x04
#define USB_SUBLANG_FRENCH_LUXEMBOURG 0x05
#define USB_SUBLANG_FRENCH_MONACO 0x06
#define USB_SUBLANG_GERMAN 0x01
#define USB_SUBLANG_GERMAN_SWISS 0x02
#define USB_SUBLANG_GERMAN_AUSTRIAN 0x03
#define USB_SUBLANG_GERMAN_LUXEMBOURG 0x04
#define USB_SUBLANG_GERMAN_LIECHTENSTEIN 0x05
#define USB_SUBLANG_ITALIAN 0x01
#define USB_SUBLANG_ITALIAN_SWISS 0x02
#define USB_SUBLANG_KASHMIRI_INDIA 0x02
#define USB_SUBLANG_KOREAN 0x01
#define USB_SUBLANG_LITHUANIAN 0x01
#define USB_SUBLANG_MALAY_MALAYSIA 0x01
#define USB_SUBLANG_MALAY_BRUNEI_DARUSSALAM 0x02
#define USB_SUBLANG_NEPALI_INDIA 0x02
#define USB_SUBLANG_NORWEGIAN_BOKMAL 0x01
#define USB_SUBLANG_NORWEGIAN_NYNORSK 0x02
#define USB_SUBLANG_PORTUGUESE 0x01
#define USB_SUBLANG_PORTUGUESE_BRAZILIAN 0x02
#define USB_SUBLANG_SERBIAN_LATIN 0x02
#define USB_SUBLANG_SERBIAN_CYRILLIC 0x03
#define USB_SUBLANG_SPANISH 0x01
#define USB_SUBLANG_SPANISH_MEXICAN 0x02
#define USB_SUBLANG_SPANISH_MODERN 0x03
#define USB_SUBLANG_SPANISH_GUATEMALA 0x04
#define USB_SUBLANG_SPANISH_COSTA_RICA 0x05
#define USB_SUBLANG_SPANISH_PANAMA 0x06
#define USB_SUBLANG_SPANISH_DOMINICAN_REPUBLIC 0x07
#define USB_SUBLANG_SPANISH_VENEZUELA 0x08
#define USB_SUBLANG_SPANISH_COLOMBIA 0x09
#define USB_SUBLANG_SPANISH_PERU 0x0a
#define USB_SUBLANG_SPANISH_ARGENTINA 0x0b
#define USB_SUBLANG_SPANISH_ECUADOR 0x0c
#define USB_SUBLANG_SPANISH_CHILE 0x0d
#define USB_SUBLANG_SPANISH_URUGUAY 0x0e
#define USB_SUBLANG_SPANISH_PARAGUAY 0x0f
#define USB_SUBLANG_SPANISH_BOLIVIA 0x10
#define USB_SUBLANG_SPANISH_EL_SALVADOR 0x11
#define USB_SUBLANG_SPANISH_HONDURAS 0x12
#define USB_SUBLANG_SPANISH_NICARAGUA 0x13
#define USB_SUBLANG_SPANISH_PUERTO_RICO 0x14
#define USB_SUBLANG_SWEDISH 0x01
#define USB_SUBLANG_SWEDISH_FINLAND 0x02
#define USB_SUBLANG_URDU_PAKISTAN 0x01
#define USB_SUBLANG_URDU_INDIA 0x02
#define USB_SUBLANG_UZBEK_LATIN 0x01
#define USB_SUBLANG_UZBEK_CYRILLIC 0x02

/* Language Identifier */
#define USB_LANGID_AFRIKAANS 0x0436
#define USB_LANGID_ALBANIAN 0x041C
#define USB_LANGID_ARABIC_SAUDI_ARABIA 0x0401
#define USB_LANGID_ARABIC_IRAQ 0x0801
#define USB_LANGID_ARABIC_EGYPT 0x0C01
#define USB_LANGID_ARABIC_LIBYA 0x1001
#define USB_LANGID_ARABIC_ALGERIA 0x1401
#define USB_LANGID_ARABIC_MOROCCO 0x1801
#define USB_LANGID_ARABIC_TUNISIA 0x1C01
#define USB_LANGID_ARABIC_OMAN 0x2001
#define USB_LANGID_ARABIC_YEMEN 0x2401
#define USB_LANGID_ARABIC_SYRIA 0x2801
#define USB_LANGID_ARABIC_JORDAN 0x2C01
#define USB_LANGID_ARABIC_LEBANON 0x3001
#define USB_LANGID_ARABIC_KUWAIT 0x3401
#define USB_LANGID_ARABIC_UAE 0x3801
#define USB_LANGID_ARABIC_BAHRAIN 0x3C01
#define USB_LANGID_ARABIC_QATAR 0x4001
#define USB_LANGID_ARMENIAN 0x042B
#define USB_LANGID_ASSAMESE 0x044D
#define USB_LANGID_AZERI_LATIN 0x042C
#define USB_LANGID_AZERI_CYRILLIC 0x082C
#define USB_LANGID_BASQUE 0x042D
#define USB_LANGID_BELARUSSIAN 0x0423
#define USB_LANGID_BENGALI 0x0445
#define USB_LANGID_BULGARIAN 0x0402
#define USB_LANGID_BURMESE 0x0455
#define USB_LANGID_CATALAN 0x0403
#define USB_LANGID_CHINESE_TAIWAN 0x0404
#define USB_LANGID_CHINESE_PRC 0x0804
#define USB_LANGID_CHINESE_HONG_KONG_SAR 0x0C04
#define USB_LANGID_CHINESE_HONG_KONG_PRC 0x0C04
#define USB_LANGID_CHINESE_SINGAPORE 0x1004
#define USB_LANGID_CHINESE_MACAU_SAR 0x1404
#define USB_LANGID_CROATIAN 0x041A
#define USB_LANGID_CZECH 0x0405
#define USB_LANGID_DANISH 0x0406
#define USB_LANGID_DUTCH_NETHERLANDS 0x0413
#define USB_LANGID_DUTCH_BELGIUM 0x0813
#define USB_LANGID_ENGLISH_UNITED_STATES 0x0409
#define USB_LANGID_ENGLISH_UNITED_KINGDOM 0x0809
#define USB_LANGID_ENGLISH_AUSTRALIAN 0x0C09
#define USB_LANGID_ENGLISH_CANADIAN 0x1009
#define USB_LANGID_ENGLISH_NEW_ZEALAND 0x1409
#define USB_LANGID_ENGLISH_IRELAND 0x1809
#define USB_LANGID_ENGLISH_SOUTH_AFRICA 0x1C09
#define USB_LANGID_ENGLISH_JAMAICA 0x2009
#define USB_LANGID_ENGLISH_CARIBBEAN 0x2409
#define USB_LANGID_ENGLISH_BELIZE 0x2809
#define USB_LANGID_ENGLISH_TRINIDAD 0x2C09
#define USB_LANGID_ENGLISH_ZIMBABWE 0x3009
#define USB_LANGID_ENGLISH_PHILIPPINES 0x3409
#define USB_LANGID_ESTONIAN 0x0425
#define USB_LANGID_FAEROESE 0x0438
#define USB_LANGID_FARSI 0x0429
#define USB_LANGID_FINNISH 0x040B
#define USB_LANGID_FRENCH_STANDARD 0x040C
#define USB_LANGID_FRENCH_BELGIAN 0x080C
#define USB_LANGID_FRENCH_CANADIAN 0x0C0C
#define USB_LANGID_FRENCH_SWITZERLAND 0x100C
#define USB_LANGID_FRENCH_LUXEMBOURG 0x140C
#define USB_LANGID_FRENCH_MONACO 0x180C
#define USB_LANGID_GEORGIAN 0x0437
#define USB_LANGID_GERMAN_STANDARD 0x0407
#define USB_LANGID_GERMAN_SWITZERLAND 0x0807
#define USB_LANGID_GERMAN_AUSTRIA 0x0C07
#define USB_LANGID_GERMAN_LUXEMBOURG 0x1007
#define USB_LANGID_GERMAN_LIECHTENSTEIN 0x1407
#define USB_LANGID_GREEK 0x0408
#define USB_LANGID_GUJARATI 0x0447
#define USB_LANGID_HEBREW 0x040D
#define USB_LANGID_HINDI 0x0439
#define USB_LANGID_HUNGARIAN 0x040E
#define USB_LANGID_ICELANDIC 0x040F
#define USB_LANGID_INDONESIAN 0x0421
#define USB_LANGID_ITALIAN_STANDARD 0x0410
#define USB_LANGID_ITALIAN_SWITZERLAND 0x0810
#define USB_LANGID_JAPANESE 0x0411
#define USB_LANGID_KANNADA 0x044B
#define USB_LANGID_KASHMIRI_INDIA 0x0860
#define USB_LANGID_KAZAKH 0x043F
#define USB_LANGID_KONKANI 0x0457
#define USB_LANGID_KOREAN 0x0412
#define USB_LANGID_KOREAN_JOHAB 0x0812
#define USB_LANGID_LATVIAN 0x0426
#define USB_LANGID_LITHUANIAN 0x0427
#define USB_LANGID_LITHUANIAN_CLASSIC 0x0827
#define USB_LANGID_MACEDONIAN 0x042F
#define USB_LANGID_MALAY_MALAYSIAN 0x043E
#define USB_LANGID_MALAY_BRUNEI_DARUSSALAM 0x083E
#define USB_LANGID_MALAYALAM 0x044C
#define USB_LANGID_MANIPURI 0x0458
#define USB_LANGID_MARATHI 0x044E
#define USB_LANGID_NEPALI_INDIA 0x0861
#define USB_LANGID_NORWEGIAN_BOKMAL 0x0414
#define USB_LANGID_NORWEGIAN_NYNORSK 0x0814
#define USB_LANGID_ORIYA 0x0448
#define USB_LANGID_POLISH 0x0415
#define USB_LANGID_PORTUGUESE_BRAZIL 0x0416
#define USB_LANGID_PORTUGUESE_STANDARD 0x0816
#define USB_LANGID_PUNJABI 0x0446
#define USB_LANGID_ROMANIAN 0x0418
#define USB_LANGID_RUSSIAN 0x0419
#define USB_LANGID_SANSKRIT 0x044F
#define USB_LANGID_SERBIAN_CYRILLIC 0x0C1A
#define USB_LANGID_SERBIAN_LATIN 0x081A
#define USB_LANGID_SINDHI 0x0459
#define USB_LANGID_SLOVAK 0x041B
#define USB_LANGID_SLOVENIAN 0x0424
#define USB_LANGID_SPANISH_TRADITIONAL_SORT 0x040A
#define USB_LANGID_SPANISH_MEXICAN 0x080A
#define USB_LANGID_SPANISH_MODERN_SORT 0x0C0A
#define USB_LANGID_SPANISH_GUATEMALA 0x100A
#define USB_LANGID_SPANISH_COSTA_RICA 0x140A
#define USB_LANGID_SPANISH_PANAMA 0x180A
#define USB_LANGID_SPANISH_DOMINICAN_REPUBLIC 0x1C0A
#define USB_LANGID_SPANISH_VENEZUELA 0x200A
#define USB_LANGID_SPANISH_COLOMBIA 0x240A
#define USB_LANGID_SPANISH_PERU 0x280A
#define USB_LANGID_SPANISH_ARGENTINA 0x2C0A
#define USB_LANGID_SPANISH_ECUADOR 0x300A
#define USB_LANGID_SPANISH_CHILE 0x340A
#define USB_LANGID_SPANISH_URUGUAY 0x380A
#define USB_LANGID_SPANISH_PARAGUAY 0x3C0A
#define USB_LANGID_SPANISH_BOLIVIA 0x400A
#define USB_LANGID_SPANISH_EL_SALVADOR 0x440A
#define USB_LANGID_SPANISH_HONDURAS 0x480A
#define USB_LANGID_SPANISH_NICARAGUA 0x4C0A
#define USB_LANGID_SPANISH_PUERTO_RICO 0x500A
#define USB_LANGID_SUTU 0x0430
#define USB_LANGID_SWAHILI_KENYA 0x0441
#define USB_LANGID_SWEDISH 0x041D
#define USB_LANGID_SWEDISH_FINLAND 0x081D
#define USB_LANGID_TAMIL 0x0449
#define USB_LANGID_TATAR_TATARSTAN 0x0444
#define USB_LANGID_TELUGU 0x044A
#define USB_LANGID_THAI 0x041E
#define USB_LANGID_TURKISH 0x041F
#define USB_LANGID_UKRAINIAN 0x0422
#define USB_LANGID_URDU_PAKISTAN 0x0420
#define USB_LANGID_URDU_INDIA 0x0820
#define USB_LANGID_UZBEK_LATIN 0x0443
#define USB_LANGID_UZBEK_CYRILLIC 0x0843
#define USB_LANGID_VIETNAMESE 0x042A

#endif

/**@}*/

