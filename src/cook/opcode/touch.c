/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2004, 2006-2009 Peter Miller
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

#include <common/error_intl.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <cook/opcode/touch.h>
#include <cook/option.h>
#include <cook/os_interface.h>
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
    string_list_ty  *value;

    trace(("opcode_touch::execute()\n{\n"));
    (void)op;
    status = opcode_status_success;
    value = opcode_context_string_list_pop(icp);
    if (!option_test(OPTION_SILENT))
    {
        string_ty       *s;
        sub_context_ty  *scp;

        /*
         * If the command has not been silenced,
         * form it into a string and echo it.
         */
        s = wl2str(value, 0, value->nstrings - 1, (char *)0);
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", s);
        error_intl(scp, i18n("touch $filename"));
        sub_context_delete(scp);
        str_free(s);
    }
    if (option_test(OPTION_ACTION))
    {
        size_t          j;

        for (j = 0; j < value->nstrings; j++)
        {
            if (os_touch(value->string[j]))
            {
                /*
                 * Error message already printed.
                 */
                status = opcode_status_error;
            }
        }
    }
    string_list_delete(value);
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
    string_list_ty  *value;
    size_t          j;
    string_ty       *s;

    trace(("opcode_touch::script()\n{\n"));
    (void)op;
    status = opcode_status_success;
    value = opcode_context_string_list_pop(icp);
    if (!option_test(OPTION_SILENT))
    {
        printf("echo touch");
        for (j = 0; j < value->nstrings; j++)
        {
            s = str_quote_shell(value->string[j]);
            printf(" %s", s->str_text);
            str_free(s);
        }
        printf("\n");
    }
    if (option_test(OPTION_ACTION))
    {
        printf("touch");
        for (j = 0; j < value->nstrings; j++)
        {
            s = str_quote_shell(value->string[j]);
            printf(" %s", s->str_text);
            str_free(s);
        }
        printf(" || exit 1\n");
    }
    string_list_delete(value);
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
    "touch",
    sizeof(opcode_ty),
    0,                          /* destructor */
    execute,
    script,
    0,                          /* disassemble */
};


/*
 * NAME
 *      opcode_touch_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_touch_new(void);
 *
 * DESCRIPTION
 *      The opcode_touch_new function is used to allocate a new instance
 *      of a touch opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_touch_new(void)
{
    opcode_ty       *op;

    trace(("opcode_touch_new()\n{\n"));
    op = opcode_new(&method);
    trace(("return %p;\n", op));
    trace(("}\n"));
    return op;
}
