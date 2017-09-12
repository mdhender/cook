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

#include <common/ac/stdio.h>

#include <cook/expr/position.h>
#include <cook/flag.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <cook/opcode/set.h>
#include <cook/option.h>
#include <common/trace.h>


typedef struct opcode_set_ty opcode_set_ty;
struct opcode_set_ty
{
    opcode_ty       inherited;
    expr_position_ty pos;
};


static void
destructor(opcode_ty *op)
{
    opcode_set_ty   *this;

    this = (opcode_set_ty *)op;
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
    const opcode_set_ty *this;
    opcode_status_ty status;
    string_list_ty  *flags_words;
    flag_ty         *flags;

    trace(("opcode_set::execute()\n{\n"));
    this = (const opcode_set_ty *)op;
    status = opcode_status_success;
    flags_words = opcode_context_string_list_pop(icp);
    flags = flag_recognize(flags_words, &this->pos);
    string_list_delete(flags_words);
    if (!flags)
        status = opcode_status_error;
    else
    {
        flag_set_options(flags, OPTION_LEVEL_COOKBOOK);
        flag_delete(flags);
    }
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


static void
disassemble(const opcode_ty *op)
{
    opcode_set_ty   *this;

    this = (opcode_set_ty *)op;
    printf("# %s:%d", this->pos.pos_name->str_text, this->pos.pos_line);
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
    "set",
    sizeof(opcode_set_ty),
    destructor,
    execute,
    execute,                    /* script */
    disassemble,
};


/*
 * NAME
 *      opcode_set_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_set_new(void);
 *
 * DESCRIPTION
 *      The opcode_set_new function is used to allocate a new instance
 *      of a set opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_set_new(const expr_position_ty *pp)
{
    opcode_ty       *op;
    opcode_set_ty   *this;

    trace(("opcode_set_new()\n{\n"));
    op = opcode_new(&method);
    this = (opcode_set_ty *)op;
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %p;\n", op));
    trace(("}\n"));
    return op;
}
