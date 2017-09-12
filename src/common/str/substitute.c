/*
 *	cook - file construction tool
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: functions to substitute strings
 */

#include <ac/string.h>

#include <str.h>
#include <stracc.h>


string_ty *
str_substitute(a, b, c)
	string_ty	*a;
	string_ty	*b;
	string_ty	*c;
{
	char		*cp;
	char		*ep;
	static stracc	sa;

	sa_open(&sa);
	cp = c->str_text;
	ep = cp + c->str_length;
	while (cp < ep)
	{
		if
		(
			ep - cp < a->str_length
		||
			0 != memcmp(a->str_text, cp, a->str_length)
		)
		{
			sa_char(&sa, *cp++);
		}
		else
		{
			sa_chars(&sa, b->str_text, b->str_length);
			cp += a->str_length;
		}
	}
	return sa_close(&sa);
}
