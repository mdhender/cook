/*
 *      cook - file construction tool
 *      Copyright (C) 2002, 2006-2008 Peter Miller
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
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <cook/builtin/read.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/stracc.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_read - get output of a command
 *
 * SYNOPSIS
 *      int builtin_read(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Collect is a built-in function of cook, described as follows:
 *      This function requires one or more arguments.
 *
 * RETURNS
 *      A word list containing the values of the output lines of the
 *      program given in the arguments.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    FILE            *fp;
    string_ty       *s;
    char            *delim;
    stracc          sa;

    trace(("read\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings != 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position(pp, scp, i18n("$name: requires one argument"));
        sub_context_delete(scp);
        return -1;
    }
    fp = fopen_and_check(args->string[1]->str_text, "r");
    stracc_constructor(&sa);
    delim = strchr(args->string[0]->str_text, '_') ? "\n" : "\n \t\f";
    for (;;)
    {
        int             c;

        for (;;)
        {
            c = fgetc(fp);
            if (c == EOF || !strchr(delim, c))
                break;
        }
        if (c == EOF)
        {
            if (ferror(fp))
            {
                error_intl_read(args->string[1]->str_text);
                stracc_destructor(&sa);
                fclose(fp);
                return -1;
            }
            break;
        }
        sa_open(&sa);
        for (;;)
        {
            sa_char(&sa, c);
            c = fgetc(fp);
            if (c == EOF || strchr(delim, c))
                break;
        }
        s = sa_close(&sa);
        string_list_append(result, s);
        str_free(s);
        if (c == EOF)
            break;
    }
    fclose_and_check(fp, args->string[1]->str_text);
    stracc_destructor(&sa);
    return 0;
}


builtin_ty builtin_read =
{
    "read",
    interpret,
    interpret, /* script */
};


builtin_ty builtin_read_lines =
{
    "read_lines",
    interpret,
    interpret, /* script */
};
