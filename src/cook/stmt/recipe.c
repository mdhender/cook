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

#include <common/trace.h>
#include <cook/expr.h>
#include <cook/expr/list.h>
#include <cook/opcode/list.h>
#include <cook/opcode/push.h>
#include <cook/opcode/recipe.h>
#include <cook/recipe.h>
#include <cook/stmt.h>
#include <cook/stmt/compound.h>
#include <cook/stmt/list.h>
#include <cook/stmt/recipe.h>


typedef struct stmt_recipe_ty stmt_recipe_ty;
struct stmt_recipe_ty
{
    stmt_ty         inherited;
    expr_list_ty    target;
    expr_list_ty    need;
    expr_list_ty    need2;
    expr_list_ty    flags;
    int             multiple;
    expr_ty         *precondition;
    expr_list_ty    single_thread;
    expr_list_ty    host_binding;
    stmt_ty         *action;
    stmt_ty         *use_action;
    expr_position_ty position;
};


/*
 *  NAME
 *      destructor - free a recipe statement node
 *
 *  SYNOPSIS
 *      void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *      The destructor function releases the resources held by a recipe
 *      statement instance after it is finished with.
 *
 *  CAVEAT
 *      Do not free the node itself, this the the destructor, not
 *      delete.
 */

static void
destructor(stmt_ty *sp)
{
    stmt_recipe_ty  *this;

    trace(("destructor(sp = %p)\n{\n", sp));
    /* assert(sp); */
    /* assert(sp->method == &method); */
    this = (stmt_recipe_ty *)sp;

    expr_list_destructor(&this->target);
    expr_list_destructor(&this->need);
    expr_list_destructor(&this->need2);
    expr_list_destructor(&this->flags);
    if (this->precondition)
        expr_delete(this->precondition);
    expr_list_destructor(&this->single_thread);
    expr_list_destructor(&this->host_binding);
    if (this->action)
        stmt_delete(this->action);
    if (this->use_action)
        stmt_delete(this->use_action);
    expr_position_destructor(&this->position);

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
    stmt_recipe_ty  *this;
    stmt_result_ty  status;
    opcode_list_ty  *need1;
    opcode_list_ty  *need2;
    opcode_list_ty  *precondition;
    opcode_list_ty  *single_thread;
    opcode_list_ty  *host_binding;
    opcode_list_ty  *out_of_date;
    opcode_list_ty  *up_to_date;

    trace(("code_generate(sp = %p)\n{\n", sp));
    assert(sp);
    this = (stmt_recipe_ty *)sp;
    status = STMT_OK;

    if (this->need.el_nexprs > 0)
    {
        need1 = opcode_list_new();
        opcode_list_append(need1, opcode_push_new());
        expr_list_code_generate(&this->need, need1);
    }
    else
        need1 = 0;

    if (this->need2.el_nexprs)
    {
        need2 = opcode_list_new();
        opcode_list_append(need2, opcode_push_new());
        expr_list_code_generate(&this->need2, need2);
    }
    else
        need2 = 0;

    if (this->precondition)
    {
        precondition = opcode_list_new();
        opcode_list_append(precondition, opcode_push_new());
        expr_code_generate(this->precondition, precondition);
    }
    else
        precondition = 0;

    if (this->single_thread.el_nexprs)
    {
        single_thread = opcode_list_new();
        opcode_list_append(single_thread, opcode_push_new());
        expr_list_code_generate(&this->single_thread, single_thread);
    }
    else
        single_thread = 0;

    if (this->host_binding.el_nexprs)
    {
        host_binding = opcode_list_new();
        opcode_list_append(host_binding, opcode_push_new());
        expr_list_code_generate(&this->host_binding, host_binding);
    }
    else
        host_binding = 0;

    /*
     * compile the actions
     */
    out_of_date = 0;
    up_to_date = 0;
    if (this->action)
    {
        if (this->use_action)
        {
            stmt_ty         *sp2;
            stmt_list_ty    sl2;

            /*
             * make a compound statement for code generation
             */
            stmt_list_constructor(&sl2);
            stmt_list_append(&sl2, this->action);
            stmt_list_append(&sl2, this->use_action);
            sp2 = stmt_compound_new(&sl2);
            stmt_list_destructor(&sl2);

            /*
             * compile the out-of-date action
             */
            out_of_date = stmt_compile(sp2);
            if (!out_of_date)
                status = STMT_ERROR;
            stmt_delete(sp2);

            /*
             * compile the up-to-date action
             */
            up_to_date = stmt_compile(this->use_action);
            if (!up_to_date)
                status = STMT_ERROR;
        }
        else
        {
            /*
             * compile the out-of-date action
             */
            out_of_date = stmt_compile(this->action);
            if (!out_of_date)
                status = STMT_ERROR;
        }
    }

    /*
     * push targets onto the stack
     */
    opcode_list_append(olp, opcode_push_new());
    expr_list_code_generate(&this->target, olp);

    /*
     * push flags onto the stack
     */
    opcode_list_append(olp, opcode_push_new());
    expr_list_code_generate(&this->flags, olp);

    /*
     * and the recipe opcode
     */
    opcode_list_append
    (
        olp,
        opcode_recipe_new
        (
            need1,
            need2,
            precondition,
            this->multiple,
            single_thread,
            host_binding,
            out_of_date,
            up_to_date,
            &this->position
        )
    );

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
    "recipe",
    sizeof(stmt_recipe_ty),
    destructor,
    code_generate,
};


/*
 * NAME
 *      stmt_recipe_new - create a new recipe statement node
 *
 * SYNOPSIS
 *      stmt_ty *stmt_recipe_new(string_ty *);
 *
 * DESCRIPTION
 *      The stmt_recipe_new function is used to create a new instance
 *      of a recipe statement node.
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
stmt_recipe_new(expr_list_ty *target, expr_list_ty *need, expr_list_ty *need2,
    expr_list_ty *flags, int multiple, expr_ty *precondition,
    expr_list_ty *single_thread, expr_list_ty *host_binding, stmt_ty *action,
    stmt_ty *use_action, expr_position_ty *pp)
{
    stmt_ty         *sp;
    stmt_recipe_ty  *this;

    trace(("stmt_recipe_new()\n{\n"));
    sp = stmt_private_new(&method);
    this = (stmt_recipe_ty *)sp;

    expr_list_copy_constructor(&this->target, target);
    expr_list_copy_constructor(&this->need, need);
    if (need2)
        expr_list_copy_constructor(&this->need2, need2);
    else
        expr_list_constructor(&this->need2);
    if (flags)
        expr_list_copy_constructor(&this->flags, flags);
    else
        expr_list_constructor(&this->flags);
    this->multiple = multiple;
    this->precondition =
        (precondition ? expr_copy(precondition) : (expr_ty *)0);
    if (single_thread)
        expr_list_copy_constructor(&this->single_thread, single_thread);
    else
        expr_list_constructor(&this->single_thread);
    if (host_binding)
        expr_list_copy_constructor(&this->host_binding, host_binding);
    else
        expr_list_constructor(&this->host_binding);
    this->action = (action ? stmt_copy(action) : (stmt_ty *)0);
    this->use_action = (use_action ? stmt_copy(use_action) : (stmt_ty *)0);
    expr_position_copy_constructor(&this->position, pp);

    trace(("return %p;\n", sp));
    trace(("}\n"));
    return sp;
}
