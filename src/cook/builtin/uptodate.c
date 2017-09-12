/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate uptodates
 */

#include <builtin/uptodate.h>
#include <desist.h>
#include <graph.h>
#include <graph/build.h>
#include <graph/file.h>
#include <graph/recipe_list.h>
#include <graph/stats.h>
#include <graph/walk.h>
#include <option.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


/*
 * NAME
 *	builtin_uptodate - test if files are up to date
 *
 * SYNOPSIS
 *	int builtin_uptodate(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The uptodate function is a built-in of cook, which may be used
 *	to determine if files are up-to-date.
 *
 * RETURNS
 *	A word list containing the names of the up-to-date files, or
 *	empty if none of them are up-to-date.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	size_t		j;
	int		retval;
	graph_ty	*gp;

	if (args->nstrings <= 1)
		return 0;

	/*
	 * set interrupts to catch
	 *
	 * Note that tee(1) [see listing.c] must ignore them
	 * for the generated messages to appear in the log file.
	 */
	trace(("interpret()\n{\n"));
	desist_enable();
	assert(result);
	assert(args);
	assert(args->nstrings);
	retval = 0;

	/*
	 * Build the dependency graph.
	 */
	gp = graph_new();
	for (j = 1; j < args->nstrings; ++j)
	{
		graph_build_status_ty gb_status;

		trace(("\"%s\"\n", args->string[j]->str_text));
		gb_status =
			graph_build
			(
				gp,
				args->string[j],
				graph_build_preference_backtrack,
				0
			);
		switch (gb_status)
		{
		case graph_build_status_error:
			retval = -1;
			break;

		case graph_build_status_backtrack:
			/* don't know how is OK */
			break;

		case graph_build_status_success:
			break;
		}
	}

	/*
	 * print the graph statistics
	 */
	trace(("mark\n"));
	if (option_test(OPTION_REASON))
		graph_print_statistics(gp);

	/*
	 * Walk the dependency graph.
	 */
	trace(("mark\n"));
	if (retval >= 0)
	{
		retval = graph_isit_uptodate(gp);
		if (retval >= 0)
		{
			/* OK, so exactly which ones are up to date? */
			for (j = 1; j < args->nstrings; ++j)
			{
				graph_file_ty	*gfp;
				string_ty	*fn;

				fn = args->string[j];
				trace(("\"%s\"\n", fn->str_text));
				gfp = symtab_query(gp->already, fn);
				trace(("gfp = %08lX\n", (long)gfp));
				assert(gfp);
				trace(("gfp->input_uptodate = %d\n",
					gfp->input_uptodate));
				trace(("gfp->input->nrecipes = %d\n",
					gfp->input->nrecipes));
				if
				(
					gfp
				&&
				     gfp->input_uptodate >= gfp->input->nrecipes
				)
					string_list_append(result, fn);
			}
		}
	}

	/*
	 * Release resources held by the graph.
	 */
	graph_delete(gp);

	/*
	 * return the result
	 */
	trace(("}\n"));
	return retval;
}


builtin_ty builtin_uptodate =
{
	"uptodate",
	interpret,
	interpret, /* script */
};
