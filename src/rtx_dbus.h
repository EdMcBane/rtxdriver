/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  rtx_dbus.h
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
 * Copyright (C) Julien Puydt, 2005
 * Copyright (C) Francesco Degrassi, 2006
 *
 * 2005-Nov Julien Puydt <> 
 *              Part of the code taken from dbus.cpp, implementation of the
 *              DBUS interface of gnomemeeting.
 *
 * 2006-Aug Francesco Degrassi <francesco.degrassi@gmail.com>
 *              
 */

#ifndef _RTX_DBUS_H_
#define _RTX_DBUS_H_

#define DBUS_API_SUBJECT_TO_CHANGE
#include "rtx.h"
#include "common.h"
#include <dbus/dbus-glib.h>

/* Beginning of a classic GObject declaration */
typedef struct RtxPhone RtxPhone;
typedef struct RtxPhonePrivate RtxPhonePrivate;
typedef struct RtxPhoneClass RtxPhoneClass;

GType rtx_phone_get_type (void);

struct RtxPhone
{
    GObject parent;
};

struct RtxPhonePrivate
{
    guint state;
    usbphone* phone;
};

struct RtxPhoneClass
{
    GObjectClass parent;
};

static guint StateChangedSignal;
static guint RequestedCallSignal;

#define RTX_PHONE_TYPE_OBJECT (rtx_phone_get_type ())
#define RTX_PHONE_OBJECT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), RTX_PHONE_TYPE_OBJECT, RtxPhone))
#define RTX_PHONE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), RTX_PHONE_TYPE_OBJECT, RtxPhoneClass))
#define RTX_PHONE_IS_OBJECT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), RTX_PHONE_TYPE_OBJECT))
#define RTX_PHONE_IS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), RTX_PHONE_TYPE_OBJECT))
#define RTX_PHONE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RTX_PHONE_TYPE_OBJECT, RtxPhonePrivate))

/* End of a classic GObject declaration */


/* declaration of all the methods of this object */
gboolean rtx_phone_call (RtxPhone *self,
					const char *url,
					GError **error);

gboolean rtx_phone_accept (RtxPhone *self,
					const char *url,
					GError **error);

gboolean rtx_phone_hangup (RtxPhone *self,
					   GError **error);

gboolean rtx_phone_claim_ownership (RtxPhone *self);

void rtx_phone_state_changed(RtxPhone* self, guint state);

void rtx_phone_requested_call(RtxPhone* self, const char* dest);

GObject * rtx_phone_new (usbphone* phone);

#endif
