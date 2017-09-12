/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement builtin word function
 */

#include <ac/ctype.h>

#include <builtin/word.h>
#include <error_intl.h>
#include <expr/position.h>
#include <str_list.h>
#include <trace.h>


static long number _((char *));

static long
number(s)
	char		*s;
{
	long		n;

	n = 0;
	while (isspace(*s))
		++s;
	while (isdigit(*s))
		n = n * 10 + *s++ - '0';
	while (isspace(*s))
		++s;
	if (*s)
		return 0;
	return n;
}


static int interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, arg, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *arg;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	long		n;

	trace(("word\n"));
	if (arg->nstrings < 2)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", arg->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires one or more arguments")
		);
		sub_context_delete(scp);
		return -1;
	}
	n = number(arg->string[1]->str_text);
	if (n <= 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", arg->string[0]);
		sub_var_set(scp, "Number", "1");
		error_with_position
		(
			pp,
			scp,
	      i18n("$name: argument $number: must be a positive decimal number")
		);
		sub_context_delete(scp);
		return -1;
	}
	if (n + 1 < arg->nstrings)
		string_list_append(result, arg->string[n + 1]);
	return 0;
}


builtin_ty builtin_word =
{
	"word",
	interpret,
	interpret, /* script */
};
