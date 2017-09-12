/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 2003 Peter Miller;
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
 * MANIFEST: interface definition for cook/graph/file_pair.c
 */

#ifndef COOK_GRAPH_FILE_PAIR_H
#define COOK_GRAPH_FILE_PAIR_H

#include <main.h>

struct expr_position_ty; /* existence */
struct string_list_ty; /* existence */
struct string_ty; /* existence */

typedef struct graph_file_pair_ty graph_file_pair_ty;
struct graph_file_pair_ty
{
	struct symtab_ty *stp;
	struct symtab_ty *foreign_derived;
};

graph_file_pair_ty *graph_file_pair_new _((struct string_list_ty *));
void graph_file_pair_delete _((graph_file_pair_ty *));
void graph_file_pair_remember _((graph_file_pair_ty *, struct string_ty *,
	struct string_ty *, const struct expr_position_ty *));
void graph_file_pair_remember_tlist _((graph_file_pair_ty *,
	struct string_list_ty *, struct string_ty *,
	const struct expr_position_ty *));
void graph_file_pair_remember_lists _((graph_file_pair_ty *,
	struct string_list_ty *, struct string_list_ty *,
	const struct expr_position_ty *));
void graph_file_pair_check _((graph_file_pair_ty *, struct string_ty *,
	struct string_ty *, struct graph_ty *));
void graph_file_pair_foreign_derived _((graph_file_pair_ty *,
	const struct string_list_ty *));
int graph_file_pair_exists _((graph_file_pair_ty *, struct string_ty *,
	struct string_ty *));

#endif /* COOK_GRAPH_FILE_PAIR_H */
