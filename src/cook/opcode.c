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

#include <cook/opcode/private.h>
#include <common/mem.h>
#include <common/trace.h>


/*
 * NAME
 *      opcode_delete
 *
 * SYNOPSIS
 *      void opcode_delete(opcode_ty *);
 *
 * DESCRIPTION
 *      The opcode_delete function is used to release the resources held
 *      by an opcode.
 */

void
opcode_delete(opcode_ty *op)
{
    trace(("opcode_delete(op = %p)\n{\n", op));
    assert(op);
    assert(op->method);
    if (op->method->destructor)
        op->method->destructor(op);
    op->method = 0; /* paranoia */
    mem_free(op);
    trace(("}\n"));
}


/*
 * NAME
 *      opcode_execute
 *
 * SYNOPSIS
 *      opcode_status_ty opcode_execute(opcode_ty *);
 *
 * DESCRIPTION
 *      The opcode_execute function is used to execute an opcode within
 *      the given execution context.
 *
 * RETURNS
 *      opcode_status_ty - indicating succes or failure
 */

opcode_status_ty
opcode_execute(const opcode_ty *op, struct opcode_context_ty *icp)
{
    opcode_status_ty status;

    trace(("opcode_execute(op = %p)\n{\n", op));
    assert(op);
    assert(op->method);
    assert(op->method->execute);
    status = op->method->execute(op, icp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      opcode_script
 *
 * SYNOPSIS
 *      opcode_status_ty opcode_script(opcode_ty *);
 *
 * DESCRIPTION
 *      The opcode_script function is used to script an opcode within
 *      the given execution context.
 *
 * RETURNS
 *      opcode_status_ty - indicating succes or failure
 */

opcode_status_ty
opcode_script(const opcode_ty *op, struct opcode_context_ty *icp)
{
    opcode_status_ty status;

    trace(("opcode_script(op = %p)\n{\n", op));
    assert(op);
    assert(op->method);
    assert(op->method->script);
    status = op->method->script(op, icp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      opcode_disassemble
 *
 * SYNOPSIS
 *      void opcode_disassemble(opcode_ty *);
 *
 * DESCRIPTION
 *      The opcode_disassemble function is used to print a textual
 *      representation of the opcode on the standard output.
 *
 * CAVEAT
 *      This is for debugging only, the command line option is not
 *      documented.
 */

void
opcode_disassemble(const opcode_ty *op)
{
    trace(("opcode_disassemble(op = %p)\n{\n", op));
    assert(op);
    assert(op->method);
    printf("%s", op->method->name);
    if (op->method->disassemble)
    {
        printf(" ");
        op->method->disassemble(op);
    }
    trace(("}\n"));
}
