/*
 *	cook - file construction tool
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate tempfilenames
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <str.h>
#include <tempfilename.h>


string_ty *
temporary_filename()
{
	char		iname[100];
	static int	nnn;

#ifdef __CYGWIN32__
	/*
	 * For some reason CygWin32's tmpnam() always produces
	 * filenames which give ``No such file or directory''
	 */
	sprintf(iname, "t%dp%d.tmp", ++nnn, getpid());
#else
	/*
	 * I'd use tmpnam(), but the GNU linker now says it is too
	 * dangerous and won't link it any more.
	 */
	char *tmpdir = 0;
	if (!geteuid())
		tmpdir = "/tmp";
	else
	{
		tmpdir = getenv("TMPDIR");
		if (!tmpdir || *tmpdir != '/')
			tmpdir = "/tmp";
	}
	sprintf(iname, "%s/t%dp%d", tmpdir, ++nnn, getpid());
#endif
	return str_from_c(iname);
}


string_ty *
dot_temporary_filename()
{
	static long temp_file_number;

	return str_format(".%d.%ld", getpid(), ++temp_file_number);
}
