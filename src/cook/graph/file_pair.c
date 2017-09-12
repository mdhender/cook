/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate graph file pairs
 *
 * If the relationship between a target and a derived ingredient appears
 * only in a derived cookbook, it is likely that a clean build (solely
 * from primary source files) will fail.  It is recommended that
 * relationships such as this be placed in a primary source cookbook.
 *
 * The functions in this file are used to detect such situations, and
 * issue warnings about them when found.
 */

#include <error_intl.h>
#include <expr/position.h>
#include <graph.h>
#include <graph/file_pair.h>
#include <mem.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


typedef struct target_ty target_ty;
struct target_ty
{
	string_ty	*filename;
	string_list_ty	pos;
};

typedef struct ingredient_ty ingredient_ty;
struct ingredient_ty
{
	string_ty	*filename;
	int		include_cooked;
	symtab_ty	*target;
};


/*
 * NAME
 *	target_new
 *
 * SYNOPSIS
 *	 target_ty *target_new(string_ty *);
 *
 * DESCRIPTION
 *	The target_new function is used to allocate a new instance of a
 *	target.  This is used to remember the location of a relationship
 *	between a target and an ingredient.
 *
 * CAVEAT
 *	Use target_delete when you are done with it.
 */

static target_ty *target_new _((string_ty *));

static target_ty *
target_new(filename)
	string_ty	*filename;
{
	target_ty	*tp;

	trace(("target_new()\n{\n"));
	tp = mem_alloc(sizeof(target_ty));
	tp->filename = str_copy(filename);
	string_list_constructor(&tp->pos);
	trace(("}\n"));
	return tp;
}


/*
 * NAME
 *	target_delete
 *
 * SYNOPSIS
 *	 void target_delete(target_ty *);
 *
 * DESCRIPTION
 *	The target_delete function is used to release the resources held
 *	by a target instance.
 */

static void target_delete _((target_ty *));

static void
target_delete(tp)
	target_ty	*tp;
{
	trace(("target_delete()\n{\n"));
	str_free(tp->filename);
	string_list_destructor(&tp->pos);
	mem_free(tp);
	trace(("}\n"));
}


/*
 * NAME
 *	target_reap
 *
 * SYNOPSIS
 *	 void target_reap(void);
 *
 * DESCRIPTION
 *	The target_reap function is used to delete target values in a
 *	target symbol table.
 */

static void target_reap _((void *));

static void
target_reap(p)
	void		*p;
{
	target_ty	*tp;

	tp = p;
	target_delete(tp);
}


/*
 * NAME
 *	target_append
 *
 * SYNOPSIS
 *	 void target_append(target_ty *, const expr_position_ty *);
 *
 * DESCRIPTION
 *	The target_append function is used to append recipe locations to
 *	a target relationship.  This will be checked later.
 */

static void target_append _((target_ty *, const expr_position_ty *));

static void
target_append(tp, pp)
	target_ty	*tp;
	const expr_position_ty *pp;
{
	trace(("target_append()\n{\n"));
	string_list_append_unique(&tp->pos, pp->pos_name);
	trace(("}\n"));
}


/*
 * NAME
 *	foreign_derived
 *
 * SYNOPSIS
 *	int foreign_derived(graph_file_pair_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The foreign_derived function is used to test if the named file
 *	is a foreign derived file.  This is used in generating warnings
 *	for cascaded ingredients in dependency include files.
 */

static int foreign_derived _((graph_file_pair_ty *, string_ty *));

static int
foreign_derived(gfpp, filename)
	graph_file_pair_ty *gfpp;
	string_ty	*filename;
{
	void		*p;

	if (!gfpp->foreign_derived)
		return 0;
	p = symtab_query(gfpp->foreign_derived, filename);
	return (p != 0);
}


/*
 * NAME
 *	target_check
 *
 * SYNOPSIS
 *	 void target_check(graph_file_pair_ty *, target_ty *, string_ty *,
 *		graph_ty *);
 *
 * DESCRIPTION
 *	The target_check function is used to check that at least one of
 *	the files describing the relationship between this target and
 *	this ingredient is a leaf file.  The ingredient is known to be
 *	derived (non-leaf).
 *
 *	A warning will be issued if this problem is found.  The long
 *	warning will be issued, also, but only once per Cook command.
 */

static void target_check _((graph_file_pair_ty *, target_ty *, string_ty *,
	graph_ty *));

