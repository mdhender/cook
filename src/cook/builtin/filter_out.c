/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2006, 2007 Peter Miller;
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
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <cook/builtin/filter_out.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/match.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_filter_out - wilcard filtering
 *
 * SYNOPSIS
 *      int builtin_filter_out(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Filter_out is a built-in function of cook, described as follows:
 *      This function requires at least one argument.
 *      Filter_out uses the first argument as a pattern, and the result
 *      is the those of the second and subsequent arguments which do
 *      not match the patern.
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
    size_t          j;
    match_ty        *mp;
    int             retval;
    int             retval2;

    trace(("filter_out\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings < 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires one or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    retval = 0;
    mp = match_new();
    if (match_compile(mp, args->string[1], pp) < 0)
    {
        retval = -1;
        goto error_ret;
    }
    for (j = 2; j < args->nstrings; j++)
    {
        retval2 = match_execute(mp, args->string[j], pp);
        if (retval2 < 0)
        {
            retval = -1;
            break;
        }
        if (!retval2)
            string_list_append(result, args->string[j]);
    }
    error_ret:
    match_delete(mp);
    return retval;
}


builtin_ty builtin_filter_out =
{
    "filter_out",
    interpret,
    interpret,                  /* script */
};


builtin_ty builtin_filter_out_ =
{
    "filter-out",
    interpret,
    interpret,                  /* script */
};
