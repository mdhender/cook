/*
 *	cook - file construction tool
 *	Copyright (C) 1994-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to cook targets
 *
 * the kitchen
 *
 * This file contains the part of cook that actually decides which
 * recipes will be cooked.
 *
 * When cook has a target, cook performs the following actions in the order
 * given:
 *
 * 1.	Cook scans through the instantiated prerequisite recipes. All
 *	prerequisite recipes with the target in their target list are used.
 *
 *	If the recipe is used, any prerequisite files are recursively
 *	cooked, then if any of the prerequisite files were out of date, then
 *	all other explicit or implicit recipes with the same target will be
 *	deemed to be out of date.
 *
 * 2.	Cook then scans through the instantiated explicit recipes. All
 *	explicit recipes with the target in their target list are used.
 *
 *	If the recipe is used, any prerequisite files are recursively
 *	cooked, then if any prerequisites were out of date (including those
 *	of prerequisite recipes) then the actions bound to this recipe will
 *	be evaluated.
 *
 *	If there are no ingredients, then it is not out-of-date.  The
 *	body will be performed if (a) the target does not yet exist, or
 *	(b) the "force" flag is set, usually in the "set" clause of
 *	the recipe.
 *
 * 3.	If the target was not the subject of any explicit recipe, cook then
 *	scans the instantiated implicit recipes. Only first implicit recipe
 *	for which cook knows how to cook will be used.
 *
 *	Implicit recipe targets and prerequisites may contain a wilcard
 *	character (%), which is why they are implicit. If more than one
 *	wildcard character appears, only the last is considered the wilcard
 *	charcater.
 *
 *	If an implicit recipe is used, when expressions are evaluaded into
 *	word lists, any word containing the wildcard charcater (%) will be
 *	expanded out by the current wildcard expansion.
 *
 * 4.	If the target is not the subject of any prerequisite or explicit
 *	recipe, and no implicit recipes can be applied, then two things can
 *	happen.
 *		a. If the file exists, then it is up to date, or
 *		b. If the file does not exist then cook doesn't know how.
 *
 * If a command in the actions bound to any recipe fail, cook will not
 * evaluate those actions any further, and will not evaluate the actions
 * of any recipe for which the target of the failed actions was a
 * prerequisite.
 *
 * Cook will trap recursive looping of targets. If a recursion loop is
 * detected, then
 *	1. If the file exists, then it is up to date, or
 *	2. If the file does not exist then cook doesn't know how.
 */

#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/time.h>

#include <cascade.h>
#include <cook.h>
#include <desist.h>
#include <error.h>
#include <error_intl.h>
#include <expr.h>
#include <flag.h>
#include <fingerprint.h>
#include <fingerprint/value.h>
#include <graph.h>
#include <graph/build.h>
#include <graph/file_pair.h>
#include <graph/leaf.h>
#include <graph/stats.h>
#include <graph/walk.h>
#include <graph/web.h>
#include <id.h>
#include <id/variable.h>
#include <match/new_by_recip.h>
#include <mem.h>
#include <opcode/context.h>
#include <option.h>
#include <os_interface.h>
#include <recipe.h>
#include <recipe/list.h>
#include <star.h>
#include <stmt.h>
#include <symtab.h>
#include <trace.h>


static	recipe_list_ty	explicit;	/* the explicit recipes */
static	symtab_ty	*explicit_stp;	/* the explicit recipes, indexed */
static	recipe_list_ty	implicit;	/* the implicit recipes */
static	symtab_ty	*implicit_stp;	/* the explicit recipes, indexed */
static	string_list_ty	cook_auto_list;
static	string_list_ty	cook_auto_list_nonleaf;


/*
 * NAME
 *	cook_search_list
 *
 * SYNOPSIS
 *	cook_search_list(string_list_ty *slp);
 *
 * DESCRIPTION
 *	The cook_search_list function is used to get the search list
 *	from the "search_list" variable.
 *
 *	Defaulting and clean-up are done here, also.
 *	If absent, defaults to ".".
 *	If the first element is not "." then it is inserted.
 *
 * ARGUMENTS
 *	slp - where to put the result
 */

