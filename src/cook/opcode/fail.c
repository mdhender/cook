/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006, 2007 Peter Miller;
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

#include <common/error.h>
#include <cook/opcode/context.h>
#include <cook/opcode/fail.h>
#include <cook/opcode/private.h>
#include <cook/option.h>
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
    opcode_status_ty status;
    string_list_ty  *slp;

    trace(("opcode_fail::execute()\n{\n"));
    (void)op;
    status = opcode_status_success;
    slp = opcode_context_string_list_pop(icp);
    if (option_test(OPTION_ACTION))
    {
        if (slp->nstrings > 0)
        {
            string_ty       *s;

            s = wl2str(slp, 0, slp->nstrings, " ");
            error_raw("%s", s->str_text);
            str_free(s);
        }
        status = opcode_status_error;
    }
    string_list_delete(slp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      script
 *
 * SYNOPSIS
 *      opcode_status_ty script(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *      The script function is used to script the given opcode within
 *      the given interpretation context.
 *
 * RETURNS
 *      opcode_status_ty to indicate the result
 */

static opcode_status_ty
script(const opcode_ty *op, opcode_context_ty *icp)
{
    opcode_status_ty status;
    string_list_ty  *slp;

    trace(("opcode_fail::script()\n{\n"));
    (void)op;
    status = opcode_status_success;
    slp = opcode_context_string_list_pop(icp);
    if (option_test(OPTION_ACTION))
    {
        if (slp->nstrings)
        {
            string_ty       *s1;
            string_ty       *s2;

            s1 = wl2str(slp, 0, slp->nstrings, " ");
            s2 = str_quote_shell(s1);
            str_free(s1);
            printf("echo %s 1>&2\n", s2->str_text);
            str_free(s2);
        }
        printf("exit 1\n");
    }
    string_list_delete(slp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
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
    "fail",
    sizeof(opcode_ty),
    0,                          /* destructor */
    execute,
    script,
    0,                          /* disassemble */
};


/*
 * NAME
 *      opcode_fail_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_fail_new(void);
 *
 * DESCRIPTION
 *      The opcode_fail_new function is used to allocate a new instance
 *      of a fail opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_fail_new(void)
{
    opcode_ty       *op;

    trace(("opcode_fail_new()\n{\n"));
    op = opcode_new(&method);
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
