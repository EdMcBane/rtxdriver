/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  phoneusb.c
 *
 * RTX-Driver is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * RTX-Driver is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * 
 *
 * Copyright (C) Francesco Degrassi, 2006
 *
 * 2006-Aug Francesco Degrassi <francesco.degrassi@gmail.com>
 *              Initial release, bits and parts taken from uncopyrighted code        
 */

#include <usb.h>
#include "common.h"
#include "phoneusb.h"

//
// USB DEVICE FUNCTIONS
//
usb_dev_handle * claim_phone(int interface) {
	usb_dev_handle * phone;

	if (!(phone = find_phone())) {
		LOG(ERROR, "phone device not found\n");
		return NULL;
	}

	if (usb_claim_interface(phone, interface) < 0) {
		LOG(ERROR, "Could not claim interface: %s\n", usb_strerror());
		return NULL;
	}
	return phone;
}

int free_and_reset_device() {
	char dev_name[MAX_BUF_LEN];
	usb_dev_handle * phone;

	if (!(phone = find_phone())) {
		LOG(ERROR, "phone device not found\n");
		return -1;
	}
	if (LIBUSB_HAS_GET_DRIVER_NP && (usb_get_driver_np(phone, DEVINTERFACE, dev_name, 254)) >= 0) {
		LOG(ERROR, "Current driver for %d: %s\n", DEVINTERFACE, dev_name);
		if (LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP && (usb_detach_kernel_driver_np(phone, DEVINTERFACE) < 0)) {
			LOG(ERROR, "Error detaching kernel driver:  %s\n", usb_strerror());
			return -1;
		} 
	}
	
	if (usb_reset(phone) < 0) {
		LOG(ERROR, "Error resetting device:  %s\n", usb_strerror());
		return -1;
	}
	if (usb_close(phone) < 0) {
		LOG(ERROR, "Error closing device:  %s\n", usb_strerror());
		return -1;
	}
	
	return 0;
}

usb_dev_handle* find_phone() {
	struct usb_bus *bus;
	struct usb_device *dev;

	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == VENDOR_ID &&
					dev->descriptor.idProduct == PRODUCT_ID) {
				struct usb_dev_handle *handle;
				LOG(ERROR, "Phone found\n");
				if (!(handle = usb_open(dev))) {
					LOG(ERROR, "Could not open device\n");
					return NULL;
				}
				return handle;
			}
		}
	}
	return NULL;
}

int init_phone (usb_dev_handle * phone) {
	if (usb_set_altinterface(phone,ALTERNATE) < 0) {
		LOG(ERROR, "Could not set alternate on interface\n");
		return -1;
	}

	// s 21 0a 0000 0003 0000 0  // set_idle request
	if (usb_control_msg(phone,	
				0x21, 	// int requesttype,
				0x0a,	// int request,
				0x0000,	// int value,
				0x0003, // int index,
				NULL,	// char* bytes,
				0,		// int size,
				0		// int timeout
				) < 0) {
		LOG(ERROR, "Could not send control message #5\n");
		return -1;
	}
	return 0;

}


int release_phone(usb_dev_handle* phone) {
	if (usb_release_interface(phone,DEVINTERFACE) < 0) {
		LOG(ERROR, "Could not release interface\n");
		return -1;
	}

	if (usb_close(phone) < 0) {
		LOG(ERROR, "Could not close device\n");
		return -1;
	}
	return 0;    
}

