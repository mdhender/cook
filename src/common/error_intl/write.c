/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2004 Peter Miller;
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
 * MANIFEST: functions to emit write error messages
 */

#include <ac/stdio.h>

#include <error_intl.h>
#include <fflush_slow.h>
#include <sub.h>


void
fatal_intl_write(fn)
	const char	*fn;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set(scp, "File_Name", "%s", fn);
	fatal_intl(scp, i18n("write $filename: $errno"));
	/* NOTREACHED */
}


void
error_intl_write(fn)
	const char	*fn;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set(scp, "File_Name", "%s", fn);
	error_intl(scp, i18n("write $filename: $errno"));
	sub_context_delete(scp);
}


void
fflush_and_check(fp, fn)
	FILE		*fp;
	const char	*fn;
{
	if (fflush_slowly(fp))
	{
		fatal_intl_write(fn);
		/* NOTREACHED */
	}
}
