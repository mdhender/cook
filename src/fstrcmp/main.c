/*
 *	cook - file construction tool
 *	Copyright (C) 1995, 1997, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: operating system entry point
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error_intl.h>
#include <fstrcmp.h>
#include <help.h>
#include <progname.h>
#include <str.h>
#include <version.h>


static arglex_table_ty argtab[] =
{
	{ 0, 0, } /* end marker */
};


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"Usage: %s [ <option>... ] <string1> <string2>\n",
		progname
	);
	fprintf(stderr, "       %s -VERSion\n", progname);
	exit(1);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	char		*s1;
	char		*s2;

	arglex_init(argc, argv, argtab);
	str_initialize();
	switch (arglex())
	{
	case arglex_token_help:
		help((char *)0, usage);
		exit(0);

	case arglex_token_version:
		version();
		exit(0);

	default:
		break;
	}

	s1 = 0;
	s2 = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_string:
			if (!s1)
				s1 = arglex_value.alv_string;
			else if (!s2)
				s2 = arglex_value.alv_string;
			else
			{
				error_intl
				(
					0,
					i18n("too many strings specified")
				);
				usage();
			}
			break;
		}
		arglex();
	}

	/*
	 * must have exactly 2 strings defined
	 */
	if (!s1 || !s2)
	{
		error_intl(0, i18n("must specify two strings"));
		usage();
	}

	printf("%8.6f\n", fstrcmp(s1, s2));
	exit(0);
	return 0;
}
