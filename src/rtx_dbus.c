/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  rtx_dbus.c
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
 * Copyright (C) Julien Puydt, 2005
 * Copyright (C) Francesco Degrassi, 2006
 *
 * 2005-Nov Julien Puydt <> 
 *              Original code taken from dbus.cpp, implementation of the DBUS 
 *              interface of gnomemeeting.
 *
 * 2006-Aug Francesco Degrassi <francesco.degrassi@gmail.com>
 *              
 */

#include "rtx_dbus.h"
G_DEFINE_TYPE(RtxPhone, rtx_phone, G_TYPE_OBJECT);
/* get the code to make the GObject accessible through dbus
 * (this is especially where we get dbus_glib__object_info !)
 */
#include "dbus_stub.h"

/* implementation of the GObject's methods */

static void
rtx_phone_init (RtxPhone *self)
{
  /* nothing to do */
}

static void
rtx_phone_class_init (RtxPhoneClass *klass)
{
    g_type_class_add_private (klass, sizeof (RtxPhonePrivate));
  /* creation of all the signals */
  StateChangedSignal = g_signal_new ("state-changed",
					 G_OBJECT_CLASS_TYPE (klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL,
                     dbus_glib_marshal__BOOLEAN__POINTER,
					 G_TYPE_NONE,
					 1, G_TYPE_UINT);

  RequestedCallSignal = g_signal_new ("requested-call",
					 G_OBJECT_CLASS_TYPE (klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL,
                     dbus_glib_marshal__BOOLEAN__STRING_POINTER,
					 G_TYPE_NONE,
					 1, G_TYPE_STRING);

  /* initializing as dbus object */
  dbus_g_object_type_install_info (RTX_PHONE_TYPE_OBJECT,
				   &dbus_glib__object_info);

}

void rtx_phone_state_changed(RtxPhone* self, guint state) {
  g_signal_emit (self, StateChangedSignal, 0, state);
}

void rtx_phone_requested_call(RtxPhone* self, const char* dest) {
  g_signal_emit (self, RequestedCallSignal, 0, dest);
}
//    g_signal_emit (self, signals[NAME_INFO], 0, token, name);

gboolean
rtx_phone_call (RtxPhone *self,
			const char *caller,
			GError **error)
{
  LOG(INFO, "DBUS: Incoming call from %s\n" , caller);
  RtxPhonePrivate *data = RTX_PHONE_GET_PRIVATE (self);
  send_call_notification (data->phone, caller);
  return TRUE;
}

gboolean
rtx_phone_hangup (RtxPhone *self,
			   GError **error)
{
  LOG(INFO, "DBUS: Hangup request\n");
  RtxPhonePrivate *data = RTX_PHONE_GET_PRIVATE (self);
  send_hangup (data->phone);
  return TRUE;
}

gboolean
rtx_phone_accept (RtxPhone *self,
			const char *caller,
			GError **error)
{
  LOG(INFO, "DBUS: Outgoing call to %s accepted\n" , caller);
  RtxPhonePrivate *data = RTX_PHONE_GET_PRIVATE (self);
  accept_call(data->phone, caller);
  return TRUE;
}


gboolean
rtx_phone_claim_ownership (RtxPhone *self)
{
  RtxPhonePrivate *data = RTX_PHONE_GET_PRIVATE (self);
  DBusGConnection *bus = NULL;
  DBusGProxy *bus_proxy = NULL;
  guint request_name_result;
  GError *error = NULL;
    
  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {

    LOG (ERROR, "Couldn't connect to session bus : %s\n", error->message);
    return FALSE;
  }

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
                                         "/org/freedesktop/DBus",
                                         "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
                          G_TYPE_STRING, "it.clockworks.rtxdriver",
                          G_TYPE_UINT, DBUS_NAME_FLAG_DO_NOT_QUEUE,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &request_name_result,
                          G_TYPE_INVALID)) {

    LOG(ERROR, "Couldn't get the net.franz.instance name : %s\n", error->message);
    return FALSE;
  }

  dbus_g_connection_register_g_object (bus, "/it/clockworks/rtxdriver",
				       G_OBJECT (self));

  return TRUE;
}

GObject *
rtx_phone_new (usbphone* phone)
{
  RtxPhone *result = NULL;

  result = RTX_PHONE_OBJECT (g_object_new (RTX_PHONE_TYPE_OBJECT,
						NULL));
  
  RtxPhonePrivate *data = RTX_PHONE_GET_PRIVATE (result);
  data->phone = phone;
  phone->dbusobj = G_OBJECT (result);

  if (! rtx_phone_claim_ownership (result)) exit(1);
  return G_OBJECT (result);
}



