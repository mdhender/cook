/*
 *	cook - file construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997 Peter Miller;
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
 * MANIFEST: interface definition for c_incl/cache.c
 */

#ifndef CACHE_H
#define CACHE_H

#include <sys/types.h>
#include <sys/stat.h>

#include <main.h>
#include <str_list.h>

typedef struct cache_ty cache_ty;
struct cache_ty
{
	struct stat	st;
	string_list_ty	ingredients;
};

void cache_initialize _((void));
cache_ty *cache_search _((string_ty *filename));
void cache_read _((void));
void cache_write _((void));
void cache_update_notify _((void));

#endif /* CACHE_H */
