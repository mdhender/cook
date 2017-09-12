/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate the program name
 */

#include <ac/string.h>

#include <exeext.h>
#include <progname.h>


static	char	*progname;


void
progname_set(s)
	char		*s;
{
	/*
	 * do NOT put tracing in this function
	 * do NOT put asserts in this function
	 *	they both depend on progname, which is not yet set
	 */
	for (;;)
	{
		progname = strrchr(s, '/');

		/*
		 * we were invoked as
		 *	progname -args
		 */
		if (!progname)
		{
			/*
			 * Nuke any ugly progname suffix
			 * if it has one.
			 */
			int n = exeext(s);
			if (n > 0)
				s[n] = 0;

			progname = s;
			break;
		}

		/*
		 * we were invoked as
		 *	/usr/local/progname -args
		 */
		if (progname[1])
		{
			++progname;
			break;
		}

		/*
		 * this is real nasty:
		 * it is possible to invoke us as
		 *	/usr//local///bin////progname///// -args
		 * and it is legal!!
		 */
		*progname = 0;
	}
}


char *
progname_get()
{
	/* do NOT put tracing in this function */
	return (progname ? progname : "");
}
