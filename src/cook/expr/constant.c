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

#include <cook/expr.h>
#include <cook/expr/constant.h>
#include <cook/expr/position.h>
#include <cook/match.h>
#include <cook/opcode/list.h>
#include <cook/opcode/string.h>
#include <common/str_list.h>


typedef struct expr_constant_ty expr_constant_ty;
struct expr_constant_ty
{
    expr_ty         inherited;
    string_ty       *value;
    expr_position_ty pos;
};


/*
 * NAME
 *      destructor - release expression node resources
 *
 * SYNOPSIS
 *      void destructor(expr_ty *);
 *
 * DESCRIPTION
 *      The destructor function is used to release any resources
 *      (dynamic memory, file descriptors, etc) which may be used the
 *      this expression node.
 *
 * CAVEAT
 *      It does NOT free the expression_node itself (this is the
 *      destructor, not delete).
 */

static void
destructor(expr_ty *ep)
{
    expr_constant_ty *this;

    /* assert(ep->method == &method); */
    this = (expr_constant_ty *)ep;
    str_free(this->value);
    this->value = 0;
    expr_position_destructor(&this->pos);
}


/*
 * NAME
 *      equal - test expression node equality
 *
 * SYNOPSIS
 *      int equal(expr_ty *e1, expr_ty *e2);
 *
 * DESCRIPTION
 *      The equal function is called to determine if two expression
 *      nodes are the same.
 *
 * RETURNS
 *      int; 1 if equal, 00 if not
 *
 * CAVEAT
 *      The expression nodes are already known to the the same class
 *      before this method is invoked.
 */

static int
equal(const expr_ty *a1, const expr_ty *a2)
{
    const expr_constant_ty *e1;
    const expr_constant_ty *e2;

    /* assert(e1->method == &method); */
    /* assert(e2->method == &method); */
    e1 = (const expr_constant_ty *)a1;
    e2 = (const expr_constant_ty *)a2;
    return str_equal(e1->value, e2->value);
}


/*
 * NAME
 *      code_generate
 *
 * SYNOPSIS
 *      void code_generate(const expr_ty *, opcode_list_ty *);
 *
 * DESCRIPTION
 *      The code_generate function is used to generate code for the
 *      expression tree represented by this node.
 */

static void
code_generate(const expr_ty *ep, opcode_list_ty *olp)
{
    const expr_constant_ty *this;

    /* assert(ep->method == &method); */
    this = (const expr_constant_ty *)ep;

    opcode_list_append(olp, opcode_string_new(this->value, &this->pos));
}


/*
 * NAME
 *      method - class method table
 *
 * DESCRIPTION
 *      This is the class method table.  It contains a description of
 *      the class, its name, size and pointers to its virtual methods.
 *
 * CAVEAT
 *      This symbol is NOT to be exported from this file scope.
 */

static expr_method_ty method =
{
    "constant",
    sizeof(expr_constant_ty),
    destructor,
    equal,
    code_generate,
};


/*
 * NAME
 *      expr_constant_new - create a new constant expression node
 *
 * SYNOPSIS
 *      expr_ty *expr_constant_new(string_ty *);
 *
 * DESCRIPTION
 *      The expr_constant_new function is used to create a new instance
 *      of a constant expression node.
 *
 * RETURNS
 *      expr_ty *; pointer to polymorphic expression instance.
 *
 * CAVEAT
 *      This function allocates data in dynamic memory.  It is the
 *      caller's responsibility to free this data, using expr_delete,
 *      when it is no longer required.
 */

expr_ty *
expr_constant_new(string_ty *s, const expr_position_ty *pp)
{
    expr_ty         *ep;
    expr_constant_ty *this;

    ep = expr_private_new(&method);
    this = (expr_constant_ty *)ep;
    this->value = str_copy(s);
    expr_position_copy_constructor(&this->pos, pp);
    return ep;
}