static void
target_check(gfpp, tp, ingredient, gp)
	graph_file_pair_ty *gfpp;
	target_ty	*tp;
	string_ty	*ingredient;
	graph_ty	*gp;
{
	size_t		j;
	static int	the_long_version;
	sub_context_ty	*scp;
	string_ty	*fn;

	/*
	 * If any of the cookbooks which describe this relationship are
	 * leaf files, there is no problem.  Leave quietly.
	 */
	trace(("ingredient_new()\n{\n"));
	assert(tp->pos.nstrings);
	if (!tp->pos.nstrings)
	{
		trace(("}\n"));
		return;
	}
	for (j = 0; j < tp->pos.nstrings; ++j)
	{
		fn = tp->pos.string[j];
		if (!foreign_derived(gfpp, fn) && graph_file_leaf_p(gp, fn))
		{
			trace(("}\n"));
			return;
		}
	}

	/*
	 * Houston, we have a problem.
	 */
	scp = sub_context_new();
	sub_var_set(scp, "RELATionship", "%S: %S;", tp->filename, ingredient);
	sub_var_set(scp, "File_Name", "%S", tp->pos.string[0]);
	error_intl
	(
		scp,
	      i18n("warning: the ``$relationship'' recipe is only in $filename")
	);
	sub_context_delete(scp);

	/*
	 * also issue a longer warning, but only once
	 */
	if (!the_long_version)
	{
		the_long_version = 1;
		error_intl(0, i18n("this means that a clean build will fail"));
	}
	trace(("}\n"));
}


/*
 * NAME
 *	ingredient_new
 *
 * SYNOPSIS
 *	 ingredient_ty *ingredient_new(string_ty *, int);
 *
 * DESCRIPTION
 *	The ingredient_new function is used to allocate a new instance
 *	of an ingredient.  Targets which lead to this ingredient, and
 *	the position of the recipes within in the cookbooks, are
 *	remembered within this data structure.
 *
 * CAVEAT
 *	Release with ingredient_delete when you are done with it.
 */

static ingredient_ty *ingredient_new _((string_ty *, int));

static ingredient_ty *
ingredient_new(filename, include_cooked)
	string_ty	*filename;
	int		include_cooked;
{
	ingredient_ty	*ip;

	trace(("ingredient_new()\n{\n"));
	ip = mem_alloc(sizeof(ingredient_ty));
	ip->filename = str_copy(filename);
	ip->include_cooked = include_cooked;
	ip->target = 0;
	trace(("}\n"));
	return ip;
}


/*
 * NAME
 *	ingredient_delete
 *
 * SYNOPSIS
 *	 void ingredient_delete(ingredient_ty *);
 *
 * DESCRIPTION
 *	The ingredient_delete function is used to release the resources
 *	held by an ingredient instance.
 */

static void ingredient_delete _((ingredient_ty *));

static void
ingredient_delete(ip)
	ingredient_ty	*ip;
{
	trace(("ingredient_delete()\n{\n"));
	str_free(ip->filename);
	if (ip->target)
		symtab_free(ip->target);
	mem_free(ip);
	trace(("}\n"));
}


/*
 * NAME
 *	ingredient_reap
 *
 * SYNOPSIS
 *	 void ingredient_reap(void *);
 *
 * DESCRIPTION
 *	The ingredient_reap function is used to delete ingredient values
 *	in an ingredient symbol table.
 */

static void ingredient_reap _((void *));

static void
ingredient_reap(p)
	void		*p;
{
	ingredient_ty	*ip;

	ip = p;
	ingredient_delete(ip);
}


/*
 * NAME
 *	ingredient_append
 *
 * SYNOPSIS
 *	 void ingredient_append(ingredient_ty *, string_ty *,
 *		const expr_position_ty *);
 *
 * DESCRIPTION
 *	The ingredient_append function is used to remember the location
 *	of a relationship between the given target and the given
 *	ingredient.
 */

static void ingredient_append _((ingredient_ty *, string_ty *,
	const expr_position_ty *));

static void
ingredient_append(ip, target, pp)
	ingredient_ty	*ip;
	string_ty	*target;
	const expr_position_ty *pp;
{
	target_ty	*tp;

	trace(("ingredient_append()\n{\n"));
	if (!ip->target)
	{
		ip->target = symtab_alloc(1);
		ip->target->reap = target_reap;
	}
	tp = symtab_query(ip->target, target);
	if (!tp)
	{
		tp = target_new(target);
		symtab_assign(ip->target, target, tp);
	}
	target_append(tp, pp);
	trace(("}\n"));
}


/*
 * NAME
 *	ingredient_check
 *
 * SYNOPSIS
 *	 void ingredient_check(graph_file_pair_ty *, ingredient_ty *,
 *		string_ty *, graph_ty *);
 *
 * DESCRIPTION
 *	The ingredient_check function is used to check that at least one
 *	of the files describing the relationship between this target and
 *	this ingredient is a leaf file.  The ingredient is known to be
 *	derived (non-leaf).
 */

