/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997-1999, 2006, 2007 Peter Miller;
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
 * The builtin function all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <common/trace.h>
#include <cook/builtin/exists.h>
#include <cook/expr.h>
#include <cook/os_interface.h>


/*
 * NAME
 *      builtin_exists - test for file existence
 *
 * SYNOPSIS
 *      int builtin_exists(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The builtin_exists function is a built-in function of cook,
 *      described as follows:
 *      This function requires one argument,
 *      being the name of a file to test for existence.
 *      The resulting wordlist is "" (false) if the file does not exist,
 *      and "1" (true) if the file does exist.
 *
 * RETURNS
 *      A word list containing the expanded pathname of the named commands
 *      given as arguments.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
exists_interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("exists\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; ++j)
    {
        string_ty       *s;

        s = args->string[j];
        if (os_exists(s))
            string_list_append(result, str_true);
        else
            string_list_append(result, str_false);
    }
    return 0;
}


builtin_ty builtin_exists =
{
    "exists",
    exists_interpret,
    exists_interpret,           /* script */
};


static int
exists_symlink_interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("exists-symlink\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; ++j)
    {
        string_ty       *s;

        s = args->string[j];
        if (os_exists_symlink(s))
            string_list_append(result, str_true);
        else
            string_list_append(result, str_false);
    }
    return 0;
}


builtin_ty builtin_exists_symlink =
{
    "exists-symlink",
    exists_symlink_interpret,
    exists_symlink_interpret,   /* script */
};
