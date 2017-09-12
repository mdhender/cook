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
#include <cook/opcode/label.h>
#include <cook/opcode/list.h>
#include <cook/opcode/goto.h>
#include <cook/opcode/jmpf.h>
#include <cook/opcode/push.h>
#include <cook/stmt.h>
#include <cook/stmt/if.h>
#include <common/trace.h>


typedef struct stmt_if_ty stmt_if_ty;
struct stmt_if_ty
{
    stmt_ty         inherited;
    expr_ty         *condition;
    stmt_ty         *then_clause;
    stmt_ty         *else_clause;
};


/*
 *  NAME
 *      destructor - free a if statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a if
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
destructor(stmt_ty *sp)
{
    stmt_if_ty      *this;

    trace(("stmt_if::destructor(sp = %p)\n{\n", sp));
    assert(sp);
    /* assert(sp->method == &method); */
    this = (stmt_if_ty *)sp;

    expr_delete(this->condition);
    stmt_delete(this->then_clause);
    if (this->else_clause)
        stmt_delete(this->else_clause);

    trace(("}\n"));
}


/*
 * NAME
 *      code_generate
 *
 * SYNOPSIS
 *      stmt_result_ty code_generate(stmt_ty *sp, opcode_list_ty *olp);
 *
 * DESCRIPTION
 *      The code_generate function is used to generate the opcodes for
 *      this statement node.
 *
 * RETURNS
 *      The value returned indicates why the code generation terminated.
 */

static stmt_result_ty
code_generate(stmt_ty *sp, opcode_list_ty *olp)
{
    stmt_if_ty      *this;
    stmt_result_ty  status;
    opcode_label_ty *t1;

    trace(("stmt_if::code_generate(sp = %p)\n{\n", sp));
    assert(sp);
    this = (stmt_if_ty *)sp;

    t1 = opcode_label_new();
    opcode_list_append(olp, opcode_push_new());
    expr_code_generate(this->condition, olp);
    opcode_list_append(olp, opcode_jmpf_new(t1));
    status = stmt_code_generate(this->then_clause, olp);
    if (this->else_clause)
    {
        opcode_label_ty *t2;
        stmt_result_ty  status2;

        t2 = opcode_label_new();
        opcode_list_append(olp, opcode_goto_new(t2));
        opcode_label_define(t1, olp->length);
        status2 = stmt_code_generate(this->else_clause, olp);
        if (status == STMT_OK)
            status = status2;
        opcode_label_define(t2, olp->length);
        opcode_label_delete(t2);
    }
    else
        opcode_label_define(t1, olp->length);
    opcode_label_delete(t1);
    trace(("return %d;\n", status));
    trace(("}\n"));
    return status;
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

static stmt_method_ty method =
{
    "if",
    sizeof(stmt_if_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_if_new - create a new if statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_if_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_if_new function is used to create a new instance
 *      of a if statement node.
 *
 * RETURNS
 *      stmt_ty *; pointer to polymorphic statement instance.
 *
 * CAVEAT
 *      This function allocates data in dynamic memory.  It is the
 *      caller's responsibility to free this data, using stmt_delete,
 *      when it is no longer required.
 */

stmt_ty *
stmt_if_new(expr_ty *condition, stmt_ty *then_clause, stmt_ty *else_clause)
{
    stmt_ty         *sp;
    stmt_if_ty      *this;

    trace(("stmt_if_new()\n{\n"));
    sp = stmt_private_new(&method);
    this = (stmt_if_ty *)sp;

    this->condition = expr_copy(condition);
    this->then_clause = stmt_copy(then_clause);
    this->else_clause = (else_clause ? stmt_copy(else_clause) : (stmt_ty *)0);

    trace(("return %p;\n", sp));
    trace(("}\n"));
    return sp;
}
