/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate resolves
 */

#include <builtin/resolve.h>
#include <cook.h>
#include <error.h> /* for assert */
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_resolve - resolve search path
 *
 * SYNOPSIS
 *	int builtin_resolve(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The resolve function is a built-in of cook, described as follows:
 *	This builtin function is used to resolve file names when using
 *	the search_list variable to locate files.  This builtin
 *	function produces resolved file names as output.  This is
 *	useful when taking partial copies of a source to perform
 *	controlled updates.  The targets of recipes are always cooked
 *	into the current directory.
 *
 * RETURNS
 *	A word list containing the resolved names.
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
	trace(("resolve\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	return cook_mtime_resolve(ocp, result, args, 1);
}


builtin_ty builtin_resolve =
{
	"resolve",
	interpret,
	interpret, /* script */
};
