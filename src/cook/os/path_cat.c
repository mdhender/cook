/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate path_cats
 */

#include <os_interface.h>


string_ty *
os_path_cat(lhs, rhs)
	string_ty	*lhs;
	string_ty	*rhs;
{
	static string_ty *dot;
	size_t		lhs_len;

	if (!dot)
		dot = str_from_c(".");
	if (!lhs)
		lhs = dot;
	if (!rhs)
		rhs = dot;
	if (rhs->str_text[0] == '/')
		return str_copy(rhs);
	if (str_equal(lhs, dot))
		return str_copy(rhs);
	if (str_equal(rhs, dot))
		return str_copy(lhs);
	lhs_len = lhs->str_length;
	while (lhs_len > 0 && lhs->str_text[lhs_len - 1] == '/')
		--lhs_len;
	return str_format("%.*S/%S", (int)lhs_len, lhs, rhs);
}
