/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: interface definition for common/timing.c
 */

#ifndef COMMON_TIMING_H
#define COMMON_TIMING_H

#include <main.h>

#ifdef __GNUC__

void timing_register _((char *, void **));
void timing_push _((void *));
void timing_pop _((void *));

#define timing_entry() do { static void *__timing_p; if (!__timing_p) \
timing_register( __FILE__ ":" __FUNCTION__, &__timing_p); \
timing_push(__timing_p); } while (0)

#define timing_exit() do { static void *__timing_p; if (!__timing_p) \
timing_register( __FILE__ ":" __FUNCTION__, &__timing_p); \
timing_pop(__timing_p); } while (0)

#else

#define timing_entry() ((void)0)
#define timing_exit() ((void)0)

#endif

void timing_print _((void));

#endif /* COMMON_TIMING_H */
