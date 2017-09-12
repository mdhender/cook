/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006, 2007 Peter Miller;
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

#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/opcode/goto.h>
#include <cook/opcode/label.h>
#include <cook/opcode/list.h>
#include <cook/stmt.h>
#include <cook/stmt/loop.h>
#include <common/trace.h>


typedef struct stmt_loop_ty stmt_loop_ty;
struct stmt_loop_ty
{
    stmt_ty         inherited;
    stmt_ty         *body;
};


/*
 *  NAME
 *      destructor - free a loop statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a loop
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
destructor(stmt_ty *sp)
{
    stmt_loop_ty    *this;

    trace(("stmt_loop::destructor(sp = %08X)\n{\n", sp));
    assert(sp);
    /* assert(sp->method == &method); */
    this = (stmt_loop_ty *) sp;
    assert(this->body);
    stmt_delete(this->body);
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
    stmt_loop_ty    *this;
    stmt_result_ty  status;
    opcode_label_ty *continue_hold;
    opcode_label_ty *break_hold;

    trace(("stmt_loop::code_generate(sp = %08X)\n{\n", sp));
    assert(sp);
    this = (stmt_loop_ty *)sp;
    assert(this->body);

    continue_hold = olp->continue_label;
    break_hold = olp->break_label;

    olp->continue_label = opcode_label_new();
    olp->break_label = opcode_label_new();

    opcode_label_define(olp->continue_label, olp->length);
    status = stmt_code_generate(this->body, olp);
    opcode_list_append(olp, opcode_goto_new(olp->continue_label));
    opcode_label_define(olp->break_label, olp->length);
    opcode_label_delete(olp->continue_label);
    opcode_label_delete(olp->break_label);
    olp->continue_label = continue_hold;
    olp->break_label = break_hold;

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
    "loop",
    sizeof(stmt_loop_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_loop_new - create a new loop statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_loop_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_loop_new function is used to create a new instance
 *      of a loop statement node.
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
stmt_loop_new(stmt_ty *body)
{
    stmt_ty         *sp;
    stmt_loop_ty    *this;

    trace(("stmt_loop_new(body = %08lX)\n{\n", (long)body));
    sp = stmt_private_new(&method);
    this = (stmt_loop_ty *)sp;
    this->body = stmt_copy(body);
    assert(this->body);
    trace(("return %8.8lX;\n", (long)sp));
    trace(("}\n"));
    return sp;
}


typedef struct stmt_loopstop_ty stmt_loopstop_ty;
struct stmt_loopstop_ty
{
    stmt_ty         inherited;
    expr_position_ty pos;
};


/*
 *  NAME
 *      destructor - free a loopstop statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a loopstop
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
loopstop_destructor(stmt_ty *sp)
{
    stmt_loopstop_ty *this;

    trace(("stmt_loopstop::destructor(sp = %08X)\n{\n", sp));
    assert(sp);
    /* assert(sp->method == &loopstop_method); */
    this = (stmt_loopstop_ty *)sp;
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
loopstop_code_generate(stmt_ty *sp, opcode_list_ty *olp)
{
    stmt_loopstop_ty *this;
    stmt_result_ty  status;

    trace(("stmt_loopstop::code_generate(sp = %08X)\n{\n", sp));
    assert(sp);
    this = (stmt_loopstop_ty *)sp;
    status = STMT_OK;
    if (olp->break_label)
        opcode_list_append(olp, opcode_goto_new(olp->break_label));
    else
    {
        error_with_position
        (
            &this->pos,
            0,
            i18n("'loopstop' encountered outside a loop")
        );
        status = STMT_ERROR;
    }
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

static stmt_method_ty loopstop_method =
{
    "loopstop",
    sizeof(stmt_loopstop_ty),
    loopstop_destructor,
    loopstop_code_generate,
};


/*
 * NAME
 *      stmt_loopstop_new - create a new loopstop statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_loopstop_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_loopstop_new function is used to create a new instance
 *      of a loopstop statement node.
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
stmt_loopstop_new(expr_position_ty *pp)
{
    stmt_ty         *sp;
    stmt_loopstop_ty *this;

    trace(("stmt_loopstop_new()\n{\n"));
    sp = stmt_private_new(&loopstop_method);
    this = (stmt_loopstop_ty *)sp;
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %8.8lX;\n", (long)sp));
    trace(("}\n"));
    return sp;
}
