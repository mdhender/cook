/*
 *	cook - file construction tool
 *	Copyright (C) 1990, 1991, 1992, 1993, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to isolate operating system interface
 */

#include <ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <error_intl.h>
#include <os_interface.h>

/*
 *  NAME
 *	os_exists - tests for the existence of a file
 *
 *  SYNOPSIS
 *	int os_exists(char *filename);
 *
 *  DESCRIPTION
 *	Os_exists returns 1 if the file exists, 0 if it does not.
 */

int
os_exists(filename)
	char		*filename;
{
	struct stat	st;

	if (stat(filename, &st))
	{
		switch (errno)
		{
		case ENOENT:
		case ENOTDIR:
			break;

		default:
			fatal_intl_stat(filename);
			/* NOTREACHED */
		}
		return 0;
	}
	return 1;
}
