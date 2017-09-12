/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2002, 2006, 2007 Peter Miller;
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

#include <common/ac/stdio.h>

#include <cook/cascade.h>
#include <cook/cook.h>
#include <cook/expr/position.h>
#include <cook/opcode/cascade.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <cook/option.h>
#include <common/str_list.h>
#include <common/sub.h>
#include <common/trace.h>


typedef struct opcode_cascade_ty opcode_cascade_ty;
struct opcode_cascade_ty
{
    opcode_ty       inherited;
    expr_position_ty pos;
};


/*
 * NAME
 *      destructor
 *
 * SYNOPSIS
 *      void destructor(opcode_ty *);
 *
 * DESCRIPTION
 *      The destructor function is used to release resources held by
 *      this opcode.  Do not free the opcode itself, this is done by the
 *      base class.
 */

static void
destructor(opcode_ty *op)
{
    opcode_cascade_ty *this;

    trace(("opcode_cascade::destructor()\n{\n"));
    this = (opcode_cascade_ty *)op;
    expr_position_destructor(&this->pos);
    trace(("}\n"));
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
    const opcode_cascade_ty *this;
    opcode_status_ty status;
    string_list_ty  *target;
    string_list_ty  *need;

    trace(("opcode_cascade::execute()\n{\n"));
    this = (const opcode_cascade_ty *)op;
    status = opcode_status_success;
    target = opcode_context_string_list_pop(icp);
    need = opcode_context_string_list_pop(icp);

    if (target->nstrings == 0)
    {
        error_with_position
        (
            &this->pos,
            0,
            i18n("attempt to instantiate recipe with no targets")
        );
        status = opcode_status_error;
        goto done;
    }

    /*
     * set ingredients cascades
     */
    cascade_recipe(target, need, &this->pos);

    /*
     * emit trace information, if enabled
     */
    if (option_test(OPTION_REASON))
    {
        error_with_position
        (
            &this->pos,
            0,
            i18n("cascade instantiated (reason)")
        );
    }

  done:
    string_list_delete(target);
    string_list_delete(need);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      disassemble
 *
 * SYNOPSIS
 *      void disassemble(opcode_ty *);
 *
 * DESCRIPTION
 *      The disassemble function is used to disassemble the copdode and
 *      its arguments onto the standard output.  Don't worry about the
 *      location or a trailing newline.
 */

static void
disassemble(const opcode_ty *op)
{
    const opcode_cascade_ty *this;

    trace(("opcode_cascade::disassemle()\n{\n"));
    this = (const opcode_cascade_ty *)op;
    if (this->pos.pos_name && this->pos.pos_name->str_length)
    {
        printf(" # %s:%d", this->pos.pos_name->str_text, this->pos.pos_line);
    }
    trace(("}\n"));
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
    "cascade",
    sizeof(opcode_cascade_ty),
    destructor,
    execute,
    execute,                    /* script */
    disassemble,
};


/*
 * NAME
 *      opcode_cascade_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_cascade_new(void);
 *
 * DESCRIPTION
 *      The opcode_cascade_new function is used to allocate a new instance
 *      of a cascade opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_cascade_new(const expr_position_ty *pp)
{
    opcode_ty       *op;
    opcode_cascade_ty *this;

    trace(("opcode_cascade_new()\n{\n"));
    op = opcode_new(&method);
    this = (opcode_cascade_ty *)op;
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
