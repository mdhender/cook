/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>

#include <cook/expr.h>
#include <common/error_intl.h>
#include <cook/flag.h>
#include <cook/match/new_by_recip.h>
#include <cook/match/wl.h>
#include <common/mem.h>
#include <cook/opcode/list.h>
#include <cook/option.h>
#include <cook/recipe.h>
#include <cook/stmt.h>
#include <cook/strip_dot.h>
#include <common/trace.h>


/*
 * NAME
 *      recipe_destructor
 *
 * SYNOPSIS
 *      void recipe_destructor(recipe_ty *);
 *
 * DESCRIPTION
 *      The recipe_destructor function is used to release the resources
 *      held by a recipe_ty structure.
 */

static void
recipe_destructor(recipe_ty *rp)
{
    trace(("recipe_destructor(rp = %08lX)\n{\n", (long)rp));
    if (rp->target)
        string_list_delete(rp->target);
    if (rp->need1)
        opcode_list_delete(rp->need1);
    if (rp->need2)
        opcode_list_delete(rp->need2);
    if (rp->flags)
        flag_delete(rp->flags);
    if (rp->precondition)
        opcode_list_delete(rp->precondition);
    if (rp->single_thread)
        opcode_list_delete(rp->single_thread);
    if (rp->host_binding)
        opcode_list_delete(rp->host_binding);
    if (rp->out_of_date)
        opcode_list_delete(rp->out_of_date);
    if (rp->up_to_date)
        opcode_list_delete(rp->up_to_date);
    expr_position_destructor(&rp->pos);
    trace(("}\n"));
}


/*
 * NAME
 *      recipe_delete
 *
 * SYNOPSIS
 *      void recipe_delete(recipe_ty *);
 *
 * DESCRIPTION
 *      The recipe_delete function is used to release the resources
 *      held by a recipe_ty structure in dynamic memory.
 */

