/*
 *	cook - file construction tool
 *	Copyright (C) 1997-1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to build dependency graphs
 */

#include <cascade.h>
#include <cook.h>
#include <desist.h>
#include <error.h>
#include <error_intl.h>
#include <expr.h>
#include <fingerprint/sync.h>
#include <graph.h>
#include <graph/build.h>
#include <graph/file.h>
#include <graph/file_list.h>
#include <graph/file_pair.h>
#include <graph/leaf.h>
#include <graph/recipe.h>
#include <graph/recipe_list.h>
#include <id.h>
#include <id/variable.h>
#include <match/new_by_recip.h>
#include <match/wl.h>
#include <opcode/context.h>
#include <opcode/list.h>
#include <option.h>
#include <os_interface.h>
#include <recipe.h>
#include <recipe/list.h>
#include <star.h>
#include <str_list.h>
#include <strip_dot.h>
#include <symtab.h>
#include <trace.h>


/*
 * This is the result of building a graph.
 *   If the status is success, the graph pointer is also set.
 */
typedef struct graph_build_result_ty graph_build_result_ty;
struct graph_build_result_ty
{
	graph_build_status_ty status;
	graph_file_ty	*gfp;
};


/*
 * NAME
 *	graph_build_status_name
 *
 * SYNOPSIS
 *	char *graph_build_status_name(graph_build_status_ty n);
 *
 * DESCRIPTION
 *	The graph_build_status_name function is used to turn a graph_
 *	build_status value into an equivalent string.  This is easier
 *	for humans to grok.
 *
 * RETURNS
 *	char *; pointer to string.  Do not free.
 */

#ifdef DEBUG

static char *graph_build_status_name _((graph_build_status_ty));

static char *
graph_build_status_name(n)
	graph_build_status_ty n;
{
	switch (n)
	{
	case graph_build_status_success:
		return "success";

	case graph_build_status_error:
		return "error";

	case graph_build_status_backtrack:
		return "backtrack";
	}
	return "unknown";
}

#endif


/* forward... */
static void graph_build_file _((graph_ty *, string_ty *,
	graph_build_preference_ty, graph_build_result_ty *, int));


/*
 * NAME
 *	graph_check_ingredients - verify that ingredients are cookable
 *
 * SYNOPSIS
 *	graph_build_status_ty graph_check_ingredients( graph_ty *gp,
 *		recipe_ty *rp, graph_build_preference_ty preference,
 *		string_list_ty *target, graph_file_list_nrc_ty *need,
 *		string_list_ty *extras);
 *
 * DESCRIPTION
 *	The graph_check_ingredients function is used to verify that the
 *	ingredients of a recipe are cookable.  It also evaluates the
 *	precondition to make sure the recipe may be applied.
 *
 * RETURNS
 *	The graph_check_ingredients function returns one of the graph
 *	build status values.
 *
 * CAVEAT
 *	The match pattern is assumed to have been pushed already.  The
 *	recipe flags apply to expression evaluations for ingredients,
 *	too.
 */

static graph_build_status_ty graph_check_ingredients _((graph_ty *,
	recipe_ty *, graph_build_preference_ty, string_list_ty *,
	graph_file_list_nrc_ty *, graph_file_list_nrc_ty *, int,
	match_ty *));

