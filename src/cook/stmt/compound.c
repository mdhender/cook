/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2009 Peter Miller
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

#include <cook/stmt.h>
#include <cook/stmt/compound.h>
#include <cook/stmt/list.h>
#include <common/trace.h>


typedef struct stmt_compound_ty stmt_compound_ty;
struct stmt_compound_ty
{
    stmt_ty         inherited;
    stmt_list_ty    body;
};


/*
 *  NAME
 *      destructor - free a compound statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a compound
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
destructor(stmt_ty *sp)
{
    stmt_compound_ty *this;

    trace(("destructor(sp = %p)\n{\n", sp));
    /* assert(sp); */
    /* assert(sp->method == &method); */
    this = (stmt_compound_ty *) sp;

    stmt_list_destructor(&this->body);

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
code_generate(stmt_ty *sp, struct opcode_list_ty *olp)
{
    stmt_compound_ty *this;
    stmt_result_ty  status;
    size_t          j;

    trace(("code_generate(sp = %p)\n{\n", sp));
    assert(sp);
    this = (stmt_compound_ty *)sp;
    status = STMT_OK;

    for (j = 0; j < this->body.sl_nstmts; ++j)
    {
        status = stmt_code_generate(this->body.sl_stmt[j], olp);
        if (status != STMT_OK)
            break;
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

static stmt_method_ty method =
{
    "compound",
    sizeof(stmt_compound_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_compound_new - create a new compound statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_compound_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_compound_new function is used to create a new instance
 *      of a compound statement node.
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
stmt_compound_new(stmt_list_ty *body)
{
    stmt_ty         *sp;
    stmt_compound_ty *this;

    trace(("stmt_compound_new()\n{\n"));
    sp = stmt_private_new(&method);
    this = (stmt_compound_ty *) sp;

    stmt_list_copy_constructor(&this->body, body);

    trace(("return %p;\n", sp));
    trace(("}\n"));
    return sp;
}
