/*
 *	cook - file construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the builtin defined function
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <builtin/defined.h>
#include <error_intl.h>
#include <expr/position.h>
#include <opcode/context.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_defined - is a variable defined
 *
 * SYNOPSIS
 *	int builtin_defined(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Defined is a built-in function of cook, described as follows:
 *	This function requires a single argument,
 *	the name of a variable to be tested for existence.
 *
 * RETURNS
 *	It returns "1" (true) if the named variable is defined
 *	and "" (false) if it is not.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const opcode_context_ty *ocp;
{
	size_t		j;

	trace(("defined\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 1; j < args->nstrings; j++)
	{
		if (opcode_context_id_search(ocp, args->string[j]))
			string_list_append(result, str_true);
		else
			string_list_append(result, str_false);
	}
	return 0;
}


builtin_ty builtin_defined =
{
	"defined",
	interpret,
	interpret, /* script */
};