static graph_build_status_ty
graph_check_ingredients(gp, rp, preference, target, need_gfl,
		common_ingredients_gfl, implicit_allowed, mp)
	graph_ty	*gp;
	recipe_ty	*rp;
	graph_build_preference_ty preference;
	string_list_ty	*target;
	graph_file_list_nrc_ty	*need_gfl;
	graph_file_list_nrc_ty	*common_ingredients_gfl;
	int		implicit_allowed;
	match_ty	*mp;
{
	graph_build_status_ty status;
	size_t		j;
	size_t		k;
	int		recipe_is_explicit;
	string_list_ty	need;
	string_list_ty	*wlp1;
	string_list_ty	*wlp2;
	string_ty	*target1;
	sub_context_ty	*scp;
	cascade_list_ty	cascade;
	int		cascade_enabled;
	opcode_context_ty *ocp;

	/*
	 * be optimistic, assume success
	 */
	trace(("graph_check_ingredients(gp = %8.8lX, rp = %8.8lX, \
preference = %d)\n{\n"/*}*/,
		(long)gp, (long)rp, (int)preference));
	ocp = 0;
	cascade_list_constructor(&cascade);
	if (rp->inhibit)
	{
		trace(("return backtrack;\n"));
		trace((/*{*/"}\n"));
		return graph_build_status_backtrack;
	}
	status = graph_build_status_success;

	/*
	 * Initialize the list of ingredients (empty unless earlier
	 * ingredients (body-less) recipes were present).
	 */
	string_list_constructor(&need);
	graph_file_list_nrc_constructor(need_gfl);

	/*
	 * Construct the actual target.
	 */
	string_list_constructor(target);
	if (mp)
	{
		if
		(
			match_wl_reconstruct_lhs
			(
				mp,
				target,
				rp->target,
				&rp->pos
			)
		<
			0
		)
		{
			trace(("return error;\n"));
			trace((/*{*/"}\n"));
			return graph_build_status_error;
		}
		recipe_is_explicit = 0;
	}
	else
	{
		string_list_copy_constructor(target, rp->target);
		recipe_is_explicit = 1;
	}

	/*
	 * remove "./" for the start of file names,
	 * so that more recipes apply
	 */
	strip_dot_list(target);

	/*
	 * define the [targets] variable.
	 */
	assert(target->nstrings);
	ocp = opcode_context_new(0, mp);
	opcode_context_id_assign(ocp, id_targets, id_variable_new(target), -1);

	/*
	 * Define the [target] variable to be the first target in the list.
	 */
	target1 = target->string[0];
	wlp2 = 0;
	wlp1 = string_list_new();
	string_list_append(wlp1, target1);
	opcode_context_id_assign(ocp, id_target, id_variable_new(wlp1), -1);
	string_list_delete(wlp1);
	wlp1 = 0;

	/*
	 * Flags apply to the precondition and to the ingredients evaluation.
	 * That is why the grammar puts them first.
	 */
	recipe_flags_set(rp);
	cascade_enabled = option_test(OPTION_CASCADE);

	/*
	 * See if implicit is allowed.  The cookbook or the recipe may
	 * want to over-ride the default action.
	 */
	option_set
	(
		OPTION_IMPLICIT_ALLOWED,
		OPTION_LEVEL_DEFAULT,
		implicit_allowed
	);
	implicit_allowed = option_test(OPTION_IMPLICIT_ALLOWED);
	option_undo(OPTION_IMPLICIT_ALLOWED, OPTION_LEVEL_DEFAULT);

	/*
	 * If there is a precondition and if the gatefirst option is set
	 * then check the precondition before evaluating the ingredients.
	 * If the precondition fails then skip out of this recipe
	 * right now...
	 */
	trace(("check gatefirst precondition\n"));
	if (rp->precondition && option_test(OPTION_GATEFIRST))
	{
		int		flag;

		flag = opcode_context_run_bool(ocp, rp->precondition);

		switch (flag)
		{
		case -1:
			gp->statistic.error_in_expr++;
                        status = graph_build_status_error;
                        goto done;

		case 0:
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target1);
				error_with_position
				(
					&rp->pos,
					scp,
			     i18n("\"$filename\" precondition rejects (reason)")
				);
				sub_context_delete(scp);
			}
			gp->statistic.precondition_rejection++;
			status = graph_build_status_backtrack;
			trace(("failed\n"));
			goto done;
		}
		trace(("passed\n"));
	}

	/*
	 * wander through the ingredients, seeing if we know how to cook them
	 *	the first set is used to determine if to use this recipe
	 *	the second set must be cookable
	 */
	trace(("check first ingredients\n"));
	wlp1 = opcode_context_run(ocp, rp->need1);
	if (!wlp1)
	{
		gci_error:
		status = graph_build_status_error;
		goto done;
	}

	/*
	 * remove "./" for the start of file names,
	 * so that more recipes apply
	 */
	strip_dot_list(wlp1);

	for (k = 0; k < wlp1->nstrings; ++k)
	{
		string_ty	*target2;
		graph_build_result_ty result2;
		edge_type_ty	type2;

		edge_type_extract(wlp1->string[k], &target2, &type2);
		if (!os_legal_path(target2))
		{
			string_list_delete(wlp1);
			str_free(target2);
backtrack:
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target1);
				error_with_position
				(
					&rp->pos,
					scp,
	       i18n("recipe does not apply \"$filename\" backtracking (reason)")
				);
				sub_context_delete(scp);
			}
			status = graph_build_status_backtrack;
			gp->statistic.backtrack_bad_path++;
			goto done;
		}
		if (option_test(OPTION_REASON))
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name1", "%S", target1);
			sub_var_set(scp, "File_Name2", "%S", target2);
			if (preference == graph_build_preference_error)
			{
				error_with_position
				(
					&rp->pos,
					scp,
			 i18n("\"$filename1\" requires \"$filename2\" (reason)")
				);
			}
			else
			{
				error_with_position
				(
					&rp->pos,
					scp,
		      i18n("\"$filename1\" may require \"$filename2\" (reason)")
				);
			}
			sub_context_delete(scp);
		}
		if (mp && !option_test(OPTION_RECURSE))
		{
			match_ty	*mp2;
			int		inhibit;

			/*
			 * if any of the ingredients of an
			 * implicit recipe match the targets of
			 * that same recipe, inhibit the
			 * recursive application the recipe.
			 */
			trace(("mark\n"));
			mp2 = match_clone(mp);
			inhibit =
				match_wl_attempt
				(
					mp2,
					rp->target,
					target2,
					&rp->pos
				);
			match_delete(mp2);
			if (inhibit < 0)
			{
				str_free(target2);
				goto gci_error;
			}
			if (inhibit)
			{
				rp->inhibit = 1;
				gp->statistic.inhibit_self_recursion++;
			}
		}
		option_undo_level(OPTION_LEVEL_RECIPE);

		graph_build_file
		(
			gp,
			target2,
			preference,
			&result2,
			implicit_allowed
		);

		recipe_flags_set(rp);
		switch (result2.status)
		{
		case graph_build_status_backtrack:
			string_list_delete(wlp1);
			gp->statistic.backtrack_by_ingredient++;
			goto backtrack;

		case graph_build_status_error:
			string_list_delete(wlp1);
			scp = sub_context_new();
			sub_var_set(scp, "File_Name1", "%S", target1);
			sub_var_set(scp, "File_Name2", "%S", target2);
			error_with_position
			(
				&rp->pos,
				scp,
	i18n("\"$filename1\" not derived due to errors deriving \"$filename2\"")
			);
			sub_context_delete(scp);
			gp->statistic.error_by_ingredient++;
			str_free(target2);
			goto gci_error;

		case graph_build_status_success:
			string_list_append_unique(&need, target2);
			graph_file_list_nrc_append
			(
				need_gfl,
				result2.gfp,
				type2
			);
			break;
		}
		str_free(target2);
	}

	trace(("check second ingredients\n"));
	wlp2 = opcode_context_run(ocp, rp->need2);
	if (!wlp2)
	{
		string_list_delete(wlp1);
		goto gci_error;
	}
	trace(("mark\n"));

	/*
	 * remove "./" for the start of file names,
	 * so that more recipes apply
	 */
	strip_dot_list(wlp2);

	for (k = 0; k < wlp2->nstrings; ++k)
	{
		string_ty	*target2;
		graph_build_result_ty result2;
		edge_type_ty	type2;

		edge_type_extract(wlp2->string[k], &target2, &type2);
		if (!os_legal_path(target2))
		{
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			gp->statistic.backtrack_bad_path++;
			goto backtrack;
		}
		if (option_test(OPTION_REASON))
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name1", "%S", target1);
			sub_var_set(scp, "File_Name2", "%S", target2);
			error_with_position
			(
				&rp->pos,
				scp,
		 i18n("\"$filename1\" requires \"$filename2\" (reason)")
			);
			sub_context_delete(scp);
		}
		if (mp && !option_test(OPTION_RECURSE))
		{
			match_ty	*mp2;
			int		inhibit;

			/*
			 * if any of the ingredients of an
			 * implicit recipe match the targets of
			 * that same recipe, inhibit the
			 * recursive application the recipe.
			 */
			trace(("mark\n"));
			mp2 = match_clone(mp);
			inhibit =
				match_wl_attempt
				(
					mp2,
					rp->target,
					target2,
					&rp->pos
				);
			match_delete(mp2);
			if (inhibit < 0)
			{
				str_free(target2);
				goto gci_error;
			}
			if (inhibit)
			{
				rp->inhibit = 1;
				gp->statistic.inhibit_self_recursion++;
			}
		}
		option_undo_level(OPTION_LEVEL_RECIPE);

		graph_build_file
		(
			gp,
			target2,
			graph_build_preference_error,
			&result2,
			implicit_allowed
		);

		recipe_flags_set(rp);
		switch (result2.status)
		{
		case graph_build_status_backtrack:
			/*
			 * highly unlikely, especially since we
			 * said we preferred errors to
			 * backtracking.
			 */
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			gp->statistic.backtrack_by_ingredient++;
			goto backtrack;

		case graph_build_status_error:
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			scp = sub_context_new();
			sub_var_set(scp, "File_Name1", "%S", target1);
			sub_var_set(scp, "File_Name2", "%S", target2);
			error_with_position
			(
				&rp->pos,
				scp,
	i18n("\"$filename1\" not derived due to errors deriving \"$filename2\"")
			);
			sub_context_delete(scp);
			gp->statistic.error_by_ingredient++;
			str_free(target2);
			goto gci_error;

		case graph_build_status_success:
			string_list_append_unique(&need, target2);
			graph_file_list_nrc_append
			(
				need_gfl,
				result2.gfp,
				type2
			);
			break;
		}
		str_free(target2);
	}

	/*
	 * Now add the common ingredients.  This could have been added
	 * at the start, but users don't expect the common ingredients
	 * there.  (They seem to read the recipe left-to-right, and
	 * expect the common ingredients on the end as ``extra''.)
	 */
	trace(("mark\n"));
	if (common_ingredients_gfl)
	{
		for (j = 0; j < common_ingredients_gfl->nfiles; ++j)
		{
			string_ty       *fn;

			fn = common_ingredients_gfl->item[j].file->filename;
			string_list_append_unique(&need, fn);
			string_list_append_unique(wlp2, fn);
		}
		graph_file_list_nrc_append_list
		(
			need_gfl,
			common_ingredients_gfl
		);
	}

	/*
	 * Find the cascaded ingredients, and add them.
	 */
	trace(("mark\n"));
	if (cascade_enabled)
		cascade_find(&need, &cascade);
	for (k = 0; k < cascade.length; ++k)
	{
		cascade_ty	*cp;
		string_ty	*target2;
		graph_build_result_ty result2;
		edge_type_ty	type2;

		cp = &cascade.list[k];
		edge_type_extract(cp->ingredient, &target2, &type2);
#if 0
		if (string_list_member(&need, target2))
		{
			/* But the edge type could be more strict! */
			continue;
		}
#endif
		if (!os_legal_path(target2))
		{
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			gp->statistic.backtrack_bad_path++;
			goto backtrack;
		}
		if (option_test(OPTION_REASON))
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name1", "%S", target1);
			sub_var_set(scp, "File_Name2", "%S", target2);
			error_with_position
			(
				&cp->pos,
				scp,
		 	 i18n("\"$filename1\" requires \"$filename2\" (reason)")
			);
			sub_context_delete(scp);
		}
		if (mp && !option_test(OPTION_RECURSE))
		{
			match_ty	*mp2;
			int		inhibit;

			/*
			 * if any of the ingredients of an
			 * implicit recipe match the targets of
			 * that same recipe, inhibit the
			 * recursive application the recipe.
			 */
			mp2 = match_clone(mp);
			inhibit =
				match_wl_attempt
				(
					mp2,
					rp->target,
					target2,
					&rp->pos
				);
			match_delete(mp2);
			if (inhibit < 0)
				goto gci_error;
			if (inhibit)
			{
				rp->inhibit = 1;
				gp->statistic.inhibit_self_recursion++;
			}
		}
		option_undo_level(OPTION_LEVEL_RECIPE);

		graph_build_file
		(
			gp,
			target2,
			graph_build_preference_error,
			&result2,
			implicit_allowed
		);

		recipe_flags_set(rp);
		switch (result2.status)
		{
		case graph_build_status_backtrack:
			/*
			 * highly unlikely, especially since we
			 * said we preferred errors to
			 * backtracking.
			 */
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			gp->statistic.backtrack_by_ingredient++;
			goto backtrack;

		case graph_build_status_error:
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			scp = sub_context_new();
			sub_var_set(scp, "File_Name1", "%S", target1);
			sub_var_set(scp, "File_Name2", "%S", target2);
			error_with_position
			(
				&rp->pos,
				scp,
	i18n("\"$filename1\" not derived due to errors deriving \"$filename2\"")
			);
			sub_context_delete(scp);
			gp->statistic.error_by_ingredient++;
			goto gci_error;

		case graph_build_status_success:
			string_list_append_unique(&need, target2);
			string_list_append_unique(wlp2, target2);
			graph_file_list_nrc_append
			(
				need_gfl,
				result2.gfp,
				type2
			);
			break;
		}
	}

	/*
	 * If there is a precondition and if the gatefirst option is
	 * not set then check the precondition, now that we have evaluated
	 * the ingredients.  If the precondition fails then skip out of this
	 * recipe right now...
	 */
	trace(("check precondition\n"));
	if (rp->precondition && !option_test(OPTION_GATEFIRST))
	{
		int		flag;

		/*
		 * We now have all the ingredients, define the [need]
		 * variable.  (Note: can't define [younger] until we
		 * walk the graph.)
		 */
		opcode_context_id_assign
		(
			ocp,
			id_need,
			id_variable_new(&need),
			-1
		);

		/*
		 * evaluate the predicate
		 */
		flag = opcode_context_run_bool(ocp, rp->precondition);

		/*
		 * act on the results of the predicate
		 */
		switch (flag)
		{
		case -1:
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			gp->statistic.error_in_expr++;
			goto gci_error;

		case 0:
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target1);
				error_with_position
				(
					&rp->pos,
					scp,
			     i18n("\"$filename\" precondition rejects (reason)")
				);
				sub_context_delete(scp);
			}
			string_list_delete(wlp1);
			string_list_delete(wlp2);
			gp->statistic.precondition_rejection++;
			goto backtrack;
		}
	}

	/*
	 * Remember the file pairs without the common ingredients; they
	 * have already been remembered.  We want this recipe only.
	 */
	if (gp->file_pair)
	{
		for (k = 0; k < wlp1->nstrings; ++k)
		{
		    string_ty       *ingredient1;
		    edge_type_ty    type1;

		    edge_type_extract(wlp1->string[k], &ingredient1, &type1);
		    graph_file_pair_remember_tlist
		    (
			    gp->file_pair,
			    target,
			    ingredient1,
			    &rp->pos
		    );
		    str_free(ingredient1);
		}
		for (k = 0; k < wlp2->nstrings; ++k)
		{
		    string_ty       *ingredient2;
		    edge_type_ty    type2;

		    edge_type_extract(wlp2->string[k], &ingredient2, &type2);
		    graph_file_pair_remember_tlist
		    (
			    gp->file_pair,
			    target,
			    ingredient2,
			    &rp->pos
		    );
		    str_free(ingredient2);
		}

		/* remember cascaded ingredients */
		for (j = 0; j < cascade.length; ++j)
		{
			cascade_ty	*cp;

			cp = &cascade.list[j];
			graph_file_pair_remember_tlist
			(
				gp->file_pair,
				target,
				cp->ingredient,
				&cp->pos
			);
		}
	}
	string_list_delete(wlp1);
	string_list_delete(wlp2);

	/*
	 * clean up and go home
	 */
