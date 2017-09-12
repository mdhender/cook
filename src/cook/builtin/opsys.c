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
 * MANIFEST: functions to implement the builtin os function
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <ac/sys/utsname.h>

#include <builtin/opsys.h>
#include <error_intl.h>
#include <expr/position.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


static symtab_ty *stp;


static void find_out_about_system _((void));

static void
find_out_about_system()
{
	struct utsname	uts;
	string_ty	*name;
	string_ty	*value;

	stp = symtab_alloc(10);

	uname(&uts);

	value = str_from_c(uts.sysname);
	name = str_from_c("name");
	symtab_assign(stp, name, value);
	str_free(name);
	name = str_from_c("system");
	symtab_assign(stp, name, value);
	str_free(name);

	value = str_from_c(uts.nodename);
	name = str_from_c("node");
	symtab_assign(stp, name, value);
	str_free(name);
	name = str_from_c("nodename");
	symtab_assign(stp, name, value);
	str_free(name);
	name = str_from_c("host");
	symtab_assign(stp, name, value);
	str_free(name);
	name = str_from_c("hostname");
	symtab_assign(stp, name, value);
	str_free(name);

	value = str_from_c(uts.release);
	name = str_from_c("release");
	symtab_assign(stp, name, value);
	str_free(name);

	value = str_from_c(uts.version);
	name = str_from_c("version");
	symtab_assign(stp, name, value);
	str_free(name);

	value = str_from_c(uts.machine);
	name = str_from_c("machine");
	symtab_assign(stp, name, value);
	str_free(name);
}


/*
 * NAME
 *	builtin_opsys - describe operating system
 *
 * SYNOPSIS
 *	int builtin_opsys(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Operating_system is a built-in function of cook, described as follows:
 *	This function must have zero or more arguments.
 *
 * RETURNS
 *	The resulting wordlist contains the values of various
 *	attributes of the operating system, as named in the arguments.
 *	If no attributes are named "name" is assumed.
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
	size_t		j;

	trace(("opsys\n"));
	if (!stp)
	{
		find_out_about_system();
		assert(stp);
	}
	if (args->nstrings < 2)
	{
		static string_ty *name;
		string_ty	*value;

		if (!name)
			name = str_from_c("name");
		value = symtab_query(stp, name);
		assert(value);
		string_list_append(result, value);
	}
	for (j = 1; j < args->nstrings; j++)
	{
		string_ty	*name;
		string_ty	*value;

		name = args->string[j];
		value = symtab_query(stp, name);
		if (!value)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", args->string[0]);
			sub_var_set(scp, "ATTRibute", "%S", name);
			error_with_position
			(
				pp,
				scp,
				i18n("$name: unknown \"$attribute\" attribute")
			);
			sub_context_delete(scp);
			return -1;
		}
		string_list_append(result, value);
	}
	return 0;
}


builtin_ty builtin_operating_system =
{
	"operating_system",
	interpret,
	interpret, /* script */
};


builtin_ty builtin_os =
{
	"os",
	interpret,
	interpret, /* script */
};
