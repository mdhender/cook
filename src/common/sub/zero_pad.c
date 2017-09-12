/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate zero_pads
 */

#include <mem.h>
#include <str.h>
#include <sub/private.h>
#include <sub/zero_pad.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_zero_pad(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	string_ty	*s;
	long		n;

	trace(("sub_zero_pad()\n{\n"/*}*/));
	if (arg->nitems != 3)
	{
		sub_context_error_set(scp, i18n("requires two arguments"));
		trace(("return NULL;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	s = wstr_to_str(arg->item[2]);
	n = atol(s->str_text);
	trace(("n = %ld;\n", n));
	str_free(s);

	if (n <= arg->item[1]->wstr_length)
		result = wstr_copy(arg->item[1]);
	else
	{
		size_t		len;
		char		*buffer;
		string_ty	*s2;
		size_t		j;

		len = n - arg->item[1]->wstr_length;
		s = wstr_to_str(arg->item[1]);
		buffer = mem_alloc(len + 1);
		for (j = 0; j < len; ++j)
			buffer[j] = '0';
		buffer[len] = 0;
		s2 = str_format("%s%S", buffer, s);
		trace(("result = \"%s\";\n", s2->str_text));
		mem_free(buffer);
		str_free(s);
		result = str_to_wstr(s2);
		str_free(s2);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