done:
	cascade_list_destructor(&cascade);
	rp->inhibit = 0;
	string_list_destructor(&need);
	option_undo_level(OPTION_LEVEL_RECIPE);
	if (status != graph_build_status_success)
	{
		string_list_destructor(target);
		graph_file_list_nrc_destructor(need_gfl);
	}
	trace(("target->nstrings = %ld;\n", (long)target->nstrings));
	trace(("need_gfl->nfiles = %ld;\n", (long)need_gfl->nfiles));
	star_as_specified('*');
	if (ocp)
		opcode_context_delete(ocp);
	trace(("return %s;\n", graph_build_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
}


/*
 * NAME
 *	graph_check_recipe
 *
 * SYNOPSIS
 *	graph_build_status_ty graph_check_recipe(graph_ty *, recipe_ty *,
 *		graph_build_preference_ty, graph_file_list_nrc_ty *);
 *
 * DESCRIPTION
 *	The graph_check_recipe function is used to check that a recipe is
 *	applicable, that its ingredients are buildable, and that its
 *	precondition applies.  If so, it is added to its targets'
 *	inputs, and its ingredients' outputs.
 *
 * RETURNS
 *	The graph_check_recipe function returns one of the graph
 *	build status values.
 *
 * CAVEAT
 *	The match pattern is assumed to have been pushed already.  The
 *	recipe flags apply to expression evaluations for ingredients,
 *	too.
 */

static graph_build_status_ty graph_check_recipe _((graph_ty *, recipe_ty *,
	graph_build_preference_ty, graph_file_list_nrc_ty *, int,
	match_ty *));

static graph_build_status_ty
graph_check_recipe(gp, rp, preference, common_ingredients_gfl, implicit_allowed,
		mp)
	graph_ty	*gp;
	recipe_ty	*rp;
	graph_build_preference_ty preference;
	graph_file_list_nrc_ty	*common_ingredients_gfl;
	int		implicit_allowed;
	match_ty	*mp;
{
	graph_build_status_ty status;
	graph_file_list_nrc_ty need_gfl;
	graph_recipe_ty	*grp;
	string_list_ty	target;
	size_t		k;
	opcode_context_ty *ocp;

	trace(("graph_check_recipe(gp = %8.8lX, rp = %8.8lX, \
preference = %d)\n{\n",
		(long)gp, (long)rp, (int)preference));
	ocp = 0;
	status =
		graph_check_ingredients
		(
			gp,
			rp,
			preference,
			&target,
			&need_gfl,
			common_ingredients_gfl,
			implicit_allowed,
			mp
		);

	switch (status)
	{
	case graph_build_status_error:
	case graph_build_status_backtrack:
		/*
		 * no need to run destructors, because
		 * graph_check_ingredients already cleaned up
		 *
		string_list_destructor(&target);
		graph_file_list_nrc_destructor(&need_gfl);
		 */
		if (mp)
			match_delete(mp);
		goto done;

	case graph_build_status_success:
		/*
		 * Don't use default.  This lets GCC warn us if we
		 * forget a new enum value.
		 */
		break;
	}

	/*
	 * remember this one
	 */
	trace(("remember this one\n"));
	grp = graph_recipe_new(rp);
	grp->mp = mp;
	graph_recipe_list_append(gp->already_recipe, grp);
	grp->reference_count--; /* that append bumped it by one */

	/*
	 * Calculate the single_thread values.
	 */
	ocp = opcode_context_new(0, mp);
	if (rp->single_thread)
	{
		grp->single_thread = opcode_context_run(ocp, rp->single_thread);
		if (!grp->single_thread)
			status = graph_build_status_error;
		else if (grp->single_thread->nstrings == 0)
		{
			string_list_delete(grp->single_thread);
			grp->single_thread = 0;
		}
	}

	/*
	 * Calculate the host_binding values.
	 */
	if (rp->host_binding)
	{
		grp->host_binding = opcode_context_run(ocp, rp->host_binding);
		if (!grp->host_binding)
			status = graph_build_status_error;
		else if (grp->host_binding->nstrings == 0)
		{
			string_list_delete(grp->host_binding);
			grp->host_binding = 0;
		}
	}

	/*
	 * Add all of the targets to the graph recipe node's outputs,
	 * and the recipe node to all of the file nodes' inputs.
	 */
	trace(("double link list thru target\n"));
	for (k = 0; k < target.nstrings; ++k)
	{
		string_ty	*fn;
		graph_file_ty	*gfp;

		fn = target.string[k];
		trace(("%s\n", fn->str_text));
		gfp = symtab_query(gp->already, fn);
		if (!gfp)
		{
			gfp = graph_file_new(fn);
			symtab_assign(gp->already, fn, gfp);
		}
		graph_file_list_nrc_append(grp->output, gfp, edge_type_default);
		graph_recipe_list_nrc_append(gfp->input, grp);
	}
	string_list_destructor(&target);

	/*
	 * Add all of the ingredients to the graph recipe node's inputs,
	 * and the recipe node to all of the file nodes' outputs.
	 */
	trace(("double link list thru need\n"));
	for (k = 0; k < need_gfl.nfiles; ++k)
	{
		graph_file_and_type_ty *gftp;

		gftp = need_gfl.item + k;
		trace(("%s\n", gftp->file->filename->str_text));
		graph_file_list_nrc_append
		(
			grp->input,
			gftp->file,
			gftp->edge_type
		);
		graph_recipe_list_nrc_append(gftp->file->output, grp);
	}
	graph_file_list_nrc_destructor(&need_gfl);

#ifdef DEBUG
	/*
	 * Make sure all of the targets and ingredients appear as file pairs.
	 *
	 * Q: Why not generate the pairs here?
	 * A: Because by this point in the code, we have lost all
	 * association with the location information necessary to generate
	 * useful error messages.  And error messages is what the file
	 * pair information is all about.
	 */
	if (gp->file_pair)
	{
	    size_t          j;
	    int             ok;

	    ok = 1;
	    for (j = 0; j < grp->output->nfiles; ++j)
	    {
		string_ty       *target1;

		target1 = grp->output->item[j].file->filename;
		for (k = 0; k < grp->input->nfiles; ++k)
		{
		    string_ty       *ingredient;

		    ingredient = grp->input->item[k].file->filename;
		    if
		    (
			!graph_file_pair_exists
			(
			    gp->file_pair,
			    target1,
			    ingredient
			)
		    )
		    {
		        error_raw
			(
			    "%s: %d: file pair ''%S: %S'' missing",
			    __FILE__,
			    __LINE__,
			    target1,
			    ingredient
			);
			ok = 0;
		    }
		}
	    }
	    assert(ok);
	}
#endif

	/*
	 * return result
	 */
	done:
	if (ocp)
		opcode_context_delete(ocp);
	trace(("return %s;\n", graph_build_status_name(status)));
	trace(("}\n"));
	return status;
}


/*
 * NAME
 *	say_dont_know_how
 *
 * SYNOPSIS
 *	void say_dont_know_how(graph_ty *gp, string_ty *target);
 *
 * DESCRIPTION
 *	The say_dont_know_how function is used to say that cook doesn't
 *	know how to build a given target.  The message may be improved
 *	if there is a try-list available.
 */

static void say_dont_know_how _((graph_ty *, string_ty *));

static void
say_dont_know_how(gp, target)
	graph_ty	*gp;
	string_ty	*target;
{
	sub_context_ty	*scp;

	if (gp->try_list && gp->try_list->nstrings)
	{
		string_ty	*why;

		why = wl2str(gp->try_list, 0, gp->try_list->nstrings, ", ");
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", target);
		sub_var_set(scp, "File_Name_List", "%S", why);
		error_intl
		(
			scp,
		      i18n("$filename: don't know how, attempted $filenamelist")
		);
		sub_context_delete(scp);
		str_free(why);

		/*
		 * Free the try_list, so that the
		 * next one does not include spurious
		 * targets from this one.
		 */
		string_list_delete(gp->try_list);
		gp->try_list = 0;
	}
	else
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", target);
		error_intl(scp, i18n("$filename: don't know how"));
		sub_context_delete(scp);
	}
}


