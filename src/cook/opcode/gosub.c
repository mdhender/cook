/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006-2009 Peter Miller
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
#include <cook/opcode.h>
#include <cook/opcode/context.h>
#include <cook/opcode/gosub.h>
#include <cook/opcode/private.h>
#include <common/str_list.h>
#include <common/trace.h>


typedef struct opcode_gosub_ty opcode_gosub_ty;
struct opcode_gosub_ty
{
    opcode_ty       inherited;
    expr_position_ty pos;
};


static void
destructor(opcode_ty *op)
{
    opcode_gosub_ty *this;

    this = (opcode_gosub_ty *)op;
    expr_position_destructor(&this->pos);
}


/*
 * NAME
 *      execute
 *
 * SYNOPSIS
 *      opcode_status_ty execute(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *      The execute function is used to execute the given opcode within
 *      the given interpretation context.
 *
 * RETURNS
 *      opcode_status_ty to indicate the result of the execution
 */

static opcode_status_ty
execute(const opcode_ty *op, opcode_context_ty *icp)
{
    const opcode_gosub_ty *this;
    string_list_ty  *value;
    int             flag;
    opcode_status_ty result;

    trace(("opcode_gosub::execute()\n{\n"));
    this = (const opcode_gosub_ty *)op;
    result = opcode_status_success;

    value = opcode_context_string_list_pop(icp);
    flag = string_list_bool(value);
    string_list_delete(value);

    if (flag)
    {
        error_with_position
        (
            &this->pos,
            0,
            i18n("function call returned non-zero exit status")
        );
        result = opcode_status_error;
    }

    trace(("return %s;\n", opcode_status_name(result)));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      method
 *
 * DESCRIPTION
 *      The method variable describes this class.
 *
 * CAVEAT
 *      This symbol is not exported from this file.
 */

static opcode_method_ty method =
{
    "gosub",
    sizeof(opcode_gosub_ty),
    destructor,
    execute,
    execute,                    /* script */
    0,                          /* disassemble */
};


/*
 * NAME
 *      opcode_gosub_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_gosub_new(void);
 *
 * DESCRIPTION
 *      The opcode_gosub_new function is used to allocate a new instance
 *      of a gosub opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_gosub_new(expr_position_ty *pp)
{
    opcode_ty       *op;
    opcode_gosub_ty *this;

    trace(("opcode_gosub_new()\n{\n"));
    op = opcode_new(&method);
    this = (opcode_gosub_ty *)op;
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %p;\n", op));
    trace(("}\n"));
    return op;
}
