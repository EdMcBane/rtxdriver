# rtxdriver
RTX Skype DualPhone USB driver for linux

## NOTE
this code belongs to the author: Francesco Degrassi <francesco dot degrassi at gmail dot com>

## INTRO

This is a driver for the RTX Skype DualPhone USB phone for Linux, obtained entirely studying the traffic between the phone and the MS Windows driver. The driver runs in userspace, using libusb, so no kernel recompile is necessary. Development was made on a Ubuntu Dapper system. The driver connects to the phone via libusb/usbfs, connects to the user's session DBUS bus and is available for use by any voip application (for which a connector has been written). Currently the only plugin available is for Ekiga. It was tested only on a Ubuntu Dapper 6.06 LTS system, kernel 2.6.15, but should work on a broad range of systems

WARNING The code is sometimes ugly and hacked, since it was developed while reverse engineering the protocol spoken on the USB bus. Please understand that at the moment it is NOT usable by the faint of heart, you should be know what you are doing if you try to use it.

I REPEAT: this is work in progress, not a clean finished product. It is meant for experienced linux people who want to experiment with it and are willing to help. I take no responsibility for any problem / damage / illness this source could cause you.

Expect it to fail, crash, explode, steal your soul etc...

## FEATURES

Supported features:

Calling out simple numbers from the phone

Ringing and accepting / rejecting incoming calls

Updating phone clock

Partially supported features:

Getting on/off hook (phone base) notifications (missing dbus signal)

Notifying the phone of contacts' status (missing dbus method)

Unsupported features:

Call on hold

Address book navigation / call, xml-like dialect (bad stinking example code is in the docs folder)

Called numbers redial (uses same xml-like dialect)

## CONTENTS

doc
Documentation

misc
Miscellaneous dev tools and scripts, udev rules

plugins
VOIP app plugins (only Ekiga for now)

src
Source code for the phone driver

## BUILD

You'll need several libraries to build the binary, here are the package names for debian/ubuntu:

libusb-dev

libglib2.0-dev

libdbus-1-dev

libdbus-glib-1-dev

To actually compile:

cd /tmp; tar zxvf rtxdriver-0.01.tgz

cd rtxdriver-0.01

cd src

make

Please note that you will need a DBUS enabled version of Ekiga to experiment. The default Ekiga binary for Dapper has DBUS support disabled, to enable it you should recompile it using the "--enable-dbus" configure option.

Here is how i did it:

   $> apt-get source ekiga
   $> cd ekiga-2.xxx
   $> cd debian
   $> vi rules
add "--enable-dbus" to the configure invocation and save file

   $> vi changelog
add this text (or similar) to the head of the changelog file WITHOUT THE "------ START" stuff. Notice how i simply appended ".dbus1" to the previous version.

 --------- START
 ekiga (2.0.1-0ubuntu6.dbus1) dapper; urgency=low
   * debian/rules:
     - enabled dbus support
  -- root <root@localhost.localdomain>  Thu, 24 Aug 2006 22:41:21 +0200
 --------- STOP
   $> cd ..
   $> dpkg-buildpackage -b
   $> dpkg -i ekiga*.deb

## SETUP

IMPORTANT The following instructions are meant for an Ubuntu Dapper system. If you use another distro (why would you ??), you'll probably need to tweak a thing or two.

For the driver to work for a standard user (not root), you will need to tell udev to grant read and write permissions to users of group plugdev to the usb device files.

To do so:

 $> sudo copy misc/50-rtxdriver.rules /etc/udev/rules.d/
 $> addgroup YOUR_USER plugdev
and reboot

Alternatively you could do something like this (but it is NOT secure at all)

 $> chmod -R 777 /dev/bus/usb
WARNING USING THE DRIVER AS ROOT WILL PROBABLY NOT WORK, SINCE THE USER'S SESSION DBUS WILL NOT BE ACCESSIBLE FROM ROOT. THE DRIVER IS MEANT TO BE LAUNCHED WITH THE PERMISSIONS OF THE USER.

IF YOU KNOW BETTER, BE MY GUEST.

## USAGE

Start the driver in a shell from your user session:

   $> ./rtxdriver
Start Ekiga (MUST HAVE DBUS SUPPORT)

Start the ekiga-rtx plugin from another shell, specifing the destination voip server for outgoing calls

   $> python ekiga-rtx-plugin.py fwd.pulver.com
Try to call out a number, ekiga should start a call to number@fwd.pulver.com

When called, the phone should ring and it should be possible to answer

## DEVELOPMENT

The system works like this:

 PHONE     <-libusb->  RTXDRIVER  
 RTXDRIVER <-DBUS->  ekiga-rtx-plugin  <-DBUS->  Ekiga
 RTXDRIVER <-DBUS->  skype-rtx-plugin  <-DBUS->  Skype
 RTXDRIVER <-DBUS->  .....             <-????->   ....
In this way adding support for more voip apps is a simple matter of writing a python script to handle the bridging and DBUS communication.

To troubleshoot the stuff, you should:

Recompile kernel with support for debugfs and usbmon (as module)

Mount debugfs in /sys/kernel/debug

modprobe usbmon

cat /sys/kernel/debug/usbmon/1t (or whatever bus your phone is connected to)

save the traffic to dump.log

use the phone, make it crash, whatever

cat dump.log | python edump.py # it will provide a decent decoded output

To dump the traffic between the phone and the windows driver, i used VMWare player running inside my linux box and used the above method to dump the traffic

I hope to be able to clean up and provide with a real end user package as soon as possible.

I am very open to suggestions, fixes and help. Feel free to contact me for assistance, but please no "what is this kernel thing you all speak about" kind of questions.

Francesco Degrassi

francesco dot degrassi at gmail dot com
