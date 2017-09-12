/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate gosub statement nodes
 */

#include <error_intl.h>
#include <expr/list.h>
#include <expr/position.h>
#include <opcode/function.h>
#include <opcode/gosub.h>
#include <opcode/list.h>
#include <opcode/push.h>
#include <stmt.h>
#include <stmt/gosub.h>
#include <trace.h>


typedef struct stmt_gosub_ty stmt_gosub_ty;
struct stmt_gosub_ty
{
	stmt_ty		inherited;
	expr_list_ty	text;
	expr_position_ty pos;
};


/*
 *  NAME
 *	destructor - free a gosub statement node
 *
 *  SYNOPSIS
 *	void destructor(stmt_ty *sp);
 *
 *  DESCRIPTION
 *	The destructor function releases the resources held by a gosub
 *	statement instance after it is finished with.
 *
 *  CAVEAT
 *	Do not free the node itself, this the the destructor, not
 *	delete.
 */

static void destructor _((stmt_ty *));

static void
destructor(sp)
	stmt_ty		*sp;
{
	stmt_gosub_ty	*this;

	this = (stmt_gosub_ty *)sp;
	expr_list_destructor(&this->text);
	expr_position_destructor(&this->pos);
}


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

static stmt_result_ty code_generate _((stmt_ty *, opcode_list_ty *));

static stmt_result_ty
code_generate(sp, olp)
	stmt_ty		*sp;
	opcode_list_ty	*olp;
{
	stmt_gosub_ty	*this;

	trace(("stmt_gosub::code_generate(sp = %08X)\n{\n"/*}*/, sp));
	assert(sp);
	this = (stmt_gosub_ty *)sp;
	opcode_list_append(olp, opcode_push_new());

	/*
	 * Emit a call opcode for a normal function call.
	 */
	opcode_list_append(olp, opcode_push_new());
	expr_list_code_generate(&this->text, olp);
	opcode_list_append(olp, opcode_function_new(&this->pos));

	/*
	 * The gosub opcode merely cleans up any return value
	 * and checks to see if it indicates an error.
	 */
	opcode_list_append(olp, opcode_gosub_new(&this->pos));
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
	"gosub",
	sizeof(stmt_gosub_ty),
	destructor,
	code_generate,
};


/*
 * NAME
 *	stmt_gosub_new - create a new gosub statement node
 *
 * SYNOPSIS
 *	stmt_ty *stmt_gosub_new(string_ty *);
 *
 * DESCRIPTION
 *	The stmt_gosub_new function is used to create a new instance
 *	of a gosub statement node.
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
stmt_gosub_new(arg, pp)
	expr_list_ty	*arg;
	expr_position_ty *pp;
{
	stmt_ty		*sp;
	stmt_gosub_ty	*this;
	expr_position_ty *pos;

	trace(("stmt_gosub_new()\n{\n"/*}*/));
	sp = stmt_private_new(&method);
	this = (stmt_gosub_ty *)sp;
	expr_list_copy_constructor(&this->text, arg);
	pos = expr_list_position(arg);
	if (!pos)
		pos = pp;
	expr_position_copy_constructor(&this->pos, pos);
	trace(("return %8.8lX;\n", (long)sp));
	trace((/*{*/"}\n"));
	return sp;
}
