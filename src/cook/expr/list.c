/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006-2009 Peter Miller
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

#include <cook/expr.h>
#include <cook/expr/list.h>
#include <cook/match.h>
#include <common/mem.h>
#include <cook/opcode/list.h>
#include <cook/opcode/push.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      expr_list_destructor - free expression lists
 *
 * SYNOPSIS
 *      void expr_list_destructor(expr_list_ty *elp);
 *
 * DESCRIPTION
 *      The expr_list_destructor function is used to free expression lists,
 *      it calls expr_free for each expression in the list.
 *
 * CAVEAT
 *      It is assumed that the expressions are dynamically allocated,
 *      and that the expression list was grown using expr_list_append().
 *      The actual structure pointed to is NOT assumed to be in dynamic memory
 *      and should not be passed to free().
 */

void
expr_list_destructor(expr_list_ty *elp)
{
    size_t          j;

    trace(("expr_list_destructor(elp = %p)\n{\n", elp));
    for (j = 0; j < elp->el_nexprs; j++)
        expr_delete(elp->el_expr[j]);
    if (elp->el_nexprs)
        mem_free(elp->el_expr);
    elp->el_nexprs = 0;
    elp->el_nexprs_max = 0;
    elp->el_expr = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      expr_list_delete
 *
 * SYNOPSIS
 *      void expr_list_delete(expr_list_ty *);
 *
 * DESCRIPTION
 *      The expr_list_delete function is used to release the resources
 *      held by an expression list.
 */

void
expr_list_delete(expr_list_ty *elp)
{
    expr_list_destructor(elp);
    mem_free(elp);
}


/*
 * NAME
 *      expr_list_copy_constructor - copy expression list
 *
 * SYNOPSIS
 *      void expr_list_copy_constructor(expr_list_ty *to, expr_list_ty *from);
 *
 * DESCRIPTION
 *      The expr_list_copy_constructor function is used to copy the list
 *      of expression trees pointed to by `from' into the expression
 *      list pointed to by `to'.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      The expr_list_destructor function must be used to dispose of
 *      the list when finished with.
 */

void
expr_list_copy_constructor(expr_list_ty *to, const expr_list_ty *from)
{
    size_t          j;

    trace(("expr_list_copy_constructor(to = %p, from = %p)\n{\n",
            to, from));
    expr_list_constructor(to);
    for (j = 0; j < from->el_nexprs; j++)
        expr_list_append(to, from->el_expr[j]);
    trace(("}\n"));
}


/*
 * NAME
 *      expr_list_append - append to an expression list
 *
 * SYNOPSIS
 *      void expr_list_append(expr_list_ty *el, expr *e);
 *
 * DESCRIPTION
 *      The expr_list_append function is used to append an expression
 *      to an expression list.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      The expression has not been copied, so do not hand it
 *      to expr_free after you append it.
 *
 *      It is assumed that the expr_list_ty has been previously initialised by a
 *          expr_list_ty el;
 *          expr_list_constructor(&el);
 *      statement (or similar) before this function is called.
 */

void
expr_list_append(expr_list_ty *el, expr_ty *e)
{
    size_t          nbytes;

    trace(("expr_list_append(el = %p, e = %p)\n{\n", el, e));
    assert(el);
    assert(e);
    assert(!el->el_nexprs_max || !!el->el_expr);
    if (el->el_nexprs >= el->el_nexprs_max)
    {
        el->el_nexprs_max = el->el_nexprs_max * 2 + 8;
        nbytes = el->el_nexprs_max * sizeof(expr_ty *);
        el->el_expr = mem_change_size(el->el_expr, nbytes);
    }
    el->el_expr[el->el_nexprs++] = expr_copy(e);
    trace(("}\n"));
}


/*
 * NAME
 *      expr_list_evaluate - expression list to word list
 *
 * SYNOPSIS
 *      int expr_list_evaluate(string_list_ty *wl, expr_list_ty *el);
 *
 * DESCRIPTION
 *      The expr_list_evaluate function is used to turn an expression
 *      list into a word list.
 *
 * RETURNS
 *      string_list_ty *; or the NULL pointer on error
 *
 * CAVEAT
 *      Use string_list_delete when you are done with it.
 */

string_list_ty *
expr_list_evaluate(const expr_list_ty *elp, const match_ty *mp)
{
    opcode_list_ty  *olp;
    string_list_ty  *result;

    trace(("expr_list_evaluate(elp = %p)\n{\n", elp));
    olp = opcode_list_new();
    opcode_list_append(olp, opcode_push_new());
    expr_list_code_generate(elp, olp);
    result = opcode_list_run(olp, mp);
    opcode_list_delete(olp);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      expr_list_constructor
 *
 * SYNOPSIS
 *      void expr_list_constructor(expr_list_ty *);
 *
 * DESCRIPTION
 *      The expr_list_constructor function is used to initialize an
 *      expression list before use.
 */

void
expr_list_constructor(expr_list_ty *elp)
{
    elp->el_nexprs = 0;
    elp->el_nexprs_max = 0;
    elp->el_expr = 0;
}


/*
 * NAME
 *      expr_list_new
 *
 * SYNOPSIS
 *      opcode_list_ty *expr_list_new(void);
 *
 * DESCRIPTION
 *      The expr_list_new function is used to allocate and initialize an
 *      new expression list instance in dynamic memory.
 *
 * CAVEAT
 *      Use opcode_list_delete when you are done with it.
 */

expr_list_ty *
expr_list_new(void)
{
    expr_list_ty    *elp;

    elp = mem_alloc(sizeof(expr_list_ty));
    expr_list_constructor(elp);
    return elp;
}


/*
 * NAME
 *      expr_list_code_generate
 *
 * SYNOPSIS
 *      void expr_list_code_generate(expr_list_ty *, opcode_list_ty *);
 *
 * DESCRIPTION
 *      The expr_list_code_generate function is used to generate the
 *      code stream for a list of expressions.
 */

void
expr_list_code_generate(const expr_list_ty *elp, opcode_list_ty *olp)
{
    size_t          j;

    for (j = 0; j < elp->el_nexprs; ++j)
        expr_code_generate(elp->el_expr[j], olp);
}


/*
 * NAME
 *      expr_list_position
 *
 * SYNOPSIS
 *      expr_position_ty *expr_list_position(expr_list_ty *);
 *
 * DESCRIPTION
 *      The expr_list_position function is used to find a position
 *      indicator for an expressin list.
 */

expr_position_ty *
expr_list_position(const expr_list_ty *elp)
{
    if (elp->el_nexprs > 0)
        return &elp->el_expr[0]->e_position;
    return 0;
}
