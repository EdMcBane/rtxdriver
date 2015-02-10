 #
 #  This file is part of RTX Driver
 #  Linux driver for RTX DualPhone USB
 #
 #  Copyright (C) Francesco Degrassi, 2006
 # 
 #  ekiga-rtx-plugin.py
 #
 # RTX-Driver is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation; either version 2 of the License, or
 # (at your option) any later version.
 #
 # RTX-Driver is distributed in the hope that it will be useful, 
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License 
 # along with this program; if not, write to the Free Software
 # Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 #
 # 
 #
 # Copyright (C) Francesco Degrassi, 2006
 #
 # 2006-Aug Francesco Degrassi <francesco.degrassi@gmail.com>


#!/usr/bin/python
import sys
import gobject 
import dbus
if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
    import dbus.glib

usage="""
Usage:      
            python ekiga-rtx-plugin.py <DESTINATION SIP HOST>

Example:
            python ekiga-rtx-plugin.py sip.myvoipprovider.net

"""
if len(sys.argv) < 2:
    print usage
    sys.exit(1)
suffix = "@"+sys.argv[1]

phone_calls = None
ekiga_calls = None
ekiga_token = None
ekiga_url = None
ekiga_state = 1
phone_state = 0

def p_state_changed_callback(state):
    global ekiga_state, ekiga_token, ekiga_url, phone_state
    print "phone state changed: %d" % state
    phone_state = int(state)
    if phone_state == 0:
        print "ekiga state %d" % ekiga_state
        if ekiga_state in (2,3,4):
            print "disconnecting ekiga"
            ekiga_calls.Disconnect(ekiga_token)
            ekiga_url = None
            ekiga_token = None
    if phone_state == 3:
        if ekiga_state in (2,4):
            ekiga_calls.Connect(ekiga_url)
            ekiga_url = None
            ekiga_token = None
    
def p_requested_call_callback(dest):
    global ekiga_state, ekiga_token, ekiga_url, phone_state
    print "phone requested call to: %s" % dest
    if ekiga_state in (0,1):
        ekiga_url = dest + suffix
        ekiga_token = ekiga_calls.Connect(ekiga_url)
        phone_calls.Accept(dest)

def e_state_changed_callback(str, state):
    global ekiga_state, ekiga_token, ekiga_url, phone_state
    print "ekiga state changed: %s %d" % (str,state)
    ekiga_state = int(state)
    if ekiga_state in (0,1):
        ekiga_url = None
        ekiga_token = None
        phone_calls.Hangup()

bus = dbus.SessionBus()

phone_obj = bus.get_object('it.clockworks.rtxdriver', '/it/clockworks/rtxdriver')
phone_calls = dbus.Interface(phone_obj, 'it.clockworks.rtxdriver.calls')
phone_calls.connect_to_signal('StateChanged', p_state_changed_callback)
phone_calls.connect_to_signal('RequestedCall', p_requested_call_callback)

ekiga_obj = bus.get_object('net.ekiga.instance', '/net/ekiga/instance')
ekiga_calls = dbus.Interface(ekiga_obj, 'net.ekiga.calls')
ekiga_calls.connect_to_signal('StateChanged', e_state_changed_callback)

mainloop = gobject.MainLoop()
mainloop.run()
