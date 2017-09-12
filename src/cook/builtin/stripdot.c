/*
 *	cook - file construction tool
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate stripdots
 */

#include <builtin/stripdot.h>
#include <str_list.h>
#include <strip_dot.h>


static int interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, arg, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *arg;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	size_t		j;
	string_ty	*s;

	for (j = 1; j < arg->nstrings; ++j)
	{
		/*
                 * The strip_dot() function is conditional on the ``set
                 * (no)stripdot;'' control.  The strip_dot_inner function
                 * is not.
		 */
		s = strip_dot_inner(arg->string[j]);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_stripdot =
{
	"stripdot",
	interpret,
	interpret, /* script */
};
