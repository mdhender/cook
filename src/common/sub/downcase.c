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
 * MANIFEST: functions to manipulate downcases
 */

#include <sub/downcase.h>
#include <sub/private.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_downcase - the downcase substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_downcase(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_downcase function implements the downcase substitution.
 *	The downcase substitution is replaced by the single argument
 *	mapped to lower case.
 *
 *	Requires exactly one argument.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_downcase(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_downcase()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		sub_context_error_set(scp, i18n("requires one argument"));
		result = 0;
	}
	else
		result = wstr_to_lower(arg->item[1]);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
