/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the builtin basename function
 */

#include <ac/string.h>

#include <builtin/basename.h>
#include <str_list.h>


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
	char		*cp;
	char		*ep;

	for (j = 1; j < arg->nstrings; ++j)
	{
		s = arg->string[j];
		cp = strrchr(s->str_text, '/');
		if (!cp)
			cp = s->str_text;
		ep = strrchr(cp, '.');
		if (!ep || ep == cp)
			string_list_append(result, s);
		else
		{
			s = str_n_from_c(s->str_text, ep - s->str_text);
			string_list_append(result, s);
			str_free(s);
		}
	}
	return 0;
}


builtin_ty builtin_basename =
{
	"basename",
	interpret,
	interpret, /* script */
};
