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

#include <common/mem.h>
#include <cook/opcode/private.h>
#include <common/trace.h>


/*
 * NAME
 *      opcode_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_new(void);
 *
 * DESCRIPTION
 *      The opcode_new function is used to allocate a new instance of an
 *      opcode in dyna,mic memory, and perform base class
 *      initializations (the vptr, mostly).
 *
 * RETURNS
 *      opcode_ty *; generic pointer to opcode
 *
 * CAVEAT
 *      Must be used only internally to creating instances of derived
 *      opcode classes.
 */

opcode_ty *
opcode_new(opcode_method_ty *mp)
{
    opcode_ty       *op;

    trace(("opcode_new()\n{\n"));
    trace(("is a \"%s\" %d\n", mp->name, mp->size));
    op = mem_alloc(mp->size);
    op->method = mp;
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
