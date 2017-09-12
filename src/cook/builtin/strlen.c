/*
 * cook - file construction tool
 * Copyright (C) 2008, 2009 Peter Miller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <common/ac/string.h>

#include <common/str_list.h>
#include <common/trace.h>

#include <cook/builtin/strlen.h>
#include <cook/expr/position.h>


/*
 * NAME
 *      builtin_strlen - strlen strings
 *
 * SYNOPSIS
 *      int builtin_strlen(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Defined is a built-in function of cook, described as follows:
 *      This function requires one or more arguments,
 *      which will be replaced by their length (in bytes, not characters).
 *
 * RETURNS
 *      It returns a list containing the lengths of each of the argments.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
strlen_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("%s\n", __PRETTY_FUNCTION__));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        string_ty       *s;

        s = str_format("%ld", (long)args->string[j]->str_length);
        string_list_append(result, s);
        str_free(s);
    }
    return 0;
}


builtin_ty builtin_strlen =
{
    "strlen",
    strlen_interpret,
    strlen_interpret,           /* script */
};
