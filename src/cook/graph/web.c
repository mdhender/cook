/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to print dependency graphs as a shell script
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <dir_part.h>
#include <graph.h>
#include <graph/file.h>
#include <graph/file_list.h>
#include <graph/recipe.h>
#include <graph/recipe_list.h>
#include <graph/web.h>
#include <id.h>
#include <id/variable.h>
#include <option.h>
#include <opcode/context.h>
#include <recipe.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


/*
 * NAME
 *	graph_recipe_web
 *
 * SYNOPSIS
 *	graph_walk_status_ty graph_recipe_web(graph_recipe_ty *);
 *
 * DESCRIPTION
 *	The graph_recipe_web function is used to print a shell web
 *	fragment on the standard output which approximates this recipe
 *	instance.
 *
 * RETURNS
 *	graph_walk_status_ty
 *		error		something went wrong
 *		uptodate	sucecss
 */

static void graph_recipe_web _((graph_recipe_ty *));

static void
graph_recipe_web(grp)
	graph_recipe_ty	*grp;
{
	size_t		j;
	string_list_ty	wl;
	int		forced;
	string_ty	*s;

	trace(("graph_recipe_web(grp = %08lX)\n{\n", (long)grp));
	if (grp->input->nfiles == 0 && grp->output->nfiles == 0)
	{
		trace(("}\n"));
		return;
	}

	grp->ocp = opcode_context_new(0, grp->mp);

	/*
	 * construct the ``target'' variable
	 */
	string_list_constructor(&wl);
	assert(grp->output);
	assert(grp->output->nfiles > 0);
	if (grp->output->nfiles > 0)
		string_list_append(&wl, grp->output->item[0].file->filename);
	opcode_context_id_assign(grp->ocp, id_target, id_variable_new(&wl), -1);
	string_list_destructor(&wl);

	/*
	 * construct the ``targets'' variable
	 */
	string_list_constructor(&wl);
	assert(grp->input);
	for (j = 0; j < grp->output->nfiles; ++j)
		string_list_append(&wl, grp->output->item[j].file->filename);
	opcode_context_id_assign
	(
		grp->ocp,
		id_targets,
		id_variable_new(&wl),
		-1
	);
	string_list_destructor(&wl);

	/*
	 * construct the ``need'' variable
	 * (and ``younger'' will be identical)
	 */
	string_list_constructor(&wl);
	assert(grp->input);
	for (j = 0; j < grp->input->nfiles; ++j)
		string_list_append(&wl, grp->input->item[j].file->filename);
	opcode_context_id_assign(grp->ocp, id_need, id_variable_new(&wl), -1);
	opcode_context_id_assign
	(
		grp->ocp,
		id_younger,
		id_variable_new(&wl),
		-1
	);
	string_list_destructor(&wl);

	/*
	 * Flags apply to the precondition and to the ingredients
	 * evaluation.  That is why the grammar puts them first.
	 */
	recipe_flags_set(grp->rp);

	/*
	 * see of the recipe is forced to activate
	 */
	forced = option_test(OPTION_FORCE);

	/*
	 * Print the original position, so the user can tell where it
	 * came from.
	 */
	printf("\n");
	printf("<h3><a name=\"Recipe_Instance_%d\">\n", grp->id);
	printf("Recipe Instance %d</h3>\n", grp->id);
	printf("<dl>\n");
	if (grp->rp->pos.pos_line)
	{
		assert(grp->rp->pos.pos_name);
		printf("<dt>Location:\n<dd>\n");
		printf
		(
			"<a href=\"file:%s#%d\">\n",
			grp->rp->pos.pos_name->str_text,
			grp->rp->pos.pos_line
		);
		printf
		(
			"file <tt>%s</tt>, line %d</a>\n",
			grp->rp->pos.pos_name->str_text,
			grp->rp->pos.pos_line
		);
	}

	/*
	 * print the targets
	 */
	printf("<dt>Target%s:<dd>\n", (grp->output->nfiles==1?"":"s"));
	for (j = 0; j < grp->output->nfiles; ++j)
	{
		s = grp->output->item[j].file->filename;
		printf
		(
			"<tt><a href=\"#%s\">%s</a></tt>,\n",
			s->str_text,
			s->str_text
		);
	}

	/*
	 * Print the ingredients
	 */
	printf("<dt>Ingredient%s:<dd>\n", (grp->input->nfiles==1?"":"s"));
	for (j = 0; j < grp->input->nfiles; ++j)
	{
		s = grp->input->item[j].file->filename;
		printf
		(
			"<tt><a href=\"#%s\">%s</a></tt>,\n",
			s->str_text,
			s->str_text
		);
	}

	if (grp->single_thread && grp->single_thread->nstrings)
	{
		printf("<dt>Single Thread:<dd>\n");
		for (j = 0; j < grp->single_thread->nstrings; ++j)
		{
			s = grp->single_thread->string[j];
			printf("%s,\n", s->str_text);
		}
	}

	if (grp->host_binding && grp->host_binding->nstrings)
	{
		printf("<dt>Host Binding:<dd>\n");
		for (j = 0; j < grp->host_binding->nstrings; ++j)
		{
			s = grp->host_binding->string[j];
			printf("%s,\n", s->str_text);
		}
	}

	/*
	 * See if we need to perform the actions attached to this recipe.
	 */
	if (grp->rp->out_of_date)
	{
		int		echo;

		printf("<dt>Body:<dd>\n");
		printf("<pre>"); /* NO newline! */
		echo = !option_test(OPTION_SILENT);
		if (option_test(OPTION_MKDIR))
		{
			for (j = 0; j < grp->output->nfiles; ++j)
			{
				graph_file_ty	*gfp;

				gfp = grp->output->item[j].file;
				s = dir_part(gfp->filename);
				if (!s)
					continue;
				printf("if test ! -d %s; then\n", s->str_text);
				if (echo)
				{
					printf
					(
						"echo mkdir -p %s\n",
						s->str_text
					);
				}
				printf("mkdir -p %s", s->str_text);
				if (!option_test(OPTION_ERROK))
					printf(" || exit 1");
				printf("\nfi\n");
				str_free(s);
			}
		}
		if (option_test(OPTION_UNLINK))
		{
			for (j = 0; j < grp->output->nfiles; ++j)
			{
				graph_file_ty	*gfp;

				gfp = grp->output->item[j].file;
				s = gfp->filename;
				if (echo)
					printf("echo rm %s\n", s->str_text);
				printf("rm %s", s->str_text);
				if (!option_test(OPTION_ERROK))
					printf(" || exit 1");
				printf("\n");
			}
		}
		if (option_test(OPTION_TOUCH))
		{
			for (j = 0; j < grp->output->nfiles; ++j)
			{
				graph_file_ty	*gfp;

				gfp = grp->output->item[j].file;
				s = gfp->filename;
				if (echo)
					printf("echo touch %s\n", s->str_text);
				printf("touch %s", s->str_text);
				if (!option_test(OPTION_ERROK))
					printf(" || exit 1");
				printf("\n");
			}
		}
		else
		{
			trace(("doing it now\n"));
			opcode_context_call(grp->ocp, grp->rp->out_of_date);
			opcode_context_script(grp->ocp);
		}
		printf("</pre>\n");
	}

	/*
	 * This recipe is being used, so
	 * perform its 'use' action.
	 *
	 * Ignore the 'touch' option,
	 * ignore the 'errok' option,
	 * don't delete files on errors.
	 */
	if (grp->rp->up_to_date)
	{
		printf("<dt>Then Clause:<dd>\n");
		printf("<pre>"); /* NO newline! */
		trace(("perform ``use'' clause\n"));
		opcode_context_call(grp->ocp, grp->rp->up_to_date);
		opcode_context_script(grp->ocp);
		printf("</pre>\n");
	}
	printf("</dl>\n");

	/*
	 * cancel the recipe flags
	 */
	option_undo_level(OPTION_LEVEL_RECIPE);
	opcode_context_delete(grp->ocp);
	grp->ocp = 0;
	trace(("}\n"));
}


