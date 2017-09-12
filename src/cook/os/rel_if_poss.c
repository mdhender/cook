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
 * MANIFEST: functions to manipulate rel_if_posss
 */

#include <os_interface.h>
#include <os/below_dir.h>
#include <os/rel_if_poss.h>


string_ty *
os_relative_if_possible(path)
	string_ty	*path;
{
	string_ty	*dot;
	string_ty	*rel;

	if (path->str_text[0] != '/')
		return str_copy(path);
	dot = os_curdir();
	rel = os_below_dir(dot, path);
	if (!rel)
		return str_copy(path);
	return rel;
}
