/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  rtx.c
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

#include "common.h"
#include "rtx.h"
#include "rtx_dbus.h"

//TODO: protocol implementation is really limited
//TODO: debug protocol regarding calling an history contact
//TODO: implement XML-like dialect
//TODO: cleanup/rearrange the code
//TODO: complete/define a clear interface
//TODO: integrate with udev for automatic startup
//TODO: develop skype python plugin
//TODO: release tools used to test the stuff
//TODO: release plugin for ekiga
//TODO: write minimal docs
//TODO: solve permissions problems (must be root to run rtxdriver)
//TODO: handle USB and DBUS disconnects
//TODO: many many more things to fix

inline void init_usbphone(usbphone* p) {
    p->dev = NULL;  
    p->packet_pad = 0x02;
    p->packet_index = 0x01;
    p->last_time_update = 0;
    p->call_state,NO_CALL;
    p->session_index = 0;
    p->session_id = 0;
    p->session_open = 0;
    p->call_flags=DEF_CALL_FLAGS;
    p->dbusobj = NULL;
};

inline unsigned char get_session_index(usbphone* p) {
	p->session_index++;
	if (p->session_index == 0x10) p->session_index = 0;
	return (unsigned char) p->session_index;
}

inline void set_state(usbphone* p, unsigned int state) {
    p->call_state = state;
    if (p->dbusobj) rtx_phone_state_changed((RtxPhone*)(p->dbusobj), state);
}

GMainLoop *loop;
DBusGConnection *bus;

int init_dbus_stuff(usbphone* p) {
  GError* error = NULL;
  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {
        g_printerr ("Error getting bus: %s\n", error->message);
        g_error_free (error);
        return -1;
  }
  GObject* dbusphone = rtx_phone_new(p);
  return 0;
}


//
// MAIN 
//
int main (int argc, char ** argv) {
    
	usb_init();
	usb_find_busses();
	usb_find_devices();
	if (free_and_reset_device() < 0) exit(1);
	LOG(NOTICE, "Phone interface freed\n");
	usb_dev_handle* dev = claim_phone(3);
	if (!dev) exit(1);
	LOG(NOTICE, "Phone interface claimed\n");
	if (init_phone(dev) < 0) exit(1);
	LOG(NOTICE, "Low level system initialized\n");
    
	usbphone phone;
    init_usbphone(&phone);
    phone.dev = dev;
    
	if (comm_init(&phone) < 0) exit(1);
	LOG(NOTICE, "Phone initialization complete\n");
    
    // DBUS
    g_type_init ();
    loop = g_main_loop_new (NULL, FALSE);
    if (init_dbus_stuff(&phone)<0) return -1;
    g_timeout_add(100, (GSourceFunc) handle_com, &phone);
    g_main_loop_run (loop);
    
    // Low level release
	release_phone(dev);
	return 0;        
}


//
// Phone protocol message functions
//

int comm_init(usbphone* p) {
	// Initialize interrupt communication
	// sets up the channel between driver and phone
	//
	msg_t msg;
	memset(&msg, 0, sizeof(msg_t));
	int res;
	if (waitMessage(p, &msg,0,10) <0) return -1;
	if (sendSimpleMessage(p, 0x00) < 0) return -1;
	if (waitMessage(p, &msg,1,5) <0) return -1;
	if (waitMessage(p, &msg,0,5) <0) return -1;
	if (sendSimpleMessage(p, 0x01) < 0) return -1;
	if (send_time_upd(p) < 0) return -1;
	if (waitMessage(p, &msg,4,5) <0) return -1;
	LOG(NOTICE, "Comm initialized\n");
	return 0;
}

int sendPacket(usbphone* p, const char* buf) {
	char buffer[MSG_LEN];
	memcpy(buffer,buf,MSG_LEN);	
	// Set upper nibble of first byte to msg index
	buffer[0] = buffer[0] & (0x0f) | (p->packet_index << 4);

	if (usb_interrupt_write(
	   	p->dev,
		EP_OUT,		// int ep, 
		buffer,		// char *bytes, 
		MSG_LEN,	// int size,
		0			// int timeout
	) != MSG_LEN) {
		LOG(FATAL, "Could not write to phone\n");
		return -1;
	}
	p->packet_index++;
	if (p->packet_index == 0x10) p->packet_index = 0;
	p->packet_pad++;
	return MSG_LEN;
}

int sendSimpleMessage(usbphone* p, const char type) {
	assert(type == 0 || type == 1 || type == 4 || type == 6 || type == 7); 
	char buf[MSG_LEN];
	buf[0] = type;
	memset(buf+1,p->packet_pad,15);
	return sendPacket(p, buf);
}