void
cook_search_list(ocp, slp)
	const opcode_context_ty *ocp;
	string_list_ty	*slp;
{
	string_ty	*s;
	id_ty		*idp;

	/*
	 * make sure the variable exists
	 */
	trace(("cook_search_list()\n{\n"/*}*/));
	idp = opcode_context_id_search(ocp, id_search_list);
	if (!idp)
	{
		string_list_constructor(slp);
		s = str_from_c(".");
		string_list_append(slp, s);
		str_free(s);
		opcode_context_id_assign
		(
			(opcode_context_ty *)ocp,
			id_search_list,
			id_variable_new(slp),
			0
		);
		string_list_destructor(slp);
	}

	/*
	 * extract its string value
	 */
	id_variable_query(idp, slp);

	/*
	 * make sure the search list isn't empty
	 * make sure the search list has "." as the first element
	 */
	if
	(
		!slp->nstrings
	||
		slp->string[0]->str_length != 1
	||
		slp->string[0]->str_text[0] != '.'
	)
	{
		s = str_from_c(".");
		string_list_prepend(slp, s);
		str_free(s);
		opcode_context_id_assign
		(
			(opcode_context_ty *)ocp,
			id_search_list,
			id_variable_new(slp),
			0
		);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	cook_mtime_oldest
 *
 * SYNOPSIS
 *	time_t cook_mtime_oldest(string_ty *path, long *depth_p);
 *
 * DESCRIPTION
 *	The cook_mtime_oldest function is used to scan the search path
 *	for a file to determine the last-modified time of the file.
 *
 *	Look for the copies of the file which are identical to the
 *	shallowest copy; use the oldest time of all. Return the deepest
 *	level found.
 *
 * ARGUMENTS
 *	path	- file to get the mtime for
 *	depth_p	- where to put the depth
 *
 * RETURNS
 *	long; -1 on error, 0 if no such file, >0 for time
 *
 * CAVEAT
 *	The user must design recipes using the [resolve] function.
 */

time_t
cook_mtime_oldest(ocp, path, depth_p, max_fp_depth)
	const opcode_context_ty *ocp;
	string_ty	*path;
	long		*depth_p;
	long		max_fp_depth;
{
	time_t		result;
	long		bogus;

	trace(("cook_mtime_oldest(path = \"%s\", max = %ld)\n{\n"/*}*/,
		path->str_text, max_fp_depth));
	if (!depth_p)
		depth_p = &bogus;
	if (path->str_text[0] == '/')
	{
		result = os_mtime_oldest(path);
		*depth_p = 32767;
	}
	else
	{
		fp_value_ty	*prv_fp;
		string_list_ty	sl;
		long		j;

		prv_fp = 0;
		result = 0;
		cook_search_list(ocp, &sl);
		*depth_p = sl.nstrings;
		for (j = 0; j < sl.nstrings; ++j)
		{
			fp_value_ty	*fp;
			string_ty	*s1;
			string_ty	*s2;
			time_t		t;

			s1 = sl.string[j];
			if (s1->str_text[0] == '.' && !s1->str_text[1])
				s2 = str_copy(path);
			else
				s2 = str_format("%S/%S", s1, path);
			/*
			 * This allows the safe use of fp_search below,
			 * since os_mtime_oldest updates the fingerprint
			 * cache for the file.
			 */
			t = os_mtime_oldest(s2);

			if (!t)
			{
				/* File not found */
				str_free(s2);
				continue;
			}

			trace(("mtime(\"%s\") was %ld\n",
				s2->str_text, (long)t));

			/* File found */
			if (!prv_fp)
			{
				/* Shallowest file found */
				result = t;
				*depth_p = j;
				trace(("is the first\n"));
				if (option_test(OPTION_FINGERPRINT))
				{
					prv_fp = fp_search(s2);
					if (prv_fp)
					{
						/* Look for deeper copies */
						str_free(s2);
						continue;
					}
				}
				str_free(s2);
				break;
			}

			/* Found a deeper version */
			assert(option_test(OPTION_FINGERPRINT));
			if (j >= max_fp_depth)
			{
				str_free(s2);
				break;
			}
			fp = fp_search(s2);
			str_free(s2);

			if
			(
				fp
			&&
				str_equal
				(
					fp->contents_fingerprint,
					prv_fp->contents_fingerprint
				)
			)
			{
				/* Deeper version is same as shallow one */
				if (t < result)
					result = t;
				*depth_p = j;
			}
			else
			{
				trace(("was different\n"));
				break;
			}
		}
		string_list_destructor(&sl);
	}
	trace(("return %ld (%d);\n", (long)result, *depth_p));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	cook_mtime_newest
 *
 * SYNOPSIS
 *	time_t cook_mtime_newest(string_ty *path, long *depth_p);
 *
 * DESCRIPTION
 *	The cook_mtime_newest function is used to scan the search path
 *	for a file to determine the last-modified time of the file.
 *
 *	Look for the copies of the file which are identical to the
 *	shallowest copy; use the newest time of all. Return the
 *	shallowest level found.
 *
 * ARGUMENTS
 *	path	- file to get the mtime for
 *	depth_p	- where to put the depth
 *
 * RETURNS
 *	long; -1 on error, 0 if no such file, >0 for time
 *
 * CAVEAT
 *	The user must design recipes using the [resolve] function.
 */

time_t
cook_mtime_newest(ocp, path, depth_p, max_fp_depth)
	const opcode_context_ty *ocp;
	string_ty	*path;
	long		*depth_p;
	long		max_fp_depth;
{
	time_t		result;

	trace(("cook_mtime_newest(path = \"%s\")\n{\n"/*}*/, path->str_text));
	if (path->str_text[0] == '/')
	{
		result = os_mtime_newest(path);
		*depth_p = 0;
	}
	else
	{
		fp_value_ty	*prv_fp;
		string_list_ty	sl;
		long		j;

		prv_fp = 0;
		result = 0;
		cook_search_list(ocp, &sl);
		*depth_p = sl.nstrings;
		for (j = 0; j < sl.nstrings; ++j)
		{
			fp_value_ty	*fp;
			string_ty	*s1;
			string_ty	*s2;
			time_t		t;

			s1 = sl.string[j];
			if (s1->str_text[0] == '.' && !s1->str_text[1])
				s2 = str_copy(path);
			else
				s2 = str_format("%S/%S", s1, path);
			/*
			 * This allows the safe use of fp_search below,
			 * since os_mtime_newest updates the fingerprint
			 * cache for the file.
			 */
			t = os_mtime_newest(s2);

			if (!t)
			{
				/* File was not found */
				str_free(s2);
				continue;
			}

			trace(("mtime(\"%s\") was %ld\n",
				s2->str_text, (long)t));

			/* File found */
			if (!prv_fp)
			{
				/* Shallowest file found */
				result = t;
				*depth_p = j;
				trace(("is the first\n"));

				if (option_test(OPTION_FINGERPRINT))
				{
					prv_fp = fp_search(s2);
					if (prv_fp)
					{
						/* Look for deeper copies */
						str_free(s2);
						continue;
					}
				}
				str_free(s2);
				break;
			}

			/* Found a deeper version */
			assert(option_test(OPTION_FINGERPRINT));
			if (j >= max_fp_depth)
			{
				str_free(s2);
				break;
			}
			fp = fp_search(s2);
			str_free(s2);

			if
			(
				fp
			&&
				str_equal
				(
					fp->contents_fingerprint,
					prv_fp->contents_fingerprint
				)
			)
			{
				/* Deeper version is same as shallow one */
				if (t > result)
					result = t;
				/* do not alter depth */
			}
			else
			{
				trace(("was different\n"));
				break;
			}
		}
		string_list_destructor(&sl);
	}
	trace(("return %ld (%d);\n", (long)result, *depth_p));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	cook_mtime_resolve
 *
 * SYNOPSIS
 *	int cook_mtime_resolve(string_list_ty *output, string_list_ty *input);
 *
 * DESCRIPTION
 *	The cook_mtime_resolve function is used to
 *	resolve the name used for a file in the search list.
 *
 *	It implements the "resolve" built-in function.
 *
 * ARGUMENTS
 *	input - the function arguments (0 is the func name)
 *	output - where to put the results
 *
 * RETURNS
 *	int; 0 on success, -1 on error
 *
 * CAVEAT
 *	The user must design rules using the [resolve] function.
 */

int
cook_mtime_resolve(ocp, output, input, start)
	const opcode_context_ty *ocp;
	string_list_ty	*output;
	const string_list_ty *input;
	int		start;
{
	int		result;
	long		j;
	string_list_ty	sl;

	trace(("cook_mtime_resolve(input = %08lX, output = %08lX)\n{\n"/*}*/,
		input, output));
	cook_search_list(ocp, &sl);
	result = 0;
	for (j = start; j < input->nstrings; ++j)
	{
		string_ty	*arg;

		arg = input->string[j];
		if (arg->str_text[0] == '/')
			string_list_append(output, arg);
		else
		{
			int		done;
			long		k;

			done = 0;
			for (k = 0; k < sl.nstrings; ++k)
			{
				string_ty	*s1;
				string_ty	*s2;
				time_t		t;

				s1 = sl.string[k];
				if (s1->str_text[0] == '.' && !s1->str_text[1])
					s2 = str_copy(arg);
				else
					s2 = str_format("%S/%S", s1, arg);
				t = os_mtime_newest(s2);
				if (t < 0)
				{
					result = -1;
					str_free(s2);
					break;
				}
				if (t > 0)
				{
					string_list_append(output, s2);
					str_free(s2);
					done = 1;
					break;
				}
				str_free(s2);
			}
			if (!done)
				string_list_append(output, arg);
		}
		if (result < 0)
			break;
	}
	string_list_destructor(&sl);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	cook - construct files
 *
 * SYNOPSIS
 *	int cook(string_list_ty *targets);
 *
 * DESCRIPTION
 *	The cook function is used to cook the given set of targets.
 *
 * RETURNS
 *	The cook function returns 0 if all of the targets cooked sucessfully,
 *	or 1 if there was any problem (exit statii).
 *
 * CAVEAT
 *	This function must be called after evrything has been initialized,
 *	and the cookbook read in.
 */

int
cook(wlp)
	string_list_ty	*wlp;
{
	int		retval;
	graph_ty	*gp;
	graph_build_status_ty gb_status;
	graph_walk_status_ty gw_status;

	/*
	 * set interrupts to catch
	 *
	 * Note that tee(1) [see listing.c] must ignore them
	 * for the generated messages to appear in the log file.
	 */
	trace(("cook(wlp = %08lX)\n{\n"/*}*/, wlp));
	desist_enable();

	/*
	 * Build the dependency graph.
	 */
	retval = 0;
	gp = graph_new();
	if
	(
		cook_auto_list_nonleaf.nstrings > 0
	&&
		cascade_used()
	&&
		option_test(OPTION_CASCADE)
	&&
		!option_test(OPTION_SILENT)
	&&
		option_test(OPTION_INCLUDE_COOKED_WARNING)
	)
	{
		gp->file_pair = graph_file_pair_new((string_list_ty *)0);
		graph_file_pair_foreign_derived
		(
			gp->file_pair,
			&cook_auto_list_nonleaf
		);
	}
	gb_status = graph_build_list(gp, wlp, graph_build_preference_error, 1);
	if (option_test(OPTION_REASON))
		graph_print_statistics(gp);
	switch (gb_status)
	{
	case graph_build_status_error:
		retval = 1;
		break;

	case graph_build_status_backtrack:
		/* assert(0); */
		retval = 1;
		break;

	case graph_build_status_success:
		break;
	}

	/*
	 * Walk the dependency graph.
	 */
	if (retval == 0)
	{
		gw_status = graph_walk(gp);
		switch (gw_status)
		{
		case graph_walk_status_uptodate:
		case graph_walk_status_uptodate_done:
		case graph_walk_status_done:
			break;

		case graph_walk_status_done_stop:
		case graph_walk_status_wait:
			assert(0);
			/* fall through... */

		case graph_walk_status_error:
			retval = 1;
			break;
		}
	}

	/*
	 * Release any resources held by the graph.
	 */
	graph_delete(gp);

	/*
	 * Return the result to the caller.
	 */
	trace(("return %d;\n", retval));
	trace((/*{*/"}\n"));
	return retval;
}


/*
 * NAME
 *	cook_pairs
 *
 * SYNOPSIS
 *	int cook_pairs(string_list_ty *);
 *
 * DESCRIPTION
 *	The cook_pairs function is used to print generate pair-wise file
 *	dependencies for the ancestors of the given targets.
 *
 * RETURNS
 *	int; 0 on success, 1 on failure (exit statii)
 */

int
cook_pairs(wlp)
	string_list_ty	*wlp;
{
	int		retval;
	graph_ty	*gp;
	graph_build_status_ty gb_status;
	graph_walk_status_ty gw_status;

	/*
	 * set interrupts to catch
	 *
	 * Note that tee(1) [see listing.c] must ignore them
	 * for the generated messages to appear in the log file.
	 */
	trace(("cook(wlp = %08lX)\n{\n"/*}*/, wlp));
	desist_enable();

	/*
	 * Build the dependency graph.
	 */
	retval = 0;
	gp = graph_new();
	gb_status = graph_build_list(gp, wlp, graph_build_preference_error, 0);
	if (option_test(OPTION_REASON))
		graph_print_statistics(gp);
	switch (gb_status)
	{
	case graph_build_status_error:
		retval = 1;
		break;

	case graph_build_status_backtrack:
		/* assert(0); */
		retval = 1;
		break;

	case graph_build_status_success:
		break;
	}

	/*
	 * Walk the dependency graph.
	 */
	if (retval == 0)
	{
		gw_status = graph_walk_pairs(gp);
		switch (gw_status)
		{
		case graph_walk_status_uptodate:
		case graph_walk_status_uptodate_done:
		case graph_walk_status_done:
			break;

		case graph_walk_status_done_stop:
		case graph_walk_status_wait:
			assert(0);
			/* fall through... */

		case graph_walk_status_error:
			retval = 1;
			break;
		}
	}

	/*
	 * Release any resources held by the graph.
	 */
	graph_delete(gp);

	/*
	 * Return the result to the caller.
	 */
	trace(("return %d;\n", retval));
	trace((/*{*/"}\n"));
	return retval;
}


/*
 * NAME
 *	cook_script
 *
 * SYNOPSIS
 *	void cook_script(string_list_ty *);
 *
 * DESCRIPTION
 *	The cook_script function is used to print a shell script to
 *	build the the given targets.  It's only an approximation of the
 *	full cook semantics.
 *
 * RETURNS
 *	int; 0 on success, 1 on failure (exit statii)
 */

int
cook_script(wlp)
	string_list_ty	*wlp;
{
	int		retval;
	graph_ty	*gp;
	graph_build_status_ty gb_status;
	graph_walk_status_ty gw_status;

	/*
	 * set interrupts to catch
	 *
	 * Note that tee(1) [see listing.c] must ignore them
	 * for the generated messages to appear in the log file.
	 */
	trace(("cook(wlp = %08lX)\n{\n"/*}*/, wlp));
	desist_enable();

	/*
	 * Build the dependency graph.
	 */
	retval = 0;
	gp = graph_new();
	gb_status = graph_build_list(gp, wlp, graph_build_preference_error, 0);
	if (option_test(OPTION_REASON))
		graph_print_statistics(gp);
	switch (gb_status)
	{
	case graph_build_status_error:
		retval = 1;
		break;

	case graph_build_status_backtrack:
		/* assert(0); */
		retval = 1;
		break;

	case graph_build_status_success:
		break;
	}

	/*
	 * Walk the dependency graph.
	 */
	if (retval == 0)
	{
		gw_status = graph_walk_script(gp);
		switch (gw_status)
		{
		case graph_walk_status_uptodate:
		case graph_walk_status_uptodate_done:
		case graph_walk_status_done:
			break;

		case graph_walk_status_done_stop:
		case graph_walk_status_wait:
			assert(0);
			/* fall through... */

		case graph_walk_status_error:
			retval = 1;
			break;
		}
	}

	/*
	 * Release any resources held by the graph.
	 */
	graph_delete(gp);

	/*
	 * Return the result to the caller.
	 */
	trace(("return %d;\n", retval));
	trace((/*{*/"}\n"));
	return retval;
}


/*
 * NAME
 *	cook_web
 *
 * SYNOPSIS
 *	void cook_web(string_list_ty *);
 *
 * DESCRIPTION
 *	The cook_web function is used to print a shell web to
 *	build the the given targets.  It's only an approximation of the
 *	full cook semantics.
 *
 * RETURNS
 *	int; 0 on success, 1 on failure (exit statii)
 */

int
cook_web(wlp)
	string_list_ty	*wlp;
{
	int		retval;
	graph_ty	*gp;
	graph_build_status_ty gb_status;

	/*
	 * set interrupts to catch
	 *
	 * Note that tee(1) [see listing.c] must ignore them
	 * for the generated messages to appear in the log file.
	 */
	trace(("cook(wlp = %08lX)\n{\n", wlp));
	desist_enable();

	/*
	 * Build the dependency graph.
	 */
	retval = 0;
	gp = graph_new();
	gb_status = graph_build_list(gp, wlp, graph_build_preference_error, 0);
	if (option_test(OPTION_REASON))
		graph_print_statistics(gp);
	switch (gb_status)
	{
	case graph_build_status_error:
		retval = 1;
		break;

	case graph_build_status_backtrack:
		/* assert(0); */
		retval = 1;
		break;

	case graph_build_status_success:
		break;
	}

	/*
	 * Walk the dependency graph.
	 */
	if (retval == 0)
		graph_walk_web(gp);

	/*
	 * Release any resources held by the graph.
	 */
	graph_delete(gp);

	/*
	 * Return the result to the caller.
	 */
	trace(("return %d;\n", retval));
	trace(("}\n"));
	return retval;
}


/*
 * NAME
 *	cook_auto
 *
 * SYNOPSIS
 *	void cook_auto(string_ty *);
 *
 * DESCRIPTION
 *	The cook_auto function is used to that this file needs to be
 *	automaticaly cooked.  This is done for files inccluded using the
 *	#include-cooked mechanism.
 */

void
cook_auto(wlp)
	string_list_ty	*wlp;
{
	string_list_append_list_unique(&cook_auto_list, wlp);
}


/*
 * NAME
 *	cook_auto_required
 *
 * SYNOPSIS
 *	int cook_auto_required(void);
 *
 * DESCRIPTION
 *	The cook_auto_required function is used to automaticly re-build
 *	any files included by the #include-cooked mechanism which may
 *	have been out of date.
 *
 * RETURNS
 *	int;	-1	on error
 *		0	if everything was up-to-date, the cookbook does not
 *			need to be read in again
 *		1	if one or more include files were re-build, and the
 *			cookbook must be read again.
 */

int
cook_auto_required()
{
	int		retval;
	graph_ty	*gp;
	graph_build_status_ty gb_status;
	graph_walk_status_ty gw_status;
	size_t		j;

	/*
	 * This may have been explicitly forbidden on the command line.
	 */
	if (!option_test(OPTION_INCLUDE_COOKED))
		return 0;

	retval = 0;

	option_set(OPTION_ACTION, OPTION_LEVEL_AUTO, 1);
	option_set(OPTION_TOUCH, OPTION_LEVEL_AUTO, 0);
	option_set(OPTION_REASON, OPTION_LEVEL_COOKBOOK, 0);

	/*
	 * Build the dependency graph.
	 */
	gp = graph_new();
	if
	(
		!option_test(OPTION_SILENT)
	&&
		option_test(OPTION_INCLUDE_COOKED_WARNING)
	)
		gp->file_pair = graph_file_pair_new(&cook_auto_list);
	gb_status =
		graph_build_list
		(
			gp,
			&cook_auto_list,
			graph_build_preference_error,
			0	/* not primary, no up-to-date commentary */
		);
	if (option_test(OPTION_REASON))
		graph_print_statistics(gp);
	switch (gb_status)
	{
	case graph_build_status_error:
		retval = -1;
		break;

	case graph_build_status_backtrack:
		/* assert(0); */
		retval = -1;
		break;

	case graph_build_status_success:
		break;
	}

	/*
	 * Build a list of non-leaf cook-auto files.
	 */
	string_list_destructor(&cook_auto_list_nonleaf);
	for (j = 0; j < cook_auto_list.nstrings; ++j)
	{
		string_ty	*fn;

		fn = cook_auto_list.string[j];
		if (!graph_file_leaf_p(gp, fn))
			string_list_append(&cook_auto_list_nonleaf, fn);
	}

	/*
	 * Walk the dependency graph.
	 */
	if (retval == 0)
	{
		gw_status = graph_walk(gp);
		switch (gw_status)
		{
		case graph_walk_status_uptodate:
		case graph_walk_status_uptodate_done:
			break;

		case graph_walk_status_done:
			retval = 1;
			break;

		case graph_walk_status_done_stop:
		case graph_walk_status_wait:
			assert(0);
			/* fall through... */

		case graph_walk_status_error:
			retval = -1;
			break;
		}
	}

	/*
	 * Release any resources held by the graph.
	 */
	graph_delete(gp);

	option_undo(OPTION_REASON, OPTION_LEVEL_COOKBOOK);
	option_undo(OPTION_ACTION, OPTION_LEVEL_AUTO);
	option_undo(OPTION_TOUCH, OPTION_LEVEL_AUTO);

	return retval;
}


/*
 * NAME
 *	cook_reset
 *
 * SYNOPSIS
 *	void cook_reset(void);
 *
 * DESCRIPTION
 *	The cook_reset function is used to reset the recipe lists in
 *	preparation for re-reading a cookbook.  Usually the result of a
 *	#include-cooked file being re-build.
 */

void
cook_reset()
{
	leaf_reset();
	string_list_destructor(&cook_auto_list);
	/* Don't nuke cook_auto_list_nonleaf, we need it for later */
	cook_implicit_nth_by_name(0, 0);
	if (explicit_stp)
	{
		symtab_free(explicit_stp);
		explicit_stp = 0;
	}
	if (implicit_stp)
	{
		symtab_free(implicit_stp);
		implicit_stp = 0;
	}
	recipe_list_destructor(&explicit);
	recipe_list_destructor(&implicit);
	cascade_reset();
}


/*
 * NAME
 *	cook_find_default
 *
 * SYNOPSIS
 *	void cook_find_default(string_list_ty *);
 *
 * DESCRIPTION
 *	The cook_find_default function is used to find the default
 *	target(s).  The explicit recipes are examined - the targets of
 *	the first one (that doesn't have nodefault set) are the default
 *	targets.
 */

void
cook_find_default(wlp)
	string_list_ty	*wlp;
{
	size_t		j;
	recipe_ty	*rp;

	/*
	 * use the forst one
	 * explicitly flagged default
	 */
	for (j = 0; j < explicit.nrecipes; ++j)
	{
		rp = explicit.recipe[j];
		if (flag_query(rp->flags, RF_DEFAULT))
		{
			string_list_copy_constructor(wlp, rp->target);
			return;
		}
	}

	/*
	 * use the first one
	 * not flagged nodefault
	 */
	for (j = 0; j < explicit.nrecipes; ++j)
	{
		rp = explicit.recipe[j];
		if (!flag_query(rp->flags, RF_DEFAULT_OFF))
		{
			string_list_copy_constructor(wlp, rp->target);
			return;
		}
	}

	/*
	 * fatal error otherwise
	 */
	fatal_intl(0, i18n("no default target"));
}


static void explicit_reap _((void *));

static void
explicit_reap(p)
	void		*p;
{
	recipe_list_ty	*rlp;

	rlp = p;
	recipe_list_delete(rlp);
}


static recipe_list_ty *cook_explicit_find _((string_ty *));

static recipe_list_ty *
cook_explicit_find(filename)
	string_ty	*filename;
{
	recipe_list_ty	*rlp;

	if (!explicit_stp)
	{
		explicit_stp = symtab_alloc(200);
		explicit_stp->reap = explicit_reap;
	}
	rlp = symtab_query(explicit_stp, filename);
	if (!rlp)
	{
		rlp = recipe_list_new();
		symtab_assign(explicit_stp, filename, rlp);
	}
	return rlp;
}


static recipe_list_ty *cook_implicit_find _((string_ty *));

static recipe_list_ty *
cook_implicit_find(filename)
	string_ty	*filename;
{
	recipe_list_ty	*rlp;

	if (!implicit_stp)
	{
		implicit_stp = symtab_alloc(200);
		implicit_stp->reap = explicit_reap;
	}
	rlp = symtab_query(implicit_stp, filename);
	if (!rlp)
	{
		rlp = recipe_list_new();
		symtab_assign(implicit_stp, filename, rlp);
	}
	return rlp;
}


/*
 * NAME
 *	cook_explicit_append
 *
 * SYNOPSIS
 *	void cook_explicit_append(recipe_ty *);
 *
 * DESCRIPTION
 *	The cook_explicit_append function is used to append a recipe to
 *	the explicit recipe list.  Used by the cookbook evaluation
 *	functions.
 */

void
cook_explicit_append(rp)
	recipe_ty	*rp;
{
	size_t		j;

	trace(("cook_explicit_append(rp = %08lX)\n{\n"/*}*/, (long)rp));
	recipe_list_append(&explicit, rp);
	for (j = 0; j < rp->target->nstrings; ++j)
	{
		string_ty	*filename;
		recipe_list_ty	*rlp;

		filename = rp->target->string[j];
		rlp = cook_explicit_find(filename);
		recipe_list_append(rlp, rp);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	cook_implicit_append
 *
 * SYNOPSIS
 *	void cook_implicit_append(recipe_ty *);
 *
 * DESCRIPTION
 *	The cook_implicit_append function is used to append a recipe to
 *	the explicit recipe list.  Used by the cookbook evaluation
 *	functions.
 */

void
cook_implicit_append(rp)
	recipe_ty	*rp;
{
	string_ty	*base;
	string_list_ty	base_list;
	recipe_list_ty	*rlp;
	size_t		j;
	match_ty	*mp;

	/*
	 * Create a suitable matching object.  We need to set the recipe
	 * flags, to know which matching flavour.
	 */
	mp = match_new_by_recipe(rp);

	string_list_constructor(&base_list);
	for (j = 0; j < rp->target->nstrings; ++j)
	{
		base = os_entryname(rp->target->string[j]);
		if (match_usage_mask(mp, base, &rp->pos) != 0)
		{
			match_delete(mp);
			str_free(base);
			string_list_destructor(&base_list);
			recipe_list_append(&implicit, rp);
			return;
		}
		string_list_append_unique(&base_list, base);
		str_free(base);
	}
	match_delete(mp);
	for (j = 0; j < base_list.nstrings; ++j)
	{
		base = base_list.string[j];
		rlp = cook_implicit_find(base);
		recipe_list_append(rlp, rp);
	}
	string_list_destructor(&base_list);
}


/*
 * NAME
 *	cook_explicit_nth
 *
 * SYNOPSIS
 *	recipe_ty *cook_explicit_nth(long);
 *
 * DESCRIPTION
 *	The cook_explicit_nth function is used to get the n'th recipe
 *	from the explicit recipe list.
 *
 * RETURNS
 *	recipe_ty *; the recipe you asked for, or NULL if you went off
 *	the end.
 */

const recipe_list_ty *
cook_explicit_by_name(filename)
	string_ty	*filename;
{
	return cook_explicit_find(filename);
}


/*
 * NAME
 *	cook_implicit_nth
 *
 * SYNOPSIS
 *	recipe_ty *cook_implicit_nth(long);
 *
 * DESCRIPTION
 *	The cook_implicit_nth function is used to get the n'th recipe
 *	from the implicit recipe list.
 *
 * RETURNS
 *	recipe_ty *; the recipe you asked for, or NULL if you went off
 *	the end.
 */

recipe_ty *
cook_implicit_nth(n)
	long	n;
{
	if (n < 0 || n >= implicit.nrecipes)
		return 0;
	return implicit.recipe[n];
}


/*
 * NAME
 *	cook_implicit_nth_by_name
 *
 * SYNOPSIS
 *	recipe_ty *cook_implicit_nth_by_name(long, string_ty *);
 *
 * DESCRIPTION
 *	The cook_implicit_nth_by_name function is used to get the n'th
 *	recipe from the implicit recipe list; indexed by the last filename
 *	element if non-pattern.
 *
 * RETURNS
 *	recipe_ty *; the recipe you asked for, or NULL if you went off
 *	the end.
 */

recipe_ty *
cook_implicit_nth_by_name(n, name)
	long 		n;
	string_ty	*name;
{
	static string_ty *prev;
	static recipe_list_ty *rlp;

	if (!name)
	{
		/* used to clear the state between passes */
		if (prev)
			str_free(prev);
		prev = 0;
		rlp = 0;
		return 0;
	}
	if (!prev || !str_equal(prev, name))
	{
		if (prev)
			str_free(prev);
		prev = str_copy(name);
		rlp = cook_implicit_find(name);
	}
	if (n < 0 || n >= rlp->nrecipes)
		return 0;
	return rlp->recipe[n];
}
