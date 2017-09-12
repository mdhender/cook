/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006, 2007 Peter Miller;
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

#include <cook/graph/recipe.h>
#include <cook/graph/recipe_list.h>
#include <common/mem.h>
#include <common/trace.h>


/*
 * NAME
 *      graph_recipe_list_constructor
 *
 * SYNOPSIS
 *      void graph_recipe_list_constructor(graph_recipe_list_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_list_constructor function is used to initialize
 *      a graph_recipe_list_ty data structure.  The list is initially
 *      empty.
 */

void
graph_recipe_list_constructor(graph_recipe_list_ty *grlp)
{
    trace(("graph_recipe_list_constructor(grlp = %8.8lX)\n{\n", (long)grlp));
    grlp->nrecipes = 0;
    grlp->nrecipes_max = 0;
    grlp->recipe = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      graph_recipe_list_destructor
 *
 * SYNOPSIS
 *      void graph_recipe_list_destructor(graph_recipe_list_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_list_destructor function is used to release the
 *      resources used by a graph_recipe_list_ty data structure.  Items
 *      on the list will also be deleted.
 */

void
graph_recipe_list_destructor(graph_recipe_list_ty *grlp)
{
    size_t          j;

    trace(("graph_recipe_list_destructor(grlp = %8.8lX)\n{\n", (long)grlp));
    for (j = 0; j < grlp->nrecipes; ++j)
        graph_recipe_delete(grlp->recipe[j]);
    if (grlp->recipe)
        mem_free(grlp->recipe);
    grlp->nrecipes = 0;
    grlp->nrecipes_max = 0;
    grlp->recipe = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      recipe_list_append
 *
 * SYNOPSIS
 *      void recipe_list_append(graph_recipe_list_ty *, graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The recipe_list_append function is used to append a graph recipe
 *      to the a list, if it is not there already.  The graph recipe
 *      reference count will be incrimented, if it is used.
 */

void
graph_recipe_list_append(graph_recipe_list_ty *grlp, graph_recipe_ty *grp)
{
    size_t          j;

    trace(("graph_recipe_list_append(grlp = %8.8lX, grp = %8.8lX)\n{\n",
        (long)grlp, (long)grp));
    for (j = 0; j < grlp->nrecipes; ++j)
    {
        if (grlp->recipe[j] == grp)
        {
            trace(("}\n"));
            return;
        }
    }
    if (grlp->nrecipes >= grlp->nrecipes_max)
    {
        size_t          nbytes;

        grlp->nrecipes_max = grlp->nrecipes_max * 2 + 4;
        nbytes = grlp->nrecipes_max * sizeof(grlp->recipe[0]);
        grlp->recipe = mem_change_size(grlp->recipe, nbytes);
    }
    grlp->recipe[grlp->nrecipes++] = graph_recipe_copy(grp);
    trace(("}\n"));
}


/*
 * NAME
 *      recipe_list_append_list
 *
 * SYNOPSIS
 *      void recipe_list_append_list(graph_recipe_list_ty *,
 *              graph_recipe_list_ty *);
 *
 * DESCRIPTION
 *      The recipe_list_append_list function is used to append a graph
 *      recipe list to the end of a list, if ther contents are not there
 *      already.  The graph recipe reference counts will be incrimented,
 *      if they are used.
 */

void
graph_recipe_list_append_list(graph_recipe_list_ty *to,
    graph_recipe_list_ty *from)
{
    size_t          j;

    trace(("graph_recipe_list_append_list(to = %8.8lX, from = %8.8lX)\n{\n",
        (long)to, (long)from));
    for (j = 0; j < from->nrecipes; ++j)
        graph_recipe_list_append(to, from->recipe[j]);
    trace(("}\n"));
}


/*
 * NAME
 *      graph_recipe_list_new
 *
 * SYNOPSIS
 *      graph_recipe_list_ty *graph_recipe_list_new(void);
 *
 * DESCRIPTION
 *      The graph_recipe_list_new function is used to allocate a nre
 *      graph recipe list is dynamic memory.  It is initially empty.
 *
 * RETURNS
 *      graph_recipe_list_ty *; pointer to list is dynamic memory.
 *      Release with graph_recipe_list_delete when you are throgh with
 *      it.
 */

graph_recipe_list_ty *
graph_recipe_list_new(void)
{
    graph_recipe_list_ty *grlp;

    trace(("graph_recipe_list_new()\n{\n"));
    grlp = mem_alloc(sizeof(graph_recipe_list_ty));
    graph_recipe_list_constructor(grlp);
    trace(("return %8.8lX;\n", (long)grlp));
    trace(("}\n"));
    return grlp;
}


/*
 * NAME
 *      graph_recipe_list_delete
 *
 * SYNOPSIS
 *      void graph_recipe_list_delete(graph_recipe_list_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_list_delete function is used to release the
 *      resoures referenced by a recipe list in dynamic memory.
 */

void
graph_recipe_list_delete(graph_recipe_list_ty *grlp)
{
    trace(("graph_recipe_list_delete(grlp = %8.8lX)\n{\n", (long)grlp));
    graph_recipe_list_destructor(grlp);
    mem_free(grlp);
    trace(("}\n"));
}


/*
 * NAME
 *      graph_recipe_list_nrc_constructor
 *
 * SYNOPSIS
 *      void graph_recipe_list_nrc_constructor(graph_recipe_list_nrc_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_list_nrc_constructor function is used to initialize
 *      a graph_recipe_list_nrc_ty data structure.  The list is initially
 *      empty.
 */

void
graph_recipe_list_nrc_constructor(graph_recipe_list_nrc_ty *grlp)
{
    trace(("graph_recipe_list_nrc_constructor(grlp = %8.8lX)\n{\n",
        (long)grlp));
    grlp->nrecipes = 0;
    grlp->nrecipes_max = 0;
    grlp->recipe = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      graph_recipe_list_nrc_destructor
 *
 * SYNOPSIS
 *      void graph_recipe_list_nrc_destructor(graph_recipe_list_nrc_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_list_nrc_destructor function is used to release the
 *      resources used by a graph_recipe_list_nrc_ty data structure.  Items
 *      on the list will NOT be deleted.
 */

void
graph_recipe_list_nrc_destructor(graph_recipe_list_nrc_ty *grlp)
{
    trace(("graph_recipe_list_nrc_destructor(grlp = %8.8lX)\n{\n", (long)grlp));
    if (grlp->recipe)
        mem_free(grlp->recipe);
    grlp->nrecipes = 0;
    grlp->nrecipes_max = 0;
    grlp->recipe = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      recipe_list_nrc_append
 *
 * SYNOPSIS
 *      void recipe_list_nrc_append(graph_recipe_list_nrc_ty *,
 *              graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The recipe_list_nrc_append function is used to append a graph recipe
 *      to the a list, if it is not there already.  The graph recipe
 *      reference count will NOT be incrimented.
 */

void
graph_recipe_list_nrc_append(graph_recipe_list_nrc_ty *grlp,
    graph_recipe_ty *grp)
{
    size_t          j;

    trace(("graph_recipe_list_nrc_append(grlp = %8.8lX, grp = %8.8lX)\n{\n",
        (long)grlp, (long)grp));
    for (j = 0; j < grlp->nrecipes; ++j)
    {
        if (grlp->recipe[j] == grp)
        {
            trace(("}\n"));
            return;
        }
    }
    if (grlp->nrecipes >= grlp->nrecipes_max)
    {
        size_t          nbytes;

        grlp->nrecipes_max = grlp->nrecipes_max * 2 + 4;
        nbytes = grlp->nrecipes_max * sizeof(grlp->recipe[0]);
        grlp->recipe = mem_change_size(grlp->recipe, nbytes);
    }
    grlp->recipe[grlp->nrecipes++] = grp;
    trace(("}\n"));
}


/*
 * NAME
 *      recipe_list_nrc_append_list
 *
 * SYNOPSIS
 *      void recipe_list_nrc_append_list(graph_recipe_list_nrc_ty *,
 *              graph_recipe_list_nrc_ty *);
 *
 * DESCRIPTION
 *      The recipe_list_nrc_append_list function is used to append a graph
 *      recipe list to the end of a list, if ther contents are not there
 *      already.  The graph recipe reference counts will NOT be incrimented,
 *      if they are used.
 */

void
graph_recipe_list_nrc_append_list(graph_recipe_list_nrc_ty *to,
    graph_recipe_list_nrc_ty *from)
{
    size_t          j;

    trace(("graph_recipe_list_nrc_append_list(to = %8.8lX, from = %8.8lX)\n{\n",
        (long)to, (long)from));
    for (j = 0; j < from->nrecipes; ++j)
        graph_recipe_list_nrc_append(to, from->recipe[j]);
    trace(("}\n"));
}


/*
 * NAME
 *      graph_recipe_list_nrc_new
 *
 * SYNOPSIS
 *      graph_recipe_list_nrc_ty *graph_recipe_list_nrc_new(void);
 *
 * DESCRIPTION
 *      The graph_recipe_list_nrc_new function is used to allocate a nre
 *      graph recipe list is dynamic memory.  It is initially empty.
 *
 * RETURNS
 *      graph_recipe_list_nrc_ty *; pointer to list is dynamic memory.
 *      Release with graph_recipe_list_nrc_delete when you are throgh with
 *      it.
 */

graph_recipe_list_nrc_ty *
graph_recipe_list_nrc_new(void)
{
    graph_recipe_list_nrc_ty *grlp;

    trace(("graph_recipe_list_nrc_new()\n{\n"));
    grlp = mem_alloc(sizeof(graph_recipe_list_nrc_ty));
    graph_recipe_list_nrc_constructor(grlp);
    trace(("return %8.8lX;\n", (long)grlp));
    trace(("}\n"));
    return grlp;
}


/*
 * NAME
 *      graph_recipe_list_nrc_delete
 *
 * SYNOPSIS
 *      void graph_recipe_list_nrc_delete(graph_recipe_list_nrc_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_list_nrc_delete function is used to release the
 *      resoures referenced by a recipe list in dynamic memory.
 */

void
graph_recipe_list_nrc_delete(graph_recipe_list_nrc_ty *grlp)
{
    trace(("graph_recipe_list_nrc_delete(grlp = %8.8lX)\n{\n", (long)grlp));
    graph_recipe_list_nrc_destructor(grlp);
    mem_free(grlp);
    trace(("}\n"));
}
