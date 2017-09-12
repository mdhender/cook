/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: interface definition for cook/stat.cache.c
 */

#ifndef COOK_STAT_CACHE_H
#define COOK_STAT_CACHE_H

#include <ac/time.h>
#include <str.h>

time_t stat_cache_oldest _((string_ty *, int));
time_t stat_cache_newest _((string_ty *, int));
void stat_cache_set _((string_ty *, time_t, int));
void stat_cache_clear _((string_ty *));

#endif /* COOK_STAT_CACHE_H */