/*
 * NAME
 *	graph_build_file
 *
 * SYNOPSIS
 *	void graph_build_file(graph_ty *, string_ty *,
 *		graph_build_preference_ty, graph_build_result_ty *);
 *
 * DESCRIPTION
 *	The graph_build_file function is used to find and remember the
 *	actions required to build the given target.
 *
 * RETURNS
 *	The graph_build_file function returns one of the graph
 *	build status values.
 */

static void
graph_build_file(gp, target, preference, result, implicit_allowed)
	graph_ty	*gp;
	string_ty	*target;
	graph_build_preference_ty preference;
	graph_build_result_ty *result;
	int		implicit_allowed;
{
	int		used_ingredients_recipe;
	int		used_explicit_recipe;
	int		used_implicit_recipe;
	graph_file_ty	*gfp;
	size_t		j;
	const recipe_list_ty *explicit_recipes;
	sub_context_ty	*scp;
	leaf_ness_ty	leaf;
	opcode_context_ty *ocp;

	/*
	 * The gp->try_list is a list of files that were not used, and
	 * subsequently backtracked.  This list is printed out in the
	 * error message when cook doesn't know how.  For the most
	 * useful messages, this list must be pruned on success, hence
	 * the try_list_saved variable.
	 */
	string_list_ty	*try_list_saved;

	/*
	 * Recipes with no body are ingredients recipes.  They
	 * accumulate, and are added to the list of ingredients for all
	 * other relevant non-ingredients recipes.
	 */
	graph_file_list_nrc_ty common_ingredients_gfl;

	trace(("graph_build_file(gp = %8.8lX, target = %08lX, preference = %d, \
result = %8.8lX, imp=%d)\n{\n",
		(long)gp, (long)target, (int)preference, (long)result,
		implicit_allowed));
	result->status = graph_build_status_error;
	result->gfp = 0;
	gfp = 0;
	trace_string(target->str_text);
	try_list_saved = gp->try_list;
	gp->try_list = 0;
	graph_file_list_nrc_constructor(&common_ingredients_gfl);

	/*
	 * remove "./" for the start of file names,
	 * so that more recipes apply
	 */
	target = strip_dot(target);
	used_ingredients_recipe = 0;
	used_explicit_recipe = 0;
	used_implicit_recipe = 0;

	/*
	 * stop cleanly if we are interrupted
	 */
	if (desist_requested())
	{
		str_free(target);
		result->status = graph_build_status_error;
		result->gfp = 0;
		trace((/*{*/"}\n"));
		return;
	}
	fp_sync();

	/*
	 * Check to see if this one has been cooked already.
	 * It may have failed, too.  If it is currently being cooked
	 * a value of COOK_BACKTRACK will be returned, this is to
	 * trap recursive recipes.
	 */
	gfp = symtab_query(gp->already, target);
	if (gfp)
	{
		if (gfp->previous_backtrack)
		{
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target);
				error_intl
				(
					scp,
		     i18n("\"$filename\" does not exist, backtracking (reason)")
				);
				sub_context_delete(scp);
			}
			trace(("previous backtrack\n"));
			result->status = graph_build_status_backtrack;
			result->gfp = 0;
			gp->statistic.backtrack_cache++;
		}
		else if (gfp->previous_error)
		{
			trace(("previous error\n"));
			result->status = graph_build_status_error;
			result->gfp = 0;
			gp->statistic.error_cache++;
		}
		else if (gfp->pending > 0)
		{
			trace(("infinite loop\n"));
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", target);
			error_intl
			(
				scp,
			      i18n("$filename: subject of recipe infinite loop")
			);
			sub_context_delete(scp);
			option_set_errors();
			result->status = graph_build_status_error;
			result->gfp = 0;
			gp->statistic.infinite_loop++;
			gfp->previous_error++;
		}
		else
		{
			trace(("previous success\n"));
			result->status = graph_build_status_success;
			result->gfp = gfp;
			gp->statistic.success_reuse++;
		}
		gfp = 0; /* so pending not decrimented */
		goto ret;
	}

	/*
	 * allocate a new graph file node
	 */
	gfp = graph_file_new(target);
	gfp->pending++;
	result->status = graph_build_status_success;
	result->gfp = gfp;

	/*
	 * add it to the "currently being cooked" list
	 */
	symtab_assign(gp->already, target, gfp);

	/*
	 * Do a quick check to see if this is an explicit leaf node.
	 * If so, there is no need to look further.
	 */
	leaf = leaf_query(target, 0);
	switch (leaf)
	{
	case leaf_ness_error:
		goto bomb_with_error;

	case leaf_ness_leaf_exists:
	case leaf_ness_leaf_explicit:
		goto short_cut_leaf;

	case leaf_ness_exterior_explicit:
		goto short_cut_exterior;

	case leaf_ness_interior_exists:
	case leaf_ness_interior_explicit:
	case leaf_ness_indeterminate:
		break;
	}

	/*
	 * The explicit recipes are indexed by name, for faster access.
	 * There could still be more than one relevant recipe, or there
	 * could be none.
	 */
	explicit_recipes = cook_explicit_by_name(target);
	assert(explicit_recipes);

	/*
	 * If the file is named explicitly, implicit recipes are
	 * allowed.  This enables recipes such as
	 *	fred:;
	 * to over-ride the no-implicit-allowed flag.
	 */
	if (explicit_recipes->nrecipes > 0)
		implicit_allowed = 1;

	/*
	 * Look for the named file in the recipes.
	 * If it matches, cook its ingredients.
	 * It is an explicit recipe if there is any action attached to
	 * any recipe targeting the named file.
	 *
	 * Cook scans through the instantiated ingredients recipes.  All
	 * ingredients recipes with the target in their target list are used.
	 *
	 * If the recipe is used, any prerequisite files are recursively
	 * cooked, then if any of the prerequisite files were out of date, then
	 * all other explicit or implicit recipes with the same target will be
	 * deemed to be out of date.
	 */
	trace(("check for explicit ingredients recipes\n"));
	used_ingredients_recipe = 0;
	for (j = 0; j < explicit_recipes->nrecipes; ++j)
	{
		recipe_ty	*rp;
		graph_build_status_ty status2;
		graph_file_list_nrc_ty need2_gfl;
		string_list_ty	target2;

		/*
		 * Don't use this recipe if it has an out-of-date action,
		 * we are looking for ingredients recipes.
		 */
		rp = explicit_recipes->recipe[j];
		assert(string_list_member(rp->target, target));
		if (rp->out_of_date)
			continue;

		status2 =
			graph_check_ingredients
			(
				gp,
				rp,
				preference,
				&target2,
				&need2_gfl,
				&common_ingredients_gfl,
				implicit_allowed,
				(match_ty *)0
			);

		switch (status2)
		{
		case graph_build_status_backtrack:
			gp->statistic.explicit_ingredients_not_applicable++;
			continue;

		case graph_build_status_error:
			trace(("child build error\n"));
			bomb_with_error:
			result->status = graph_build_status_error;
			result->gfp = 0;
			goto ret;

		case graph_build_status_success:
			gp->statistic.explicit_ingredients_applicable++;
			break;
		}
		graph_file_list_nrc_append_list
		(
			&common_ingredients_gfl,
			&need2_gfl
		);
		string_list_destructor(&target2);
		graph_file_list_nrc_destructor(&need2_gfl);
		used_ingredients_recipe = 1;
		/* ingredients recipes are multiple, keep looking */
	}

	/*
	 * Check for implicit ingredients recipes, in addition to the
	 * explicit ingredients recipes.  They are complimentary, not
	 * exclusive - unlike the non-ingredients recipes.
	 */
	trace(("check for implicit ingredients recipes\n"));
	if (implicit_allowed)
	{
		string_ty	*base;

		/*
		 * First try faster focused implicit recipes.
		 */
		base = os_entryname(target);
		for (j = 0; ; j++)
		{
			recipe_ty	*rp;
			graph_build_status_ty status2;
			match_ty	*mp;
			string_list_ty	target2;
			graph_file_list_nrc_ty need2_gfl;
			int		ok;

			rp = cook_implicit_nth_by_name(j, base);
			if (!rp)
				break;
			if (rp->out_of_date)
				continue;
			gp->statistic.pattern_match_query++;

			/*
			 * new matcher per recipe, since each one can
			 * have a different matcher
			 */
			trace(("mark\n"));
			mp = match_new_by_recipe(rp);

			ok = match_wl_attempt(mp, rp->target, target, &rp->pos);
			if (ok < 0)
			{
				match_delete(mp);
				goto bomb_with_error;
			}
			if (!ok)
			{
				match_delete(mp);
				continue;
			}

			status2 =
				graph_check_ingredients
				(
					gp,
					rp,
					preference,
					&target2,
					&need2_gfl,
					&common_ingredients_gfl,
					implicit_allowed,
					mp
				);
			match_delete(mp);
			mp = 0;

			switch (status2)
			{
			case graph_build_status_backtrack:
			    gp->statistic.implicit_ingredients_not_applicable++;
				continue;

			case graph_build_status_error:
				trace(("child build error\n"));
				goto bomb_with_error;

			case graph_build_status_success:
				gp->statistic.implicit_ingredients_applicable++;
				break;
			}
			graph_file_list_nrc_append_list
			(
				&common_ingredients_gfl,
				&need2_gfl
			);
			string_list_destructor(&target2);
			graph_file_list_nrc_destructor(&need2_gfl);
			used_ingredients_recipe = 1;
			/* ingredients recipes are multiple, keep looking */
		}
		str_free(base);

		/*
		 * The slower, more generic, implicit recipes.
		 * There should be fewer of these.
		 */
		for (j = 0; ; j++)
		{
			recipe_ty	*rp;
			graph_build_status_ty status2;
			match_ty	*mp;
			string_list_ty	target2;
			graph_file_list_nrc_ty need2_gfl;
			int		ok;

			rp = cook_implicit_nth(j);
			if (!rp)
				break;
			if (rp->out_of_date)
				continue;
			gp->statistic.pattern_match_query++;

			/*
			 * new matcher per recipe, since each one can
			 * have a different matcher
			 */
			trace(("mark\n"));
			mp = match_new_by_recipe(rp);

			ok = match_wl_attempt(mp, rp->target, target, &rp->pos);
			if (ok < 0)
			{
				match_delete(mp);
				goto bomb_with_error;
			}
			if (!ok)
			{
				match_delete(mp);
				continue;
			}

			status2 =
				graph_check_ingredients
				(
					gp,
					rp,
					preference,
					&target2,
					&need2_gfl,
					&common_ingredients_gfl,
					implicit_allowed,
					mp
				);
			match_delete(mp);
			mp = 0;

			switch (status2)
			{
			case graph_build_status_backtrack:
			    gp->statistic.implicit_ingredients_not_applicable++;
				continue;

			case graph_build_status_error:
				trace(("child build error\n"));
				goto bomb_with_error;

			case graph_build_status_success:
				gp->statistic.implicit_ingredients_applicable++;
				break;
			}
			graph_file_list_nrc_append_list
			(
				&common_ingredients_gfl,
				&need2_gfl
			);
			string_list_destructor(&target2);
			graph_file_list_nrc_destructor(&need2_gfl);
			used_ingredients_recipe = 1;
			/* ingredients recipes are multiple, keep looking */
		}
	}

	/*
	 * Cook then scans through the instantiated explicit recipes.
	 * Keep looping until a single-colon recipe is met.
	 *
	 * If the recipe is used, any prerequisite files are recursively
	 * cooked, then if any prerequisites were out of date (including those
	 * of prerequisite recipes) then the actions bound to this recipe will
	 * be evaluated.
	 */
	trace(("check explicit recipes\n"));
	used_explicit_recipe = 0;
	for (j = 0; j < explicit_recipes->nrecipes; ++j)
	{
		recipe_ty	*rp;
		graph_build_status_ty status2;

		/*
		 * Ignore this recipe if it does not have an out-of-date
		 * action attached, we are only looking at
		 * non-ingredients recipes.
		 */
		rp = explicit_recipes->recipe[j];
		assert(string_list_member(rp->target, target));
		if (!rp->out_of_date)
			continue;

		trace(("mark\n"));
		status2 =
			graph_check_recipe
			(
				gp,
				rp,
				preference,
				&common_ingredients_gfl,
				implicit_allowed,
				(match_ty *)0
			);

		trace(("mark\n"));
		switch (status2)
		{
		case graph_build_status_error:
			trace(("child build error\n"));
			goto bomb_with_error;

		case graph_build_status_backtrack:
			trace(("child build backtrack\n"));
			gp->statistic.explicit_not_applicable++;
			continue;

		case graph_build_status_success:
			trace(("child build success\n"));
			gp->statistic.explicit_applicable++;
			break;
		}

		/*
		 * remember that we used a recipe
		 */
		used_explicit_recipe = 1;

		/*
		 * keep looking if it is multiple
		 */
		if (!rp->multiple)
			break;
	}
	trace(("mark\n"));

	used_implicit_recipe = 0;
	if (!used_explicit_recipe && implicit_allowed)
	{
		string_ty	*base;

		/*
		 * None of the recipes specified an action,
		 * although prerequisites may have been specified.
		 * Check out the implicit recipes, instead.
		 *
		 * The tricky part is when going through several
		 * layers of recipes.  If a cook of a prerequisite comes back
		 * with BACKTRACK, then try something else.
		 *
		 * Only the first applicable implicit recipe is used.
		 */
		trace(("check implicit recipes\n"));

		/*
		 * First try the focused implicit recipes.
		 */
		base = os_entryname(target);
		for (j = 0; ; j++)
		{
			recipe_ty	*rp;
			match_ty	*mp;
			graph_build_status_ty status2;
			size_t		k;
			int		used;

			rp = cook_implicit_nth_by_name(j, base);
			if (!rp)
				break;

			if (!rp->out_of_date)
				continue;
			used = 0;
			for (k = 0; k < rp->target->nstrings; ++k)
			{
				string_ty	*target_pattern;
				int		ok;

				/*
				 * new matcher per recipe, since each one can
				 * have a different matcher
				 */
				trace(("mark\n"));
				mp = match_new_by_recipe(rp);

				target_pattern = rp->target->string[k];
				ok =
					match_attempt
					(
						mp,
						target_pattern,
						target,
						&rp->pos
					);
				if (ok < 0)
				{
					match_delete(mp);
					goto bomb_with_error;
				}
				if (!ok)
				{
					match_delete(mp);
					continue;
				}

				status2 =
					graph_check_recipe
					(
						gp,
						rp,
					       graph_build_preference_backtrack,
						&common_ingredients_gfl,
						implicit_allowed,
						mp
					);
				mp = 0;

				switch (status2)
				{
				case graph_build_status_error:
					trace(("child build error\n"));
					goto bomb_with_error;

				case graph_build_status_backtrack:
					trace(("child build backtrack\n"));
					gp->statistic.implicit_not_applicable++;
					continue;

				case graph_build_status_success:
					trace(("child build success\n"));
					gp->statistic.implicit_applicable++;
					break;
				}

				/*
				 * remember that we used a recipe
				 */
				used_implicit_recipe = 1;

				/*
				 * keep looking if it is multiple
				 */
				used = 1;
				if (!rp->multiple)
					break;
				used = -1;
			}

			/*
			 * keep looking if not used or all multiple
			 */
			if (used > 0)
				goto implicit_done;
		}

		/*
		 * Now try the slower, more generic implicit recipes.
		 * There should be fewer of these.
		 */
		for (j = 0; ; j++)
		{
			recipe_ty	*rp;
			match_ty	*mp;
			graph_build_status_ty status2;
			size_t		k;
			int		used;

			rp = cook_implicit_nth(j);
			if (!rp)
				break;

			if (!rp->out_of_date)
				continue;
			used = 0;
			for (k = 0; k < rp->target->nstrings; ++k)
			{
				string_ty	*target_pattern;
				int		ok;

				/*
				 * new matcher per recipe, since each one can
				 * have a different matcher
				 */
				trace(("mark\n"));
				mp = match_new_by_recipe(rp);

				target_pattern = rp->target->string[k];
				ok =
					match_attempt
					(
						mp,
						target_pattern,
						target,
						&rp->pos
					);
				if (ok < 0)
				{
					match_delete(mp);
					goto bomb_with_error;
				}
				if (!ok)
				{
					match_delete(mp);
					continue;
				}

				status2 =
					graph_check_recipe
					(
						gp,
						rp,
					       graph_build_preference_backtrack,
						&common_ingredients_gfl,
						implicit_allowed,
						mp
					);
				mp = 0;

				switch (status2)
				{
				case graph_build_status_error:
					trace(("child build error\n"));
					goto bomb_with_error;

				case graph_build_status_backtrack:
					trace(("child build backtrack\n"));
					gp->statistic.implicit_not_applicable++;
					continue;

				case graph_build_status_success:
					trace(("child build success\n"));
					gp->statistic.implicit_applicable++;
					break;
				}

				/*
				 * remember that we used a recipe
				 */
				used_implicit_recipe = 1;

				/*
				 * keep looking if it is multiple
				 */
				used = 1;
				if (!rp->multiple)
					break;
				used = -1;
			}

			/*
			 * keep looking if not used or all multiple
			 */
			if (used > 0)
				break;
		}

		implicit_done:
		str_free(base);
	}

	if (!used_explicit_recipe && !used_implicit_recipe)
	{
		/*
		 * None of the implicit recipes worked, either (perhapse
		 * none applied) so we don't know how to make this one.
		 *
		 * If it already exists, it must be up-to-date; but if
		 * it doesn't exist, then we may want to backtrack.
		 */
		trace(("no recipe applied\n"));
		leaf = leaf_query(target, 1);
		switch (leaf)
		{
		case leaf_ness_error:
			/* error message already printed */
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target);
				error_intl
				(
					scp,
					i18n("\"$filename\" error (reason)")
				);
				sub_context_delete(scp);
			}
			result->status = graph_build_status_error;
			result->gfp = 0;
			goto ret;

		case leaf_ness_exterior_explicit:
			short_cut_exterior:
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target);
				error_intl
				(
					scp,
		   i18n("\"$filename\" is explicitly not a graph node (reason)")
				);
				sub_context_delete(scp);
			}
			trace(("backtrack\n"));
			gp->statistic.leaf_backtrack++;
			gfp->previous_backtrack++;
			result->status = graph_build_status_backtrack;
			result->gfp = 0;
			goto ret;

		case leaf_ness_indeterminate:
			/*
			 * Should not happen, treat it as non-existence.
			 * Fall through...
			 */

		case leaf_ness_interior_exists:
		case leaf_ness_interior_explicit:
			if (used_ingredients_recipe)
			{
				recipe_ty	*rp;
				graph_build_status_ty status2;
				string_list_ty	tl;

				/*
				 * This is a ``phony'' target.  While
				 * the file does not exist (and may
				 * never exist) it has appeared on the
				 * left-hand-side of an ingredients
				 * recipe.
				 */
				if (option_test(OPTION_REASON))
				{
					scp = sub_context_new();
					sub_var_set
					(
						scp,
						"File_Name",
						"%S",
						target
					);
					if (leaf == leaf_ness_interior_explicit)
					{
						error_intl
						(
							scp,
		    i18n("\"$filename\" is explicitly not a leaf node (reason)")
						);
					}
					else
					{
						error_intl
						(
							scp,
		  i18n("\"$filename\" does not exist, assume non-leaf (reason)")
						);
					}
					sub_context_delete(scp);
				}
				trace(("phony\n"));
				result->status = graph_build_status_success;
				result->gfp = gfp;
				gp->statistic.phony++;

				/*
				 * build a recipe just for this instance
				 */
				string_list_constructor(&tl);
				string_list_append(&tl, target);
				rp =
					recipe_new
					(
						&tl,	/* targets */
						(opcode_list_ty *)0, /* need1 */
						(opcode_list_ty *)0, /* need2 */
						0,	/* flags */
						0,	/* multiple */
						(opcode_list_ty *)0, /* pred */
						(opcode_list_ty *)0, /* sing */
						(opcode_list_ty *)0, /* host */
						(opcode_list_ty *)0, /* ood */
						(opcode_list_ty *)0, /* utd */
						(expr_position_ty *)0
					);
				string_list_destructor(&tl);
				if (!rp)
				{
					/*
					 * Unlikely.  Impossible,
					 * actually.  This means an
					 * error in the targets of an
					 * implicit recipe.  The error
					 * has already been printed.
					 */
					result->status =
						graph_build_status_error;
					result->gfp = 0;
					goto ret;
				}
				status2 =
					graph_check_recipe
					(
						gp,
						rp,
						preference,
						&common_ingredients_gfl,
						implicit_allowed,
						(match_ty *)0
					);
				assert(status2 == graph_build_status_success);
				assert(rp->reference_count == 2);
				recipe_delete(rp);
			}
			else if (preference == graph_build_preference_error)
			{
				say_dont_know_how(gp, target);
				if (option_test(OPTION_REASON))
				{
					scp = sub_context_new();
					sub_var_set
					(
						scp,
						"File_Name",
						"%S",
						target
					);
					error_intl
					(
						scp,
			    i18n("\"$filename\" does not exist, error (reason)")
					);
					sub_context_delete(scp);
				}
				trace(("dont know how\n"));
				result->status = graph_build_status_error;
				result->gfp = 0;
				gp->statistic.leaf_error++;
			}
			else
			{
				trace(("backtrack\n"));
				gp->statistic.leaf_backtrack++;
				if (option_test(OPTION_REASON))
				{
					scp = sub_context_new();
					sub_var_set
					(
						scp,
						"File_Name",
						"%S",
						target
					);
					error_intl
					(
						scp,
		     i18n("\"$filename\" does not exist, backtracking (reason)")
					);
					sub_context_delete(scp);
				}
				gfp->previous_backtrack++;
				result->status = graph_build_status_backtrack;
				result->gfp = 0;
			}
			goto ret;

		case leaf_ness_leaf_explicit:
		case leaf_ness_leaf_exists:
			short_cut_leaf:
			if (option_test(OPTION_REASON))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target);
				if (leaf == leaf_ness_leaf_exists)
				{
					error_intl
					(
						scp,
			      i18n("\"$filename\" exists, assume leaf (reason)")
					);
				}
				else
				{
					error_intl
					(
						scp,
			i18n("\"$filename\" is explicitly a leaf node (reason)")
					);
				}
				sub_context_delete(scp);
			}
			trace(("leaf\n"));

			ocp = opcode_context_new(0, 0);
			if
			(
				leaf == leaf_ness_leaf_explicit
			&&
			  cook_mtime_oldest(ocp, target, (long *)0, 32767L) == 0
			)
			{
				/*
				 * It is an error if an explicit primary
				 * source file does not exist.
				 */
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", target);
				error_intl
				(
					scp,
			     i18n("\"$filename\" primary source file not found")
				);
				sub_context_delete(scp);
				opcode_context_delete(ocp);
				goto bomb_with_error;
			}
			opcode_context_delete(ocp);

			result->status = graph_build_status_success;
			result->gfp = gfp;
			gp->statistic.leaf_exists++;

			break;
		}
	}

	/*
	 * manage the try_list
	 */
