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
 * MANIFEST: functions to manipulate filenames
 */

#include <fingerprint/filename.h>
#include <progname.h>
#include <str.h>


/*
 * NAME
 *	fp_filename
 *
 * SYNOPSIS
 *	string_ty *fp_filename(void);
 *
 * DESCRIPTION
 *	The fp_filename function is used to determine the name of the
 *	fingerprint cache file in each directory.
 */

string_ty *
fp_filename()
{
	static string_ty *s;

	if (!s)
		s = str_format(".%.10s.fp", progname_get());
	return s;
}
