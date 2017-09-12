/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006-2008 Peter Miller
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

#include <common/ac/stdlib.h>

#include <cook/builtin/sort_newest.h>
#include <cook/cook.h>
#include <cook/opcode/context.h>
#include <common/str_list.h>
#include <common/trace.h>


static const opcode_context_ty *cntx;


static int
cmp(const void *va, const void *vb)
{
    string_ty       *a;
    string_ty       *b;
    long            tmp;
    long            da;
    long            db;

    a = *(string_ty **)va;
    b = *(string_ty **)vb;
    da = 32767;
    db = 32767;
    tmp =
        (
            cook_mtime_newest(cntx, b, &db, db)
        -
            cook_mtime_newest(cntx, a, &da, da)
        );
    if (tmp == 0)
        return 0;
    return (tmp < 0 ? -1 : 1);
}


/*
 * NAME
 *      builtin_sort_newest - sort the arguments
 *
 * SYNOPSIS
 *      int builtin_sort_newest(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The sort_newest function is a built-in of cook, described as
 *      follows: sorts the arguments my their last-modified file times,
 *      youngest to oldest.
 *      This function requires zero or more arguments.
 *
 * RETURNS
 *      A sorted word list.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const opcode_context_ty *ocp)
{
    size_t          j;
    int             start;

    trace(("sort_newest\n"));
    (void)pp;
    assert(result);
    assert(args);
    switch (args->nstrings)
    {
    case 0:
        assert(0);

    case 1:
        return 0;

    case 2:
        string_list_append(result, args->string[1]);
        return 0;
    }
    start = result->nstrings;
    for (j = 1; j < args->nstrings; ++j)
        string_list_append(result, args->string[j]);
    cntx = ocp;
    qsort
    (
        &result->string[start],
        args->nstrings - 1,
        sizeof(result->string[0]),
        cmp
    );
    cntx = 0;
    return 0;
}


builtin_ty builtin_sort_newest =
{
    "sort_newest",
    interpret,
    interpret,                  /* script */
};
