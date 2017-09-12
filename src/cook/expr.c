/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1997, 2006-2009 Peter Miller
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
 * This file contains the functions for manipulating expression
 * trees; building, interpreting and freeing them.
 */

#include <common/ac/stddef.h>
#include <common/ac/stdarg.h>
#include <common/ac/stdio.h>

#include <cook/builtin.h>
#include <cook/cook.h>
#include <common/error_intl.h>
#include <cook/expr.h>
#include <cook/id.h>
#include <cook/lex.h>
#include <cook/match.h>
#include <common/mem.h>
#include <cook/opcode/list.h>
#include <cook/opcode/push.h>
#include <cook/option.h>
#include <cook/stmt.h>
#include <common/trace.h>
#include <common/str_list.h>


/*
 * NAME
 *      expr_alloc - allocate a pointer structure
 *
 * SYNOPSIS
 *      expr_ty *expr_alloc(void);
 *
 * DESCRIPTION
 *      The expr_alloc function is used to allocate an expression node
 *      structure in dynamic memory.  It will initially be filled with zeros.
 *      The e_op field is defined as being non-zero to allow detection
 *      of some classes of missuse of the expression nodes.
 *
 * RETURNS
 *      A pointer to a "expr_ty" in dynamic memory.
 *
 * CAVEAT
 *      The expression node is allocated in dynamic memory,
 *      it is the callers responsibility to ensure that it is freed
 *      when it is finished with, by a call to expr_delete().
 */

expr_ty *
expr_private_new(expr_method_ty *mp)
{
    expr_ty         *ep;

    trace(("expr_new()\n{\n"));
    ep = mem_alloc(mp->size);
    ep->method = mp;
    ep->e_references = 1;
    expr_position_constructor(&ep->e_position, lex_cur_file(), lex_cur_line());
    trace(("return %p;\n", ep));
    trace(("}\n"));
    return ep;
}


/*
 * NAME
 *      expr_copy - copy and expression
 *
 * SYNOPSIS
 *      expr_ty *expr_copy(expr_ty *);
 *
 * DESCRIPTION
 *      The expr_copy function is used to make a copy of an expression tree.
 *
 * RETURNS
 *      The expr_copy function returns a pointer to the root of the copied
 *      expression tree.
 *
 * CAVEAT
 *      The result is in dynamic memory, used expr_delete to dispose of it when
 *      finished with.
 */

expr_ty *
expr_copy(expr_ty *ep)
{
    trace(("expr_copy(ep = %p)\n{\n", ep));
    ep->e_references++;
    trace(("return %p;\n", ep));
    trace(("}\n"));
    return ep;
}


/*
 * NAME
 *      expr_delete - free expression tree
 *
 * SYNOPSIS
 *      void expr_delete(expr_ty *ep);
 *
 * DESCRIPTION
 *      The expr_delete function is used to free expression trees.
 *
 * CAVEAT
 *      It is assumed that the expression trees are all
 *      dynamically allocated.  Use expr_alloc() to allocate them.
 */

void
expr_delete(expr_ty *ep)
{
    trace(("expr_delete(ep = %p)\n{\n", ep));
    assert(ep);
    ep->e_references--;
    if (ep->e_references <= 0)
    {
        if (ep->method->destructor)
            ep->method->destructor(ep);
        expr_position_destructor(&ep->e_position);
        mem_free(ep);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      expr_evaluate - evaluate an expression
 *
 * SYNOPSIS
 *      string_list_ty *expr_evaluate(expr_ty *ep);
 *
 * DESCRIPTION
 *      The expr_evaluate function is used to evaluate an expression.
 *
 * RETURNS
 *      string_list_ty *; or the NULL pointer on error
 *
 * CAVEAT
 *      The result returned from this function are allocated in dynamic memory.
 *      It is the responsibility of the caller to ensure that they are freed
 *      when they are finished with, using string_list_destructor().
 */

string_list_ty *
expr_evaluate(const expr_ty *ep, const match_ty *mp)
{
    opcode_list_ty  *olp;
    string_list_ty  *result;

    trace(("expr_evaluate(ep = %p)\n{\n", ep));
    assert(ep);
    olp = opcode_list_new();
    opcode_list_append(olp, opcode_push_new());
    expr_code_generate(ep, olp);
    result = opcode_list_run(olp, mp);
    opcode_list_delete(olp);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      expr_eval_condition - evaluate condition
 *
 * SYNOPSIS
 *      int expr_eval_condition(expr_ty *);
 *
 * DESCRIPTION
 *      The expr_eval_condition function is used to evaluate an expression to
 *      yeild a true/false result.  The expression is evaluated into a word
 *      list.  A false result is if all of the resulting strings are empty or
 *      0, true otherwise.
 *
 * RETURNS
 *      The expr_eval_condition function returns 0 if the condition is false,
 *      and nonzero if it is true.  The value -1 is returned on error.
 *
 * CAVEAT
 *      The str_bool function is used to test the booean value of a string;
 *      changeing the behaviour of that function will change the behaviour of
 *      this one.
 */

int
expr_eval_condition(const expr_ty *ep, const match_ty *mp)
{
    opcode_list_ty  *olp;
    int             result;

    trace(("expr_eval_condition(ep = %p)\n{\n", ep));
    assert(ep);
    olp = opcode_list_new();
    opcode_list_append(olp, opcode_push_new());
    expr_code_generate(ep, olp);
    result = opcode_list_run_bool(olp, mp);
    opcode_list_delete(olp);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      expr_equal
 *
 * SYNOPSIS
 *      int expr_equal(expr_ty *, expr_ty *);
 *
 * DESCRIPTION
 *      The expr_equal function is used to test whether two expression
 *      trees are identical.
 */

int
expr_equal(const expr_ty *e1, const expr_ty *e2)
{
    assert(e1);
    assert(e1->method);
    assert(e2);
    assert(e2->method);
    assert(e1->method->equal);
    assert(e2->method->equal);
    return (e1->method == e1->method && e1->method->equal(e1, e2));
}


/*
 * NAME
 *      expr_code_generate
 *
 * SYNOPSIS
 *      void expr_code_generate(expr_ty *, opcode_list_ty *);
 *
 * DESCRIPTION
 *      The expr_code_generate function is used to generate the opcode
 *      stream for the given expression node.
 */

void
expr_code_generate(const expr_ty *ep, struct opcode_list_ty *olp)
{
    assert(ep);
    assert(ep->method);
    assert(ep->method->code_generate);
    ep->method->code_generate(ep, olp);
}