void
recipe_delete(recipe_ty *rp)
{
    trace(("recipe_delete(rp = %08lX)\n{\n", (long)rp));
    assert(rp->reference_count > 0);
    rp->reference_count--;
    if (rp->reference_count <= 0)
    {
        recipe_destructor(rp);
        mem_free(rp);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      recipe_copy
 *
 * SYNOPSIS
 *      recipe_ty *recipe_copy(recipe_ty *);
 *
 * DESCRIPTION
 *      The recipe_copy function is used to make a copy of a recipe in
 *      dynamic memory.
 *
 * RETURNS
 *      recipe_ty *; pointer to new copy.
 *
 * CAVEAT
 *      Actually uses reference counting.
 */

recipe_ty *
recipe_copy(recipe_ty *rp)
{
    trace(("recipe_copy(rp = %08lX)\n{\n", (long)rp));
    assert(rp);
    assert(rp->reference_count > 0);
    rp->reference_count++;
    trace(("}\n"));
    return rp;
}


/*
 * NAME
 *      recipe_constructor
 *
 * SYNOPSIS
 *      void recipe_constructor(recipe_ty *, string_list_ty *,
 *              expr_list_ty *, expr_list_ty *, flag_ty *, int, expr_ty *,
 *              stmt_ty *, stmt_ty *, expr_position_ty *);
 *
 * DESCRIPTION
 *      The recipe_constructor function is used to intitialize a
 *      recipe_ty structure.
 */

static int
recipe_constructor(recipe_ty *rp, string_list_ty *target, opcode_list_ty *need1,
    opcode_list_ty *need2, flag_ty *flags, int multiple,
    opcode_list_ty *precondition, opcode_list_ty *single_thread,
    opcode_list_ty *host_binding, opcode_list_ty *out_of_date,
    opcode_list_ty *up_to_date, const expr_position_ty *pp)
{
    size_t          j;
    match_ty        *mp;

    trace(("recipe_constructor(rp = %08lX)\n{\n", (long)rp));
    rp->target = string_list_new_copy(target);
    rp->need1 = need1 ? opcode_list_copy(need1) : 0;
    rp->need2 = need2 ? opcode_list_copy(need2) : 0;
    rp->precondition = precondition ? opcode_list_copy(precondition) : 0;
    rp->flags = flags ? flag_copy(flags) : 0;
    rp->multiple = multiple;
    rp->precondition = precondition ? opcode_list_copy(precondition) : 0;
    rp->single_thread = single_thread ? opcode_list_copy(single_thread) : 0;
    rp->host_binding = host_binding ? opcode_list_copy(host_binding) : 0;
    rp->out_of_date = out_of_date ? opcode_list_copy(out_of_date) : 0;
    rp->up_to_date = up_to_date ? opcode_list_copy(up_to_date) : 0;
    expr_position_copy_constructor(&rp->pos, pp);

    strip_dot_list(rp->target);
    rp->inhibit = 0;

    /*
     * is it implicit or explicit?
     */
    mp = match_new_by_recipe(rp);
    rp->implicit = match_wl_usage_mask(mp, rp->target, &rp->pos);
    if (rp->implicit)
    {
        string_ty       *s;
        int             all_used;

        /* make sure all the bits get used at least once */
        all_used = 0;
        for (j = 0; j < rp->target->nstrings; ++j)
        {
            s = rp->target->string[j];
            if (rp->implicit == match_usage_mask(mp, s, &rp->pos))
                ++all_used;
        }
        if (!all_used)
        {
            error_with_position
            (
                &rp->pos,
                0,
                i18n("at least one target of an implicit recipe must use all "
                    "of the named pattern elements")
            );
            match_delete(mp);
            trace(("}\n"));
            return -1;
        }
    }
    match_delete(mp);
    trace(("}\n"));
    return 0;
}


/*
 * NAME
 *      recipe_new
 *
 * SYNOPSIS
 *      recipe_ty *recipe_new(string_list_ty *target, expr_list_ty *need,
 *              expr_list_ty *need2, unsigned long flags, int multiple,
 *              expr_ty *precondition, stmt_ty *action, stmt_ty *use_action,
 *              expr_position_ty *pp);
 *
 * DESCRIPTION
 *      The recipe_new function is used to allocate a new recipe_ty
 *      structure in dynamic memory and initialize it.
 *
 * RETURNS
 *      recipe_ty *
 */

recipe_ty *
recipe_new(string_list_ty *target, opcode_list_ty *need1, opcode_list_ty *need2,
    flag_ty *flags, int multiple, opcode_list_ty *precondition,
    opcode_list_ty *single_thread, opcode_list_ty *host_binding,
    opcode_list_ty *out_of_date, opcode_list_ty *up_to_date,
    const expr_position_ty *pp)
{
    recipe_ty       *rp;
    int             result;

    trace(("recipe_new()\n{\n"));
    rp = mem_alloc(sizeof(recipe_ty));
    rp->reference_count = 1;
    result =
        recipe_constructor
        (
            rp,
            target,
            need1,
            need2,
            flags,
            multiple,
            precondition,
            single_thread,
            host_binding,
            out_of_date,
            up_to_date,
            pp
        );
    if (result < 0)
    {
        recipe_delete(rp);
        rp = 0;
    }
    trace(("return %08lX;\n", (long)rp));
    trace(("}\n"));
    return rp;
}


/*
 * NAME
 *      recipe_flags_set - set them
 *
 * SYNOPSIS
 *      void recipe_flags_set(recipe_ty *);
 *
 * DESCRIPTION
 *      The recipe_flags_set function is used to take a flags variable
 *      and set the appropriate options at the given level.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Use the option_undo_level function to remove the flag settings.
 */

void
recipe_flags_set(recipe_ty *rp)
{
    trace(("recipe_flags_set(rp = %8.8lX)\n{\n", (long)rp));
    if (rp->flags)
        flag_set_options(rp->flags, OPTION_LEVEL_RECIPE);
    trace(("}\n"));
}
