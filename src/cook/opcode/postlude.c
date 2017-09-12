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
 * MANIFEST: functions to manipulate postlude opcodes
 */

#include <id.h>
#include <opcode/context.h>
#include <opcode/private.h>
#include <opcode/postlude.h>
#include <trace.h>


/*
 * NAME
 *	execute
 *
 * SYNOPSIS
 *	opcode_status_ty execute(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *	The execute function is used to execute the given opcode within
 *	the given interpretation context.
 *
 * RETURNS
 *	opcode_status_ty to indicate the result of the execution
 */

static opcode_status_ty execute _((const opcode_ty *, opcode_context_ty *));

static opcode_status_ty
execute(op, icp)
	const opcode_ty	*op;
	opcode_context_ty *icp;
{
	/*
	 * delete the variables defined by the prelude opcode
	 */
	trace(("opcode_postlude::execute()\n{\n"/*}*/));
	opcode_context_match_pop(icp);
	trace(("return success;\n"));
	trace((/*{*/"}\n"));
	return opcode_status_success;
}


/*
 * NAME
 *	method
 *
 * DESCRIPTION
 *	The method variable describes this class.
 *
 * CAVEAT
 *	This symbol is not exported from this file.
 */

static opcode_method_ty method =
{
	"postlude",
	sizeof(opcode_ty),
	0, /* destructor */
	execute,
	execute, /* script */
	0, /* disassemble */
};


/*
 * NAME
 *	opcode_postlude_new
 *
 * SYNOPSIS
 *	opcode_ty *opcode_postlude_new(void);
 *
 * DESCRIPTION
 *	The opcode_postlude_new function is used to allocate a new instance
 *	of a postlude opcode.
 *
 * RETURNS
 *	opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_postlude_new()
{
	opcode_ty	*op;

	trace(("opcode_postlude_new()\n{\n"/*}*/));
	op = opcode_new(&method);
	trace(("return %08lX;\n", (long)op));
	trace((/*{*/"}\n"));
	return op;
}
