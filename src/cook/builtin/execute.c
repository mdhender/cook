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
 * MANIFEST: functions to implement the builtin execute function
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <builtin/execute.h>
#include <error.h>
#include <error_intl.h>
#include <expr/position.h>
#include <option.h>
#include <os_interface.h>
#include <star.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_execute - execute a command
 *
 * SYNOPSIS
 *	int builtin_execute(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Exec is a built-in function of cook, described as follows:
 *	This function requires at least one argument, and
 *	executes the command given by the arguments.
 *
 * RETURNS
 *	If the executed command returns an error code the resulting value
 *	is "" (false), otherwise it is "1" (true).
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	string_list_ty	wl;
	int		j;
	string_ty	*s;
	int		silent;

	trace(("execute\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	if (args->nstrings < 2)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires one or more arguments")
		);
		sub_context_delete(scp);
		return -1;
	}
	string_list_constructor(&wl);
	for (j = 1; j < args->nstrings; j++)
		string_list_append(&wl, args->string[j]);
	option_set(OPTION_SILENT, OPTION_LEVEL_EXECUTE, 1);
	silent = option_test(OPTION_SILENT);
	if (silent)
		star_bang();
	else
	{
		s = wl2str(&wl, 0, wl.nstrings - 1, (char *)0);
		error_raw("%s", s->str_text);
		str_free(s);
	}
	j = os_execute(&wl, (string_ty *)0, 0);
	if (!silent)
		star_sync();
	option_undo_level(OPTION_LEVEL_EXECUTE);
	string_list_destructor(&wl);
	if (j < 0)
		return -1;
	s = (j ? str_false : str_true);
	string_list_append(result, s);
	return 0;
}


builtin_ty builtin_execute =
{
	"execute",
	interpret,
	interpret, /* script */
};
