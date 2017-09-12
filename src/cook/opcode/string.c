/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate string opcodes
 */

#include <ac/stdio.h>

#include <expr/position.h>
#include <opcode/context.h>
#include <match.h>
#include <opcode/private.h>
#include <opcode/string.h>
#include <str.h>
#include <trace.h>


typedef struct opcode_string_ty opcode_string_ty;
struct opcode_string_ty
{
	opcode_ty	inherited;
	string_ty	*value;
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
	opcode_string_ty *this;

	trace(("opcode_string::destructor()\n{\n"/*}*/));
	this = (opcode_string_ty *)op;
	str_free(this->value);
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
execute(op, icp)
	const opcode_ty	*op;
	opcode_context_ty *icp;
{
	const opcode_string_ty *this;
	const match_ty	*mp;

	trace(("opcode_string::execute()\n{\n"/*}*/));
	this = (const opcode_string_ty *)op;

	/*
	 * If a wildcard mapping is in force (we are performing
	 * actions bound to an implicit recipe) the word will be
	 * mapped before it is returned.
	 */
	mp = opcode_context_match_top(icp);
	if (mp)
	{
		string_ty *s;

		s = match_reconstruct_rhs(mp, this->value, &this->pos);
		if (!s)
		{
			trace(("return error;\n"));
			trace((/*{*/"}\n"));
			return opcode_status_error;
		}
		opcode_context_string_push(icp, s);
		str_free(s);
	}
	else
		opcode_context_string_push(icp, this->value);
	trace(("return success;\n"));
	trace((/*{*/"}\n"));
	return opcode_status_success;
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
	const opcode_string_ty *this;
	string_ty	*s;

	trace(("opcode_string::disassemle()\n{\n"/*}*/));
	this = (const opcode_string_ty *)op;
	s = str_quote_cook(this->value, '"');
	printf("%s", s->str_text);
	str_free(s);
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
	"string",
	sizeof(opcode_string_ty),
	destructor,
	execute,
	execute, /* script */
	disassemble,
};


/*
 * NAME
 *	opcode_string_new
 *
 * SYNOPSIS
 *	opcode_ty *opcode_string_new(void);
 *
 * DESCRIPTION
 *	The opcode_string_new function is used to allocate a new instance
 *	of a string opcode.
 *
 * RETURNS
 *	opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_string_new(s, pp)
	string_ty	*s;
	const expr_position_ty *pp;
{
	opcode_ty	*op;
	opcode_string_ty *this;

	trace(("opcode_string_new()\n{\n"/*}*/));
	op = opcode_new(&method);
	this = (opcode_string_ty *)op;
	this->value = str_copy(s);
	expr_position_copy_constructor(&this->pos, pp);
	trace(("return %08lX;\n", (long)op));
	trace((/*{*/"}\n"));
	return op;
}
