/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2003, 2006-2009 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#include <common/ac/stdio.h>

#include <cook/dir_part.h>
#include <common/error_intl.h>
#include <cook/graph/file.h>
#include <cook/graph/file_list.h>
#include <cook/graph/recipe.h>
#include <cook/graph/script.h>
#include <cook/id.h>
#include <cook/id/variable.h>
#include <cook/match.h>
#include <cook/opcode/context.h>
#include <cook/option.h>
#include <cook/os_interface.h>
#include <cook/recipe.h>
#include <common/star.h>
#include <cook/stmt.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      graph_recipe_script
 *
 * SYNOPSIS
 *      graph_walk_status_ty graph_recipe_script(graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_script function is used to print a shell script
 *      fragment on the standard output which approximates this recipe
 *      instance.
 *
 * RETURNS
 *      graph_walk_status_ty
 *              error           something went wrong
 *              uptodate        sucecss
 */

graph_walk_status_ty
graph_recipe_script(graph_recipe_ty *grp, struct graph_ty *gp)
{
    graph_walk_status_ty status;
    size_t          j;
    string_list_ty  wl;
    int             forced;
    long            file_pos = 0;

    trace(("graph_recipe_script(grp = %p)\n{\n", grp));
    status = graph_walk_status_done;

    grp->ocp = opcode_context_new(0, grp->mp);
    grp->ocp->gp = gp;

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
    opcode_context_id_assign(grp->ocp, id_targets, id_variable_new(&wl), -1);
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
    opcode_context_id_assign(grp->ocp, id_younger, id_variable_new(&wl), -1);
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
    if (grp->rp->pos.pos_line)
    {
        string_ty       *tmp;

        assert(grp->rp->pos.pos_name);
        tmp = str_quote_shell(grp->rp->pos.pos_name);
        printf("\n#line %d %s\n", grp->rp->pos.pos_line, tmp->str_text);
        str_free(tmp);
    }

    /*
     * print the test to see if this recipe should be run
     */
    if (!forced)
    {
        printf("if test");
        for (j = 0; j < grp->output->nfiles; ++j)
        {
            string_ty       *js;
            size_t          k;

            js = str_quote_shell(grp->output->item[j].file->filename);
            if (j)
                printf(" \\\n  -o");
            /* target does not exist */
            printf(" ! -e %s", js->str_text);
            for (k = 0; k < grp->input->nfiles; ++k)
            {
                graph_file_and_type_ty *gftp;
                graph_file_ty   *gfp;
                string_ty       *ingr;

                /* ingredient is newer than target */
                gftp = grp->input->item + k;
                gfp = gftp->file;
                ingr = str_quote_shell(gfp->filename);
                switch (gftp->edge_type)
                {
                case edge_type_exists:
                    /*
                     * The "exists" edge type is
                     * merely an ordering constraint.
                     * It guarantees that the
                     * ingredient will be up to date
                     * before this recipie's body is
                     * evaluated.  The actual relative
                     * ages of the ingredient and
                     * the target don't come into it.
                     */
                    break;

                case edge_type_weak:
                    /*
                     * The "weak" edge type allows
                     * two files to have the same time
                     * stamp, and still be up-to-date.
                     */
                    printf(" \\\n  -o %s -nt %s", ingr->str_text, js->str_text);
                    break;

                case edge_type_strict:
                case edge_type_default:
                    /*
                     * The "strict" edge type requires
                     * two files to have distinct
                     * time stamps.
                     */
                    printf
                        (" \\\n  -o ! %s -nt %s", js->str_text, ingr->str_text);
                    break;
                }
                str_free(ingr);
            }
            str_free(js);
        }
        printf("\nthen\n");
        file_pos = ftell(stdout);
    }

    /*
     * See if we need to perform the actions attached to this recipe.
     */
    if (grp->rp->out_of_date)
    {
        int             echo;

        trace(("do recipe body\n"));
        echo = !option_test(OPTION_SILENT);
        if (option_test(OPTION_MKDIR))
        {
            for (j = 0; j < grp->output->nfiles; ++j)
            {
                graph_file_ty   *gfp;
                string_ty       *s;
                string_ty       *tmp;

                gfp = grp->output->item[j].file;
                s = dir_part(gfp->filename);
                if (!s)
                    continue;
                tmp = str_quote_shell(s);
                str_free(s);
                printf("if test ! -d %s; then\n", tmp->str_text);
                if (echo)
                {
                    printf("echo mkdir -p %s\n", tmp->str_text);
                }
                printf("mkdir -p %s", tmp->str_text);
                if (!option_test(OPTION_ERROK))
                    printf(" || exit 1");
                printf("\nfi\n");
                str_free(tmp);
            }
        }
        if (option_test(OPTION_UNLINK))
        {
            for (j = 0; j < grp->output->nfiles; ++j)
            {
                graph_file_ty   *gfp;
                string_ty       *tmp;

                gfp = grp->output->item[j].file;
                tmp = str_quote_shell(gfp->filename);
                if (echo)
                    printf("echo rm %s\n", tmp->str_text);
                printf("rm %s", tmp->str_text);
                if (!option_test(OPTION_ERROK))
                    printf(" || exit 1");
                printf("\n");
                str_free(tmp);
            }
        }
        if (option_test(OPTION_TOUCH))
        {
            for (j = 0; j < grp->output->nfiles; ++j)
            {
                graph_file_ty   *gfp;
                string_ty       *tmp;

                gfp = grp->output->item[j].file;
                tmp = str_quote_shell(gfp->filename);
                if (echo)
                {
                    printf("echo touch %s\n", tmp->str_text);
                }
                printf("touch %s", tmp->str_text);
                if (!option_test(OPTION_ERROK))
                    printf(" || exit 1");
                printf("\n");
                str_free(tmp);
            }
        }
        else
        {
            opcode_status_ty status2;

            trace(("doing it now\n"));
            opcode_context_call(grp->ocp, grp->rp->out_of_date);
            status2 = opcode_context_script(grp->ocp);
            if (status2 != opcode_status_success)
                status = graph_walk_status_error;
        }
    }

    /*
     * This recipe is being used, so
     * perform its 'use' action.
     *
     * Ignore the 'touch' option,
     * ignore the 'errok' option,
     * don't delete files on errors.
     *
     * Note: it looks odd to have the "else" in the script.  This is
     * because the use clause is *duplicated* in the compiled
     * opcode stream for the out-of-date case.
     */
    if (grp->rp->up_to_date)
    {
        opcode_status_ty status2;

        trace(("perform ``use'' clause\n"));
        if (!forced)
        {
            if (file_pos == ftell(stdout))
                printf(":\n");
            printf("else\n");
            file_pos = ftell(stdout);
        }
        opcode_context_call(grp->ocp, grp->rp->up_to_date);
        status2 = opcode_context_script(grp->ocp);
        if (status2 != opcode_status_success)
            status = graph_walk_status_error;
    }

    /*
     * finish the conditional around this recipe
     */
    if (!forced)
    {
        if (file_pos == ftell(stdout))
            printf(":\n");
        printf("fi\n");
    }

    /*
     * cancel the recipe flags
     */
    option_undo_level(OPTION_LEVEL_RECIPE);
    if (grp->ocp)
    {
        opcode_context_delete(grp->ocp);
        grp->ocp = 0;
    }

    star_as_specified('*');
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}
