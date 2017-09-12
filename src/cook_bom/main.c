/*
 *	cook - file construction tool
 *	Copyright (C) 1998, 2001 Peter Miller;
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
 * MANIFEST: operating system start point, and command line argument parsing
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error_intl.h>
#include <help.h>
#include <progname.h>
#include <sniff.h>
#include <str.h>
#include <version.h>


enum
{
	arglex_token_directory,
	arglex_token_ignore,
	arglex_token_output,
	arglex_token_prefix,
	arglex_token_suffix
};

static arglex_table_ty argtab[] =
{
	{ "-DIRectory",		arglex_token_directory,	},
	{ "-IGnore",		arglex_token_ignore,	},
	{ "-Output",		arglex_token_output,	},
	{ "-Prefix",		arglex_token_prefix,	},
	{ "-Suffix",		arglex_token_suffix,	},
	{ 0, 0, } /* end marker */
};


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "Usage: %s [ <option>... ] <dirname>\n", progname);
	fprintf(stderr, "       %s -Help\n", progname);
	exit(1);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	char		*infile;
	char		*outfile;

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

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
			{
				arg_needs_string(arglex_token_directory, usage);
				/* NOTREACHED */
			}
			sniff_directory(arglex_value.alv_string);
			break;

		case arglex_token_ignore:
			if (arglex() != arglex_token_string)
			{
				arg_needs_string(arglex_token_ignore, usage);
				/* NOTREACHED */
			}
			sniff_ignore(arglex_value.alv_string);
			break;

		case arglex_token_prefix:
			if (sniff_prefix(arglex_value.alv_string))
				arg_duplicate_cur(usage);
			break;

		case arglex_token_suffix:
			if (sniff_suffix(arglex_value.alv_string))
				arg_duplicate_cur(usage);
			break;

		case arglex_token_string:
			if (!infile)
				infile = arglex_value.alv_string;
			else if (!outfile)
				outfile = arglex_value.alv_string;
			else
			{
				too_many:
				error_intl
				(
					0,
					i18n("too many filenames specified")
				);
				usage();
			}
			break;

		case arglex_token_output:
			if (outfile)
				goto too_many;
			switch (arglex())
			{
			default:
				usage();

			case arglex_token_string:
				outfile = arglex_value.alv_string;
				break;

			case arglex_token_stdio:
				outfile = "";
				break;
			}
			break;

		case arglex_token_stdio:
			if (outfile)
				goto too_many;
			outfile = "";
			break;
		}
		arglex();
	}
	if (!infile || !*infile)
		infile = ".";
	if (outfile && !*outfile)
		outfile = 0;

	/*
	 * read the directory and write the manifest
	 */
	sniff(infile, outfile);
	exit(0);
	return 0;
}


#if 0
void bogus() { i18n("bogus for cook_bom"); }
#endif
