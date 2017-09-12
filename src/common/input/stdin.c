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
 * MANIFEST: functions to read the standard inptu
 */

#include <ac/stdio.h>

#include <error_intl.h>
#include <input/private.h>
#include <input/stdin.h>
#include <sub.h>
#include <str.h>


static string_ty *standard_input _((void));

static string_ty *
standard_input()
{
	static string_ty *name;
	sub_context_ty	*scp;

	if (!name)
	{
		scp = sub_context_new();
		name = subst_intl(scp, i18n("standard input"));
		sub_context_delete(scp);
	}
	return name;
}


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
	long		result;

	if (len <= 0)
		return 0;
	result = fread(data, 1, len, stdin);
	if (result <= 0 && ferror(stdin))
		fatal_intl_read(standard_input()->str_text);
	return result;
}


static int get _((input_ty *));

static int
get(this)
	input_ty	*this;
{
	int		c;

	c = getchar();
	if (c == EOF)
	{
		if (ferror(stdin))
			fatal_intl_read(standard_input()->str_text);
		return INPUT_EOF;
	}
	return c;
}


static string_ty *filename _((input_ty *));

static string_ty *
filename(this)
	input_ty	*this;
{
	return standard_input();
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
input_stdin()
{
	return input_new(&vtbl);
}
