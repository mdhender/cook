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

#include <cook/opcode/catenate.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <common/str_list.h>
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
    string_list_ty  *left;
    string_list_ty  *right;
    size_t          j;
    string_ty       *s;

    trace(("opcode_catenate::execute()\n{\n"));
    (void)op;

    /*
     * Form the two word lists.
     * Tack the last word of the left list
     * onto the first word of the right list.
     *
     * There are other conceivable ways to do this,
     * but this definition gives the fewest surprises.
     */
    right = opcode_context_string_list_pop(icp);
    left = opcode_context_string_list_pop(icp);

    switch ((left->nstrings ? 1 : 0) | (right->nstrings ? 2 : 0))
    {
    case 0:
        /* both lists empty */
        break;

    case 1:
        /* right list empty */
        for (j = 0; j < left->nstrings; j++)
            opcode_context_string_push(icp, left->string[j]);
        break;

    case 2:
        /* left list empty */
        for (j = 0; j < right->nstrings; j++)
            opcode_context_string_push(icp, right->string[j]);
        break;

    case 3:
        /* at least one word in each list */
        for (j = 0; j < left->nstrings - 1; j++)
            opcode_context_string_push(icp, left->string[j]);
        s = str_catenate(left->string[j], right->string[0]);
        opcode_context_string_push(icp, s);
        str_free(s);
        for (j = 1; j < right->nstrings; j++)
            opcode_context_string_push(icp, right->string[j]);
        break;
    }
    string_list_delete(left);
    string_list_delete(right);
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
    "catenate",
    sizeof(opcode_ty),
    0,                          /* destructor */
    execute,
    execute,                    /* script */
    0,                          /* disassemble */
};


/*
 * NAME
 *      opcode_catenate_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_catenate_new(void);
 *
 * DESCRIPTION
 *      The opcode_catenate_new function is used to allocate a new instance
 *      of a catenate opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_catenate_new(void)
{
    opcode_ty       *op;

    trace(("opcode_catenate_new()\n{\n"));
    op = opcode_new(&method);
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
