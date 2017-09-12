/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2006-2009 Peter Miller
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
#include <cook/id/variable.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <cook/opcode/prelude.h>
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
    static string_ty *arg;
    string_list_ty  *slp;
    size_t          j;

    (void)op;

    /*
     * This set 10 specific variables:
     *      [arg]   is the complete list of function arguments
     *      [1]..[9] are the first nine arguments assigned to their
     *               own variables, or assigned empty if there are
     *               fewer arguments.
     */
    trace(("opcode_prelude::execute()\n{\n"));
    slp = opcode_context_string_list_pop(icp);
    /* first argument is the name of the function */
    if (slp->nstrings > 0)
    {
        string_ty       *name;
        string_list_ty  value;

        name = str_from_c("__FUNCTION__");
        string_list_constructor(&value);
        string_list_append(&value, slp->string[0]);
        opcode_context_id_assign(icp, name, id_variable_new(&value), 1);
        str_free(name);
        string_list_destructor(&value);

        string_list_remove(slp, slp->string[0]);
    }
    if (!arg)
        arg = str_from_c("arg");
    opcode_context_id_assign(icp, arg, id_variable_new(slp), 1);
    for (j = 1; j <= 9; ++j)
    {
        string_ty       *name;
        string_list_ty  value;

        name = str_format("@%ld", (long)j);
        string_list_constructor(&value);
        if (j <= slp->nstrings)
            string_list_append(&value, slp->string[j - 1]);
        opcode_context_id_assign(icp, name, id_variable_new(&value), 1);
        str_free(name);
        string_list_destructor(&value);
    }
    string_list_delete(slp);
    opcode_context_match_push(icp, (struct match_ty *)0);
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
    "prelude",
    sizeof(opcode_ty),
    0,                          /* destructor */
    execute,
    execute,                    /* script */
    0,                          /* disassemble */
};


/*
 * NAME
 *      opcode_prelude_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_prelude_new(void);
 *
 * DESCRIPTION
 *      The opcode_prelude_new function is used to allocate a new instance
 *      of a prelude opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_prelude_new(void)
{
    opcode_ty       *op;

    trace(("opcode_prelude_new()\n{\n"));
    op = opcode_new(&method);
    trace(("return %p;\n", op));
    trace(("}\n"));
    return op;
}