int sendMessage(usbphone* p, char subtype, int len, const char* buffer) {
	char buf[MSG_LEN];
	int curoffset = 4; 	// for type 2 packets, the initial header is 4 bytes long
	int curindex = 0;	
	int curlen;
	buf[0] = 2;			// type = message 
	buf[1] = len+1;		// len is calculated from the subtype field, included
	buf[2] = 0;			
	buf[3] = subtype;	

	while (len > 0) {
		curlen = min(MSG_LEN-curoffset, len);
		len -= curlen;
		memcpy(buf+curoffset, buffer+curindex, curlen);
		if (sendPacket(p,buf) < 0) return -1;
		curindex += curlen;
		curoffset = 1;	// for type 3 packets the header is 1 byte long
		buf[0] = 3;
	}
	return 1;
}

int waitMessage(usbphone* p, msg_t *msg, unsigned char type, int timeout) {
    time_t end;
    time(&end);
    end += timeout;
    int res;
	do { 
        if(time(NULL) > end) {
            LOG(ERROR, "timeout when waiting for packet: type=%d\n", type);
            return -1;
        }
        res = recvMessage(p, msg, timeout*1000);
        
    } while (res != MSG_COMPLETE || msg->type != type);
    return 0;
}

int recvMessage(usbphone* p, msg_t *msg, int timeout) {
    if (!timeout) timeout = POLL_INTERVAL;
	char buffer[MSG_LEN];
	int curlen;
	if(usb_interrupt_read(
				p->dev,
				EP_IN, 			// int ep, 
				buffer, 		// char *bytes, 
				MSG_LEN, 		// int size,
				timeout	// int timeout
				) != MSG_LEN) 
		return MSG_NOMSG;

	int type = buffer[0] & 0x0f; // extract packet type 

	switch (type) { 
		case 0:
		case 1:
		case 4:
		case 6:
		case 7:
			msg->type = type;
			msg->subtype = 0;
			msg->len = 2;
			msg->curpos = 2;
			msg->pad = buffer[1];
			memcpy(msg->data, buffer, 15);
			break;
		case 2:
            // start saving it in the partial buffer
            msg->type = 2;
			msg->subtype = buffer[3];
			msg->len = buffer[1] - 1;
			msg->curpos = 0;
			msg->pad = 0;
			curlen = min(msg->len, 12);
			if (curlen > 0) {
				memcpy(msg->data, buffer+4, curlen); 
				msg->curpos = curlen;
			}
            if (msg->curpos < msg->len) // Message is incomplete, copy to pmsg buffer
                memcpy(&(p->pmsg), msg, sizeof(msg_t));
			break;
		case 3:
            // Leave type, subtype etc as it was, but add more data to the partial msg
			curlen = min(p->pmsg.len - p->pmsg.curpos, 15);
			if (curlen > 0) {
                if (p->pmsg.curpos + curlen > MAX_BUF_LEN) {
                    LOG(FATAL, "Oversize message\n");
                    return -1;
                }
				memcpy(p->pmsg.data + p->pmsg.curpos, buffer+1, curlen);
				p->pmsg.curpos += curlen;
			}
            // Copy message back to user buffer
            memcpy(msg, &(p->pmsg), sizeof(msg_t));
			break;
	}

    if (msg->curpos < msg->len)
		return MSG_INCOMPLETE;
    return MSG_COMPLETE;
}

//
// Helper functions
// 

int send_time_upd(usbphone* p) {
	time (&p->last_time_update);
	struct tm * t = localtime(&p->last_time_update);
	char buffer[14];
	buffer[0] = t->tm_year;
	buffer[1] = t->tm_mon+1;
	buffer[2] = t->tm_mday;
	buffer[3] = t->tm_hour;
	buffer[4] = t->tm_min;
	buffer[5] = 0x2b;
	memcpy(buffer+6, "H\x2emm\x2ess\x00", 8);
	return sendMessage(p, 0x62, 14, buffer);
	
}

int send_status_notification (usbphone* p, const char *nickname, char status) {
	char buffer[MAX_BUF_LEN];
	buffer[0] = 0x01;
	buffer[1] = 0x01;
	buffer[2] = 0x01;
	buffer[3] = status;
	int efflen = min(strlen(nickname), MAX_BUF_LEN-4);
	memcpy(buffer+4, nickname, efflen);
	return sendMessage(p, 0x61, efflen+4, buffer);
}

