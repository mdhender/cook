/*
 *	cook - file construction tool
 *	Copyright (C) 1996, 1997 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: presence vs absence insulation for sys/utsname.h
 */

#ifndef COMMON_AC_SYS_UTSNAME_H
#define COMMON_AC_SYS_UTSNAME_H

#include <config.h>

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#else

struct utsname
{
	char sysname[10];
	char nodename[50];
	char release[10];
	char version[10];
	char machine[10];
};

#endif

#endif /* COMMON_AC_SYS_UTSNAME_H */
