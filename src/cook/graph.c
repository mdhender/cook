/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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

#include <cook/graph.h>
#include <cook/graph/file.h>
#include <cook/graph/file_pair.h>
#include <cook/graph/recipe_list.h>
#include <common/mem.h>
#include <common/str_list.h>
#include <common/symtab.h>


/*
 * NAME
 *      already_reap
 *
 * SYNOPSIS
 *      void already_reap(void *);
 *
 * DESCRIPTION
 *      The already_reap function is used to delete graph file instances
 *      when the graph file symbol table is deleted.
 */

static void
already_reap(void *p)
{
    graph_file_ty   *gfp;

    gfp = p;
    graph_file_delete(gfp);
}


/*
 * NAME
 *      graph_new
 *
 * SYNOPSIS
 *      graph_ty *graph_new(void);
 *
 * DESCRIPTION
 *      The graph_new function is used to allocate a new empty file
 *      dependency graph in dynamic memory.
 *
 * RETURNS
 *      graph_ty *
 *
 * CAVEAT
 *      Use graph_delete when you are done with it.
 */

graph_ty *
graph_new(void)
{
    graph_ty        *gp;

    gp = mem_alloc(sizeof(graph_ty));
    gp->try_list = 0;
    gp->statistic.backtrack_bad_path = 0;
    gp->statistic.backtrack_by_ingredient = 0;
    gp->statistic.backtrack_cache = 0;
    gp->statistic.error_by_ingredient = 0;
    gp->statistic.error_cache = 0;
    gp->statistic.error_in_expr = 0;
    gp->statistic.explicit_applicable = 0;
    gp->statistic.explicit_ingredients_applicable = 0;
    gp->statistic.explicit_ingredients_not_applicable = 0;
    gp->statistic.explicit_not_applicable = 0;
    gp->statistic.implicit_applicable = 0;
    gp->statistic.implicit_ingredients_applicable = 0;
    gp->statistic.implicit_ingredients_not_applicable = 0;
    gp->statistic.implicit_not_applicable = 0;
    gp->statistic.infinite_loop = 0;
    gp->statistic.inhibit_self_recursion = 0;
    gp->statistic.leaf_backtrack = 0;
    gp->statistic.leaf_error = 0;
    gp->statistic.leaf_exists = 0;
    gp->statistic.pattern_match_query = 0;
    gp->statistic.phony = 0;
    gp->statistic.precondition_rejection = 0;
    gp->statistic.success = 0;
    gp->statistic.success_reuse = 0;
    gp->already = symtab_alloc(100);
    gp->already->reap = already_reap;
    gp->already_recipe = graph_recipe_list_new();
    gp->file_pair = 0;
    return gp;
}


/*
 * NAME
 *      graph_delete
 *
 * SYNOPSIS
 *      void graph_delete(graph_ty *);
 *
 * DESCRIPTION
 *      The graph_delete function is used to release resources held by a
 *      file dependency graph.
 */

void
graph_delete(graph_ty *gp)
{
    if (gp->try_list)
        string_list_delete(gp->try_list);
    symtab_free(gp->already);
    graph_recipe_list_delete(gp->already_recipe);
    if (gp->file_pair)
        graph_file_pair_delete(gp->file_pair);
    mem_free(gp);
}


int
graph_file_leaf_p(graph_ty *gp, string_ty *filename)
{
    graph_file_ty   *gfp;

    gfp = symtab_query(gp->already, filename);
    if (!gfp)
        return -1;
    return (gfp->input->nrecipes == 0);
}


static void
walk_interior_files(symtab_ty *stp, string_ty *key, void *data_p, void *aux_p)
{
    graph_file_ty   *gfp;
    string_list_ty  *result;

    (void)stp;
    gfp = data_p;
    result = aux_p;
    if (gfp->input->nrecipes != 0)
        string_list_append(result, key);
}


void
graph_interior_files(graph_ty *gp, string_list_ty *result)
{
    symtab_walk(gp->already, walk_interior_files, result);
}


static void
walk_leaf_files(symtab_ty *stp, string_ty *key, void *data_p, void *aux_p)
{
    graph_file_ty   *gfp;
    string_list_ty  *result;

    (void)stp;
    gfp = data_p;
    result = aux_p;
    if (gfp->input->nrecipes == 0)
        string_list_append(result, key);
}


void
graph_leaf_files(graph_ty *gp, string_list_ty *result)
{
    symtab_walk(gp->already, walk_leaf_files, result);
}