int send_call_notification (usbphone* p, const char *caller) {
	char buffer[MAX_BUF_LEN];
	if (p->session_open) {
		LOG(WARN, "Cannot open a session while another is in progress\n");
		return -1;	
	}
	p->session_open = 1;
	p->session_id = (get_session_index(p) & 0x0f) | 0xf0;	
	buffer[0] = p->session_id;
	p->call_flags = DEF_CALL_FLAGS;
	buffer[1] = p->call_flags;
	buffer[2] = 0x01;
	buffer[3] = 0x03;
	buffer[4] = 0x00;
	int efflen = min(strlen(caller), MAX_BUF_LEN - 5);
	memcpy(buffer+5, caller, efflen);
	if (sendMessage(p, 0x50, efflen+5, buffer) < 0) return -1;
	set_state(p,CALL_INCOMING);
}

int accept_call(usbphone* p, const char* caller) {
    char buf[MAX_BUF_LEN];
    if (p->call_state != CALLING_OUT) {
        LOG(WARN, "Call accepted by software but phone not in CALLING_OUT state\n");
        return -1;
    }
    buf[0] = p->session_id;
    buf[1] = 0x01;
    buf[2] = 0x01;
    buf[3] = 0x00;

    int curlen = min(strlen(caller), MAX_BUF_LEN-4);
    memcpy(buf+4,caller, curlen);
    buf[curlen+4] = 0x00;
    if (sendMessage(p, 0x5d, curlen + 4, buf) < 0) return -1;	// Call accept / reject
    
    // FIXME: Ringing should be sent only when app notifies about remote ring
    buf[0] = p->session_id;
    buf[1] = p->call_flags;
    if (sendMessage(p, 0x53, 2, buf) < 0) return -1;	// Ringing	

    // FIXME: Answered / Connected should be sent only when app notifies about it
    buf[0] = p->session_id;
    buf[1] = p->call_flags;
    if (sendMessage(p, 0x54, 2, buf) < 0) return -1;	// Answered
    if (sendMessage(p, 0x55, 2, buf) < 0) return -1;	// Connected
    set_state(p,CONNECTED_OUT);
    return 0;
}

int reject_call(usbphone* p) {
    // TODO: send the "NET BUSY" event or the "VOIP software not running" notification instead
    char buf[MAX_BUF_LEN];
    if (p->call_state != CALLING_OUT) {
        LOG(INFO, "Call reject timer went by, but not in CALLING_OUT state anymore\n");
        return 0;
    }
    LOG(WARN, "Rejecting call after timeout\n");
    buf[0] = p->session_id;
    buf[1] = 0x02;
    buf[2] = 0x01;
    buf[3] = 0x00;
    sendMessage(p, 0x5d, 4, buf) < 0;	// Call accept / reject
    send_hangup(p);
    return 0;
}

int send_hangup (usbphone* p) {
	char buffer[2];
	if (!p->session_open) {
		LOG(WARN, "Cannot hang up a closed session\n");
		return -1;	
	}
	LOG(NOTICE, "hanging up %x\n", p->session_id);
	buffer[0] = p->session_id;
	buffer[1] = 0x00;
	if (sendMessage(p, 0x57, 2, buffer) < 0) return -1;
	p->session_open = 0;
	p->session_id = 0;
	set_state(p,NO_CALL);
}

//
// HOST-PHONE COMMUNICATION
//
int handle_com(usbphone* p) {
	msg_t msg;
	memset(&msg, 0, sizeof(msg_t));

	if (do_comm(p) < 0) {
        g_main_loop_quit (loop);
        return -1;
    }
    
	int res = recvMessage(p, &msg, 0);
	if (res == MSG_COMPLETE) {
		switch (msg.type) {
			case 4:
			    break;
			case 2: 
			    if (sendSimpleMessage(p,0x04) < 0) {
                    g_main_loop_quit (loop);
                    return -1;
                }
				if (do_handle_msg(p, &msg) < 0) {
                    g_main_loop_quit (loop);
                    return -1;
                }
				break;
			case 6:
				if (sendSimpleMessage(p,0x07) < 0) {
                    g_main_loop_quit (loop);
                    return -1;
                };
				break;
			default:	
				LOG(WARN, "Bad packet type received: %x\n", msg.type);
                g_main_loop_quit (loop);
                return -1;
				break;
		}
	}	
	return 1;
}


int do_comm(usbphone* p) {
	char buf[MAX_BUF_LEN];
	
	// Handle time updates
	if (difftime(time(NULL), p->last_time_update) > 5*60) {
		if (send_time_upd(p) < 0) return -1;
	}
	return 0;
}


