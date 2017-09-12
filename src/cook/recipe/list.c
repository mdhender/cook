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

#include <common/error.h> /* for assert */
#include <common/mem.h>
#include <cook/recipe.h>
#include <cook/recipe/list.h>
#include <common/trace.h>


/*
 * NAME
 *      recipe_list_append - append a recipe
 *
 * SYNOPSIS
 *      void recipe_list_append(recipe_list_ty *rlp, recipe_ty *rp);
 *
 * DESCRIPTION
 *      Rl_append is used to append a recipe to a recipe list.
 *
 * CAVEAT
 *      Recipes need to overwrite other recipes with
 *      identical target and prerequisite lists.
 */

void
recipe_list_append(recipe_list_ty *rlp, recipe_ty *rp)
{
    trace(("rl_append(rlp = %p, rp = %p)\n{\n", rlp, rp));
    assert(rlp);
    assert(rp);

    if (rlp->nrecipes >= rlp->nrecipes_max)
    {

        size_t          nbytes;

        rlp->nrecipes_max = rlp->nrecipes_max * 2 + 4;
        nbytes = rlp->nrecipes_max * sizeof(rlp->recipe[0]);
        rlp->recipe = mem_change_size(rlp->recipe, nbytes);
    }
    rlp->recipe[rlp->nrecipes++] = recipe_copy(rp);
    trace(("}\n"));
}


void
recipe_list_constructor(recipe_list_ty *rlp)
{
    trace(("recipe_list_constructor(rlp = %p)\n{\n", rlp));
    rlp->recipe = 0;
    rlp->nrecipes = 0;
    rlp->nrecipes_max = 0;
    trace(("}\n"));
}


void
recipe_list_destructor(recipe_list_ty *rlp)
{
    size_t          j;

    trace(("recipe_list_destructor(rlp = %p)\n{\n", rlp));
    assert(rlp);
    for (j = 0; j < rlp->nrecipes; ++j)
        recipe_delete(rlp->recipe[j]);
    if (rlp->recipe)
        mem_free(rlp->recipe);
    rlp->recipe = 0;
    rlp->nrecipes_max = 0;
    rlp->nrecipes = 0;
    trace(("}\n"));
}


recipe_list_ty *
recipe_list_new(void)
{
    recipe_list_ty  *rlp;

    rlp = mem_alloc(sizeof(recipe_list_ty));
    recipe_list_constructor(rlp);
    return rlp;
}


void
recipe_list_delete(recipe_list_ty *rlp)
{
    recipe_list_destructor(rlp);
    mem_free(rlp);
}
