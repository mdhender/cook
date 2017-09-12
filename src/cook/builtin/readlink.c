/*
 *	cook - file construction tool
 *	Copyright (C) 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate readlinks
 */

#include <ac/unistd.h>

#include <builtin/readlink.h>
#include <error_intl.h>
#include <expr/position.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_readlink - builtin function for reading symbolic links
 *
 * SYNOPSIS
 *	int builtin_readlink(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The builtin_readlink function is used to implement the
 *	"readlink" builtin function of cook to read symbolic links.
 *
 * RETURNS
 *	int; 0 on success, -1 on any error
 *
 * CAVEAT
 *	This function is designed to be used as a "builtin" function.
 */

static int interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty	*args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;
	string_ty	*s;
	int		nbytes;
	char		buffer[2000];

	trace(("readlink(result = %08X, args = %08X)\n{\n"/*}*/, result, args));
	for (j = 1; j < args->nstrings; ++j)
	{
		s = args->string[j];
		nbytes = readlink(s->str_text, buffer, sizeof(buffer));
		if (nbytes < 0)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%S", s);
			error_with_position
			(
				pp,
				scp,
				i18n("readlink \"$filename\": $errno")
			);
			sub_context_delete(scp);
			return -1;
		}
		s = str_n_from_c(buffer, nbytes);
		string_list_append(result, s);
		str_free(s);
	}
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


builtin_ty builtin_readlink =
{
	"readlink",
	interpret,
	interpret, /* script */
};
