/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001, 2004 Peter Miller;
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
 * MANIFEST: functions to implement the write builtin function
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <builtin/write.h>
#include <error_intl.h>
#include <expr/position.h>
#include <fflush_slow.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	builtin_write - builtin function for writing files
 *
 * SYNOPSIS
 *	int builtin_write(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The builtin_write function is used to implement the
 *	"write" builtin function of cook to write text files.
 *
 * RETURNS
 *	int; 0 on success, -1 on any error
 *
 * CAVEAT
 *	This function is designed to be used as a "builtin" function.
 */

static int interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	string_ty	*fn;
	FILE		*fp;
	size_t		j;
	int		retval;

	trace(("write::interpret(result = %08X, args = %08X)\n{\n"/*}*/,
		result, args));
	retval = 0;

	/*
	 * make sure we have at least the file name
	 */
	if (args->nstrings < 2)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires one or more arguments")
		);
		sub_context_delete(scp);
		retval = -1;
		goto dead;
	}

	/*
	 * open the file
	 *	on windows NT, it's specifically a text file
	 */
	fn = args->string[1];
	fp = fopen(fn->str_text, "w");
	if (!fp)
	{
		error_intl_open(fn->str_text);
		retval = -1;
		goto dead;
	}

	/*
	 * write the words, one per line
	 */
	for (j = 2; j < args->nstrings; ++j)
	{
		string_ty	*s;

		s = args->string[j];
		fwrite(s->str_text, 1, s->str_length, fp);
		if (!s->str_length || s->str_text[s->str_length - 1] != '\n')
			fputc('\n', fp);
		if (ferror(fp))
		{
			error_intl_write(fn->str_text);
			fclose(fp);
			retval = -1;
			goto dead;
		}
	}

	/*
	 * finish up writing the file
	 */
	if (fflush_slowly(fp))
	{
		error_intl_write(fn->str_text);
		fclose(fp);
		retval = -1;
		goto dead;
	}
	if (fclose(fp))
	{
		error_intl_close(fn->str_text);
		retval = -1;
	}

	/*
	 * all done
	 */
	dead:
	trace(("return %d;\n", retval));
	trace((/*{*/"}\n"));
	return retval;
}


static int strliststr _((const string_list_ty *, string_ty *));

static int
strliststr(slp, s)
	const string_list_ty *slp;
	string_ty	*s;
{
	size_t		j;

	for (j = 0; j < slp->nstrings; ++j)
		if (strstr(slp->string[j]->str_text, s->str_text))
			return 1;
	return 0;
}


static int script _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const struct opcode_context_ty *));

static int
script(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	size_t		j;
	int		retval;
	string_ty	*the_end;
	static int	the_end_n;
	string_ty	*s;

	trace(("write::script(result = %08X, args = %08X)\n{\n"/*}*/,
		result, args));
	retval = 0;

	/*
	 * make sure we have at least the file name
	 */
	if (args->nstrings < 2)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires one or more arguments")
		);
		sub_context_delete(scp);
		retval = -1;
		goto dead;
	}

	/*
	 * find a terminator
	 */
	the_end = 0;
	for (;;)
	{
		int n = ++the_end_n;
		the_end = str_format("the-end-%d", n);
		if (!strliststr(args, the_end))
			break;
		str_free(the_end);
	}
	assert(the_end);

	/*
	 * Write out the start of the here-document
	 */
	s = str_quote_shell(args->string[1]);
	printf("cat > %s << '%s'\n", s->str_text, the_end->str_text);
	str_free(s);

	/*
	 * write the words, one per line
	 */
	for (j = 2; j < args->nstrings; ++j)
	{

		s = args->string[j];
		fwrite(s->str_text, 1, s->str_length, stdout);
		if (!s->str_length || s->str_text[s->str_length - 1] != '\n')
			fputc('\n', stdout);
	}

	/*
	 * write the end of the here-document
	 */
	printf("%s\n", the_end->str_text);
	str_free(the_end);

	/*
	 * all done
	 */
	dead:
	trace(("return %d;\n", retval));
	trace((/*{*/"}\n"));
	return retval;
}


builtin_ty builtin_write =
{
	"write",
	interpret,
	script,
};
