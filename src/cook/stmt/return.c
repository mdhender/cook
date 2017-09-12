/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2001, 2006-2009 Peter Miller
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
#include <cook/expr/list.h>
#include <cook/expr/position.h>
#include <cook/opcode/goto.h>
#include <cook/opcode/list.h>
#include <cook/stmt.h>
#include <cook/stmt/return.h>
#include <common/trace.h>


typedef struct stmt_return_ty stmt_return_ty;
struct stmt_return_ty
{
    stmt_ty         inherited;
    expr_list_ty    text;
    expr_position_ty pos;
};


/*
 *  NAME
 *      destructor - free a return statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a return
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
destructor(stmt_ty *sp)
{
    stmt_return_ty  *this;

    this = (stmt_return_ty *)sp;
    expr_list_destructor(&this->text);
    expr_position_destructor(&this->pos);
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
    stmt_return_ty  *this;

    /*
     * Emit the return value expression, but do not emit a ``push''
     * opcode first.  We want to put it on the returnee's list, not
     * our own.
     */
    trace(("stmt_return::code_generate(sp = %p)\n{\n", sp));
    assert(sp);
    this = (stmt_return_ty *)sp;
    expr_list_code_generate(&this->text, olp);

    /*
     * Zip on over to the return label.
     * It is an error if we are not inside a function.
     */
    if (olp->return_label)
        opcode_list_append(olp, opcode_goto_new(olp->return_label));
    else
    {
        error_with_position
        (
            &this->pos,
            0,
            i18n("'return' encountered outside a function")
        );
    }
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
    "return",
    sizeof(stmt_return_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_return_new - create a new return statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_return_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_return_new function is used to create a new instance
 *      of a return statement node.
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
stmt_return_new(expr_list_ty *arg, expr_position_ty *pp)
{
    stmt_ty         *sp;
    stmt_return_ty  *this;
    expr_position_ty *pos;

    trace(("stmt_return_new()\n{\n"));
    sp = stmt_private_new(&method);
    this = (stmt_return_ty *)sp;
    expr_list_copy_constructor(&this->text, arg);
    pos = expr_list_position(arg);
    if (!pos)
        pos = pp;
    expr_position_copy_constructor(&this->pos, pos);
    trace(("return %p;\n", sp));
    trace(("}\n"));
    return sp;
}
