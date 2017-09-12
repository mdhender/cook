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

#include <cook/id.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <cook/opcode/postlude.h>
#include <common/trace.h>


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
    /*
     * delete the variables defined by the prelude opcode
     */
    trace(("opcode_postlude::execute()\n{\n"));
    (void)op;
    opcode_context_match_pop(icp);
    trace(("return success;\n"));
    trace(("}\n"));
    return opcode_status_success;
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
    "postlude",
    sizeof(opcode_ty),
    0,                          /* destructor */
    execute,
    execute,                    /* script */
    0,                          /* disassemble */
};


/*
 * NAME
 *      opcode_postlude_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_postlude_new(void);
 *
 * DESCRIPTION
 *      The opcode_postlude_new function is used to allocate a new instance
 *      of a postlude opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_postlude_new(void)
{
    opcode_ty       *op;

    trace(("opcode_postlude_new()\n{\n"));
    op = opcode_new(&method);
    trace(("return %p;\n", op));
    trace(("}\n"));
    return op;
}
