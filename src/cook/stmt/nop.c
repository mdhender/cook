/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate nop statement nodes
 */

#include <stmt.h>
#include <stmt/nop.h>
#include <trace.h>


/*
 * NAME
 *	code_generate
 *
 * SYNOPSIS
 *	stmt_result_ty code_generate(stmt_ty *sp, opcode_list_ty *olp);
 *
 * DESCRIPTION
 *	The code_generate function is used to generate the opcodes for
 *	this statement node.
 *
 * RETURNS
 *	The value returned indicates why the code generation terminated.
 */

static stmt_result_ty code_generate _((stmt_ty *, struct opcode_list_ty *));

static stmt_result_ty
code_generate(sp, olp)
	stmt_ty		*sp;
	struct opcode_list_ty *olp;
{
	trace(("code_generate(sp = %08X)\n{\n"/*}*/, sp));
	assert(sp);
	trace((/*{*/"}\n"));
	return STMT_OK;
}


/*
 * NAME
 *	method - class method table
 *
 * DESCRIPTION
 *	This is the class method table.  It contains a description of
 *	the class, its name, size and pointers to its virtual methods.
 *
 * CAVEAT
 *	This symbol is NOT to be exported from this file scope.
 */

static stmt_method_ty method =
{
	"nop",
	sizeof(stmt_ty),
	0, /* destructor */
	code_generate,
};


/*
 * NAME
 *	stmt_nop_new - create a new nop statement node
 *
 * SYNOPSIS
 *	stmt_ty *stmt_nop_new(string_ty *);
 *
 * DESCRIPTION
 *	The stmt_nop_new function is used to create a new instance
 *	of a nop statement node.
 *
 * RETURNS
 *	stmt_ty *; pointer to polymorphic statement instance.
 *
 * CAVEAT
 *	This function allocates data in dynamic memory.  It is the
 *	caller's responsibility to free this data, using stmt_delete,
 *	when it is no longer required.
 */

stmt_ty *
stmt_nop_new()
{
	stmt_ty		*sp;

	trace(("stmt_nop_new()\n{\n"/*}*/));
	sp = stmt_private_new(&method);
	trace(("return %8.8lX;\n", (long)sp));
	trace((/*{*/"}\n"));
	return sp;
}
