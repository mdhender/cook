/*
 *	cook - file construction tool
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: interface definition for meter.c
 */

#ifndef COOK_METER_H
#define COOK_METER_H

#include <ac/time.h>
#ifdef HAVE_WAIT3
#include <sys/resource.h>
#endif

#include <main.h>

typedef struct meter_ty meter_ty;
struct meter_ty
{
#ifdef HAVE_GETTIMEOFDAY
    struct timeval  start;
#else
    time_t          start;
#endif
#ifdef HAVE_WAIT3
    struct rusage   ru;
#endif
};

meter_ty *meter_alloc _((void));
void meter_free _((meter_ty *));
void meter_begin _((meter_ty *));
void meter_print _((meter_ty *));

#endif /* COOK_METER_H */
