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
 * MANIFEST: interface definition for cook/graph/file.c
 */

#ifndef COOK_GRAPH_FILE_H
#define COOK_GRAPH_FILE_H

#include <main.h>

typedef struct graph_file_ty graph_file_ty;
struct graph_file_ty
{
	long		reference_count;
	struct string_ty *filename;
	struct graph_recipe_list_nrc_ty *input;
	struct graph_recipe_list_nrc_ty *output;
	long		pending;
	int		previous_backtrack;
	int		previous_error;
	long		mtime_oldest;	/* used by graph_recipe_run */
	long		input_satisfied; /* used by graph_walk */
	long		done;		/* used by graph_walk */
	long		input_uptodate; /* used by graph_walk */
	int		primary_target;
};

graph_file_ty *graph_file_new _((struct string_ty *));
void graph_file_delete _((graph_file_ty *));
graph_file_ty *graph_file_copy _((graph_file_ty *));

#endif /* COOK_GRAPH_FILE_H */
