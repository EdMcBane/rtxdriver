/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  phoneusb.h
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
 *              
 */

#include <usb.h>

#define VENDOR_ID   0x0d9a
#define PRODUCT_ID  0x0004
#define DEVCONFIG   1
#define DEVINTERFACE   3

#define ALTERNATE   0
#define EP_IN       0x81
#define EP_OUT      0x01
#define POLL_INTERVAL 10

usb_dev_handle* claim_phone();
int free_and_reset_device();
usb_dev_handle* find_phone();
int init_phone (usb_dev_handle * phone);
int release_phone(usb_dev_handle* phone);
