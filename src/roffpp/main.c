/*
 *	cook - file construction tool
 *	Copyright (C) 1992, 1993, 1994, 1997, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: operating system start point, and parse command line arguments
 */

#include <ac/stdio.h>
#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error_intl.h>
#include <help.h>
#include <preprocess.h>
#include <progname.h>
#include <str.h>
#include <version.h>


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s [ <option>... ][ <infile> [ <outfile> ]]\n",
		progname
	);
	fprintf(stderr, "       %s -Help\n", progname);
	fprintf(stderr, "       %s -VERSion\n", progname);
	exit(1);
}


enum
{
	arglex_token_include
};

static arglex_table_ty argtab[] =
{
	{ "-\\I*",	(arglex_token_ty)arglex_token_include,	},
	{ "-Include",	(arglex_token_ty)arglex_token_include,	},
	{ 0, (arglex_token_ty)0, }, /* end marker */
};


int main _((int, char **));

int
main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*infile;
	char	*outfile;

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

	infile = 0;
	outfile = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_string:
			if (!infile)
				infile = arglex_value.alv_string;
			else if (!outfile)
				outfile = arglex_value.alv_string;
			else
			{
				too_many:
				fatal_intl
				(
					0,
					i18n("too many filenames specified")
				);
			}
			break;

		case arglex_token_stdio:
			if (!infile)
				infile = "";
			else if (!outfile)
				outfile = "";
			else
				goto too_many;
			break;

		case arglex_token_include:
			if (arglex() != arglex_token_string)
			{
				arg_needs_string(arglex_token_include, usage);
				/* NOTREACHED */
			}
			preprocess_include(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (infile && !*infile)
		infile = 0;
	if (outfile && !*outfile)
		outfile = 0;
	preprocess(infile, outfile);
	exit(0);
	return 0;
}

#if 0
i18n("bogus for roffpp");
#endif
