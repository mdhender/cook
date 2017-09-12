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

#include <common/error_intl.h>
#include <common/trace.h>
#include <cook/builtin/pathname.h>
#include <cook/expr/position.h>
#include <cook/os_interface.h>


/*
 * NAME
 *      builtin_dir - dir part
 *
 * SYNOPSIS
 *      int builtin_dir(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      "dir" is a built-in function of cook, described as follows:
 *      This function requires one or more arguments,
 *      the name of a files of which to get the dir parts.
 *
 * RETURNS
 *      It returns a string containing the directory parts
 *      of the named files.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
dir_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("dir\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        string_ty       *s;

        s = os_dirname(args->string[j]);
        if (!s)
            return -1;
        string_list_append(result, s);
        str_free(s);
    }
    return 0;
}


builtin_ty builtin_dir =
{
    "dir",
    dir_interpret,
    dir_interpret,              /* script */
};


builtin_ty builtin_dirname =
{
    "dirname",
    dir_interpret,
    dir_interpret,              /* script */
};


/*
 * NAME
 *      builtin_entryname - entryname part
 *
 * SYNOPSIS
 *      int builtin_entryname(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Defined is a built-in function of cook, described as follows:
 *      This function requires one or more arguments,
 *      the name of a files of which to get the entryname parts.
 *
 * RETURNS
 *      It returns a string containing the entryname parts
 *      of the named files.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
entryname_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("entryname\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        string_ty       *s;

        s = os_entryname(args->string[j]);
        if (!s)
            return -1;
        string_list_append(result, s);
        str_free(s);
    }
    return 0;
}


builtin_ty builtin_entryname =
{
    "entryname",
    entryname_interpret,
    entryname_interpret,        /* script */
};


builtin_ty builtin_notdir =
{
    "notdir",
    entryname_interpret,
    entryname_interpret,        /* script */
};


/*
 * NAME
 *      builtin_pathname - pathname part
 *
 * SYNOPSIS
 *      int builtin_pathname(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Defined is a built-in function of cook, described as follows:
 *      This function requires one or more arguments,
 *      the name of a files of which to get the pathname parts.
 *
 * RETURNS
 *      It returns a string containing the pathname parts
 *      of the named files.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
pathname_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("pathname\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        string_ty       *s;

        s = os_pathname(args->string[j]);
        if (!s)
            return -1;
        string_list_append(result, s);
        str_free(s);
    }
    return 0;
}


builtin_ty builtin_pathname =
{
    "pathname",
    pathname_interpret,
    pathname_interpret,         /* script */
};
