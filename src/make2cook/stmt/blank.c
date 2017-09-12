/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate blank statements
 */

#include <stmt/blank.h>

static stmt_method_ty method =
{
	sizeof(stmt_ty),
	"blank",
	0, /* constructor */
	0, /* destructor */
	0, /* emit */
	0, /* regroup */
	0, /* sort */
};


stmt_ty *
stmt_blank_alloc()
{
	stmt_ty		*result;

	result = stmt_alloc(&method);
	result->white_space = 1;
	return result;
}
