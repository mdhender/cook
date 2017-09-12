/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to determine if a graph is up-to-date
 */

#include <cook.h>
#include <graph/check.h>
#include <graph/file.h>
#include <graph/file_list.h>
#include <graph/recipe.h>
#include <opcode/context.h>
#include <option.h>
#include <recipe.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	graph_recipe_check
 *
 * SYNOPSIS
 *	graph_walk_status_ty graph_recipe_check(graph_recipe_ty *);
 *
 * DESCRIPTION
 *	The graph_recipe_check function is used to see if a recipe
 *	required building.  It does not actually build anything, it is
 *	simply used by graph_walk_inner to impliment the isit_uptodate
 *	built-in function.
 *
 * RETURNS
 *	graph_walk_status_ty
 *		error		something went wrong
 *		uptodate	no work needed
 *		done_stop	work required
 */

graph_walk_status_ty
graph_recipe_check(grp, gp)
	graph_recipe_ty	*grp;
	struct graph_ty	*gp;
{
	graph_walk_status_ty status;
	time_t		target_age;
	long		target_depth;
	int		up_to_date;
	time_t		need_age;
	size_t		j;
	opcode_context_ty *ocp;

	trace(("graph_recipe_check(grp = %08lX)\n{\n"/*}*/, (long)grp));
	status = graph_walk_status_uptodate;

	/*
	 * Flags apply to the precondition and to the ingredients
	 * evaluation.  That is why the grammar puts them first.
	 */
	recipe_flags_set(grp->rp);

	/*
	 * see of the recipe is forced to activate
	 */
	up_to_date = !option_test(OPTION_FORCE);

	/*
	 * age should be set to the worst case of all the targets
	 *
	 * The depth of the target search should be less than or equal
	 * to the depth of the worst (shallowest) ingredients search.
	 * This is to guarantee that when ingredients change they
	 * result in targets shallower in the path being updated.
	 */
	target_age = 0;
	target_depth = 32767;
	ocp = opcode_context_new(0, 0);
	for (j = 0; j < grp->output->nfiles; ++j)
	{
		graph_file_and_type_ty *gftp2;
		graph_file_ty	*gfp2;
		time_t		age2;
		long		depth2;
		edge_type_ty	type2;

		gftp2 = grp->output->item + j;
		gfp2 = gftp2->file;
		type2 = gftp2->edge_type;

		depth2 = 32767;
		age2 = cook_mtime_newest(ocp, gfp2->filename, &depth2, depth2);
		if (age2 < 0)
		{
			/* error message already printed */
			status = graph_walk_status_error;
			goto ret;
		}
		if (depth2 < target_depth)
			target_depth = depth2;
		if (!target_age || !(type2 & edge_type_exists))
			target_age = age2;
	}
	if (target_age == 0)
		up_to_date = 0;

	/*
	 * Look at the mtimes for each of the ingredients.
	 */
	need_age = 0;
	for (j = 0; j < grp->input->nfiles; ++j)
	{
		graph_file_and_type_ty *gftp2;
		graph_file_ty	*gfp2;
		time_t		age2;
		long		depth2;

		gftp2 = grp->input->item + j;
		gfp2 = gftp2->file;
		depth2 = 32767;
		age2 = cook_mtime_oldest(ocp, gfp2->filename, &depth2, depth2);
		if (age2 < 0)
		{
			/* error message already printed */
			status = graph_walk_status_error;
			goto ret;
		}

		/*
		 * track the youngest ingredient,
		 * in case we need to adjust the targets' times
		 */
		if (age2 > need_age)
			need_age = age2;

		/*
		 * This function is only called AFTER an ingredient has
		 * been derived. It will exist (well, almost: it could
		 * be a phony) and so the mtime in the stat cache will
		 * have been set.
		 */
		assert(age2 != 0);

		/*
		 * Check to see if this ingredient invalidates the
		 * target, based on its age.  (Don't say anything if we
		 * already know it's out of date.)
		 */
		if (gfp2->done)
			up_to_date = 0;

		/*
		 * Check to see if this ingredient invalidates the
		 * target, based on its age.  (Don't say anything if we
		 * already know it's out of date.)
		 */
		if (age2 >= target_age)
			up_to_date = 0;

		/*
		 * Check to see if this ingredient invalidates the
		 * target, based on its depth.  (Don't say anything if we
		 * already know its out of date.)
		 */
		if (depth2 < target_depth)
			up_to_date = 0;
	}

	/*
	 * See if we need to perform the actions attached to this
	 * recipe.  Don't actually do anything, just indicate that we
	 * would do something if we were allowed to.
	 */
	if (!up_to_date)
		status = graph_walk_status_done_stop;

	/*
	 * cancel the recipe flags
	 */
ret:
	opcode_context_delete(ocp);
	option_undo_level(OPTION_LEVEL_RECIPE);

	trace(("return %s;\n", graph_walk_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
}