ret:
 	switch (result->status)
 	{
 	case graph_build_status_backtrack:
		if (!gp->try_list)
			gp->try_list = string_list_new();
		string_list_append_unique(gp->try_list, target);
		if (try_list_saved)
		{
			string_list_append_list_unique
			(
				gp->try_list,
				try_list_saved
			);
			string_list_delete(try_list_saved);
		}
		if (gfp)
			gfp->previous_backtrack++;
		break;

 	case graph_build_status_success:
		assert(result->gfp);
		if (gp->try_list)
			string_list_delete(gp->try_list);
		gp->try_list = try_list_saved;
		break;

 	case graph_build_status_error:
		if (gp->try_list)
		{
			string_list_delete(gp->try_list);
			gp->try_list = 0;
		}
		if (try_list_saved)
			string_list_delete(try_list_saved);
		if (gfp)
			gfp->previous_error++;
		break;
	}

	/*
	 * revert pending status (used to check for infinite loops)
	 */
	if (gfp)
		gfp->pending--;

	/*
	 * release any common ingredients
	 */
	graph_file_list_nrc_destructor(&common_ingredients_gfl);

	/*
	 * report results for debug
	 */
	str_free(target);
	assert((result->status == graph_build_status_success) ==
		(result->gfp != 0));
	trace
	((
		"return { %s, %8.8lX};\n",
		graph_build_status_name(result->status),
		(long)result->gfp
	));
	trace(("}\n"));
}


