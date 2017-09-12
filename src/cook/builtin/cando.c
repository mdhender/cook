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

#include <cook/builtin/cando.h>
#include <cook/desist.h>
#include <common/error.h>       /* for assert */
#include <cook/graph.h>
#include <cook/graph/build.h>
#include <cook/graph/stats.h>
#include <cook/option.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_cando - test if know how to cook given files
 *
 * SYNOPSIS
 *      int builtin_cando(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The cando function is a built-in of cook, which may be used to
 *      determine which files can be cooked.
 *
 * RETURNS
 *      A word list containing all of the arguments for which derivations
 *      can be found, or nothing if none can.
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

    (void)pp;
    (void)ocp;
    if (args->nstrings <= 1)
        return 0;

    /*
     * set interrupts to catch
     *
     * Note that tee(1) [see listing.c] must ignore them
     * for the generated messages to appear in the log file.
     */
    trace(("cando\n"));
    assert(result);
    assert(args);
    assert(args->nstrings);
    retval = 0;
    desist_enable();

    /*
     * build the graph
     */
    gp = graph_new();
    for (j = 1; j < args->nstrings; ++j)
    {
        graph_build_status_ty gb_status;

        /*
         * Build the dependency graph.
         */
        gb_status =
            graph_build
            (
                gp,
                args->string[j],
                graph_build_preference_backtrack,
                0
            );

        /*
         * it is only relevant that we know how to build this
         * graph, not that we walk it.
         */
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
     * Release resources held by the graph.
     */
    if (option_test(OPTION_REASON))
        graph_print_statistics(gp);
    graph_delete(gp);
    return retval;
}


builtin_ty builtin_cando =
{
    "cando",
    interpret,
    interpret,                  /* script */
};
