 #
 #  This file is part of RTX Driver
 #  Linux driver for RTX DualPhone USB
 #
 #  Copyright (C) Francesco Degrassi, 2006
 # 
 #  rtx.c
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
 #              
 #

import sys

def printHex(line, filler):
    i = 0
    outhex = ""
    outascii = ""
    s = line
    while len(s) > 0:
        hex = s[0:2]
        s = s[2:]
        outhex += hex + " "
        dec = int(hex,16)
        if dec >= 32 and dec <= 126:
            outascii += " " +chr(dec) + " " 
        else:
            outascii += " . "
        i += 1
        if i % 16 == 0 or len(s)==0:
            if i % 16 == 0:
                print filler, "%-60s" % outhex
                print filler, "%-60s" % outascii
            else:
                print filler, "%-60s" % outhex
                print filler, "%-60s" % outascii
            outhex = ""
            outascii = ""
        elif i % 8 == 0:
            outhex += "   "
            outascii += "   "
                            
def printBinary(line):
    s = line
    i = 0
    outbin = ""    
    while len(s) > 0:
        hex = s[0:2]
        s = s[2:]
        dec = int(hex,16)
        for x in range (8):
            p = pow(2,7-x)
            if dec & p > 0:
                outbin += "1"
            else:
                outbin += "0"
        outbin += " "
        i+=1
        if i % 16 == 0 or len(s)==0:
            if i % 16 == 0:
                print "%2x: %s" % (i-16,outbin)
            else:
                print "%2x: %s" % (i-(i % 16),outbin)
            outbin = ""
        elif i % 8 == 0:
            outbin += " | "
                
            
            
        
        
    
import re
remlen = 0
while True:
    lineorig = sys.stdin.readline()
    if not lineorig:
        break
    if not "C I" in lineorig:
        continue
    if "-2" in lineorig:
        continue
    try:
        res = re.compile("^[0-9a-f]+ ([0-9]+) .+= ((.)(.).+)").search(lineorig)
        timestamp = res.group(1)
        line = res.group(2)
        seq = int(res.group(3),16)
        type = int(res.group(4),16)
    except Exception, e:
        print ">>>>>>>>>>>>>", lineorig
        continue

    line = line.replace(" ","")
    if type in (0,1,4,6,7):
        continue
    filler = ""
    
    if "Io" in lineorig:
        filler = ">> "
    else:
        filler = "                                                                             << "
    
    print filler, timestamp[-9:-5],
    print "SEQ: %x" % seq,
    if type in (4,):
        print "TYPE: 4 (ACK)",
        print "           ",
        print "ACKSEQ: %s" % line[2:4],
        # line = line[2:4]
        line = ""
    if type in (2,):
        print "TYPE: 2 (CMD)",
        print "SUBTYPE: %s" % line[6:8], 
        remlen = int(line[2:4],16)
        print "LEN: %x" % (remlen-1),
        curlen = min(remlen,13)
        line = line[8:curlen*2+6]
        remlen -= curlen
    if type in (3,):
        print "TYPE: 3 (CONT)",
        curlen = min(remlen,15)
        line = line[2:curlen*2+2]
        remlen -= curlen
    print
    print filler, "_"*45
    # print lineorig    
    printHex(line, filler)
    # printBinary(line)
    print 
    print
    print
        
