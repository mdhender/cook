/*
 *	cook - file construction tool
 *	Copyright (C) 2000 Peter Miller;
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
 * MANIFEST: functions to manipulate flattens
 */

#include <str_list.h>

#include <flatten.h>


static void string_list_remove_nth _((string_list_ty *, size_t));

static void
string_list_remove_nth(slp, n)
	string_list_ty	*slp;
	size_t		n;
{
	if (n >= slp->nstrings)
		return;
	str_free(slp->string[n]);
	slp->nstrings--;
	while (n < slp->nstrings)
	{
		slp->string[n] = slp->string[n + 1];
		++n;
	}
}


string_ty *
flatten(filename)
	string_ty	*filename;
{
	string_list_ty	sl;
	static string_ty *root;
	static string_ty *dot;
	static string_ty *dotdot;
	size_t		pos;
	size_t		start;
	size_t		j;
	string_ty	*s;

	/*
	 * Create some things we are going to need.
	 */
	if (!root)
	{
		root = str_from_c("");
		dot = str_from_c(".");
		dotdot = str_from_c("..");
	}

	/*
	 * leading slash is special
	 */
	pos = 0;
	string_list_constructor(&sl);
	if (filename->str_text[0] == '/')
	{
		++pos;
		string_list_append(&sl, root);
	}

	/*
	 * Break it into slash-separaed words.
	 */
	for (;;)
	{
		while
		(
			pos < filename->str_length
		&&
			filename->str_text[pos] == '/'
		)
			++pos;
		if (pos >= filename->str_length)
			break;
		start = pos;
		for (;;)
		{
			++pos;
			if
			(
				pos >= filename->str_length
			||
				filename->str_text[pos] == '/'
			)
				break;
		}

		/*
		 * remember each word (except ".")
		 */
		s = str_n_from_c(filename->str_text + start, pos - start);
		if (!str_equal(s, dot))
			string_list_append(&sl, s);
		str_free(s);
	}

	/*
	 * Try as hard as possible to chuck out redundant stuff.
	 */
	for (;;)
	{
		int changed = 0;

		/*
		 * "/.." -> "/"
		 */
		if
		(
			sl.nstrings >= 2
		&&
			str_equal(sl.string[0], root)
		&&
			str_equal(sl.string[1], dotdot)
		)
		{
			string_list_remove_nth(&sl, 1);
			++changed;
		}

		/*
		 * "name/.." -> ""
		 */
		for (j = 0; j + 1 < sl.nstrings; ++j)
		{
			if (str_equal(sl.string[j], root))
				continue;
			if (str_equal(sl.string[j], dotdot))
				continue;
			if (!str_equal(sl.string[j + 1], dotdot))
				continue;
			string_list_remove_nth(&sl, j);
			string_list_remove_nth(&sl, j);
			++changed;
			--j;
		}

		/*
		 * loop if anything changed,
		 * bail if we can't find more to do
		 */
		if (!changed)
			break;
	}

	/*
	 * reassemble the "cleaned" path
	 */
	s = wl2str_respect_empty(&sl, 0, sl.nstrings, "/", 1);
	string_list_destructor(&sl);
	return s;
}
