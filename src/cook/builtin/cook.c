/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2006-2008 Peter Miller
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
 *
 * Only a limited set of this are candidates for builtin functions,
 * these are
 *      - string manipulation [dirname, stringset, ect ]
 *      - environment manipulation [getenv(3), etc]
 *      - stat(3) related functions [exists, mtime, pathname, etc]
 *      - launching OS commands [execute, collect]
 * The above list is though to be exhaustive.
 *
 * This explicitly and forever excluded from being a builtin function
 * is anything which known or understands the format of some secific
 * class of files.
 *
 * Access to stdio(3) has been thought of, and explicitly avoided.
 * Mostly because a specialist program used through [collect]
 * will almost always be far faster.
 */

#include <cook/builtin/cook.h>
#include <cook/desist.h>
#include <cook/graph.h>
#include <cook/graph/build.h>
#include <cook/graph/stats.h>
#include <cook/graph/walk.h>
#include <cook/option.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_cook - test if files are up to date
 *
 * SYNOPSIS
 *      int builtin_cook(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The cook function is a built-in of cook, described as follows:
 *      returns true if all the arguments were or are now up-to-date.
 *      Builds them if they are not.
 *      This function requires one or more arguments.
 *
 * RETURNS
 *      A word list containing true ("1") if all arguments are up-to-date,
 *      or false ("") if one or more could not.
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
    int             retval;
    graph_ty        *gp;
    graph_build_status_ty gb_status;
    graph_walk_status_ty gw_status;

    trace(("cook\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings <= 1)
        return 0;

    /*
     * set interrupts to catch
     *
     * Note that tee(1) [see listing.c] must ignore them
     * for the generated messages to appear in the log file.
     */
    desist_enable();

    /*
     * Build the dependency graph.
     */
    retval = 0;
    gp = graph_new();
    for (j = 1; j < args->nstrings; ++j)
    {
        gb_status =
            graph_build
            (
                gp,
                args->string[j],
                graph_build_preference_backtrack,
                0
            );
        switch (gb_status)
        {
        case graph_build_status_error:
            retval = -1;
            break;

        case graph_build_status_backtrack:
            break;

        case graph_build_status_success:
            string_list_append(result, args->string[j]);
            break;
        }
    }

    /*
     * Walk the dependency graph.
     */
    if (retval >= 0)
    {
        if (option_test(OPTION_REASON))
            graph_print_statistics(gp);
        gw_status = graph_walk(gp);
        switch (gw_status)
        {
        case graph_walk_status_uptodate:
        case graph_walk_status_uptodate_done:
        case graph_walk_status_done:
            break;

        case graph_walk_status_done_stop:
        case graph_walk_status_wait:
            assert(0);
            /* fall through... */

        case graph_walk_status_error:
            retval = -1;
            break;
        }
    }

    /*
     * Release resources held by the graph.
     */
    graph_delete(gp);
    return retval;
}


builtin_ty builtin_cook =
{
    "cook",
    interpret,
    interpret,                  /* script */
};
