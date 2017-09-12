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
 * MANIFEST: functions to manipulate nothing IDs
 */

#include <id/nothing.h>
#include <id/private.h>
#include <opcode/context.h>


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
 *	evaluate
 *
 * SYNOPSIS
 *	int evaluate(id_ty *, const string_list_ty *, string_list_ty *);
 *
 * DESCRIPTION
 *	The evaluate function is used to evaluate an ID instance (there
 *	are several types).  The arguments to the evaluation are not to
 *	be changed, the results are only to be appended (not
 *	constructor'ed first).
 *
 * RETURNS
 *	int; 0 on success, -1 on error.
 */

static int interpret _((id_ty *, opcode_context_ty *,
	const struct expr_position_ty *));

static int
interpret(idp, ocp, pp)
	id_ty		*idp;
	opcode_context_ty *ocp;
	const struct expr_position_ty *pp;
{
	string_list_ty	*arg;

	arg = opcode_context_string_list_pop(ocp);
	string_list_delete(arg);
	return 0;
}


/*
 * NAME
 *	method
 *
 * DESCRIPTION
 *	The method nothing describes this ID class.
 *
 * CAVEAT
 *	This symbol is not to be exported from this file (its name is
 *	not unique).
 */

static id_method_ty method =
{
	"nothing",
	sizeof(id_ty),
	destructor,
	interpret,
	interpret, /* script */
};


/*
 * NAME
 *	id_nothing_new
 *
 * SYNOPSIS
 *	void id_nothing_new(void);
 *
 * DESCRIPTION
 *	The id_nothing_new function is used to create a new instance of
 *	a nothing ID's value.  The given value is copied.
 *
 * RETURNS
 *	id_ty *; a pointer to a ID instance is dynamic memory.
 *
 * CAVEAT
 *	Use id_instance_delete when you are done with it.
 */

id_ty *
id_nothing_new()
{
	return id_instance_new(&method);
}
