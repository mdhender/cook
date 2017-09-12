/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998, 2001 Peter Miller;
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
#include <emit.h>
#include <error_intl.h>
#include <gram.h>
#include <help.h>
#include <progname.h>
#include <stmt/assign.h>
#include <stmt/rule.h>
#include <version.h>


enum
{
	arglex_token_environment_variables,
	arglex_token_environment_variables_not,
	arglex_token_history_commands,
	arglex_token_history_commands_not,
	arglex_token_internal_rules,
	arglex_token_internal_rules_not,
	arglex_token_line_numbers,
	arglex_token_line_numbers_not
};

static arglex_table_ty argtab[] =
{
	{ "-Environment_variables", arglex_token_environment_variables, },
	{ "-History_Commands", arglex_token_history_commands, },
	{ "-Internal_Rules", arglex_token_internal_rules, },
	{ "-Line_Numbers", arglex_token_line_numbers, },
	{ "-No_Environment_variables",
		arglex_token_environment_variables_not, },
	{ "-No_History_Commands", arglex_token_history_commands_not, },
	{ "-No_Internal_Rules", arglex_token_internal_rules_not, },
	{ "-No_Line_Numbers", arglex_token_line_numbers_not, },
	{ 0, 0, }, /* end marker */
};


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s [ <infile> [ <outfile> ]]\n", progname);
	fprintf(stderr, "       %s -help\n", progname);
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

		case arglex_token_string:
			if (!infile)
				infile = arglex_value.alv_string;
			else if (!outfile)
				outfile = arglex_value.alv_string;
			else
			{
				too_many_filenames:
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
				goto too_many_filenames;
			break;

		case arglex_token_history_commands:
			stmt_rule_default_history = 1;
			break;

		case arglex_token_history_commands_not:
			stmt_rule_default_history = 0;
			break;

		case arglex_token_environment_variables:
			stmt_assign_environment_variables = 1;
			break;

		case arglex_token_environment_variables_not:
			stmt_assign_environment_variables = 0;
			break;

		case arglex_token_line_numbers:
			emit_line_numbers = 1;
			break;

		case arglex_token_line_numbers_not:
			emit_line_numbers = 0;
			break;

		case arglex_token_internal_rules:
			no_internal_rules = 0;
			break;

		case arglex_token_internal_rules_not:
			no_internal_rules = 1;
			break;
		}
		arglex();
	}
	if (infile && !*infile)
		infile = 0;
	if (outfile && !*outfile)
		outfile = 0;

	emit_open(outfile);
	gram(infile);
	emit_close();

	exit(0);
	return 0;
}
