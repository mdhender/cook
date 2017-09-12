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
 * MANIFEST: functions to implement the home builtin function
 */

#include <ac/stdlib.h>
#include <pwd.h>

#include <builtin/home.h>
#include <error_intl.h>
#include <expr/position.h>
#include <home_directo.h>
#include <str_list.h>
#include <trace.h>


static int interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, arg, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *arg;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	string_ty	*s;
	size_t		j;
	struct passwd	*pw;

	trace(("home\n"));
	if (arg->nstrings < 2)
	{
		const char	*cp;

		cp = home_directory();
		assert(cp);
		s = str_from_c(cp);
		string_list_append(result, s);
		str_free(s);
		return 0;
	}
	for (j = 1; j < arg->nstrings; ++j)
	{
		s = arg->string[j];
		pw = getpwnam(s->str_text);
		if (!pw)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", s);
			error_with_position
			(
				pp,
				scp,
				i18n("user \"$name\" unknown")
			);
			sub_context_delete(scp);
			return -1;
		}
		s = str_from_c(pw->pw_dir);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_home =
{
	"home",
	interpret,
	interpret, /* script */
};