int do_handle_msg(usbphone* p, msg_t *msg) {
	LOG(DEBUG, "Message subtype %x\n", msg->subtype);
	char buf [MAX_BUF_LEN];

	switch (msg->subtype) {
		case 0x50:		// Open session
			if (p->session_open) {
				LOG(ERROR, "Open session message while session already open\n");
				return -1;
			}
			p->session_open = 1;
			p->session_id = (msg->data[0] & 0xf0) | (get_session_index(p) & 0x0f);
			buf[0] = p->session_id;
			buf[1] = p->call_flags = msg->data[1];
			if (sendMessage(p, 0x51, 2, buf) < 0) return -1;
            
            // TODO: implement XML parsing
            // Calling history contact requires XML parsing
            // so we simply hangup on such requests
            if (msg->len > 5) 
                send_hangup(p);
			break;
		
		case 0x79: // ON/OFF HOOK
			LOG(NOTICE, "%s\n", msg->data[0] ? "on hook":"off hook");
			break;	

		case 0x59: // INFO REQUEST ON CONTACT
			if (!p->session_open) {
				LOG(ERROR, "XML exchange without an open session\n");
				return -1;
			}
            // TODO: implement code to handle XML stuff
            // for now we simply close the session
            send_hangup(p);
			break;	

        case 0x64: // Info request on contact
            // TODO: implement code to handle contact info request
            // for now we simply return a fixed response
            memcpy(buf, msg->data, 4);
            buf[4] = 0x01;
            if (sendMessage(p, 0x65, 5, buf) < 0) return -1;   
			break;	

		case 0x52: // PERFORM CALL / KEY PRESSED
			if (!p->session_open) {
				LOG(ERROR, "PERFORM CALL without an open session\n");
				return -1;
			}
            int curlen = min(msg->len, MAX_BUF_LEN-1);
            memcpy(buf,msg->data+1, curlen);
            buf[curlen] = 0x00;
            if (p->dbusobj) rtx_phone_requested_call((RtxPhone*) p->dbusobj,buf); 
            
			set_state(p,CALLING_OUT);
            // If no one accepts this call, drop it after APP_TIMEOUT ms
            g_timeout_add(APP_TIMEOUT, (GSourceFunc) reject_call, p); 
			break;			

		case 0x57:	// Hang up
			if (!p->session_open) {
				LOG(ERROR, "Hang up without an open session\n");
				return -1;
			}
			if (UC(msg->data[0]) == p->session_id) {
				p->session_open = 0;
				p->session_id = -1;
				set_state(p,NO_CALL);
			} else
				LOG(ERROR, "Hang up received for session %x, but p->session_id is %x\n", msg->data[0], p->session_id);

			buf[0] = msg->data[0];
			if (sendMessage(p, 0x58, 1, buf) < 0) return -1;	// Hung up
			break;

		case 0x58: // Hung up
			assert(!p->session_open);
			break;
	
		case 0x56:	// Connected ACK
			if (!p->session_open) {
				LOG(ERROR, "Connected ACK message while session already open\n");
				return -1;
			}

			if (UC(msg->data[0]) != p->session_id ) {
				LOG(ERROR, "Connected ACK message for session %x but our p->session_id is %x\n", msg->data[0], p->session_id);
				return -1;
			}

			if (p->call_state != CONNECTED_OUT) {
				LOG(ERROR, "Connected ACK message while not connected out\n");
				return -1;
			}

			buf[0] = p->session_id;
			buf[1] = p->call_flags;
			buf[2] = 0x00;
			buf[3] = 0x00;
			if(sendMessage(p, 0x5a, 4, buf) < 0) return -1; // Start / stop talk
			set_state(p,IN_PROGRESS);
			break;

		case 0x53: // RINGING, received instead of a 0x51 when call is INCOMING
			p->session_id = UC(msg->data[0]);
			p->call_flags = msg->data[1];
			break;
	
		case 0x54: // ANSWERED
			break;
            
		case 0x5a: // START STOP TALKING
			break;

		case 0x55:
			if (!p->session_open) {
				LOG(ERROR, "Connected message while session already open\n");
				return -1;
			}

			if (UC(msg->data[0]) != p->session_id ) {
				LOG(ERROR, "Connected message for session %x but our p->session_id is %x\n", msg->data[0], p->session_id);
				return -1;
			}

			if (p->call_state != CALL_INCOMING) {
				LOG(ERROR, "Connected message while not call incoming\n");
				return -1;
			}

			buf[0] = p->session_id;
			buf[1] = p->call_flags | 0x40;	
			if (sendMessage(p, 0x56, 2, buf) < 0) return -1; // Connected ACK
			// the following is the same as 0x56
			buf[0] = p->session_id;
			buf[1] = p->call_flags;
			buf[2] = 0x00;
			buf[3] = 0x00;
			if(sendMessage(p, 0x5a, 4, buf) < 0) return -1; // Start / stop talk
			set_state(p,IN_PROGRESS);
			break;

		default:
			LOG(ERROR, "Unknown message received: %x\n", msg->subtype);
	}
	return 0;
}