static void graph_file_web _((graph_file_ty *));

static void
graph_file_web(gfp)
	graph_file_ty	*gfp;
{
	size_t		j;
	graph_recipe_ty *grp;
	graph_file_ty	*gfp2;

	trace(("graph_file_web(gfp = %08lX)\n{\n", (long)gfp));
	if (gfp->input->nrecipes == 0 && gfp->output->nrecipes == 0)
	{
		trace(("}\n"));
		return;
	}

	printf("\n");
	printf("<h3><a name=\"%s\"><tt>\n", gfp->filename->str_text);
	printf("<a href=\"file:%s\">\n", gfp->filename->str_text);
	printf("%s</a>\n", gfp->filename->str_text);
	printf("</tt></h3><dl>\n");

	if (gfp->input->nrecipes)
	{
		printf("<dt>Created By:<dd>\n");
		for (j = 0; j < gfp->input->nrecipes; ++j)
		{
			grp = gfp->input->recipe[j];
			printf("<a href=\"#Recipe_Instance_%d\">\n", grp->id);
			printf("Recipe Instance %d</a>", grp->id);
			if (grp->input->nfiles > 0)
			{
				gfp2 = grp->input->item[0].file;
				printf
				(
					" (<a href=\"#%s\">%s</a>%s)",
					gfp2->filename->str_text,
					gfp2->filename->str_text,
					(grp->input->nfiles >= 2 ? ", ..." : "")
				);
			}
			printf(",\n");
		}
	}

	if (gfp->output->nrecipes)
	{
		printf("<dt>Consumed By:<dd>\n");
		for (j = 0; j < gfp->output->nrecipes; ++j)
		{
			grp = gfp->output->recipe[j];
			printf("<a href=\"#Recipe_Instance_%d\">\n", grp->id);
			printf("Recipe Instance %d</a>", grp->id);
			if (grp->output->nfiles > 0)
			{
				gfp2 = grp->output->item[0].file;
				printf
				(
					" (<a href=\"#%s\">%s</a>%s)",
					gfp2->filename->str_text,
					gfp2->filename->str_text,
				       (grp->output->nfiles >= 2 ? ", ..." : "")
				);
			}
			printf(",\n");
		}
	}
	printf("</dl>\n");
	trace(("}\n"));
}


