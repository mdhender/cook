/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2004 Peter Miller;
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
 * MANIFEST: functions to manipulate opcode lists
 */

#include <ac/stdio.h>

#include <fflush_slow.h>
#include <match.h>
#include <mem.h>
#include <opcode.h>
#include <opcode/context.h>
#include <opcode/list.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	opcode_list_new
 *
 * SYNOPSIS
 *	opcode_list_ty *opcode_list_new(void);
 *
 * DESCRIPTION
 *	The opcode_list_new function is used to allocate a new opcode
 *	list instance in dynamic memory.
 *
 * CAVEAT
 *	Use opcode_list_dlete when you are done with it.
 */

opcode_list_ty *
opcode_list_new()
{
	opcode_list_ty	*olp;

	trace(("opcode_list_new()\n{\n"/*}*/));
	olp = mem_alloc(sizeof(opcode_list_ty));
	olp->reference_count = 1;
	olp->length = 0;
	olp->maximum = 0;
	olp->list = 0;
	olp->break_label = 0;
	olp->continue_label = 0;
	olp->return_label = 0;
	trace(("return %08lX;\n", (long)olp));
	trace((/*{*/"}\n"));
	return olp;
}


/*
 * NAME
 *	opcode_list_delete
 *
 * SYNOPSIS
 *	void opcode_list_delete(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_list_delete function is used to release the resources
 *	held by an opcode list.
 */

void
opcode_list_delete(olp)
	opcode_list_ty	*olp;
{
	size_t		j;

	trace(("opcode_list_delete(olp = %08lX)\n{\n"/*}*/, (long)olp));
	assert(olp);
	assert(olp->reference_count >= 1);
	olp->reference_count--;
	if (olp->reference_count <= 0)
	{
		assert(olp->length <= olp->maximum);
		for (j = 0; j < olp->length; ++j)
			opcode_delete(olp->list[j]);
		if (olp->list)
			mem_free(olp->list);
		olp->length = 0;
		olp->maximum = 0;
		olp->list = 0;
		mem_free(olp);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_list_copy
 *
 * SYNOPSIS
 *	opcode_list_ty *opcode_list_copy(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_list_copy function is used to make a copy of an
 *	opcode list.  They are reference counted, so it's not as bad as
 *	it sounds.
 */

opcode_list_ty *
opcode_list_copy(olp)
	opcode_list_ty	*olp;
{
	assert(olp);
	assert(olp->reference_count >= 1);
	olp->reference_count++;
	return olp;
}


/*
 * NAME
 *	opcode_list_append
 *
 * SYNOPSIS
 *	void opcode_list_append(opcode_list_ty *, opcode_ty *);
 *
 * DESCRIPTION
 *	The opcode_list_append function is used to append an opcode to
 *	an opcode list.
 *
 * CAVEAT
 *	DO NOT delete the opcode, this will be done when the opcode list
 *	is deleted.
 */

void
opcode_list_append(olp, op)
	opcode_list_ty	*olp;
	opcode_ty	*op;
{
	trace(("opcode_list_append(olp = %08lX, op = %08lX)\n{\n"/*}*/,
		(long)olp, (long)op));
	assert(olp);
	assert(op);
	if (olp->length >= olp->maximum)
	{
		size_t		nbytes;

		olp->maximum = olp->maximum * 2 + 4;
		nbytes = olp->maximum * sizeof(olp->list[0]);
		olp->list = mem_change_size(olp->list, nbytes);
	}
	olp->list[olp->length++] = op;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_list_disassemble
 *
 * SYNOPSIS
 *	void opcode_list_disassemble(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_list_disassemble function is used to disassemble the
 *	opcodes in an opcode list onto the standard output.
 *
 * CAVEAT
 *	The option which triggers this from the command line is
 *	undocumented, principally because this is debugging
 *	functionality.
 */

void
opcode_list_disassemble(olp)
	opcode_list_ty	*olp;
{
	size_t		j;

	trace(("opcode_list_disassemble(olp = %08lX)\n{\n"/*}*/, (long)olp));
	printf("\n");
	for (j = 0; j < olp->length; ++j)
	{
		printf("%4ld:\t", (long)j);
		opcode_disassemble(olp->list[j]);
		printf("\n");
	}
	fflush_slowly(stdout);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_list_run
 *
 * SYNOPSIS
 *	string_list_ty *opcode_list_run(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_list_run function is used to execute an opcode list
 *	with the expectation of extracting a string list result.  Used
 *	to evaluate the need1 and need2 ingredients.
 *
 * RETURNS
 *	string_list_ty *; or NULL pointer on error
 *
 * CAVEAT
 *	Use string_list_delete when you are done with it.
 */

string_list_ty *
opcode_list_run(olp, mp)
	opcode_list_ty	*olp;
	const match_ty	*mp;
{
	opcode_context_ty *ocp;
	opcode_status_ty status;
	string_list_ty	*result;

	if (!olp)
		return string_list_new();
	ocp = opcode_context_new(olp, mp);
	status = opcode_context_execute(ocp);
	if (status != opcode_status_success)
		result = 0;
	else
		result = opcode_context_string_list_pop(ocp);
	opcode_context_delete(ocp);
	return result;
}


/*
 * NAME
 *	opcode_list_run_bool
 *
 * SYNOPSIS
 *	string_list_ty *opcode_list_run_bool(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_list_run_bool function is used to execute an opcode list
 *	with the expectation of extracting a boolean result.  Used
 *	to evaluate recipe predicates.
 *
 * RETURNS
 *	int; 0 if false, 1 if true, -1 on error
 */

int
opcode_list_run_bool(olp, mp)
	opcode_list_ty	*olp;
	const match_ty	*mp;
{
	opcode_context_ty *ocp;
	opcode_status_ty status;
	string_list_ty	*slp;
	int		result;

	if (!olp)
		return 1;
	ocp = opcode_context_new(olp, mp);
	status = opcode_context_execute(ocp);
	if (status != opcode_status_success)
		result = -1;
	else
	{
		slp = opcode_context_string_list_pop(ocp);
		result = string_list_bool(slp);
		string_list_delete(slp);
	}
	opcode_context_delete(ocp);
	return result;
}
