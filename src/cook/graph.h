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
 * MANIFEST: interface definition for cook/graph.c
 */

#ifndef COOK_GRAPH_H
#define COOK_GRAPH_H

#include <main.h>

typedef struct graph_ty graph_ty;
struct graph_ty
{
	/*
	 * The try_list is a list of files that were not used, and
	 * subsequently backtracked.  This list is printed out in the
	 * error message when cook doesn't know how.
	 */
	struct string_list_ty *try_list;

	/*
	 * Collect statistics about building the graph.  They were
	 * originally for debugging, but they look as impressive as
	 * hell, so I kept them.
	 */
	struct
	{
		long	backtrack_bad_path;
		long	backtrack_by_ingredient;
		long	backtrack_cache;
		long	error_by_ingredient;
		long	error_cache;
		long	error_in_expr;
		long	explicit_applicable;
		long	explicit_ingredients_applicable;
		long	explicit_ingredients_not_applicable;
		long	explicit_not_applicable;
		long	implicit_applicable;
		long	implicit_ingredients_applicable;
		long	implicit_ingredients_not_applicable;
		long	implicit_not_applicable;
		long	infinite_loop;
		long	inhibit_self_recursion;
		long	leaf_error;
		long	leaf_backtrack;
		long	leaf_exists;
		long	pattern_match_query;
		long	phony;
		long	precondition_rejection;
		long	success;
		long	success_reuse;
	}
			statistic;

	/*
	 * Symbol table of files already considered.
	 */
	struct symtab_ty *already;

	/*
	 * The list of recipe instances used in this graph.
	 */
	struct graph_recipe_list_ty *already_recipe;

	/*
	 * Used to remember file pairs when checking for essential
	 * information residing only in dependency files.
	 */
	struct graph_file_pair_ty *file_pair;
};

graph_ty *graph_new _((void));
void graph_delete _((graph_ty *));

struct string_ty; /* existence */
int graph_file_leaf_p _((graph_ty *, struct string_ty *));

struct string_list_ty; /* existence */
void graph_interior_files _((graph_ty *, struct string_list_ty *));
void graph_leaf_files _((graph_ty *, struct string_list_ty *));

#endif /* COOK_GRAPH_H */
