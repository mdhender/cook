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
 * MANIFEST: functions to manipulate calculates
 */

#include <ac/errno.h>

#include <archive.h>
#include <error_intl.h>
#include <fingerprint.h>
#include <fp/combined.h>
#include <str.h>
#include <trace.h>


/*
 * NAME
 *	fp_calculate
 *
 * SYNOPSIS
 *	string_ty *fp_calculate(string_ty *path);
 *
 * DESCRIPTION
 *	The fp_calculate function is used to read the given file
 *	to calculate its fingerprint.  The cryptographically string
 *	fingerprint is returned in a string.  If the file does not exist,
 *	the NULL pointer is returned.
 */

string_ty *
fp_fingerprint(path)
	string_ty	*path;
{
	char		buffer[1000];
	fingerprint_ty	*fp;
	int		err;
	sub_context_ty	*scp;
	string_ty	*result;

	trace(("fp_fingerprint(path = \"%s\")\n{\n", path->str_text));
	fp = fingerprint_new(&fp_combined);
	err = fingerprint_file_sum(fp, path->str_text, buffer);
	if (err && errno == ENOENT)
		err = archive_fingerprint(fp, path, buffer);
	if (err)
	{
		switch (errno)
		{
		case ENOTDIR:
		case EISDIR:
		case ENOENT:
			break;

		default:
			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%S", path);
			fatal_intl
			(
				scp,
				i18n("fingerprint \"$filename\": $errno")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
			break;
		}
		result = 0;
	}
	else
		result = str_from_c(buffer);
	fingerprint_delete(fp);
	trace(("return \"%s\";\n", result ? result->str_text : ""));
	trace(("}\n"));
	return result;
}
