/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2001, 2006-2008 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/error_intl.h>
#include <common/help.h>
#include <common/progname.h>
#include <common/version.h>
#include <make2cook/emit.h>
#include <make2cook/gram.h>
#include <make2cook/stmt/assign.h>
#include <make2cook/stmt/rule.h>


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
    { "-No_Environment_variables", arglex_token_environment_variables_not, },
    { "-No_History_Commands", arglex_token_history_commands_not, },
    { "-No_Internal_Rules", arglex_token_internal_rules_not, },
    { "-No_Line_Numbers", arglex_token_line_numbers_not, },
    { 0, 0, },  /* end marker */
};


static void
usage(void)
{
    char            *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <infile> [ <outfile> ]]\n", progname);
    fprintf(stderr, "       %s -help\n", progname);
    exit(1);
}


int
main(int argc, char **argv)
{
    char            *infile;
    char            *outfile;

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
                fatal_intl(0, i18n("too many filenames specified"));
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
