/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2009 Peter Miller
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

#include <cook/match/stack.h>
#include <common/mem.h>
#include <common/trace.h>


/*
 * NAME
 *      match_stack_push - patch match fields
 *
 * SYNOPSIS
 *      void match_stack_push(match_ty *field);
 *
 * DESCRIPTION
 *      The match_stack_push function is used to push a pattern onto the
 *      stack of match fields.  A NULL pointer may be pushed.  This
 *      mechanism is used by the chef (cook.c) to indicate implicit and
 *      explicit recipe replacements.
 *
 * RETURNS
 *      void
 */

void
match_stack_push(match_stack_ty *msp, const match_ty *field)
{
    trace(("match_stack_push(field = %p)\n{\n", field));
    if (msp->stack_depth >= msp->stack_depth_max)
    {
        size_t          nbytes;

        msp->stack_depth_max = msp->stack_depth_max * 2 + 4;
        nbytes = msp->stack_depth_max * sizeof(msp->stack[0]);
        msp->stack = mem_change_size(msp->stack, nbytes);
    }
    msp->stack[msp->stack_depth++] = field;
    trace(("}\n"));
}


/*
 * NAME
 *      match_stack_top - top of match stack
 *
 * SYNOPSIS
 *      match_ty *match_stack_top(void);
 *
 * DESCRIPTION
 *      The match_stack_top function is used to indicate the top of the
 *      match stack.
 *
 * RETURNS
 *      match_ty * - a pointer to a match strcuture, or NULL if the stack is
 *      empty, or a NULL was pashed to mak an exlpicit recipe.
 */

const match_ty *
match_stack_top(const match_stack_ty *msp)
{
    if (msp->stack_depth <= 0)
        return 0;
    return msp->stack[msp->stack_depth - 1];
}


/*
 * NAME
 *      match_stack_pop - shorten stack
 *
 * SYNOPSIS
 *      match_ty *match_stack_pop(void);
 *
 * DESCRIPTION
 *      The match_stack_pop function is used to pop a match structure
 *      from the match stack.
 *
 * RETURNS
 *      match_ty * - a pointer to a match strcuture, or NULL if the stack is
 *      empty, or a NULL was pashed to mak an exlpicit recipe.
 *
 * CAVEAT
 *      It is an error for the stack to be empty.
 */

const match_ty *
match_stack_pop(match_stack_ty *msp)
{
    const match_ty  *field;

    trace(("match_stack_pop()\n{\n"));
    assert(msp->stack_depth);
    if (msp->stack_depth > 0)
    {
        --msp->stack_depth;
        field = msp->stack[msp->stack_depth];
    }
    else
        field = 0;
    trace(("return %p;\n", field));
    trace(("}\n"));
    return field;
}


match_stack_ty *
match_stack_new(void)
{
    match_stack_ty  *msp;

    msp = mem_alloc(sizeof(match_stack_ty));
    msp->stack = 0;
    msp->stack_depth = 0;
    msp->stack_depth_max = 0;
    return msp;
}


void
match_stack_delete(match_stack_ty *msp)
{
    assert(msp);
    if (msp->stack)
        mem_free(msp->stack);
    mem_free(msp);
}
