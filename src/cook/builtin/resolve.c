/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <cook/builtin/resolve.h>
#include <cook/cook.h>
#include <common/error.h>       /* for assert */
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_resolve - resolve search path
 *
 * SYNOPSIS
 *      int builtin_resolve(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The resolve function is a built-in of cook, described as follows:
 *      This builtin function is used to resolve file names when using
 *      the search_list variable to locate files.  This builtin
 *      function produces resolved file names as output.  This is
 *      useful when taking partial copies of a source to perform
 *      controlled updates.  The targets of recipes are always cooked
 *      into the current directory.
 *
 * RETURNS
 *      A word list containing the resolved names.
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
    trace(("resolve\n"));
    (void)pp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    return cook_mtime_resolve(ocp, result, args, 1);
}


builtin_ty builtin_resolve =
{
    "resolve",
    interpret,
    interpret,                  /* script */
};
