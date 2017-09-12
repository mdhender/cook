/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006, 2007 Peter Miller;
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

#include <cook/expr/list.h>
#include <cook/expr/position.h>
#include <cook/opcode/list.h>
#include <cook/opcode/push.h>
#include <cook/opcode/set.h>
#include <cook/stmt.h>
#include <cook/stmt/set.h>
#include <common/trace.h>


typedef struct stmt_set_ty stmt_set_ty;
struct stmt_set_ty
{
    stmt_ty         inherited;
    expr_list_ty    args;
    expr_position_ty pos;
};


/*
 *  NAME
 *      destructor - free a command statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a command
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
destructor(stmt_ty *sp)
{
    stmt_set_ty     *this;

    trace(("destructor(sp = %08X)\n{\n", sp));
    assert(sp);
    /* assert(sp->method == &method); */
    this = (stmt_set_ty *)sp;
    expr_list_destructor(&this->args);
    expr_position_destructor(&this->pos);
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
    stmt_set_ty     *this;
    expr_position_ty *pp;

    trace(("code_generate(sp = %08X)\n{\n", sp));
    assert(sp);
    this = (stmt_set_ty *)sp;
    opcode_list_append(olp, opcode_push_new());
    expr_list_code_generate(&this->args, olp);
    pp = expr_list_position(&this->args);
    if (!pp)
        pp = &this->pos;
    opcode_list_append(olp, opcode_set_new(pp));
    trace(("}\n"));
    return STMT_OK;
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
    "set",
    sizeof(stmt_set_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_set_new - create a new set statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_set_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_set_new function is used to create a new instance
 *      of a set statement node.
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
stmt_set_new(expr_list_ty *elp, expr_position_ty *pp)
{
    stmt_ty         *sp;
    stmt_set_ty     *this;

    trace(("stmt_set_new()\n{\n"));
    sp = stmt_private_new(&method);
    this = (stmt_set_ty *)sp;
    expr_list_copy_constructor(&this->args, elp);
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %8.8lX;\n", (long)sp));
    trace(("}\n"));
    return sp;
}
