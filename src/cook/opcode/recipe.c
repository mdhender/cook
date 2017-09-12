/*
 *	cook - file construction tool
 *	Copyright (C) 1997-1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate recipe opcodes
 */

#include <cook.h>
#include <error_intl.h>
#include <expr/position.h>
#include <flag.h>
#include <opcode/context.h>
#include <opcode/list.h>
#include <opcode/recipe.h>
#include <opcode/private.h>
#include <option.h>
#include <recipe.h>
#include <str_list.h>
#include <trace.h>


typedef struct opcode_recipe_ty opcode_recipe_ty;
struct opcode_recipe_ty
{
	opcode_ty	inherited;
	opcode_list_ty	*need1;
	opcode_list_ty	*need2;
	opcode_list_ty	*precondition;
	int		multiple;
	opcode_list_ty	*single_thread;
	opcode_list_ty	*host_binding;
	opcode_list_ty	*out_of_date;
	opcode_list_ty	*up_to_date;
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
	opcode_recipe_ty *this;

	trace(("opcode_recipe::destructor()\n{\n"/*}*/));
	this = (opcode_recipe_ty *)op;
	if (this->need1)
		opcode_list_delete(this->need1);
	if (this->need2)
		opcode_list_delete(this->need2);
	if (this->precondition)
		opcode_list_delete(this->precondition);
	if (this->single_thread)
		opcode_list_delete(this->single_thread);
	if (this->host_binding)
		opcode_list_delete(this->host_binding);
	if (this->out_of_date)
		opcode_list_delete(this->out_of_date);
	if (this->up_to_date)
		opcode_list_delete(this->up_to_date);
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
	const opcode_recipe_ty *this;
	opcode_status_ty status;
	string_list_ty	*target;
	recipe_ty	*rp;
	string_list_ty	*flags_words;
	flag_ty		*flags;

	trace(("opcode_recipe::execute()\n{\n"/*}*/));
	this = (const opcode_recipe_ty *)op;
	flags = 0;
	status = opcode_status_success;
	flags_words = opcode_context_string_list_pop(icp);
	target = opcode_context_string_list_pop(icp);

	if (target->nstrings == 0)
	{
		error_with_position
		(
			&this->pos,
			0,
			i18n("attempt to instantiate recipe with no targets")
		);
		status = opcode_status_error;
		goto done;
	}
	else if (target->nstrings > 1 && this->multiple && this->out_of_date)
	{
		error_with_position
		(
			&this->pos,
			0,
	    i18n("double-colon recipes with multiple targets are not permitted")
		);
		status = opcode_status_error;
		goto done;
	}

	/*
	 * process recipe flags
	 */
	flags = flag_recognize(flags_words, &this->pos);
	if (!flags)
	{
		status = opcode_status_error;
		goto done;
	}

	/*
	 * build the recipe
	 */
	rp =
		recipe_new
		(
			target,
			this->need1,
			this->need2,
			flags,
			this->multiple,
			this->precondition,
			this->single_thread,
			this->host_binding,
			this->out_of_date,
			this->up_to_date,
			&this->pos
		);
	if (!rp)
	{
		/*
		 * This means an error with the targets of an implicit
		 * recipe.  The error has already been printed.
		 */
		status = opcode_status_error;
		goto done;
	}

	/*
	 * add it to the list
	 */
	if (rp->implicit)
		cook_implicit_append(rp);
	else
		cook_explicit_append(rp);

	/*
	 * emit trace information, if enabled
	 */
	if (option_test(OPTION_REASON))
	{
		if (rp->implicit)
		{
			error_with_position
			(
				&this->pos,
				0,
				i18n("implicit recipe instantiated (reason)")
			);
		}
		else
		{
			error_with_position
			(
				&this->pos,
				0,
				i18n("explicit recipe instantiated (reason)")
			);
		}
	}
	recipe_delete(rp);

	done:
	if (flags)
		flag_delete(flags);
	string_list_delete(target);
	string_list_delete(flags_words);
	trace(("return %s;\n", opcode_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
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
	"recipe",
	sizeof(opcode_recipe_ty),
	destructor,
	execute,
	execute, /* script */
	0, /* disassemble */
};


/*
 * NAME
 *	opcode_recipe_new
 *
 * SYNOPSIS
 *	opcode_ty *opcode_recipe_new(void);
 *
 * DESCRIPTION
 *	The opcode_recipe_new function is used to allocate a new instance
 *	of a recipe opcode.
 *
 * RETURNS
 *	opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_recipe_new(need1, need2, precondition, multiple, single_thread,
		host_binding, out_of_date, up_to_date, pp)
	opcode_list_ty	*need1;
	opcode_list_ty	*need2;
	opcode_list_ty	*precondition;
	int		multiple;
	opcode_list_ty	*single_thread;
	opcode_list_ty	*host_binding;
	opcode_list_ty	*out_of_date;
	opcode_list_ty	*up_to_date;
	expr_position_ty *pp;
{
	opcode_ty	*op;
	opcode_recipe_ty *this;

	trace(("opcode_recipe_new()\n{\n"/*}*/));
	op = opcode_new(&method);
	this = (opcode_recipe_ty *)op;
	this->need1 = need1;
	this->need2 = need2;
	this->precondition = precondition;
	this->multiple = multiple;
	this->single_thread = single_thread;
	this->host_binding = host_binding;
	this->out_of_date = out_of_date;
	this->up_to_date = up_to_date;
	expr_position_copy_constructor(&this->pos, pp);
	trace(("return %08lX;\n", (long)op));
	trace((/*{*/"}\n"));
	return op;
}
