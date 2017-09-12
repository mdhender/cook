/*
 *	cook - file construction tool
 *	Copyright (C) 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to implement the interior_files builtin function
 */

#include <builtin/interi_files.h>
#include <error_intl.h>
#include <expr/position.h>
#include <graph.h>
#include <opcode/context.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_interior_files - graph details
 *
 * SYNOPSIS
 *	int builtin_interior_files(string_list_ty *result,
 *		string_list_ty *args);
 *
 * DESCRIPTION
 *	Interior_files is a built-in function of cook, described as
 *	follows: This function requires no arguments.  Interior_files
 *	returns the files in the dependency graph which are derived files,
 *	i.e. are interior to the graph.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int interior_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const opcode_context_ty *));

static int
interior_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const opcode_context_ty *ocp;
{
	trace(("interior_files\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	if (args->nstrings != 1)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires no arguments")
		);
		sub_context_delete(scp);
		return -1;
	}

	/*
	 * Only meaningful *inside* a recipe body.
	 */
	if (!ocp->gp)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: only meaningful inside recipe body")
		);
		sub_context_delete(scp);
		return -1;
	}

	/*
	 * ask for the info
	 */
	graph_interior_files(ocp->gp,  result);
	return 0;
}


builtin_ty builtin_interior_files =
{
	"interior_files",
	interior_interpret,
	interior_interpret, /* script */
};


/*
 * NAME
 *	builtin_leaf_files - graph details
 *
 * SYNOPSIS
 *	int builtin_leaf_files(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Leaf_files is a built-in function of cook, described as follows:
 *	This function requires no arguments.	Leaf_files returns
 *	the files in the dependency graph which are not derived files,
 *	i.e. are leaves of the graph.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int leaf_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const opcode_context_ty *));

static int
leaf_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const opcode_context_ty *ocp;
{
	trace(("leaf_files\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	if (args->nstrings != 1)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires no arguments")
		);
		sub_context_delete(scp);
		return -1;
	}

	/*
	 * Only meaningful *inside* a recipe body.
	 */
	if (!ocp->gp)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: only meaningful inside recipe body")
		);
		sub_context_delete(scp);
		return -1;
	}

	/*
	 * ask for the info
	 */
	graph_leaf_files(ocp->gp,  result);
	return 0;
}


builtin_ty builtin_leaf_files =
{
	"leaf_files",
	leaf_interpret,
	leaf_interpret, /* script */
};
