/*
 *	cook - file construction tool
 *	Copyright (C) 1992, 1993, 1994, 1997, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to implement the builtin text functions
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <ac/stdlib.h>
#include <ac/string.h>

#include <builtin/text.h>
#include <error_intl.h>
#include <expr/position.h>
#include <str_list.h>
#include <stracc.h>
#include <trace.h>


/*
 * NAME
 *	builtin_upcase - upcase strings
 *
 * SYNOPSIS
 *	int builtin_upcase(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Defined is a built-in function of cook, described as follows:
 *	This function requires one or more arguments,
 *	which will bu upcased.
 *
 * RETURNS
 *	It returns the arguments upcased.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int upcase_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
upcase_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;

	trace(("upcase\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 1; j < args->nstrings; j++)
	{
		string_ty *s;

		s = str_upcase(args->string[j]);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_upcase =
{
	"upcase",
	upcase_interpret,
	upcase_interpret, /* script */
};


/*
 * NAME
 *	builtin_downcase - downcase strings
 *
 * SYNOPSIS
 *	int builtin_downcase(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Defined is a built-in function of cook, described as follows:
 *	This function requires one or more arguments,
 *	which will bu downcased.
 *
 * RETURNS
 *	It returns the arguments downcased.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int downcase_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
downcase_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;

	trace(("downcase\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 1; j < args->nstrings; j++)
	{
		string_ty	*s;

		s = str_downcase(args->string[j]);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_downcase =
{
	"downcase",
	downcase_interpret,
	downcase_interpret, /* script */
};


/*
 * NAME
 *	builtin_prepost - add prefix and suffix
 *
 * SYNOPSIS
 *	int builtin_prepost(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Prepost is a built-in function of cook, described as follows:
 *	This function must have at least two arguments.
 *	The first argument is a prefix and the second argument is a suffix.
 *
 * RETURNS
 *	The resulting word list is the third and later arguments each given
 *	the prefix and suffix as defined by the first and second arguments.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int prepost_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
prepost_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;

	trace(("prepost\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	if (args->nstrings < 3)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", args->string[0]);
		error_with_position
		(
			pp,
			scp,
			i18n("$name: requires two or more arguments")
		);
		sub_context_delete(scp);
		return -1;
	}
	for (j = 3; j < args->nstrings; j++)
	{
		string_ty	*s;

		s =
			str_cat_three
			(
				args->string[1],
				args->string[j],
				args->string[2]
			);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_prepost =
{
	"prepost",
	prepost_interpret,
	prepost_interpret, /* script */
};


/*
 * NAME
 *	builtin_head - head of a wordlist
 *
 * SYNOPSIS
 *	int builtin_fromto(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Head is a built-in function of cook, described as follows:
 *	This function requires zero or more arguments.
 *
 * RETURNS
 *	The wordlist returned is empty if there were no arguemnts,
 *	or the first argument if there were arguments.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int head_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
head_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	trace(("head\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	if (args->nstrings >= 2)
		string_list_append(result, args->string[1]);
	return 0;
}


builtin_ty builtin_head =
{
	"head",
	head_interpret,
	head_interpret, /* script */
};

builtin_ty builtin_firstword =
{
	"firstword",
	head_interpret,
	head_interpret, /* script */
};


/*
 * NAME
 *	builtin_tail - tail of a wordlist
 *
 * SYNOPSIS
 *	int builtin_tail(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Tail is a built-in function of cook, described as follows:
 *	This function requires zero or more arguments.
 *
 * RETURNS
 *	The word list returned will be empty if
 *	there is less than two arguemnts,
 *	otherwise it will consist of the second and later arguments.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int tail_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
tail_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;

	trace(("tail\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 2; j < args->nstrings; j++)
		string_list_append(result, args->string[j]);
	return 0;
}


builtin_ty builtin_tail =
{
	"tail",
	tail_interpret,
	tail_interpret, /* script */
};


