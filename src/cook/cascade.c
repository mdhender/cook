/*
 *	cook - file construction tool
 *	Copyright (C) 1998, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate cascades
 */

#include <cascade.h>
#include <mem.h>
#include <strip_dot.h>
#include <symtab.h>
#include <trace.h>


static	symtab_ty	*stp;
static	int		used;


void
cascade_list_constructor(clp)
	cascade_list_ty	*clp;
{
	clp->length = 0;
	clp->maximum = 0;
	clp->list = 0;
}


static cascade_list_ty *cascade_list_new _((void));

static cascade_list_ty *
cascade_list_new()
{
	cascade_list_ty	*clp;

	clp = mem_alloc(sizeof(cascade_list_ty));
	cascade_list_constructor(clp);
	return clp;
}


void
cascade_list_destructor(clp)
	cascade_list_ty	*clp;
{
	size_t		j;
	cascade_ty	*cp;

	for (j = 0; j < clp->length; ++j)
	{
		cp = &clp->list[j];
		str_free(cp->ingredient);
		expr_position_destructor(&cp->pos);
	}
	if (clp->list)
		mem_free(clp->list);
	clp->length = 0;
	clp->maximum = 0;
	clp->list = 0;
}


static void cascade_list_delete _((cascade_list_ty *));

static void
cascade_list_delete(clp)
	cascade_list_ty	*clp;
{
	cascade_list_destructor(clp);
	mem_free(clp);
}


static void reap _((void *));

static void
reap(p)
	void		*p;
{
	cascade_list_ty	*this;

	this = p;
	cascade_list_delete(this);
}


void
cascade_reset()
{
	trace(("cascade_reset()\n{\n"));
	if (stp)
	{
		symtab_free(stp);
		stp = 0;
	}
	used = 0;
	trace(("}\n"));
}


static cascade_list_ty *cascade_find_inner _((string_ty *));

static cascade_list_ty *
cascade_find_inner(name)
	string_ty	*name;
{
	cascade_list_ty	*result;

	trace(("cascade_find_inner(name = \"%s\")\n{\n", name->str_text));
	if (!stp)
	{
		stp = symtab_alloc(0);
		stp->reap = reap;
	}
	result = symtab_query(stp, name);
	if (!result)
	{
		result = cascade_list_new();
		symtab_assign(stp, name, result);
	}
	trace(("return %08lX;\n", (long)result));
	trace(("}\n"));
	return result;
}


static void cascade_list_append _((cascade_list_ty *, string_ty *,
	const struct expr_position_ty *));

static void
cascade_list_append(clp, ingredient, pp)
	cascade_list_ty	*clp;
	string_ty	*ingredient;
	const struct expr_position_ty *pp;
{
	size_t		j;
	cascade_ty	*cp;
	size_t		nbytes;

	trace(("cascade_list_append(ingredient = \"%s\")\n{\n",
		ingredient->str_text));
	for (j = 0; j < clp->length; ++j)
	{
		cp = &clp->list[j];
		/* keep (almost) duplicates for error reporting later */
		if
		(
			str_equal(ingredient, cp->ingredient)
		&&
			str_equal(pp->pos_name, cp->pos.pos_name)
		)
			return;
	}
	if (clp->length >= clp->maximum)
	{
		clp->maximum = clp->maximum * 2 + 4;
		nbytes = clp->maximum * sizeof(clp->list[0]);
		clp->list = mem_change_size(clp->list, nbytes);
	}
	cp = &clp->list[clp->length++];
	cp->ingredient = str_copy(ingredient);
	expr_position_copy_constructor(&cp->pos, pp);
	trace(("}\n"));
}


static void cascade_list_append_list _((cascade_list_ty *,
	const string_list_ty *, const struct expr_position_ty *));

static void
cascade_list_append_list(clp, ingredient, pp)
	cascade_list_ty	*clp;
	const string_list_ty	*ingredient;
	const struct expr_position_ty *pp;
{
	size_t		j;

	trace(("cascade_list_append_list()\n{\n"));
	for (j = 0; j < ingredient->nstrings; ++j)
		cascade_list_append(clp, ingredient->string[j], pp);
	trace(("}\n"));
}


static void cascade_recipe_inner _((string_ty *, const string_list_ty *,
	const expr_position_ty *));

static void
cascade_recipe_inner(target, need, pp)
	string_ty	*target;
	const string_list_ty *need;
	const expr_position_ty *pp;
{
	cascade_list_ty	*clp;

	trace(("cascade_recipe_inner()\n{\n"));
	clp = cascade_find_inner(target);
	cascade_list_append_list(clp, need, pp);
	trace(("}\n"));
}


void
cascade_recipe(target, need, pp)
	string_list_ty  *target;
	string_list_ty  *need;
	const expr_position_ty *pp;
{
	size_t		j;

	trace(("cascade_recipe()\n{\n"));
	strip_dot_list(target);
	strip_dot_list(need);
	if (target->nstrings && need->nstrings)
		used = 1;
	for (j = 0; j < target->nstrings; ++j)
		cascade_recipe_inner(target->string[j], need, pp);
	trace(("}\n"));
}


void
cascade_find(need, result)
	const string_list_ty *need;
	cascade_list_ty *result;
{
	string_list_ty	closure;
	cascade_ty	*cp;
	cascade_list_ty	*clp;
	size_t		j, k;

	trace(("cascade_find()\n{\n"));
	cascade_list_constructor(result);
	if (!used)
	{
		trace(("}\n"));
		return;
	}
	string_list_copy_constructor(&closure, need);
	trace(("mark\n"));
	for (j = 0; j < closure.nstrings; ++j)
	{
		trace(("filename %s\n", closure.string[j]->str_text));
		clp = cascade_find_inner(closure.string[j]);
		trace(("mark\n"));
		for (k = 0; k < clp->length; ++k)
		{
			cp = &clp->list[k];
			string_list_append_unique(&closure, cp->ingredient);
			/* keep duplicates for error reporting, later */
			cascade_list_append(result, cp->ingredient, &cp->pos);
		}
	}
	trace(("mark\n"));
	string_list_destructor(&closure);
	trace(("}\n"));
}


int
cascade_used()
{
	return used;
}
