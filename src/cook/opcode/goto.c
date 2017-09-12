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

#include <common/ac/stdio.h>

#include <cook/opcode/context.h>
#include <cook/opcode/goto.h>
#include <cook/opcode/label.h>
#include <cook/opcode/private.h>
#include <common/trace.h>


typedef struct opcode_goto_ty opcode_goto_ty;
struct opcode_goto_ty
{
    opcode_ty       inherited;
    size_t          destination;
};


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
    const opcode_goto_ty *this;

    trace(("opcode_goto::execute()\n{\n"));
    this = (const opcode_goto_ty *)op;
    opcode_context_goto(icp, this->destination);
    trace(("return success;\n"));
    trace(("}\n"));
    return opcode_status_success;
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
    const opcode_goto_ty *this;

    trace(("opcode_goto::disassemle()\n{\n"));
    this = (const opcode_goto_ty *)op;
    printf("%ld", (long)this->destination);
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
    "goto",
    sizeof(opcode_goto_ty),
    0,                          /* destructor */
    execute,
    execute,                    /* script */
    disassemble,
};


/*
 * NAME
 *      opcode_goto_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_goto_new(void);
 *
 * DESCRIPTION
 *      The opcode_goto_new function is used to allocate a new instance
 *      of a goto opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_goto_new(opcode_label_ty *lp)
{
    opcode_ty       *op;
    opcode_goto_ty  *this;

    trace(("opcode_goto_new()\n{\n"));
    op = opcode_new(&method);
    this = (opcode_goto_ty *) op;
    opcode_label_refer(lp, &this->destination);
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
