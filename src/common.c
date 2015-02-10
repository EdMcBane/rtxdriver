/*
 *  This file is part of RTX Driver
 *  Linux driver for RTX DualPhone USB
 *
 *  Copyright (C) Francesco Degrassi, 2006
 * 
 *  common.c
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

inline int min(int x, int y)
{
    if (x <= y) return x;
    return y;
}

