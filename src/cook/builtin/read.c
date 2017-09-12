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
 * MANIFEST: functions to implement the read builtin function
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <builtin/read.h>
#include <error_intl.h>
#include <expr/position.h>
#include <stracc.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_read - get output of a command
 *
 * SYNOPSIS
 *	int builtin_read(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Collect is a built-in function of cook, described as follows:
 *	This function requires one or more arguments.
 *
 * RETURNS
 *	A word list containing the values of the output lines of the
 *	program given in the arguments.
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
    string_list_ty  *result;
    const string_list_ty *args;
    const expr_position_ty *pp;
    const struct opcode_context_ty *ocp;
{
    FILE	    *fp;
    string_ty	    *s;
    char	    *delim;
    stracc	    sa;

    trace(("read\n"));
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings != 2)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%S", args->string[0]);
	error_with_position(pp, scp, i18n("$name: requires one argument"));
	sub_context_delete(scp);
	return -1;
    }
    fp = fopen_and_check(args->string[1]->str_text, "r");
    stracc_constructor(&sa);
    delim = strchr(args->string[0]->str_text, '_') ? "\n" : "\n \t\f";
    for (;;)
    {
	int		c;

	for (;;)
	{
	    c = fgetc(fp);
	    if (c == EOF || !strchr(delim, c))
	       	break;
	}
	if (c == EOF)
	{
	    if (ferror(fp))
	    {
		error_intl_read(args->string[1]->str_text);
		stracc_destructor(&sa);
		fclose(fp);
		return -1;
	    }
	    break;
	}
	sa_open(&sa);
	for (;;)
	{
	    sa_char(&sa, c);
	    c = fgetc(fp);
	    if (c == EOF || strchr(delim, c))
	       	break;
	}
	s = sa_close(&sa);
	string_list_append(result, s);
	str_free(s);
	if (c == EOF)
	    break;
    }
    fclose_and_check(fp, args->string[1]->str_text);
    stracc_destructor(&sa);
    return 0;
}


builtin_ty builtin_read =
{
    "read",
    interpret,
    interpret, /* script */
};


builtin_ty builtin_read_lines =
{
    "read_lines",
    interpret,
    interpret, /* script */
};
