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
 * MANIFEST: interface definition for common/wstr.c
 */

#ifndef COMMON_WSTR_H
#define COMMON_WSTR_H

#include <ac/stdlib.h>
#include <main.h>

typedef unsigned long wstr_hash_ty;

typedef struct wstring_ty wstring_ty;
struct wstring_ty
{
	wstr_hash_ty	wstr_hash;
	wstring_ty	*wstr_next;
	long		wstr_references;
	size_t		wstr_length;
	wchar_t		wstr_text[1];
};

wstring_ty *wstr_from_c _((const char *));
wstring_ty *wstr_from_wc _((const wchar_t *));
wstring_ty *wstr_n_from_c _((const char *, size_t));
wstring_ty *wstr_n_from_wc _((const wchar_t *, size_t));
wstring_ty *wstr_copy _((wstring_ty *));
void wstr_free _((wstring_ty *));
wstring_ty *wstr_catenate _((wstring_ty *, wstring_ty *));
wstring_ty *wstr_cat_three _((wstring_ty *, wstring_ty *, wstring_ty *));
wstring_ty *wstr_to_upper _((const wstring_ty *));
wstring_ty *wstr_to_lower _((const wstring_ty *));
wstring_ty *wstr_to_ident _((const wstring_ty *));
void wstr_to_mbs _((wstring_ty *, char **, size_t *));
int wstr_equal _((wstring_ty *, wstring_ty *));

#ifndef DEBUG
#define wstr_equal(s1, s2) ((s1) == (s2))
#endif

struct string_ty;
struct string_ty *wstr_to_str _((wstring_ty *));
wstring_ty *str_to_wstr _((struct string_ty *));

#endif /* COMMON_WSTR_H */