/*
 * NAME
 *	graph_build
 *
 * SYNOPSIS
 *	graph_build_status_ty graph_build_file(graph_ty *, string_ty *,
 *		graph_build_preference_ty);
 *
 * DESCRIPTION
 *	The graph_build function is used to find and add the actions
 *	necessary to build the given target to the given graph.  To
 *	actually build the targets, use the graph_walk function.
 *
 * RETURNS
 *	The graph_build_file function returns one of the graph
 *	build status values.
 */

graph_build_status_ty
graph_build(gp, target, preference, waffle)
	graph_ty	*gp;
	string_ty	*target;
	graph_build_preference_ty preference;
	int		waffle;
{
	graph_build_result_ty result;

	graph_build_file(gp, target, preference, &result, 1);
	if
	(
		result.status == graph_build_status_backtrack
	&&
		preference == graph_build_preference_error
	)
		return graph_build_status_error;
	if (result.status == graph_build_status_success)
	{
		assert(result.gfp);
		if (waffle)
			result.gfp->primary_target = 1;
	}
	return result.status;
}


/*
 * NAME
 *	graph_build_list
 *
 * SYNOPSIS
 *	graph_build_status_ty graph_build_list(graph_ty *, string_list_ty *,
 *		graph_build_preference_ty);
 *
 * DESCRIPTION
 *	The graph_build_list function is used to find and add the
 *	actions necessary to build the targets to the graph.  To
 *	actually build the targets, use the graph_walk function.
 *
 * RETURNS
 *	The graph_build_file function returns one of the graph
 *	build status values.
 */

graph_build_status_ty
graph_build_list(gp, target, preference, waffle)
	graph_ty	*gp;
	string_list_ty	*target;
	graph_build_preference_ty preference;
	int		waffle;
{
	size_t		j;
	graph_build_status_ty status;

	for (j = 0; j < target->nstrings; ++j)
	{
		status = graph_build(gp, target->string[j], preference, waffle);
		switch (status)
		{
		case graph_build_status_success:
			continue;

		case graph_build_status_backtrack:
		case graph_build_status_error:
			break;
		}
		return status;
	}
	return graph_build_status_success;
}
