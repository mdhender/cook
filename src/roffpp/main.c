/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2006, 2007 Peter Miller;
 *      All rights reserved.
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
#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/error_intl.h>
#include <common/help.h>
#include <roffpp/preprocess.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/version.h>


static void
usage(void)
{
    char            *progname;

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
    { "-\\I*", (arglex_token_ty)arglex_token_include },
    { "-Include", (arglex_token_ty)arglex_token_include },
    { 0, (arglex_token_ty)0 } /* end marker */
};


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
                too_many:
                fatal_intl(0, i18n("too many filenames specified"));
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
