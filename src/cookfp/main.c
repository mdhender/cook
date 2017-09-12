/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998, 1999, 2001 Peter Miller;
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

#include <ac/errno.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error_intl.h>
#include <fp/cksum.h>
#include <fp/cksum.h>
#include <fp/combined.h>
#include <fp/ident.h>
#include <fp/md5.h>
#include <fp/snefru.h>
#include <help.h>
#include <progname.h>
#include <version.h>
#include <str_list.h>


enum
{
	arglex_token_cksum,
	arglex_token_ident,
	arglex_token_md5,
	arglex_token_snefru
};

static arglex_table_ty argtab[] =
{
	{ "-Checksum",		arglex_token_cksum,	},
	{ "-Identifier",	arglex_token_ident,	},
	{ "-Message_Digest",	arglex_token_md5,	},
	{ "-Snefru",		arglex_token_snefru,	},
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
		"Usage: %s [ <option>... ][ <filename>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Help\n", progname);
	exit(1);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	string_ty	*minus;
	string_list_ty	file;
	long		j;
	fingerprint_methods_ty *method;
	string_ty	*s;

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

	method = 0;
	string_list_constructor(&file);
	minus = str_from_c("-");
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_snefru:
			if (method)
			{
				too_many_methods:
				error_intl
				(
					0,
					i18n("too many methods specified")
				);
				usage();
			}
			method = &fp_snefru;
			break;

		case arglex_token_ident:
			if (method)
				goto too_many_methods;
			method = &fp_ident;
			break;

		case arglex_token_md5:
			if (method)
				goto too_many_methods;
			method = &fp_md5;
			break;

		case arglex_token_cksum:
			if (method)
				goto too_many_methods;
			method = &fp_cksum;
			break;

		case arglex_token_string:
			s = str_from_c(arglex_value.alv_string);
			string_list_append(&file, s);
			str_free(s);
			break;

		case arglex_token_stdio:
			if (string_list_member(&file, minus))
			{
				error_intl
				(
					0,
				       i18n("may only name standard input once")
				);
				usage();
			}
			string_list_append(&file, minus);
			break;
		}
		arglex();
	}

	/*
	 * if no files named, read stdin
	 */
	if (!file.nstrings)
		string_list_append(&file, minus);

	/*
	 * by default, use the fp_combined class
	 */
	if (!method)
		method = &fp_combined;

	/*
	 * read the named files
	 */
	for (j = 0; j < file.nstrings; ++j)
	{
		fingerprint_ty	*p;
		char		buf[1000];

		p = fingerprint_new(method);
		s = file.string[j];
		if (str_equal(s, minus))
		{
			if (fingerprint_file_sum(p, (char *)0, buf))
			{
				int		err;
				string_ty	*s2;
				sub_context_ty	*scp;

				scp = sub_context_new();
				err = errno;
				s2 = subst_intl(scp, i18n("standard input"));
				/* re-use substitution context */
				sub_errno_setx(scp, err);
				sub_var_set(scp, "File_Name", "%S", s2);
				fatal_intl
				(
					scp,
					i18n("fingerprint $filename: $errno")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
				str_free(s2);
			}
			printf("%s", buf);
			if (file.nstrings != 1)
				printf("\tstdin");
			printf("\n");
		}
		else
		{
			if (fingerprint_file_sum(p, s->str_text, buf))
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_errno_set(scp);
				sub_var_set(scp, "File_Name", "%S", s);
				fatal_intl
				(
					scp,
					i18n("fingerprint $filename: $errno")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			printf("%s\t%s\n", buf, s->str_text);
		}
		fingerprint_delete(p);
	}
	exit(0);
	return 0;
}
