/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997, 1999, 2006, 2007 Peter Miller;
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

#include <common/ac/stdlib.h>

#include <cook/builtin/getenv.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_getenv - get environment variables
 *
 * SYNOPSIS
 *      int builtin_getenv(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Getenv is a built-in function of cook, described as follows:
 *      This function requires one or more arguments.
 *
 * RETURNS
 *      A word list containing the values of the environment variables
 *      given as arguments.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("getenv\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; ++j)
    {
        char            *cp;
        string_ty       *s;

        cp = getenv(args->string[j]->str_text);
        if (!cp)
            string_list_append(result, str_false);
        else
        {
            s = str_from_c(cp);
            string_list_append(result, s);
            str_free(s);
        }
    }
    return 0;
}


builtin_ty builtin_getenv =
{
    "getenv",
    interpret,
    interpret,                  /* script */
};