static void web_file_stash _((symtab_ty *, string_ty *, void *, void *));

static void
web_file_stash(stp, key, data, aux)
	symtab_ty	*stp;
	string_ty	*key;
	void		*data;
	void		*aux;
{
	graph_file_ty	*gfp;
	graph_file_list_nrc_ty *gflp;

	gfp = data;
	gflp = aux;
	graph_file_list_nrc_append(gflp, gfp, edge_type_default);
}


static int web_file_cmp _((const void *, const void *));

static int
web_file_cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	graph_file_and_type_ty *a;
	graph_file_and_type_ty *b;

	a = (graph_file_and_type_ty *)va;
	b = (graph_file_and_type_ty *)vb;
	return strcmp(a->file->filename->str_text, b->file->filename->str_text);
}


static int web_recipe_cmp _((const void *, const void *));

static int
web_recipe_cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	graph_recipe_ty	*a;
	graph_recipe_ty	*b;

	a = *(graph_recipe_ty **)va;
	b = *(graph_recipe_ty **)vb;
	return (a->id - b->id);
}


void
graph_walk_web(gp)
	graph_ty	*gp;
{
	graph_file_list_nrc_ty gfl;
	graph_recipe_list_nrc_ty grl;
	size_t		j;
	graph_file_ty	*gfp;
	graph_recipe_ty	*grp;

	/*
	 * Fetch the list of files, and sort it by name.
	 */
	trace(("graph_walk_web(gp = %08lX)\n{\n", (long)gp));
	graph_file_list_nrc_constructor(&gfl);
	symtab_walk(gp->already, web_file_stash, &gfl);
	qsort(gfl.item, gfl.nfiles, sizeof(gfl.item[0]), web_file_cmp);

	/*
	 * Fetch the list of recipe instances, and sort it by ID.
	 */
	graph_recipe_list_nrc_constructor(&grl);
	for (j = 0; j < gp->already_recipe->nrecipes; ++j)
	{
		graph_recipe_list_nrc_append
		(
			&grl,
			gp->already_recipe->recipe[j]
		);
	}
	qsort(grl.recipe, grl.nrecipes, sizeof(grl.recipe[0]), web_recipe_cmp);

	/*
	 * Print the page header.
	 */
	printf("<html><head><title>\n");
	printf("Dependency Graph\n");
	printf("</title></head><body><h1>\n");
	printf("Dependency Graph\n");
	printf("</h1>\n");

	/*
	 * Generate the list of files.
	 */
	printf("<h2>Files</h2>\n");
	for (j = 0; j < gfl.nfiles; ++j)
	{
		gfp = gfl.item[j].file;
		graph_file_web(gfp);
	}

	/*
	 * Generate the list of recipe instances.
	 */
	printf("<h2>Recipe Instances</h2>\n");
	for (j = 0; j < grl.nrecipes; ++j)
	{
		grp = grl.recipe[j];
		graph_recipe_web(grp);
	}

	/*
	 * Finish the page.
	 */
	printf("</body></html>\n");
	graph_file_list_nrc_destructor(&gfl);
	graph_recipe_list_nrc_destructor(&grl);
	trace(("}\n"));
}
