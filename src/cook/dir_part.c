/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate directory parts
 */

#include <dir_part.h>
#include <str.h>


string_ty *
dir_part(s)
	string_ty	*s;
{
	char		*ep;

	/*
	 * ignore trailing slashes
	 */
	ep = s->str_text + s->str_length;
	while (ep > s->str_text && ep[-1] == '/')
		--ep;
	if (ep == s->str_text)
		return 0;

	/*
	 * back up over that last word
	 */
	while (ep > s->str_text && ep[-1] != '/')
		--ep;
	if (ep == s->str_text)
		return 0;

	/*
	 * back up over the separating slash
	 */
	while (ep > s->str_text && ep[-1] == '/')
		--ep;
	if (ep == s->str_text)
		return 0;

	/*
	 * return the directory part
	 */
	return str_n_from_c(s->str_text, ep - s->str_text);
}