static void ingredient_check _((graph_file_pair_ty *, ingredient_ty *,
	string_ty *, graph_ty *));

static void
ingredient_check(gfpp, ip, target, gp)
	graph_file_pair_ty *gfpp;
	ingredient_ty	*ip;
	string_ty	*target;
	graph_ty	*gp;
{
	target_ty	*tp;

	/*
	 * Find the target.  It should always be there, but if it is
	 * not, leave quietly.
	 */
	trace(("ingredient_check()\n{\n"));
	assert(gfpp);
	assert(ip);
	assert(ip->target);
	assert(target);
	tp = symtab_query(ip->target, target);
	assert(tp);
	if (!tp)
	{
		trace(("}\n"));
		return;
	}

	/*
	 * Check the target.  This is actually a simple list of files
	 * which describe this relationship.
	 */
	target_check(gfpp, tp, ip->filename, gp);
	trace(("}\n"));
}


/*
 * NAME
 *	graph_file_pair_new
 *
 * SYNOPSIS
 *	 void graph_file_pair_new(void);
 *
 * DESCRIPTION
 *	The graph_file_pair_new function is used to allocate a new
 *	instance of the file pair location checking information.
 *
 * CAVEAT
 *	Release using graph_file_pair_delete when you are done with it.
 */

graph_file_pair_ty *
graph_file_pair_new(slp)
	string_list_ty	*slp;
{
	graph_file_pair_ty *gfpp;
	size_t		j;

	trace(("graph_file_pair_new()\n{\n"));
	gfpp = mem_alloc(sizeof(graph_file_pair_ty));
	gfpp->stp = symtab_alloc(slp ? slp->nstrings : 5);
	gfpp->stp->reap = ingredient_reap;
	gfpp->foreign_derived = 0;
	if (slp)
	{
		for (j = 0; j < slp->nstrings; ++j)
		{
			ingredient_ty	*ip;

			ip = ingredient_new(slp->string[j], 1);
			symtab_assign(gfpp->stp, slp->string[j], ip);
		}
	}
	trace(("}\n"));
	return gfpp;
}


/*
 * NAME
 *	graph_file_pair_delete
 *
 * SYNOPSIS
 *	 void graph_file_pair_delete(graph_file_pair_ty *);
 *
 * DESCRIPTION
 *	The graph_file_pair_delete function is used to relesae the
 *	resources used by a graph_file_pair instance.
 */

void
graph_file_pair_delete(gfpp)
	graph_file_pair_ty *gfpp;
{
	trace(("graph_file_pair_delete()\n{\n"));
	symtab_free(gfpp->stp);
	if (gfpp->foreign_derived)
		symtab_free(gfpp->foreign_derived);
	mem_free(gfpp);
	trace(("}\n"));
}


/*
 * NAME
 *	graph_file_pair_remember
 *
 * SYNOPSIS
 *	void graph_file_pair_remember(graph_file_pair_ty *,
 *		string_ty *target, string_ty *ingredient,
 *		const expr_position_ty *);
 *
 * DESCRIPTION
 *	The graph_file_pair_remember function is used to remember the
 *	position of a relationship between a target and an ingredient.
 *	These will be checked for ``clean'' derivability, later.
 */

void
graph_file_pair_remember(gfpp, target, ingredient, pp)
	graph_file_pair_ty *gfpp;
	string_ty	*target;
	string_ty	*ingredient;
	const expr_position_ty *pp;
{
	ingredient_ty	*ip;

	trace(("graph_file_pair_remember(target = \"%s\", \
ingredient = \"%s\")\n{\n",
		target->str_text, ingredient->str_text));
	ip = symtab_query(gfpp->stp, ingredient);
	if (!ip)
	{
		ip = ingredient_new(ingredient, 0);
		symtab_assign(gfpp->stp, ingredient, ip);
	}
	ingredient_append(ip, target, pp);
	trace(("}\n"));
}


/*
 * NAME
 *	graph_file_pair_remember_lists
 *
 * SYNOPSIS
 *	 void graph_file_pair_remember_lists(graph_file_pair_ty *,
 *		string_list_ty *targets, string_list_ty *ingredients,
 *		const expr_position_ty *);
 *
 * DESCRIPTION
 *	The graph_file_pair_remember_lists function is used to remember
 *	the position of a relationship between a list of targets and a
 *	list of ingredients.  These will be checked for ``clean''
 *	derivability, later.
 */

void
graph_file_pair_remember_tlist(gfpp, target, ingredient, pp)
	graph_file_pair_ty *gfpp;
	string_list_ty	*target;
	string_ty	*ingredient;
	const expr_position_ty *pp;
{
	size_t		j;

	trace(("graph_file_pair_remember_tlist()\n{\n"));
	for (j = 0; j < target->nstrings; ++j)
	{
		graph_file_pair_remember
		(
			gfpp,
			target->string[j],
			ingredient,
			pp
		);
	}
	trace(("}\n"));
}


