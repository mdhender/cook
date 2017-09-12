/*
 *	cook - file construction tool
 *	Copyright (C) 1997-1999, 2001, 2004 Peter Miller;
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
 * MANIFEST: functions to manipulate function opcodes
 */

#include <ac/stdio.h>

#include <builtin.h>
#include <error_intl.h>
#include <expr.h>
#include <expr/position.h>
#include <function.h>
#include <id.h>
#include <id/nothing.h>
#include <opcode/context.h>
#include <opcode/function.h>
#include <opcode/list.h>
#include <opcode/private.h>
#include <str_list.h>
#include <trace.h>


typedef struct opcode_function_ty opcode_function_ty;
struct opcode_function_ty
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
	opcode_function_ty *this;

	trace(("opcode_function::destructor()\n{\n"/*}*/));
	this = (opcode_function_ty *)op;
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
	const opcode_function_ty *this;
	opcode_status_ty status;
	string_list_ty	*arg;
	id_ty		*value;

	/*
	 * extract the function call arguments
	 */
	trace(("opcode_function::execute()\n{\n"/*}*/));
	this = (const opcode_function_ty *)op;
	status = opcode_status_success;
	arg = opcode_context_string_list_peek(ocp);
	if (arg->nstrings == 0)
		string_list_append(arg, str_false);

	/*
	 * The first word is the function name.
	 *
	 * Use fuzzy matching to get nicer error messages.
	 */
	value = opcode_context_id_search(ocp, arg->string[0]);
	if (!value)
	{
		string_ty	*other;

		status = opcode_status_error;
		value =
			opcode_context_id_search_fuzzy
			(
				ocp,
				arg->string[0],
				&other
			);
		if (value)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", arg->string[0]);
			sub_var_set(scp, "Guess", "%S", other);
			if (arg->nstrings <= 1)
			{
				error_with_position
				(
					&this->pos,
					scp,
	i18n("undefined variable \"$name\", closest is the \"$guess\" variable")
				);
			}
			else
			{
				error_with_position
				(
					&this->pos,
					scp,
	i18n("undefined function \"$name\", closest is the \"$guess\" function")
				);
			}
			sub_context_delete(scp);
		}
		else
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", arg->string[0]);
			if (arg->nstrings <= 1)
			{
				error_with_position
				(
					&this->pos,
					scp,
					i18n("undefined variable \"$name\"")
				);
			}
			else
			{
				error_with_position
				(
					&this->pos,
					scp,
					i18n("undefined function \"$name\"")
				);
			}
			sub_context_delete(scp);
			value = id_nothing_new();
			opcode_context_id_assign(ocp, arg->string[0], value, 0);
		}
	}

	/*
	 * evaluate the function call
	 * (or variable reference)
	 */
	trace(("mark\n"));
	if (id_interpret(value, ocp, &this->pos) < 0)
	{
		/*
		 * Error message already printed.
		 */
		status = opcode_status_error;
	}
	trace(("mark\n"));

	/* DO NOT string_list_delete(arg), we only peeked. */
	trace(("return %s;\n", opcode_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
}


/*
 * NAME
 *	script
 *
 * SYNOPSIS
 *	opcode_status_ty script(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *	The script function is used to script the given opcode within
 *	the given interpretation context.
 *
 * RETURNS
 *	opcode_status_ty to indicate the result of the execution
 */

static opcode_status_ty script _((const opcode_ty *, opcode_context_ty *));

static opcode_status_ty
script(op, ocp)
	const opcode_ty	*op;
	opcode_context_ty *ocp;
{
	const opcode_function_ty *this;
	opcode_status_ty status;
	string_list_ty	*arg;
	id_ty		*value;

	/*
	 * extract the function call arguments
	 */
	trace(("opcode_function::execute()\n{\n"/*}*/));
	this = (const opcode_function_ty *)op;
	status = opcode_status_success;
	arg = opcode_context_string_list_peek(ocp);
	if (arg->nstrings == 0)
		string_list_append(arg, str_false);

	/*
	 * The first word is the function name.
	 *
	 * Use fuzzy matching to get nicer error messages.
	 */
	value = opcode_context_id_search(ocp, arg->string[0]);
	if (!value)
	{
		string_ty	*other;

		status = opcode_status_error;
		value =
			opcode_context_id_search_fuzzy
			(
				ocp,
				arg->string[0],
				&other
			);
		if (value)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", arg->string[0]);
			sub_var_set(scp, "Guess", "%S", other);
			if (arg->nstrings <= 1)
			{
				error_with_position
				(
					&this->pos,
					scp,
	i18n("undefined variable \"$name\", closest is the \"$guess\" variable")
				);
			}
			else
			{
				error_with_position
				(
					&this->pos,
					scp,
	i18n("undefined function \"$name\", closest is the \"$guess\" function")
				);
			}
			sub_context_delete(scp);
		}
		else
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", arg->string[0]);
			if (arg->nstrings <= 1)
			{
				error_with_position
				(
					&this->pos,
					scp,
					i18n("undefined variable \"$name\"")
				);
			}
			else
			{
				error_with_position
				(
					&this->pos,
					scp,
					i18n("undefined function \"$name\"")
				);
			}
			sub_context_delete(scp);
			value = id_nothing_new();
			opcode_context_id_assign(ocp, arg->string[0], value, 0);
		}
	}

	/*
	 * evaluate the function call
	 * (or variable reference)
	 */
	trace(("mark\n"));
	if (id_interpret_script(value, ocp, &this->pos) < 0)
		status = opcode_status_error;
	trace(("mark\n"));

	/* DO NOT string_list_delete(arg), we only peeked. */
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
	const opcode_function_ty *this;

	trace(("opcode_function::disassemble()\n{\n"/*}*/));
	this = (const opcode_function_ty *)op;
	if (this->pos.pos_name && this->pos.pos_name->str_length)
	{
		printf
		(
			"# %s:%d",
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
	"function",
	sizeof(opcode_function_ty),
	destructor,
	execute,
	script,
	disassemble,
};


/*
 * NAME
 *	opcode_function_new
 *
 * SYNOPSIS
 *	opcode_ty *opcode_function_new(void);
 *
 * DESCRIPTION
 *	The opcode_function_new function is used to allocate a new
 *	instance of an function opcode.
 *
 * RETURNS
 *	opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_function_new(pp)
	const expr_position_ty *pp;
{
	opcode_ty	*op;
	opcode_function_ty *this;

	trace(("opcode_function_new()\n{\n"/*}*/));
	op = opcode_new(&method);
	this = (opcode_function_ty *)op;
	expr_position_copy_constructor(&this->pos, pp);
	trace(("return %08lX;\n", (long)op));
	trace((/*{*/"}\n"));
	return op;
}
