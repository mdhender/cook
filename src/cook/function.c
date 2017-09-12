/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006, 2007 Peter Miller;
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

#include <cook/function.h>
#include <cook/id.h>
#include <cook/id/function.h>
#include <cook/id/global.h>
#include <cook/opcode/label.h>
#include <cook/opcode/list.h>
#include <cook/opcode/postlude.h>
#include <cook/opcode/prelude.h>
#include <cook/option.h>
#include <cook/stmt.h>
#include <common/symtab.h>
#include <common/trace.h>


int
function_definition(string_ty *name, stmt_ty *body)
{
    stmt_result_ty  status;
    opcode_list_ty  *olp;

    /*
     * generate the opcode stream
     */
    trace(("function_definition(name = \"%s\")\n{\n", name->str_text));
    olp = opcode_list_new();
    olp->return_label = opcode_label_new();
    opcode_list_append(olp, opcode_prelude_new());
    status = stmt_code_generate(body, olp);
    if (status != STMT_OK)
    {
        opcode_label_delete(olp->return_label);
        opcode_list_delete(olp);
        trace(("}\n"));
        return 0;
    }
    opcode_label_define(olp->return_label, olp->length);
    opcode_list_append(olp, opcode_postlude_new());
    opcode_label_delete(olp->return_label);
    olp->return_label = 0;

    trace(("disassemble\n"));
    if (option_test(OPTION_DISASSEMBLE))
    {
        printf("\n%s:\n", name->str_text);
        opcode_list_disassemble(olp);
    }

    /*
     * remember the function
     */
    trace(("remember\n"));
    symtab_assign(id_global_stp(), name, id_function_new(olp));
    trace(("}\n"));
    return 1;
}