void
graph_file_pair_remember_lists(gfpp, target, ingredient, pp)
	graph_file_pair_ty *gfpp;
	string_list_ty	*target;
	string_list_ty	*ingredient;
	const expr_position_ty *pp;
{
	size_t		k;

	trace(("graph_file_pair_remember_lists()\n{\n"));
	for (k = 0; k < ingredient->nstrings; ++k)
	{
		graph_file_pair_remember_tlist
		(
			gfpp,
			target,
			ingredient->string[k],
			pp
		);
	}
	trace(("}\n"));
}


/*
 * NAME
 *	graph_file_pair_check
 *
 * SYNOPSIS
 *	 void graph_file_pair_check(void);
 *
 * DESCRIPTION
 *	The graph_file_pair_check function is used to check that the
 *	relationship between the target and the ingredient either (a)
 *	does not need deriving, or (b) is derivable.  Uses information
 *	accumulated earlier using graph_file_pair_remember function.
 */

void
graph_file_pair_check(gfpp, target, ingredient, gp)
	graph_file_pair_ty *gfpp;
	string_ty	*target;
	string_ty	*ingredient;
	graph_ty	*gp;
{
	ingredient_ty	*ip;

	/*
	 * We are looking for non-leaf ingredients.
	 * If this ingredient is a leaf, no further processing is equired.
	 */
	trace(("graph_file_pair_check()\n{\n"));
	if (graph_file_leaf_p(gp, ingredient))
	{
		trace(("}\n"));
		return;
	}

	/*
	 * Find the list of files in which this relationship is
	 * expressed.  It should not happen, but if there is no such
	 * dependency, just return as if there was no problem.
	 */
	ip = symtab_query(gfpp->stp, ingredient);
	assert(ip);
	if (!ip)
	{
		trace(("}\n"));
		return;
	}
	assert(ip->target);
	if (!ip->target)
	{
		trace(("}\n"));
		return;
	}

	/*
	 * check back to the target via the ingredient
	 */
	ingredient_check(gfpp, ip, target, gp);
	trace(("}\n"));
}


/*
 * NAME
 *	graph_file_pair_foreign_derived
 *
 * SYNOPSIS
 *	void graph_file_pair_foreign_derived(graph_file_pair_ty *,
 *		const string_list_ty *);
 *
 * DESCRIPTION
 *	The graph_file_pair_foreign_derived function is used to add
 *	a list of foreigh derived files.  These files are foreign to
 *	the graph being walked.  This is used to generate warnings for
 *	cascaded ingredients.
 */

void
graph_file_pair_foreign_derived(gfpp, slp)
	graph_file_pair_ty *gfpp;
	const string_list_ty *slp;
{
	size_t		j;
	string_ty	*fn;

	if (!gfpp->foreign_derived)
		gfpp->foreign_derived = symtab_alloc(slp->nstrings);
	for (j = 0; j < slp->nstrings; ++j)
	{
		fn = slp->string[j];
		symtab_assign(gfpp->foreign_derived, fn, fn);
	}
}


#ifdef DEBUG


/*
 * NAME
 *	graph_file_pair_check
 *
 * SYNOPSIS
 *	int graph_file_pair_exists(graph_file_pair_ty *gfpp,
 *	    string_ty *target, string_ty *ingredient);
 *
 * DESCRIPTION
 *	The graph_file_pair_exists function is used at debug time to
 *	check that the graph file pairs have been constructed correctly.
 */

int
graph_file_pair_exists(gfpp, target, ingredient)
	graph_file_pair_ty *gfpp;
	string_ty	*target;
	string_ty	*ingredient;
{
	ingredient_ty	*ip;
	int             result;

	/*
	 * We are looking for non-leaf ingredients.
	 * If this ingredient is a leaf, no further processing is equired.
	 */
	trace(("graph_file_pair_exists()\n{\n"));
	if (!gfpp)
	{
		trace(("return 0;\n}\n"));
		return 0;
	}

	/*
	 * Make sure the ingredient is known.
	 */
	ip = symtab_query(gfpp->stp, ingredient);
	if (!ip)
	{
		trace(("return 0;\n}\n"));
		return 0;
	}

	/*
	 * Make sure the ingredient has heard of the target.
	 */
	if (!ip->target)
	{
		trace(("return 0;\n}\n"));
		return 0;
	}
	result = !!symtab_query(ip->target, target);
	trace(("return %d;\n}\n", result));
	return result;
}

#endif
