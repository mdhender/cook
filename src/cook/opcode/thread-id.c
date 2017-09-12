/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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
#include <cook/opcode/thread-id.h>
#include <common/trace.h>


static size_t   stack_top;
static size_t   stack_max;
static long     *stack;
static long     next;


long
opcode_thread_id_borrow(void)
{
    long            thid;

    trace(("opcode_thread_id_borrow()\n{\n"));
    if (stack_top > 0)
        thid = stack[--stack_top];
    else
        thid = ++next;
    trace(("return %ld;\n", thid));
    trace(("}\n"));
    return thid;
}


void
opcode_thread_id_return(long thid)
{
    trace(("opcode_thread_id_return(thid = %ld)\n{\n", thid));
    assert(thid > 0);
    assert(thid <= next);
    if (stack_top >= stack_max)
    {
        size_t          nbytes;

        stack_max = stack_max * 2 + 4;
        nbytes = stack_max * sizeof(stack[0]);
        stack = mem_change_size(stack, nbytes);
    }
    stack[stack_top++] = thid;
    trace(("}\n"));
}
