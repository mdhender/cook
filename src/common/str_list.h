/*
 *	cook - file construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997, 2000, 2001 Peter Miller;
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
 * MANIFEST: interface definition for common/str_list.c
 */

#ifndef COMMON_STR_LIST_H
#define COMMON_STR_LIST_H

#include <str.h>

typedef struct string_list_ty string_list_ty;
struct	string_list_ty
{
	size_t		nstrings;
	size_t		nstrings_max;
	string_ty	**string;
};

int string_list_member _((const string_list_ty *, string_ty *));
int string_list_intersect _((const string_list_ty *, const string_list_ty *));
string_ty *wl2str_respect_empty _((const string_list_ty *, int, int,
	const char *, int));
string_ty *wl2str _((const string_list_ty *, int, int, const char *));
void str2wl _((string_list_ty *, string_ty *, char *, int));
void string_list_append _((string_list_ty *, string_ty *));
void string_list_append_list _((string_list_ty *, const string_list_ty *));
void string_list_prepend _((string_list_ty *, string_ty *));
void string_list_append_unique _((string_list_ty *, string_ty *));
void string_list_append_list_unique _((string_list_ty *,
	const string_list_ty *));
void string_list_copy_constructor _((string_list_ty *, const string_list_ty *));
void string_list_remove _((string_list_ty *, string_ty *));
void string_list_remove_list _((string_list_ty *, const string_list_ty *));
void string_list_destructor _((string_list_ty *));
void string_list_constructor _((string_list_ty *));

string_list_ty *string_list_new _((void));
string_list_ty *string_list_new_copy _((const string_list_ty *));
void string_list_delete _((string_list_ty *));
int string_list_bool _((const string_list_ty *));
void string_list_sort _((string_list_ty *));

#endif /* COMMON_STR_LIST_H */
