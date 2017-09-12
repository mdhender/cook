/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate sort_newests
 */

#include <ac/stdlib.h>

#include <builtin/sort_newest.h>
#include <cook.h>
#include <opcode/context.h>
#include <str_list.h>
#include <trace.h>


static const opcode_context_ty *cntx;


static int cmp _((const void *, const void *));

static int
cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	string_ty	*a;
	string_ty	*b;
	long		tmp;
	long		da;
	long		db;

	a = *(string_ty **)va;
	b = *(string_ty **)vb;
	da = 32767;
	db = 32767;
	tmp =
		(
			cook_mtime_newest(cntx, b, &db, db)
		-
			cook_mtime_newest(cntx, a, &da, da)
		);
	if (tmp == 0)
		return 0;
	return (tmp < 0 ? -1 : 1);
}


/*
 * NAME
 *	builtin_sort_newest - sort the arguments
 *
 * SYNOPSIS
 *	int builtin_sort_newest(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *	The sort_newest function is a built-in of cook, described as
 *	follows: sorts the arguments my their last-modified file times,
 *	youngest to oldest.
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

static int interpret _((string_list_ty *, const string_list_ty *,
	const struct expr_position_ty *, const opcode_context_ty *));

static int
interpret(result, args, pp, ocp)
	string_list_ty	*result;
	const string_list_ty *args;
	const struct expr_position_ty *pp;
	const opcode_context_ty *ocp;
{
	int		j;
	int		start;

	trace(("sort_newest\n"));
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
	cntx = ocp;
	qsort
	(
		&result->string[start],
		args->nstrings - 1,
		sizeof(result->string[0]),
		cmp
	);
	cntx = 0;
	return 0;
}


builtin_ty builtin_sort_newest =
{
	"sort_newest",
	interpret,
	interpret, /* script */
};
