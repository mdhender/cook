/*
 *	cook - file construction tool
 *	Copyright (C) 1992, 1993, 1994, 1995, 1997, 199, 2001 Peter Miller;
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
 * MANIFEST: interface definition for common/str.c
 */

#ifndef STR_H
#define STR_H

#include <ac/stddef.h>
#include <ac/stdarg.h>
#include <main.h>

typedef unsigned long str_hash_ty;

typedef struct string_ty string_ty;
struct string_ty
{
	str_hash_ty	str_hash;
	string_ty	*str_next;
	long		str_references;
	size_t		str_length;
	char		str_text[1];
};

extern string_ty *str_true;
extern string_ty *str_false;

void str_initialize _((void));
string_ty *str_from_c _((const char *));
string_ty *str_n_from_c _((const char *, size_t));
string_ty *str_copy _((string_ty *));
void str_free _((string_ty *));
string_ty *str_catenate _((string_ty *, string_ty *));
string_ty *str_cat_three _((string_ty *, string_ty *, string_ty *));
int str_bool _((string_ty *));
string_ty *str_upcase _((string_ty *));
string_ty *str_downcase _((string_ty *));
string_ty *str_substitute _((string_ty *from, string_ty *to, string_ty *to_me));

#ifdef DEBUG
int str_valid _((string_ty *));
#endif

string_ty *str_field _((string_ty *s, int sep, int fldnum));
string_ty *str_format _((const char *, ...));
string_ty *str_vformat _((const char *, va_list));

#define str_equal(s1, s2) ((s1) == (s2))

string_ty *str_quote_shell _((string_ty *));
string_ty *str_quote_cook _((string_ty *, int));

#endif /* STR_H */
