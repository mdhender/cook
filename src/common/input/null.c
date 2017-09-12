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
 * MANIFEST: functions to manipulate nulls
 */

#include <input/null.h>
#include <input/private.h>
#include <str.h>


static void destruct _((input_ty *));

static void
destruct(this)
	input_ty	*this;
{
}


static long iread _((input_ty *, void *, long));

static long
iread(this, data, len)
	input_ty	*this;
	void		*data;
	long		len;
{
	return 0;
}


static int get _((input_ty *));

static int
get(this)
	input_ty	*this;
{
	return INPUT_EOF;
}


static string_ty *filename _((input_ty *));

static string_ty *
filename(this)
	input_ty	*this;
{
	static string_ty *s;

	if (!s)
		s = str_from_c("/dev/null");
	return s;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_ty),
	destruct,
	iread,
	get,
	filename,
};


input_ty *
input_null()
{
	return input_new(&vtbl);
}
