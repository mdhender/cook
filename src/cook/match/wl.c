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
 * MANIFEST: functions to manipulate matching words lists
 */

#include <expr/position.h>
#include <match/wl.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	wl_match - find a pattern in a word list
 *
 * SYNOPSIS
 *	match_ty *wl_match(string_list_ty *pattern, string_ty *target);
 *
 * DESCRIPTION
 *	Wl_match is used to determine whether any one of the words in
 *	the wordlist (wlp) match the pattern given.
 *
 * RETURNS
 *	A zero is returned if not one of the words matches the pattern;
 *	otherwise a pointer to a "match structure" is returned,
 *	in a similar fashion to match().
 *
 * CAVEAT
 *	The information returned resides in dynamic memory.
 *	It is the responsibility of the
 *	caller to ensure that it is freed when it is finished with,
 *	by a call to match_delete();
 */

int
match_wl_attempt(mp, formal, actual, pp)
	match_ty	*mp;
	string_list_ty	*formal;
	string_ty	*actual;
	const expr_position_ty *pp;
{
	size_t		j;
	int		result;

	trace(("match_wl_attempt(mp = %08lX, formal = %08lX, actual = \"%s\", \
pp = %08lX)\n{\n", (long)mp, (long)formal, actual->str_text, (long)pp));
	result = 0;
	for (j = 0; j < formal->nstrings; j++)
	{
		result = match_attempt(mp, formal->string[j], actual, pp);
		if (result)
			break;
	}
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


/*
 * NAME
 *	wl_reconstruct - reconstruct a word list
 *
 * SYNOPSIS
 *	void wl_reconstruct(string_list_ty *to, string_list_ty *from,
 *		match_ty *field)
 *
 * DESCRIPTION
 *	Wl_reconstruct is used to reconstruct an entire word list,
 *	sort of the convers of wl_match().
 *
 * RETURNS
 *	'To' is a word list of reconstructed strings.
 *
 * CAVEAT
 *	It is the responsibility of the caller to ensire that the
 *	reconstructed word list in 'to' is freed when finished with,
 *	by a call to string_list_destructor().
 */

int
match_wl_reconstruct_lhs(mp, to, from, pp)
	const match_ty	*mp;
	string_list_ty	*to;
	string_list_ty	*from;
	const expr_position_ty *pp;
{
	size_t		j;
	string_ty	*s;

	string_list_constructor(to);
	for (j = 0; j < from->nstrings; j++)
	{
		s = match_reconstruct_lhs(mp, from->string[j], pp);
		if (!s)
			return -1;
		string_list_append(to, s);
		str_free(s);
	}
	return 0;
}


int
match_wl_reconstruct_rhs(mp, to, from, pp)
	const match_ty	*mp;
	string_list_ty	*to;
	string_list_ty	*from;
	const expr_position_ty *pp;
{
	size_t		j;
	string_ty	*s;

	string_list_constructor(to);
	for (j = 0; j < from->nstrings; j++)
	{
		s = match_reconstruct_rhs(mp, from->string[j], pp);
		if (!s)
			return -1;
		string_list_append(to, s);
		str_free(s);
	}
	return 0;
}


int
match_wl_usage_mask(mp, wlp, pp)
	const match_ty	*mp;
	string_list_ty	*wlp;
	const expr_position_ty *pp;
{
	int		result;
	size_t		j;
	int		ok;

	result = 0;
	for (j = 0; j < wlp->nstrings; ++j)
	{
		ok = match_usage_mask(mp, wlp->string[j], pp);
		if (ok < 0)
			return -1;
		result |= ok;
	}
	return result;
}
