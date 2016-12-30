#ifndef _USB_BYTEORDER_H
#define _USB_BYTEORDER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LITTLE_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD
#endif

#ifndef HTONS

#include <stdint.h>
static inline
uint32_t usb_ntohl(uint32_t x){
	return ((x)>> 24 & 0xff) 
			| ((x)>>8 & 0xff00) 
			| ((x)<<8 & 0xff0000L) 
			| ((x)<<24 & 0xff000000L)
			;
}

static inline
uint16_t usb_ntohs(uint16_t x){
	return ((x)>>8 & 0xff) | ((x)<<8 & 0xff00);
}

static inline 
uint32_t usb_htonl(uint32_t x) { return usb_ntohl(x); };

static inline 
uint16_t usb_htons(uint16_t x) {return usb_ntohs(x); };

#define HTONS(x)  usb_htons(x)
#define HTONL(x)  usb_htonl(x)
#define NTOHS(x)  usb_ntohs(x)
#define NTOHL(x)  usb_ntohl(x)

#endif


#ifdef __cplusplus
}
#endif

#endif
