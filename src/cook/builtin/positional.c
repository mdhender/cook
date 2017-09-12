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
 * MANIFEST: functions to implement the builtin positional functions
 */

#include <builtin/positional.h>
#include <error_intl.h>
#include <expr/position.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_FILE - cookbook file name
 *
 * SYNOPSIS
 *	int builtin_FILE(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The builtin_FILE function implements the __FILE__ built-in
 *	function.  This function returns the name of the current logical
 *	file name.  This function requires zero arguments,
 *
 * RETURNS
 *	It returns the name of the current logical cookbook file.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.  It is the
 *	responsibility of the caller to dispose of the result when it is
 *	finished, with a string_list_destructor() call.
 */

static int FILE_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
FILE_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	trace(("builtin_FILE()\n{\n"/*}*/));
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
		trace((/*{*/"}\n"));
		return -1;
	}
	assert(pp->pos_name);
	string_list_append(result, pp->pos_name);
	trace((/*{*/"}\n"));
	return 0;
}


builtin_ty builtin_FILE =
{
	"__FILE__",
	FILE_interpret,
	FILE_interpret, /* script */
};


/*
 * NAME
 *	builtin_LINE - cookbook line number
 *
 * SYNOPSIS
 *	int builtin_LINE(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The builtin_LINE function implements the __LINE__ built-in
 *	function.  This function returns the line number within the
 *	current logical file name.  This function requires zero
 *	arguments.
 *
 * RETURNS
 *	It returns the line number within the current logical cookbook file.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.  It is the
 *	responsibility of the caller to dispose of the result when it is
 *	finished, with a string_list_destructor() call.
 */

static int LINE_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
LINE_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	string_ty	*s;

	trace(("builtin_FILE()\n{\n"/*}*/));
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
		trace((/*{*/"}\n"));
		return -1;
	}
	assert(pp->pos_name);
	s = str_format("%ld", (long)pp->pos_line);
	string_list_append(result, s);
	str_free(s);
	trace((/*{*/"}\n"));
	return 0;
}


builtin_ty builtin_LINE =
{
	"__LINE__",
	LINE_interpret,
	LINE_interpret, /* script */
};
