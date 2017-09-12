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

#include <cook/expr.h>
#include <cook/expr/list.h>
#include <cook/expr/position.h>
#include <cook/opcode/command.h>
#include <cook/opcode/list.h>
#include <cook/opcode/push.h>
#include <cook/stmt.h>
#include <cook/stmt/command.h>
#include <common/trace.h>


typedef struct stmt_command_ty stmt_command_ty;
struct stmt_command_ty
{
    stmt_ty         inherited;
    expr_list_ty    args;
    expr_list_ty    flags;
    expr_ty         *input;
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
    stmt_command_ty *this;

    trace(("destructor(sp = %08X)\n{\n", sp));
    /* assert(sp); */
    /* assert(sp->method == &method); */
    this = (stmt_command_ty *)sp;

    expr_list_destructor(&this->args);
    expr_list_destructor(&this->flags);
    if (this->input)
        expr_delete(this->input);
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
    stmt_command_ty *this;

    trace(("code_generate(sp = %08X)\n{\n", sp));
    assert(sp);
    this = (stmt_command_ty *)sp;

    /*
     * generate the command strings
     */
    opcode_list_append(olp, opcode_push_new());
    expr_list_code_generate(&this->args, olp);

    /*
     * generate the flag strings
     */
    opcode_list_append(olp, opcode_push_new());
    expr_list_code_generate(&this->flags, olp);

    /*
     * generate the command input
     */
    if (this->input)
    {
        opcode_list_append(olp, opcode_push_new());
        expr_code_generate(this->input, olp);
    }

    /*
     * generate the command itself
     */
    opcode_list_append(olp, opcode_command_new((this->input != 0), &this->pos));

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
    "command",
    sizeof(stmt_command_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_command_new - create a new command statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_command_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_command_new function is used to create a new instance
 *      of a command statement node.
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
stmt_command_new(expr_list_ty *args, expr_list_ty *flags, expr_ty *input,
    expr_position_ty *pp)
{
    stmt_ty         *sp;
    stmt_command_ty *this;

    trace(("stmt_command_new()\n{\n"));
    sp = stmt_private_new(&method);
    this = (stmt_command_ty *)sp;

    expr_list_copy_constructor(&this->args, args);
    expr_list_copy_constructor(&this->flags, flags);
    this->input = (input ? expr_copy(input) : (expr_ty *)0);
    expr_position_copy_constructor(&this->pos, pp);

    trace(("return %8.8lX;\n", (long)sp));
    trace(("}\n"));
    return sp;
}
