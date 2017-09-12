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
 * MANIFEST: functions to manipulate stripdots
 */

#include <stripdot.h>
#include <sniff.h>
#include <str_list.h>


string_ty *
stripdot(s)
    string_ty *s;
{
    if (option.stripdot)
    {
	const char *cp = s->str_text;
	size_t len = s->str_length;
	while (len >= 2 && cp[0] == '.' && cp[1] == '/')
	{
	    cp += 2;
	    len -= 2;
	    while (*cp == '/')
	    {
		++cp;
		--len;
	    }
	}
	if (!len)
	    return str_from_c(".");
	return str_n_from_c(cp, len);
    }
    else
	return str_copy(s);
}


void
stripdot_list(slp)
    string_list_ty  *slp;
{
    size_t          j;

    if (!option.stripdot)
	return;
    for (j = 0; j < slp->nstrings; ++j)
    {
	string_ty       *s;

	s = stripdot(slp->string[j]);
	str_free(slp->string[j]);
	slp->string[j] = s;
    }
}
