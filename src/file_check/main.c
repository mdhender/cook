/*
 *      cook - file construction tool
 *      Copyright (C) 2001, 2006-2008 Peter Miller
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
#include <file_check/file_check.h>
#include <common/help.h>
#include <common/progname.h>
#include <common/str_list.h>
#include <common/version.h>


enum
{
    arglex_token_warning
};

static arglex_table_ty argtab[] =
{
    { "-Warning", arglex_token_warning },
    { 0, 0 }     /* end marker */
};


static void
usage(void)
{
    char            *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>...\n", progname);
    fprintf(stderr, "       %s -VERSion\n", progname);
    exit(1);
}


int
main(int argc, char **argv)
{
    string_ty       *s;
    string_list_ty  sl;
    size_t          j;

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

    string_list_constructor(&sl);
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_warning:
            warning++;
            break;

        case arglex_token_string:
            s = str_from_c(arglex_value.alv_string);
            string_list_append_unique(&sl, s);
            str_free(s);
            break;
        }
        arglex();
    }

    if (!sl.nstrings)
    {
        error_intl(0, i18n("no files named"));
        usage();
    }

    for (j = 0; j < sl.nstrings; ++j)
        file_check(sl.string[j]);
    exit(0);
    return 0;
}
