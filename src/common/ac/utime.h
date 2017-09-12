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
 * MANIFEST: presence vs absence insulation for <utime.h>
 */

#ifndef COMMON_AC_UTIME_H
#define COMMON_AC_UTIME_H

#include <config.h>

#ifdef HAVE_UTIME_H
#include <utime.h>
#else

#include <ac/time.h>

struct utimbuf
{
	time_t actime;
	time_t modtime;
};

#endif

#endif /* COMMON_AC_UTIME_H */
