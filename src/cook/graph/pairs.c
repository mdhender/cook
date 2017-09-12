/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001 Peter Miller;
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
 * MANIFEST: functions to print pair-wise file dependencies
 */

#include <ac/stdio.h>

#include <graph/file.h>
#include <graph/file_list.h>
#include <graph/pairs.h>
#include <graph/recipe.h>
#include <str.h>
#include <trace.h>


/*
 * NAME
 *	graph_recipe_pairs
 *
 * DESCRIPTION
 *	void graph_recipe_pairs(void);
 *
 * DESCRIPTION
 *	The graph_recipe_pairs function is used to print pair-wise file
 *	dependency information on the standard output.  The format is
 *	similar to lorder(1) output.  This can be used to draw
 *	dependency graphs.
 *
 * RETURNS
 *	done, always
 */

graph_walk_status_ty
graph_recipe_pairs(grp, gp)
	graph_recipe_ty	*grp;
	struct graph_ty	*gp;
{
	size_t		j, k;

	trace(("graph_recipe_pairs(grp = %08lX)\n{\n"/*}*/, (long)grp));
	for (j = 0; j < grp->output->nfiles; ++j)
	{
		for (k = 0; k < grp->input->nfiles; ++k)
		{
			printf
			(
				"%s %s\n",
				grp->output->item[j].file->filename->str_text,
				grp->input->item[k].file->filename->str_text
			);
		}
	}
	trace(("return done;\n"));
	trace((/*{*/"}\n"));
	return graph_walk_status_done;
}
