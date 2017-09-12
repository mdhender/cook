/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2000, 2006, 2007 Peter Miller;
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
 */

#include <cook/cook.h>
#include <cook/dir_part.h>
#include <common/error_intl.h>
#include <cook/graph/file.h>
#include <cook/graph/file_list.h>
#include <cook/graph/recipe.h>
#include <cook/id.h>
#include <cook/opcode/context.h>
#include <cook/match.h>
#include <common/mem.h>
#include <cook/option.h>
#include <cook/os_interface.h>
#include <cook/recipe.h>
#include <common/star.h>
#include <cook/stmt.h>
#include <common/trace.h>


/*
 * NAME
 *      graph_recipe_new
 *
 * SYNOPSIS
 *      graph_recipe_ty *graph_recipe_new(recipe_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_new function is used to allocate a new graph
 *      recipe instance in dynamic memory.
 *
 * RETURNS
 *      graph_recipe_ty *
 *
 * CAVEAT
 *      Use graph_recipe_delete when you are done with it.
 */

graph_recipe_ty *
graph_recipe_new(recipe_ty *rp)
{
    graph_recipe_ty *grp;
    static int      id;

    trace(("graph_recipe_new()\n{\n"));
    grp = mem_alloc(sizeof(graph_recipe_ty));
    grp->reference_count = 1;
    grp->id = ++id;
    grp->rp = recipe_copy(rp);
    grp->mp = 0;
    grp->input = graph_file_list_nrc_new();
    grp->output = graph_file_list_nrc_new();
    grp->input_satisfied = 0;
    grp->input_uptodate = 0;
    grp->ocp = 0;
    grp->single_thread = 0;
    grp->host_binding = 0;
    grp->multi_forced = 0;
    trace(("return %08lX;\n", (long)grp));
    trace(("}\n"));
    return grp;
}


/*
 * NAME
 *      graph_recipe_delete
 *
 * SYNOPSIS
 *      void graph_recipe_delete(graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_delete function is used to release the
 *      resources held by a graph recipe instance in dynamic memory.
 */

void
graph_recipe_delete(graph_recipe_ty *grp)
{
    trace(("graph_recipe_delete(grp = %08lX)\n{\n", (long)grp));
    assert(grp->reference_count > 0);
    grp->reference_count--;
    if (grp->reference_count > 0)
    {
        trace(("}\n"));
        return;
    }
    recipe_delete(grp->rp);
    if (grp->mp)
    {
        match_delete(grp->mp);
        grp->mp = 0;
    }
    graph_file_list_nrc_delete(grp->input);
    graph_file_list_nrc_delete(grp->output);
    if (grp->ocp)
        opcode_context_delete(grp->ocp);
    if (grp->single_thread)
        string_list_delete(grp->single_thread);
    if (grp->host_binding)
        string_list_delete(grp->host_binding);
    mem_free(grp);
    trace(("}\n"));
}


/*
 * NAME
 *      graph_recipe_copy
 *
 * SYNOPSIS
 *      graph_recipe_ty *graph_recipe_copy(graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_copy function is used to copy a graph recipe
 *      instance in dynamic memory.
 */

graph_recipe_ty *
graph_recipe_copy(graph_recipe_ty *grp)
{
    assert(grp->reference_count > 0);
    grp->reference_count++;
    return grp;
}


/*
 * NAME
 *      graph_recipe_getpid
 *
 * SYNOPSIS
 *      int graph_recipe_getpid(graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_getpid function is used to get the process-id
 *      of the process to wait for.
 *
 * CAVEAT
 *      Must only be used when graph_recipe_run returns
 *      graph_walk_status_wait.
 */

int
graph_recipe_getpid(graph_recipe_ty *grp)
{
    assert(grp);
    assert(grp->ocp);
    return opcode_context_getpid(grp->ocp);
}


/*
 * NAME
 *      graph_recipe_waited
 *
 * SYNOPSIS
 *      void graph_recipe_waited(graph_recipe_ty *, int);
 *
 * DESCRIPTION
 *      The graph_recipe_waited function is used to set the exit status
 *      after a waiting graph walk is about to resume.
 */

void
graph_recipe_waited(graph_recipe_ty *grp, int status)
{
    assert(grp);
    assert(grp->ocp);
    opcode_context_waited(grp->ocp, status);
}
