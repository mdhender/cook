/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate builtin IDs
 */

#include <expr/position.h>
#include <builtin/private.h>
#include <id/builtin.h>
#include <id/private.h>
#include <opcode/context.h>
#include <str_list.h>
#include <trace.h>


typedef struct id_builtin_ty id_builtin_ty;
struct id_builtin_ty
{
	id_ty		inherited;
	builtin_ty	*value;
};


/*
 * NAME
 *	destructor
 *
 * SYNOPSIS
 *	void destructor(id_ty *);
 *
 * DESCRIPTION
 *	The destructor function is used to release the resources held by
 *	an ID instance.
 */

static void destructor _((id_ty *));

static void
destructor(idp)
	id_ty		*idp;
{
}


/*
 * NAME
 *	interpret
 *
 * SYNOPSIS
 *	int interpret(id_ty *, const string_list_ty *, string_list_ty *);
 *
 * DESCRIPTION
 *	The interpret function is used to evaluate an ID instance (there
 *	are several types).  The arguments to the evaluation are not to
 *	be changed, the results are only to be appended (not
 *	constructor'ed first).
 *
 * RETURNS
 *	int; 0 on success, -1 on error.
 */

static int interpret _((id_ty *, opcode_context_ty *,
	const expr_position_ty *));

static int
interpret(idp, ocp, pp)
	id_ty		*idp;
	opcode_context_ty *ocp;
	const expr_position_ty *pp;
{
	int		status;
	id_builtin_ty	*this;
	string_list_ty	*input;
	string_list_ty	output;

	/*
	 * fetch the input arguments
	 */
	trace(("id_builtin::interpret(idp = %08lX)\n{\n", (long)idp));
	this = (id_builtin_ty *)idp;
	input = opcode_context_string_list_pop(ocp);

	/*
	 * call the builtin function
	 */
	string_list_constructor(&output);
	status = builtin_interpret(this->value, &output, input, pp, ocp);

	/*
	 * push the return value ono the value stask
	 */
	opcode_context_string_push_list(ocp, &output);

	/*
	 * clean up and go home
	 */
	string_list_delete(input);
	string_list_destructor(&output);
	trace(("return %d;\n", status));
	trace(("}\n"));
	return status;
}


/*
 * NAME
 *	script
 *
 * SYNOPSIS
 *	int script(id_ty *, const string_list_ty *, string_list_ty *);
 *
 * DESCRIPTION
 *	The script function is used to evaluate an ID instance (there
 *	are several types).  The arguments to the evaluation are not to
 *	be changed, the results are only to be appended (not
 *	constructor'ed first).
 *
 * RETURNS
 *	int; 0 on success, -1 on error.
 */

static int script _((id_ty *, opcode_context_ty *,
	const expr_position_ty *));

static int
script(idp, ocp, pp)
	id_ty		*idp;
	opcode_context_ty *ocp;
	const expr_position_ty *pp;
{
	int		status;
	id_builtin_ty	*this;
	string_list_ty	*input;
	string_list_ty	output;

	/*
	 * fetch the input arguments
	 */
	trace(("id_builtin::script(idp = %08lX)\n{\n", (long)idp));
	this = (id_builtin_ty *)idp;
	input = opcode_context_string_list_pop(ocp);

	/*
	 * call the builtin function
	 */
	string_list_constructor(&output);
	status = builtin_script(this->value, &output, input, pp, ocp);

	/*
	 * push the return value ono the value stask
	 */
	opcode_context_string_push_list(ocp, &output);

	/*
	 * clean up and go home
	 */
	string_list_delete(input);
	string_list_destructor(&output);
	trace(("return %d;\n", status));
	trace(("}\n"));
	return status;
}


/*
 * NAME
 *	method
 *
 * DESCRIPTION
 *	The method builtin describes this ID class.
 *
 * CAVEAT
 *	This symbol is not to be exported from this file (its name is
 *	not unique).
 */

static id_method_ty method =
{
	"builtin function",
	sizeof(id_builtin_ty),
	destructor,
	interpret,
	script,
};


/*
 * NAME
 *	id_builtin_new
 *
 * SYNOPSIS
 *	void id_builtin_new(void);
 *
 * DESCRIPTION
 *	The id_builtin_new function is used to create a new instance of
 *	a builtin ID's value.  The given value is copied.
 *
 * RETURNS
 *	id_ty *; a pointer to a ID instance is dynamic memory.
 *
 * CAVEAT
 *	Use id_instance_delete when you are done with it.
 */

id_ty *
id_builtin_new(bp)
	builtin_ty	*bp;
{
	id_ty		*idp;
	id_builtin_ty	*this;

	trace(("id_builtin::new()\n{\n"/*}*/));
	idp = id_instance_new(&method);
	this = (id_builtin_ty *)idp;
	this->value = bp;
	trace(("return %08lX;\n", (long)idp));
	trace((/*{*/"}\n"));
	return idp;
}
