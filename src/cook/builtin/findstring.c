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
 * MANIFEST: functions to implement the findstring builtin function
 */

#include <ac/string.h>

#include <builtin/findstring.h>
#include <error_intl.h>
#include <expr/position.h>
#include <str_list.h>
#include <trace.h>


static int contains _((string_ty *, string_ty *));

static int
contains(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	size_t		len;
	size_t		j;

	if (s1->str_length == 0)
		return 1;
	if (s1->str_length > s2->str_length)
		return 0;
	len = s2->str_length - s1->str_length;
	for (j = 0; j <= len; ++j)
	{
		if (!memcmp(s2->str_text + j, s1->str_text, s1->str_length))
			return 1;
	}
	return 0;
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
	size_t		j;

	trace(("findstring\n"));
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
	for (j = 2; j < arg->nstrings; ++j)
	{
		if (contains(arg->string[1], arg->string[j]))
			string_list_append(result, arg->string[1]);
		else
			string_list_append(result, str_false);
	}
	return 0;
}


builtin_ty builtin_findstring =
{
	"findstring",
	interpret,
	interpret, /* script */
};
