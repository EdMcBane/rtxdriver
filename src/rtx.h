/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  rtx.h
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
 *              
 */

#ifndef _RTX_H_
#define _RTX_H_
#define DBUS_API_SUBJECT_TO_CHANGE
#include "common.h"
#include "phoneusb.h"
#include <time.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

#define MSG_ERROR -1
#define MSG_COMPLETE 0
#define MSG_INCOMPLETE 1
#define MSG_NOMSG 2

#define MSG_LEN 16
#define APP_TIMEOUT 2000

#define INFO 0
#define NOTICE 1
#define WARN 2
#define ERROR 3
#define FATAL 4

#define NO_CALL 0
#define CALLING_OUT 1
#define CONNECTED_OUT 2
#define IN_PROGRESS 3
#define CALL_INCOMING 4

#define DEF_CALL_FLAGS 0x06
#define UC(x) ((unsigned char)(x))

typedef struct _msg_t {
	unsigned char type;
	unsigned char len;
	unsigned char curpos;
	unsigned char subtype;
	unsigned char pad;
	char data[MAX_BUF_LEN];
} msg_t;

typedef struct _usbphone {
    usb_dev_handle* dev;
    char packet_pad;		// 1 byte, padding for type 0,1,4,6,7 messages, wraps around 
    char packet_index; 	// 1 nibble, packet identifier, wraps around
    time_t last_time_update;
    int call_state;
    int session_index;
    unsigned char session_id;
    int session_open;
    char call_flags;
    GObject* dbusobj;
    msg_t pmsg;
} usbphone;

//
// Function declarations
//

int init_dbus_stuff(usbphone* p);
int main (int argc, char ** argv) ;
int comm_init(usbphone* p) ;
int sendPacket(usbphone* p, const char* buf) ;
int sendSimpleMessage(usbphone* p, char type) ;
int sendMessage(usbphone* p, char subtype, int len, const char* buffer) ;
int recvMessage(usbphone* p, msg_t *msg, int timeout);
int waitMessage(usbphone* p, msg_t *msg, unsigned char type, int timeout) ;
int send_time_upd(usbphone* p) ;
int send_status_notification (usbphone* p, const char *nickname, char status) ;
int send_call_notification (usbphone* p, const char *caller) ;
int send_hangup (usbphone* p) ;
int handle_xml_query (usbphone* p, msg_t* msg) ;
int handle_com(usbphone* p) ;
int call_answered() ;
int can_call(const char* dest) ;
int do_comm(usbphone* p) ;
int do_handle_msg(usbphone* p, msg_t *msg) ;


#endif
