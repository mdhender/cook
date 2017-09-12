/*
 *	cook - file construction tool
 *	Copyright (C) 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate doss
 */

#include <ac/ctype.h>

#include <builtin/dos.h>
#include <stracc.h>
#include <str_list.h>
#include <trace.h>


static string_ty *dos_path _((string_ty *));

static string_ty *
dos_path(arg)
	string_ty	*arg;
{
	stracc		sa;
	string_ty	*result;
	const char	*cp;

	stracc_constructor(&sa);
	sa_open(&sa);
	if
	(
		arg->str_text[0] == '/'
	&&
		arg->str_text[1] == '/'
	&&
		isalpha((unsigned char)arg->str_text[2])
	&&
		(
			arg->str_text[3] == '/'
		||
			arg->str_text[3] == '\0'
		)
	)
	{
		sa_char(&sa, arg->str_text[2]);
		sa_char(&sa, ':');
		cp = arg->str_text + 3;
	}
	else
		cp = arg->str_text;
	while (*cp)
	{
		if (*cp == '/')
			sa_char(&sa, '\\');
		else
			sa_char(&sa, *cp);
		++cp;
	}
	result = sa_close(&sa);
	stracc_destructor(&sa);
	return result;
}


static string_ty *dos_path_undo _((string_ty *));

static string_ty *
dos_path_undo(arg)
	string_ty	*arg;
{
	stracc		sa;
	string_ty	*result;
	const char	*cp;

	stracc_constructor(&sa);
	sa_open(&sa);
	if
	(
		isalpha((unsigned char)arg->str_text[0])
	&&
		arg->str_text[1] == ':'
	)
	{
		sa_char(&sa, '/');
		sa_char(&sa, '/');
		sa_char(&sa, arg->str_text[0]);
		cp = arg->str_text + 2;
		if (*cp != '\\')
			sa_char(&sa, '/');
	}
	else
		cp = arg->str_text;
	while (*cp)
	{
		if (*cp == '\\')
			sa_char(&sa, '/');
		else
			sa_char(&sa, *cp);
		++cp;
	}
	result = sa_close(&sa);
	stracc_destructor(&sa);
	return result;
}


/*
 * NAME
 *	builtin_dos_path - dos_path strings
 *
 * SYNOPSIS
 *	int builtin_dos_path(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Defined is a built-in function of cook, described as follows:
 *	This function requires one or more arguments,
 *	which will be converted from unix paths to dos paths.
 *
 * RETURNS
 *	It returns the arguments dos paths.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int dos_path_interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
dos_path_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;

	trace(("dos_path\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 1; j < args->nstrings; j++)
	{
		string_ty *s;

		s = dos_path(args->string[j]);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_dos_path =
{
	"dos-path",
	dos_path_interpret,
	dos_path_interpret, /* script */
};


/*
 * NAME
 *	builtin_dos_path_undo - dos_path_undo strings
 *
 * SYNOPSIS
 *	int builtin_dos_path_undo(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Defined is a built-in function of cook, described as follows:
 *	This function requires one or more arguments,
 *	which will be converted from dos paths to unix paths.
 *
 * RETURNS
 *	It returns the arguments unix paths.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int dos_path_undo_interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
dos_path_undo_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;

	trace(("dos_path_undo\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 1; j < args->nstrings; j++)
	{
		string_ty	*s;

		s = dos_path_undo(args->string[j]);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_dos_path_undo =
{
	"dos-path-undo",
	dos_path_undo_interpret,
	dos_path_undo_interpret, /* script */
};


builtin_ty builtin_un_dos_path =
{
	"un-dos-path",
	dos_path_undo_interpret,
	dos_path_undo_interpret, /* script */
};
