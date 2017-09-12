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
 * MANIFEST: functions for reading input from files
 */

#include <error_intl.h>
#include <input/file.h>
#include <input/private.h>
#include <input/stdin.h>
#include <str.h>
#include <sub.h>

typedef struct input_file_ty input_file_ty;
struct input_file_ty
{
	input_ty	inherited;
	FILE		*fp;
	string_ty	*fn;
};


static void destruct _((input_ty *));

static void
destruct(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	fclose_and_check(this->fp, this->fn->str_text);
	str_free(this->fn);
	this->fp = 0;
	this->fn = 0;
}


static long iread _((input_ty *, void *, long));

static long
iread(p, data, len)
	input_ty	*p;
	void		*data;
	long		len;
{
	input_file_ty	*this;
	long		result;

	if (len < 0)
		return 0;
	this = (input_file_ty *)p;
	result = fread(data, (size_t)1, (size_t)len, this->fp);
	if (result <= 0 && ferror(this->fp))
		fatal_intl_read(this->fn->str_text);
	return result;
}


static int get _((input_ty *));

static int
get(p)
	input_ty	*p;
{
	input_file_ty	*this;
	int		c;

	this = (input_file_ty *)p;
	c = getc(this->fp);
	if (c == EOF)
	{
		if (ferror(this->fp))
			fatal_intl_read(this->fn->str_text);
		return INPUT_EOF;
	}
	return c;
}


static string_ty *filename _((input_ty *));

static string_ty *
filename(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	return this->fn;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_file_ty),
	destruct,
	iread,
	get,
	filename,
};


input_ty *
input_file_open(fn)
	string_ty	*fn;
{
	input_ty	*result;
	input_file_ty	*this;

	if (!fn)
		return input_stdin();
	result = input_new(&vtbl);
	this = (input_file_ty *)result;
	this->fp = fopen_and_check(fn->str_text, "rb");
	this->fn = str_copy(fn);
	return result;
}
