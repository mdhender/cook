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
 * MANIFEST: functions to print graph construction statistics
 */

#include <ac/stdio.h>

#include <graph.h>
#include <graph/stats.h>
#include <star.h>


static void statistic _((char *, long));

static void
statistic(name, value)
	char		*name;
	long		value;
{
	if (!value)
		return;
	fprintf(stderr, "%8ld %s\n", value, name);
}


void
graph_print_statistics(gp)
	graph_ty	*gp;
{
	star_eoln();
	statistic("backtrack_bad_path",
		gp->statistic.backtrack_bad_path);
	statistic("backtrack_by_ingredient",
		gp->statistic.backtrack_by_ingredient);
	statistic("backtrack_cache",
		gp->statistic.backtrack_cache);
	statistic("error_by_ingredient",
		gp->statistic.error_by_ingredient);
	statistic("error_in_expr",
		gp->statistic.error_in_expr);
	statistic("explicit_applicable",
		gp->statistic.explicit_applicable);
	statistic("explicit_ingredients_applicable",
		gp->statistic.explicit_ingredients_applicable);
	statistic("explicit_ingredients_not_applicable",
		gp->statistic.explicit_ingredients_not_applicable);
	statistic("explicit_not_applicable",
		gp->statistic.explicit_not_applicable);
	statistic("implicit_applicable",
		gp->statistic.implicit_applicable);
	statistic("implicit_ingredients_applicable",
		gp->statistic.implicit_ingredients_applicable);
	statistic("implicit_ingredients_not_applicable",
		gp->statistic.implicit_ingredients_not_applicable);
	statistic("implicit_not_applicable",
		gp->statistic.implicit_not_applicable);
	statistic("infinite_loop",
		gp->statistic.infinite_loop);
	statistic("inhibit_self_recursion",
		gp->statistic.inhibit_self_recursion);
	statistic("leaf_error",
		gp->statistic.leaf_error);
	statistic("leaf_exists",
		gp->statistic.leaf_exists);
	statistic("leaf_backtrack",
		gp->statistic.leaf_backtrack);
	statistic("phony",
		gp->statistic.phony);
	statistic("precondition_rejection",
		gp->statistic.precondition_rejection);
	statistic("success",
		gp->statistic.success);
	statistic("success_reuse",
		gp->statistic.success_reuse);
}
