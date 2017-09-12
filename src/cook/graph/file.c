/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2009 Peter Miller
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

#include <cook/graph/file.h>
#include <cook/graph/recipe_list.h>
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>


/*
 * NAME
 *      graph_file_new
 *
 * SYNOPSIS
 *      graph_file_ty *graph_file_new(string_ty *);
 *
 * DESCRIPTION
 *      The graph_file_new function is used to allocate a new graph file
 *      instance in dynamic memory.
 *
 * RETURNS
 *      graph_file_ty *
 *
 * CAVEAT
 *      Use graph_file_delete when you are done with it.
 */

graph_file_ty  *
graph_file_new(string_ty *fn)
{
    graph_file_ty   *gfp;

    trace(("graph_file_new(fn = \"%s\")\n{\n", fn->str_text));
    gfp = mem_alloc(sizeof(graph_file_ty));
    gfp->reference_count = 1;
    gfp->filename = str_copy(fn);
    gfp->input = graph_recipe_list_nrc_new();
    gfp->output = graph_recipe_list_nrc_new();
    gfp->pending = 0;
    gfp->previous_backtrack = 0;
    gfp->previous_error = 0;
    gfp->primary_target = 0;
    trace(("return %p;\n", gfp));
    trace(("}\n"));
    return gfp;
}


/*
 * NAME
 *      graph_file_delete
 *
 * SYNOPSIS
 *      void graph_file_delete(graph_file_ty *);
 *
 * DESCRIPTION
 *      The graph_file_delete function is used to release the resources
 *      held by a grapg file instance in dynamic memory.
 */

void
graph_file_delete(graph_file_ty *gfp)
{
    trace(("graph_file_delete(gfp = %p)\n{\n", gfp));
    assert(gfp);
    assert(gfp->reference_count > 0);
    gfp->reference_count--;
    if (gfp->reference_count > 0)
    {
        trace(("}\n"));
        return;
    }
    trace_string(gfp->filename->str_text);
    str_free(gfp->filename);
    gfp->filename = 0;
    graph_recipe_list_nrc_delete(gfp->input);
    gfp->input = 0;
    graph_recipe_list_nrc_delete(gfp->output);
    gfp->output = 0;
    mem_free(gfp);
    trace(("}\n"));
}


/*
 * NAME
 *      graph_file_copy
 *
 * SYNOPSIS
 *      graph_file_ty *graph_file_copy(graph_file_ty *);
 *
 * DESCRIPTION
 *      The graph_file_copy function is used to copy a graph file
 *      instance in dynamic memory.
 *
 * RETURNS
 *      graph_file_ty *
 *
 * CAVEAT
 *      It actually uses reference counting, it's not as bad as it
 *      sounds.
 */

graph_file_ty  *
graph_file_copy(graph_file_ty *gfp)
{
    assert(gfp);
    assert(gfp->reference_count > 0);
    gfp->reference_count++;
    return gfp;
}