/*
 * NAME
 *	builtin_catenate - catenate a wordlist
 *
 * SYNOPSIS
 *	int builtin_catenate(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Catenate is a built-in function of cook, described as follows:
 *	This function requires zero or more arguments.
 *
 * RETURNS
 *	A word list containg zero words if there were no arguments,
 *	or a single word which is the catenation of the arguments.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int
catenate_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
catenate_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	static stracc	sa;
	int		j;
	string_ty	*s;

	trace(("catenate\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	if (args->nstrings < 2)
		return 0;
	if (args->nstrings == 2)
	{
		string_list_append(result, args->string[1]);
		return 0;
	}

	sa_open(&sa);
	for (j = 1; j < args->nstrings; j++)
	{
		s = args->string[j];
		sa_chars(&sa, s->str_text, s->str_length);
	}
	s = sa_close(&sa);
	string_list_append(result, s);
	str_free(s);
	return 0;
}


builtin_ty builtin_catenate =
{
	"catenate",
	catenate_interpret,
	catenate_interpret, /* script */
};


/*
 * NAME
 *	builtin_count - length of a word list
 *
 * SYNOPSIS
 *	int builtin_count(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	Count is a built-in function of cook, described as follows:
 *	This function requires zero or more arguments.
 *
 * RETURNS
 *	A word list containg a single word containing the (decimal)
 *	length of the argument list.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int count_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
count_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	string_ty	*s;

	trace(("count\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	s = str_format("%ld", args->nstrings - 1);
	string_list_append(result, s);
	str_free(s);
	return 0;
}


builtin_ty builtin_count =
{
	"count",
	count_interpret,
	count_interpret, /* script */
};

builtin_ty builtin_words =
{
	"words",
	count_interpret,
	count_interpret, /* script */
};


/*
 * NAME
 *	builtin_quote - quote the arguments
 *
 * SYNOPSIS
 *	int builtin_quote(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The quote function is a built-in of cook, described as follows:
 *	This function requires one or more arguments.
 *
 * RETURNS
 *	A word list containing the values of the arguments
 *	surrounded by double quotes.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int quote_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
quote_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	size_t		j;
	string_ty	*s;

	trace(("quote\n"));
	assert(result);
	assert(args);
	assert(args->nstrings);
	for (j = 1; j < args->nstrings; ++j)
	{
		s = str_quote_shell(args->string[j]);
		string_list_append(result, s);
		str_free(s);
	}
	return 0;
}


builtin_ty builtin_quote =
{
	"quote",
	quote_interpret,
	quote_interpret, /* script */
};


/*
 * NAME
 *	builtin_sort - sort the arguments
 *
 * SYNOPSIS
 *	int builtin_sort(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The builtin_sort function is a built-in of cook, described as follows:
 *	sorts the arguments lexicagraphically.
 *	This function requires zero or more arguments.
 *
 * RETURNS
 *	A sorted word list.
 *
 * CAVEAT
 *	The returned result is in dynamic memory.
 *	It is the responsibility of the caller to dispose of
 *	the result when it is finished, with a string_list_destructor() call.
 */

static int cmp _((const void *, const void *));

static int
cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	string_ty	*a;
	string_ty	*b;

	a = *(string_ty **)va;
	b = *(string_ty **)vb;
	return strcmp(a->str_text, b->str_text);
}


static int sort_interpret _((string_list_ty *, const string_list_ty *,
	const expr_position_ty *, const struct opcode_context_ty *));

static int
sort_interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const expr_position_ty *pp;
	const struct opcode_context_ty *ocp;
{
	int		j;
	int		start;

	trace(("sort\n"));
	assert(result);
	assert(args);
	switch (args->nstrings)
	{
	case 0:
		assert(0);

	case 1:
		return 0;

	case 2:
		string_list_append(result, args->string[1]);
		return 0;
	}
	start = result->nstrings;
	for (j = 1; j < args->nstrings; ++j)
		string_list_append(result, args->string[j]);
	qsort
	(
		&result->string[start],
		args->nstrings - 1,
		sizeof(result->string[0]),
		cmp
	);
	return 0;
}


builtin_ty builtin_sort =
{
	"sort",
	sort_interpret,
	sort_interpret, /* script */
};
