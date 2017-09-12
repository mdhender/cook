/*
 *	cook - a program construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to provide common -VERSion behaviour
 */

#include <ac/stdio.h>

#include <arglex.h>
#include <error.h>
#include <help.h>
#include <progname.h>
#include <quit.h>
#include <str.h>
#include <trace.h>
#include <version.h>
#include <version-stmp.h>


static void version_copyright _((void));

static void
version_copyright()
{
	static char *text[] =
	{
		"All rights reserved.",
		"",
		"The %s program comes with ABSOLUTELY NO WARRANTY;",
		"for details use the '%s -VERSion License' command.",
		"The %s program is free software, and you are welcome",
		"to redistribute it under certain conditions; for",
		"details use the '%s -VERSion License' command.",
	};

	char		**cpp;
	char		*progname;

	progname = progname_get();
	printf("%s version %s\n", progname, version_stamp());
	printf("Copyright (C) %s Peter Miller;\n", copyright_years());
	for (cpp = text; cpp < ENDOF(text); ++cpp)
	{
		printf(*cpp, progname);
		fputc('\n', stdout);
	}
}


static void version_license _((void));

static void
version_license()
{
	help("cook_lic", (void (*)_((void)))0);
}


typedef struct table_ty table_ty;
struct table_ty
{
	char	*name;
	void	(*func)_((void));
};


static	table_ty	table[] =
{
	{ "Copyright",	version_copyright,	},
	{ "License",	version_license,	},
};


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "Usage: %s -VERSion [ name ]\n", progname);
	fprintf(stderr, "       %s -Help\n", progname);
	quit(1);
}


void
version()
{
	void	(*func)_((void));
	char	*name;

	trace(("version()\n{\n"/*}*/));
	arglex();
	name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_string:
			if (name)
			      fatal_raw("too many info names");
			name = arglex_value.alv_string;
			break;
		}
		arglex();
	}

	if (name)
	{
		int		nhit;
		table_ty	*tp;
		string_ty	*s1;
		string_ty	*s2;
		table_ty	*hit[SIZEOF(table)];
		int		j;

		nhit = 0;
		for (tp = table; tp < ENDOF(table); ++tp)
		{
			if (arglex_compare(tp->name, name))
				hit[nhit++] = tp;
		}
		switch (nhit)
		{
		case 0:
			fatal_raw("no info %s", name);

		case 1:
			break;

		default:
			s1 = str_from_c(hit[0]->name);
			for (j = 1; j < nhit; ++j)
			{
				s2 = str_format("%S, %s", s1, hit[j]->name);
				str_free(s1);
				s1 = s2;
			}
			fatal_raw("info %s ambig (%S)", name, s1);
		}
		arglex();
		func = hit[0]->func;
	}
	else
		func = version_copyright;

	func();
	trace((/*{*/"}\n"));
}
