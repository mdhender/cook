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

#include <common/ac/time.h>

#include <cook/builtin/mtime.h>
#include <cook/cook.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_mtime - file last modified time
 *
 * SYNOPSIS
 *      int builtin_mtime(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Defined is a built-in function of cook, described as follows:
 *      This function requires a single argument,
 *      the name of a file of which to get the last modified time.
 *
 * RETURNS
 *      It returns a string containing the last modified time
 *      (suitable for comparing with others) of the named file,
 *      and "" (false) if the files does not exist
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
mtime_interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("mtime\n"));
    (void)pp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        time_t          mtime;
        long            depth;

        depth = 32767;
        mtime = cook_mtime_newest(ocp, args->string[j], &depth, depth);
        if (mtime < 0)
            return -1;
        if (mtime == 0)
            string_list_append(result, str_false);
        else
        {
            struct tm       *tm;
            char            buffer[1000];
            string_ty       *s;

            tm = localtime(&mtime);
            strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", tm);
            s = str_from_c(buffer);
            string_list_append(result, s);
            str_free(s);
        }
    }
    return 0;
}


builtin_ty builtin_mtime =
{
    "mtime",
    mtime_interpret,
    mtime_interpret,            /* script */
};


static int
mtime_seconds_interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("mtime-seconds\n"));
    (void)pp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        time_t          mtime;
        long            depth;

        depth = 32767;
        mtime = cook_mtime_newest(ocp, args->string[j], &depth, depth);
        if (mtime < 0)
            return -1;
        if (mtime == 0)
            string_list_append(result, str_false);
        else
        {
            string_ty       *s;

            s = str_format("%ld", (long)mtime);
            string_list_append(result, s);
            str_free(s);
        }
    }
    return 0;
}


builtin_ty builtin_mtime_seconds =
{
    "mtime-seconds",
    mtime_seconds_interpret,
    mtime_seconds_interpret,    /* script */
};
