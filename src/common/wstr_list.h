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
 * MANIFEST: interface definition for common/wstr_list.c
 */

#ifndef COMMON_WSTR_LIST_H
#define COMMON_WSTR_LIST_H

#include <wstr.h>

typedef struct wstring_list_ty wstring_list_ty;
struct wstring_list_ty
{
	size_t		nitems;
	size_t		nitems_max;
	wstring_ty	**item;
};

int wstring_list_member _((wstring_list_ty *, wstring_ty *));
wstring_ty *wstring_list_to_wstring _((wstring_list_ty *, int, int, char *));
void wstring_to_wstring_list _((wstring_list_ty *, wstring_ty *, char *, int));
void wstring_list_prepend _((wstring_list_ty *, wstring_ty *));
void wstring_list_append _((wstring_list_ty *, wstring_ty *));
void wstring_list_append_unique _((wstring_list_ty *, wstring_ty *));
void wstring_list_copy _((wstring_list_ty *, wstring_list_ty *));
void wstring_list_delete _((wstring_list_ty *, wstring_ty *));
void wstring_list_free _((wstring_list_ty *));
void wstring_list_zero _((wstring_list_ty *));

int wstring_list_equal _((wstring_list_ty *, wstring_list_ty *));
int wstring_list_subset _((wstring_list_ty *, wstring_list_ty *));

#endif /* COMMON_WSTR_LIST_H */
