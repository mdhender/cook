/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: interface definition for cook/graph/recipe.c
 */

#ifndef COOK_GRAPH_RECIPE_H
#define COOK_GRAPH_RECIPE_H

#include <main.h>

typedef struct graph_recipe_ty graph_recipe_ty;
struct graph_recipe_ty
{
	long		reference_count;
	int		id;
	struct recipe_ty *rp;
	struct match_ty *mp;
	struct graph_file_list_nrc_ty *input;
	struct graph_file_list_nrc_ty *output;
	long		input_satisfied;	/* used by graph_walk */
	long		input_uptodate;	/* used by graph_walk */
	struct opcode_context_ty *ocp;	/* used by graph_run */
	struct string_list_ty *single_thread;
	struct string_list_ty *host_binding;
	int		multi_forced; /* used by graph_walk */
};

graph_recipe_ty *graph_recipe_new _((struct recipe_ty *));
void graph_recipe_delete _((graph_recipe_ty *));
graph_recipe_ty *graph_recipe_copy _((graph_recipe_ty *));

int graph_recipe_getpid _((graph_recipe_ty *));
void graph_recipe_waited _((graph_recipe_ty *, int));

#endif /* COOK_GRAPH_RECIPE_H */
