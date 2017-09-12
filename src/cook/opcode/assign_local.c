/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate assign_locals
 */

#include <error_intl.h>
#include <expr/position.h>
#include <id.h>
#include <id/variable.h>
#include <opcode/assign_local.h>
#include <opcode/context.h>
#include <opcode/private.h>
#include <str_list.h>
#include <trace.h>


typedef struct opcode_assign_local_ty opcode_assign_local_ty;
struct opcode_assign_local_ty
{
	opcode_ty	inherited;
	expr_position_ty pos;
};


/*
 * NAME
 *	destructor
 *
 * SYNOPSIS
 *	void destructor(opcode_ty *);
 *
 * DESCRIPTION
 *	The destructor function is used to release resources held by
 *	this opcode.  Do not free the opcode itself, this is done by the
 *	base class.
 */

static void destructor _((opcode_ty *));

static void
destructor(op)
	opcode_ty	*op;
{
	opcode_assign_local_ty *this;

	trace(("opcode_assign::destructor()\n{\n"/*}*/));
	this = (opcode_assign_local_ty *)op;
	expr_position_destructor(&this->pos);
	trace((/*{*/"}\n"));
}


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
execute(op, ocp)
	const opcode_ty	*op;
	opcode_context_ty *ocp;
{
	opcode_status_ty status;
	const opcode_assign_local_ty *this;
	string_list_ty	*name;
	string_list_ty	*value;

	trace(("opcode_assign::execute()\n{\n"/*}*/));
	status = opcode_status_success;
	this = (const opcode_assign_local_ty *)op;

	value = opcode_context_string_list_pop(ocp);
	name = opcode_context_string_list_pop(ocp);

	switch (name->nstrings)
	{
	case 0:
		error_with_position
		(
			&this->pos,
			0,
			i18n("lefthand side of assignment is empty")
		);
		status = opcode_status_error;
		break;

	case 1:
		if (ocp->call_stack_length < 1)
		{
			error_with_position
			(
				&this->pos,
				0,
				i18n("local assign global scope")
			);
			status = opcode_status_error;
			break;
		}
		opcode_context_id_assign
		(
			ocp,
			name->string[0],
			id_variable_new(value),
			1
		);
		break;

	default:
		error_with_position
		(
			&this->pos,
			0,
		       i18n("lefthand side of assignment is more than one word")
		);
		status = opcode_status_error;
		break;
	}

	string_list_delete(name);
	string_list_delete(value);
	trace(("return %s;\n", opcode_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
}


/*
 * NAME
 *	disassemble
 *
 * SYNOPSIS
 *	void disassemble(opcode_ty *);
 *
 * DESCRIPTION
 *	The disassemble function is used to disassemble the copdode and
 *	its arguments onto the standard output.  Don't worry about the
 *	location or a trailing newline.
 */

static void disassemble _((const opcode_ty *));

static void
disassemble(op)
	const opcode_ty	*op;
{
	const opcode_assign_local_ty *this;

	trace(("opcode_assign::disassemle()\n{\n"/*}*/));
	this = (const opcode_assign_local_ty *)op;
	if (this->pos.pos_name && this->pos.pos_name->str_length)
	{
		printf
		(
			"\t# %s:%d",
			this->pos.pos_name->str_text,
			this->pos.pos_line
		);
	}
	trace((/*{*/"}\n"));
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
	"assign_local",
	sizeof(opcode_assign_local_ty),
	destructor,
	execute,
	execute, /* script */
	disassemble,
};


/*
 * NAME
 *	opcode_assign_new
 *
 * SYNOPSIS
 *	opcode_ty *opcode_assign_new(void);
 *
 * DESCRIPTION
 *	The opcode_assign_new function is used to allocate a new instance
 *	of a assign opcode.
 *
 * RETURNS
 *	opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_assign_local_new(pp)
	expr_position_ty *pp;
{
	opcode_ty	*op;
	opcode_assign_local_ty *this;

	trace(("opcode_assign_local_new()\n{\n"/*}*/));
	op = opcode_new(&method);
	this = (opcode_assign_local_ty *)op;
	expr_position_copy_constructor(&this->pos, pp);
	trace(("return %08lX;\n", (long)op));
	trace((/*{*/"}\n"));
	return op;
}
