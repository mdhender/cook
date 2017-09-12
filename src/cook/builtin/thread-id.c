/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the builtin thread-id function
 */

#include <builtin/thread-id.h>
#include <error_intl.h>
#include <expr/position.h>
#include <opcode/context.h>
#include <str_list.h>
#include <trace.h>


static int interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const opcode_context_ty *));

static int
interpret(result, arg, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *arg;
	const expr_position_ty *pp;
	const opcode_context_ty *ocp;
{
	string_ty	*s;

	trace(("thread-id\n"));
	if (arg->nstrings != 1)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", arg->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires no arguments")
		);
		sub_context_delete(scp);
		return -1;
	}
	s = str_format("%ld", (long)ocp->thread_id);
	string_list_append(result, s);
	str_free(s);
	return 0;
}


builtin_ty builtin_thread_id =
{
	"thread-id",
	interpret,
	interpret, /* script */
};
