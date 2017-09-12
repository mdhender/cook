/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate functions
 */

#include <ac/stdio.h>

#include <function.h>
#include <id.h>
#include <id/function.h>
#include <id/global.h>
#include <opcode/label.h>
#include <opcode/list.h>
#include <opcode/postlude.h>
#include <opcode/prelude.h>
#include <option.h>
#include <stmt.h>
#include <symtab.h>
#include <trace.h>


int
function_definition(name, body)
	string_ty	*name;
	stmt_ty		*body;
{
	stmt_result_ty	status;
	opcode_list_ty	*olp;

	/*
	 * generate the opcode stream
	 */
	trace(("function_definition(name = \"%s\")\n{\n"/*}*/, name->str_text));
	olp = opcode_list_new();
	olp->return_label = opcode_label_new();
	opcode_list_append(olp, opcode_prelude_new());
	status = stmt_code_generate(body, olp);
	if (status != STMT_OK)
	{
		opcode_label_delete(olp->return_label);
		opcode_list_delete(olp);
		trace((/*{*/"}\n"));
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
	trace((/*{*/"}\n"));
	return 1;
}
