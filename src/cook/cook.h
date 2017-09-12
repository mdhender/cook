/*
 *	cook - file construction tool
 *	Copyright (C) 1993, 1994, 1995, 1996, 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: interface definition for cook/cook.c
 */

#ifndef COOK_COOK_H
#define COOK_COOK_H

#include <ac/time.h>

#include <main.h>

struct opcode_context_ty; /* existence */
struct recipe_ty; /* existence */
struct string_ty; /* existence */
struct string_list_ty; /* existence */

int cook _((struct string_list_ty *));
int cook_pairs _((struct string_list_ty *));
int cook_script _((struct string_list_ty *));
int cook_web _((struct string_list_ty *));

time_t cook_mtime_oldest _((const struct opcode_context_ty *,
	struct string_ty *, long *, long));
time_t cook_mtime_newest _((const struct opcode_context_ty *,
	struct string_ty *, long *, long));
int cook_mtime_resolve _((const struct opcode_context_ty *,
	struct string_list_ty *, const struct string_list_ty *, int));

void cook_auto _((struct string_list_ty *));
int cook_auto_required _((void));
void cook_reset _((void));
void cook_find_default _((struct string_list_ty *));
void cook_search_list _((const struct opcode_context_ty *,
	struct string_list_ty *slp));

void cook_explicit_append _((struct recipe_ty *));
const struct recipe_list_ty *cook_explicit_by_name _((struct string_ty *));
void cook_implicit_append _((struct recipe_ty *));
struct recipe_ty *cook_implicit_nth _((long));
struct recipe_ty *cook_implicit_nth_by_name _((long, struct string_ty *));

#endif /* COOK_COOK_H */
