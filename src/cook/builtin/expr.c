/*
 *	cook - file construction tool
 *	Copyright (C) 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the builtin expr function
 *
 * The builtin function all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <builtin/expr.h>
#include <builtin/expr_parse.h>
#include <str_list.h>
#include <trace.h>


static int interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	string_ty	*s;

	trace(("expr\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	builtin_expr_parse_begin(args, pp);
	if (builtin_expr_parse_parse())
		return -1;
	s = builtin_expr_parse_end();
	string_list_append(result, s);
	str_free(s);
	return 0;
}


builtin_ty builtin_expr =
{
	"expr",
	interpret,
	interpret, /* script */
};
