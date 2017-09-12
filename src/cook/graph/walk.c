/*
 *      cook - file construction tool
 *      Copyright (C) 1997-2001, 2003, 2006-2009 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/time.h>
#include <sys/wait.h>

#include <cook/desist.h>
#include <common/error_intl.h>
#include <cook/fingerprint/sync.h>
#include <cook/graph.h>
#include <cook/graph/check.h>
#include <cook/graph/file.h>
#include <cook/graph/file_list.h>
#include <cook/graph/pairs.h>
#include <cook/graph/recipe.h>
#include <cook/graph/recipe_list.h>
#include <cook/graph/run.h>
#include <cook/graph/script.h>
#include <cook/graph/walk.h>
#include <cook/id.h>
#include <cook/id/variable.h>
#include <common/itab.h>
#include <cook/meter.h>
#include <cook/opcode/context.h>
#include <cook/option.h>
#include <cook/os/wait.h>
#include <cook/recipe.h>        /* for tracing */
#include <common/star.h>
#include <common/symtab.h>
#include <common/trace.h>


/*
 * NAME
 *      graph_walk_status_name
 *
 * SYNOPSIS
 *      char *graph_walk_status_name(graph_walk_status_ty);
 *
 * DESCRIPTION
 *      The graph_walk_status_name function is used to turn a status
 *      indicator into a string.  Used for debugging, so that the trace
 *      output is more useful.
 *
 * RETURNS
 *      char *; do NOT free
 */

#ifdef DEBUG

char *
graph_walk_status_name(graph_walk_status_ty n)
{
    switch (n)
    {
    case graph_walk_status_uptodate:
        return "uptodate";

    case graph_walk_status_uptodate_done:
        return "uptodate_done";

    case graph_walk_status_done:
        return "done";

    case graph_walk_status_done_stop:
        return "done_stop";

    case graph_walk_status_error:
        return "error";

    case graph_walk_status_wait:
        return "wait";
    }
    return "unknown";
}

#endif


/*
 * NAME
 *      implications_of_file
 *
 * SYNOPSIS
 *      void implications_of_file(graph_recipe_list_nrc_ty *candidates,
 *              graph_file_ty *gfp, int uptodate);
 *
 * DESCRIPTION
 *      The implications_of_file function is used to explore the
 *      implications of the given file being up-to-date (or done).
 *      If recipes dependent on his file have all of their inputs
 *      satisfied, they are appended to the candidates list.
 *
 *
 * ARGUMENTS
 *      candidates      The list of recipes to be walked.  Appending a
 *                      recipe to this list will have it walked in turn.
 *      gfp             The graph file to explore.
 *      uptodate        True (non-zero) if the file is up-to-date,
 *                      false otherwise.
 */

static void
implications_of_file(graph_recipe_list_nrc_ty *walk, graph_file_ty *gfp,
    int uptodate)
{
    size_t          k;
    graph_recipe_ty *grp2;

    trace(("implications_of_file(walk = %p, gfp = %p)\n{\n", walk, gfp));
    gfp->input_satisfied++;
    if (uptodate)
        gfp->input_uptodate++;
    trace(("walked \"%s\" file (%ldth of %ld recipes)\n",
        gfp->filename->str_text, (long)gfp->input_satisfied,
        (long)gfp->input->nrecipes));
    if (gfp->input_satisfied < gfp->input->nrecipes)
    {
        if (!uptodate)
        {
            /*
             * If a file is constructed by more than one
             * recipe, the mod-time will be updated, making
             * the rest of the recipes (potentially) decide
             * they have nothing to do.  Set their "force"
             * flag to make sure they do something.
             */
            for (k = 0; k < gfp->input->nrecipes; ++k)
            {
                grp2 = gfp->input->recipe[k];
                assert(grp2);
                grp2->multi_forced = 1;
            }
        }

        trace(("...come back later\n"));
        trace(("}\n"));
        return;
    }

    /*
     * All of the input to the file are
     * satisfied.  Are all of the inputs to
     * the implied recipes satisfied?
     */
    trace(("check %ld recipe outputs\n", (long)gfp->output->nrecipes));
    for (k = 0; k < gfp->output->nrecipes; ++k)
    {
        grp2 = gfp->output->recipe[k];
        assert(grp2);
        grp2->input_satisfied++;
        if (gfp->input_uptodate == gfp->input_satisfied)
            grp2->input_uptodate++;
        trace(("walked %s:%d recipe (%ldth of %ld files)\n",
            (grp2->rp->pos.pos_name ? grp2->rp->pos.pos_name->str_text : ""),
            (int)grp2->rp->pos.pos_line,
            (long)grp2->input_satisfied,
            (long)grp2->input->nfiles));
        if (grp2->input_satisfied < grp2->input->nfiles)
        {
            trace(("...come back later\n"));
            continue;
        }

        /*
         * You always push the recipe, even if you think
         * everything is up-to-date.  It may have a
         * use-clause, it may be a script or some other
         * traversal.  Always push.
         */
        trace(("recipe ingredients satisfied, push %p\n", grp2));
        graph_recipe_list_nrc_append(walk, grp2);
    }

    /*
     * Print a nice warm fuzzy message when a top-level target
     * needs no work.
     */
    if
    (
        gfp->primary_target
    &&
        gfp->input_uptodate >= gfp->input->nrecipes
    &&
        !option_test(OPTION_SILENT)
    )
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", gfp->filename);
        error_intl(scp, i18n("$filename: already up to date"));
        sub_context_delete(scp);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      excuse_me
 *
 * SYNOPSIS
 *      void excuse_me(symtab_ty *, string_ty *, void *, void *);
 *
 * DESCRIPTION
 *      The excuse_me function is used to inform the user of targets
 *      rerqueted by not built when errors are found while building the
 *      targets.
 *
 * CAVEAT
 *      The arguments are dictated by the symtab_walk function.
 */

static void
excuse_me(symtab_ty *stp, string_ty *key, void *data, void *aux)
{
    graph_ty        *gp;
    graph_file_ty   *gfp;

    trace(("excuse_me()\n{\n"));
    (void)stp;
    (void)key;
    gp = aux;
    gfp = data;

    /*
     * Print a nice warm fuzzy message when a top-level target
     * can not be built.
     */
    if (gfp->primary_target && gfp->input_satisfied < gfp->input->nrecipes)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", gfp->filename);
        error_intl(scp, i18n("$filename: not done because of errors"));
        sub_context_delete(scp);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      is_it_a_leaf
 *
 * SYNOPSIS
 *      void is_it_a_leaf(symtab_ty *stp, string_ty *key, void *data,
 *              void *aux);
 *
 * DESCRIPTION
 *      The is_it_a_leaf function is used to examine each graph file and
 *      decide if it is a leaf (i.e. it has outputs but no inputs).
 *      These files are ``uptodate'' by definition, and their
 *      implications are explored in oreder to construct the initial
 *      recipe candidate list.
 *
 * CAVEAT
 *      This is a callback used with the symtab_walk function.  The
 *      argument list is is dictated by symtab_walk.
 */

static void
is_it_a_leaf(symtab_ty *stp, string_ty *filename, void *data, void *aux)
{
    graph_file_ty   *gfp;
    graph_recipe_list_nrc_ty *walk;

    (void)stp;
    (void)filename;
    gfp = data;
    gfp->done = 0;
    gfp->input_satisfied = 0;
    gfp->input_uptodate = 0;

    walk = aux;
    if
    (
        gfp->input->nrecipes == 0
    &&
        gfp->output->nrecipes != 0
    &&
        !gfp->previous_error
    &&
        !gfp->previous_backtrack
    )
    {
        trace(("leaf file \"%s\"\n", filename->str_text));
        implications_of_file(walk, gfp, 1);
        star_as_specified('#');
    }
}


/*
 * NAME
 *      implications_of_recipe
 *
 * SYNOPSIS
 *      void implications_of_recipe(graph_recipe_list_nrc_ty *candidates,
 *              graph_recipe_ty *gfp, int uptodate);
 *
 * DESCRIPTION
 *      The implications_of_recipe function is used to explore the
 *      implications of the given recipe returning an up-to-date (or
 *      done) status.  If recipes dependent on the outputs of this
 *      recipe have all of their inputs satisfied, they are appended to
 *      the candidates list.
 *
 *
 * ARGUMENTS
 *      candidates      The list of recipes to be walked.  Appending a
 *                      recipe to this list will have it walked in turn.
 *      grp             The recipe who's outputs are to be explored.
 *      uptodate        True (non-zero) if the file is up-to-date,
 *                      false otherwise.
 */

static void
implications_of_recipe(graph_recipe_list_nrc_ty *walk, graph_recipe_ty *grp,
    int uptodate)
{
    size_t          j;

    trace(("implications_of_recipe(walk = %p, grp = %p)\n{\n", walk, grp));
    trace(("check %ld file outputs\n", (long)grp->output->nfiles));
    for (j = 0; j < grp->output->nfiles; ++j)
    {
        graph_file_ty   *gfp;

        gfp = grp->output->item[j].file;
        implications_of_file(walk, gfp, uptodate);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      graph_walk_inner
 *
 * SYNOPSIS
 *      graph_walk_status_ty graph_walk_inner(graph_ty *gp,
 *              graph_walk_status_ty (*func)(graph_recipe_ty *));
 *
 * DESCRIPTION
 *      The graph_walk_inner function is used to walk the given graph.
 *      The actions taken when walking the graph may be customized by
 *      supplying different function pointers.
 *
 * ARGUMENTS
 *      gp      The graph to be walked
 *      func    The function to walk each recipe with.
 *
 * RETURNS
 *      graph_walk_status_ty
 *              error           something went wrong
 *              uptodate        no action required
 *              uptodate_done   fingerprints indicate the file did not change
 *              done            targets are out of date, because the
 *                              recipe body was run
 *              done_stop       targets are out of date, no action taken
 *                              (used by graph_check).
 */

static graph_walk_status_ty
graph_walk_inner(graph_ty *gp,
    graph_walk_status_ty (*func)(graph_recipe_ty *, graph_ty *),
    int nproc)
{
    graph_recipe_list_nrc_ty walk;
    graph_walk_status_ty status;
    graph_walk_status_ty status2;
    graph_recipe_ty *grp;
    size_t          j;
    size_t          walk_pos;
    itab_ty         *itp;
    string_list_ty  single_thread;

    trace(("graph_walk(gp = %p, nproc = %d)\n{\n", gp, nproc));
    status = graph_walk_status_uptodate;
    itp = itab_alloc(nproc);

    /*
     * Reset the activity counter for each recipe.
     *
     * Recipes with no inputs are added to the list at this time,
     * all of their inputs are satisfied.
     */
    graph_recipe_list_nrc_constructor(&walk);
    for (j = 0; j < gp->already_recipe->nrecipes; ++j)
    {
        grp = gp->already_recipe->recipe[j];
        grp->input_satisfied = 0;
        grp->input_uptodate = 0;

        if (grp->output->nfiles != 0 && grp->input->nfiles == 0)
        {
            grp->input_uptodate = 1;
            graph_recipe_list_nrc_append(&walk, grp);
        }
    }

    /*
     * Turn the list upside down.  We want to find all of the files
     * with outputs but no inputs.  This is the initial list of file
     * nodes to walk.
     */
    symtab_walk(gp->already, is_it_a_leaf, &walk);

    /*
     * Keep chewing up graph recipe nodes until no more are left to
     * be processed.
     */
    string_list_constructor(&single_thread);
    walk_pos = 0;
    while (walk_pos < walk.nrecipes || itp->load > 0)
    {
        /*
         * Terminate elegantly, if asked to.
         */
        if (desist_requested())
        {
          desist:
            status = graph_walk_status_error;
            if (itp->load > 0 && !option_test(OPTION_SILENT))
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set(scp, "Number", "%ld", (long)itp->load);
                sub_var_optional(scp, "Number");
                error_intl(scp, i18n("waiting for outstanding processes"));
                sub_context_delete(scp);
            }

            /*
             * No matter what, don't do another recipe.
             * However, we must still wait for the
             * unfinished actions to complete in an orderly
             * fashion.
             */
          no_persevere:
            assert(gp->already_recipe);
            walk_pos = gp->already_recipe->nrecipes * 2;
            continue;
        }

        /*
         * If there is available processing resource, and
         * outstanding recipe instances, run another recipe.
         */
        trace(("itp->load = %ld;\n", (long)itp->load));
        trace(("nproc = %d;\n", nproc));
        trace(("walk_pos = %ld;\n", (long)walk_pos));
        trace(("walk.nrecipes = %ld;\n", (long)walk.nrecipes));
        while (itp->load < nproc && walk_pos < walk.nrecipes)
        {
            if (desist_requested())
                goto desist;
            fp_sync();

            /*
             * Extract a recipe from the list.  Order does
             * not matter, they are all valid candidates
             * with up-to-date ingredients.
             *
             * However: the users expect a mostly
             * left-to-right order of evaluation.  That
             * means taking the first one, NOT the last one.
             */
            grp = walk.recipe[walk_pos++];

            /*
             * Make sure there is no conflict with existing
             * single thread flags.  Go hunting for a recipe
             * not in conflict.  Come back later if we can't
             * find one.
             */
            if
            (
                grp->single_thread
            &&
                string_list_intersect(grp->single_thread, &single_thread)
            )
            {
                size_t          k;
                graph_recipe_ty *kp;

                /*
                 * go hunting
                 */
                for (k = walk_pos; k < walk.nrecipes; ++k)
                {
                    kp = walk.recipe[k];
                    if
                    (
                        !kp->single_thread
                    ||
                        !string_list_intersect
                        (
                            kp->single_thread,
                            &single_thread
                        )
                    )
                        break;
                }

                /*
                 * Come back later if we find no alternative.
                 */
                if (k >= walk.nrecipes)
                {
                    --walk_pos;
                    break;
                }

                /*
                 * Have the conflicting recipe and the
                 * alternative change places.
                 */
                kp = walk.recipe[k];
                trace(("k = %ld\n", (long)k));
                trace(("kp = %p\n", kp));
                walk.recipe[walk_pos - 1] = kp;
                walk.recipe[k] = grp;
                grp = kp;
            }
            trace(("grp = %p;\n", grp));
            trace(("grp->input->nfiles = %ld;\n", (long)grp->input->nfiles));
            trace(("grp->output->nfiles = %ld;\n", (long)grp->output->nfiles));

            /*
             * Remember the single threading, so other
             * recipes avoid conflicting with *this* one.
             */
            if (grp->single_thread)
            {
                string_list_append_list(&single_thread, grp->single_thread);
            }

            /*
             * run the recipe body
             */
          run_a_recipe:
            status2 = func(grp, gp);

            /*
             * Look at what happened.
             */
            if (grp->single_thread && status2 != graph_walk_status_wait)
            {
                string_list_remove_list(&single_thread, grp->single_thread);
            }
            switch (status2)
            {
            case graph_walk_status_wait:
                assert(itp);
                trace(("pid = %d;\n", graph_recipe_getpid(grp)));
                itab_assign(itp, graph_recipe_getpid(grp), grp);
                trace(("itp->load = %ld;\n", (long)itp->load));
                break;

            case graph_walk_status_error:
                /*
                 * It failed.  Don't do anything with the
                 * outputs of the recipe.  Usually, we stop
                 * altogether.
                 */
                trace(("error\n"));
                status = graph_walk_status_error;
                if (!option_test(OPTION_PERSEVERE))
                    goto no_persevere;
                break;

            case graph_walk_status_done_stop:
                /*
                 * It worked, but we need to stop for
                 * some reason.  This is usually only
                 * used by isit_uptodate.
                 */
                trace(("done_stop\n"));
                if (status == graph_walk_status_uptodate)
                    status = graph_walk_status_done_stop;
                assert(itp->load == 0);
                goto done;

            case graph_walk_status_uptodate:
                star_as_specified('#');
                /* fall through... */

            case graph_walk_status_uptodate_done:
                /*
                 * It worked.  Now push all of the
                 * recipes which depend on the outputs
                 * of this recipe.
                 */
                implications_of_recipe(&walk, grp, 1);
                break;

            case graph_walk_status_done:
                /*
                 * It worked.  Now push all of the
                 * recipes which depend on the outputs
                 * of this recipe.
                 */
                implications_of_recipe(&walk, grp, 0);
                if (status == graph_walk_status_uptodate)
                    status = graph_walk_status_done;
                break;
            }

#ifdef HAVE_WAIT3
            /*
             * We want to see if any children have exited.
             * Do not block (that's why wait3 is used).
             * Only do one at a time.
             */
            if (itp->load > 0)
            {
                int             pid;
                int             exit_status;
                struct rusage   ru;

                trace(("mark\n"));
                pid = os_wait3(&exit_status, WNOHANG, &ru);
                trace(("pid = %d\n", pid));
                if (pid < 0)
                {
                    sub_context_ty  *scp;

                    if (errno == EINTR)
                        continue;
                    scp = sub_context_new();
                    sub_errno_set(scp);
                    fatal_intl(scp, i18n("wait(): $errno"));
                    /* NOTREACHED */
                }
                else if (pid > 0)
                {
                    trace(("es = 0x%04X\n", exit_status));
                    grp = itab_query(itp, pid);
                    if (grp)
                    {
                        /* if it's one of ours... */
                        trace(("...waited\n"));
                        assert(pid == graph_recipe_getpid(grp));
                        if (grp->ocp->meter_p)
                            grp->ocp->meter_p->ru = ru;
                        graph_recipe_waited(grp, exit_status);
                        itab_delete(itp, pid);
                        trace(("itp->load = %ld;\n", (long)itp->load));
                        goto run_a_recipe;
                    }
                }
            }
#endif /* HAVE_WAIT3 */
        }

        /*
         * Collect the results of execution, and kick off the
         * recipes that are blocked.  Only do one at a time.
         */
        if (itp->load > 0)
        {
            int             pid;
            int             exit_status;
#ifdef HAVE_WAIT3
            struct rusage   ru;
#endif

            trace(("mark\n"));
#ifdef HAVE_WAIT3
            pid = os_wait3(&exit_status, 0, &ru);
#else
            pid = os_wait(&exit_status);
#endif
            trace(("pid = %d\n", pid));
            assert(pid != 0);
            if (pid == 0)
                errno = EINVAL;
            if (pid <= 0)
            {
                sub_context_ty  *scp;

                if (errno == EINTR)
                    continue;
                scp = sub_context_new();
                sub_errno_set(scp);
                fatal_intl(scp, i18n("wait(): $errno"));
                /* NOTREACHED */
            }
            trace(("es = 0x%04X\n", exit_status));
            grp = itab_query(itp, pid);
            if (grp)
            {
                /* if it's one of ours... */
                trace(("...waited\n"));
                assert(pid == graph_recipe_getpid(grp));
#ifdef HAVE_WAIT3
                if (grp->ocp->meter_p)
                    grp->ocp->meter_p->ru = ru;
#endif
                graph_recipe_waited(grp, exit_status);
                itab_delete(itp, pid);
                trace(("itp->load = %ld;\n", (long)itp->load));
                goto run_a_recipe;
            }
        }
        trace(("mark\n"));
    }
  done:
    itab_free(itp);

    /*
     * Confirmation for the user when things go wrong.
     */
    if (status == graph_walk_status_error)
        symtab_walk(gp->already, excuse_me, gp);

    /*
     * Free up the list of recipes (which have been) / (to be) walked.
     */
    graph_recipe_list_nrc_destructor(&walk);
    string_list_destructor(&single_thread);

    trace(("return %s;\n", graph_walk_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      graph_walk
 *
 * SYNOPSIS
 *      graph_walk_status_ty graph_walk(graph_ty *);
 *
 * DESCRIPTION
 *      The graph_walk function is used to walk a graph re-building any
 *      out-of-date files.
 *
 * RETURNS
 *      graph_walk_status_ty
 *              error           something went wrong
 *              uptodate        no action required
 *              uptodate_done   fingerprints indicate the file did not change
 *              done            targets are out of date, because the
 *                              recipe body was run
 */

graph_walk_status_ty
graph_walk(graph_ty *gp)
{
    int             nproc;
    string_ty       *key;
    string_ty       *s;
    id_ty           *idp;
    string_list_ty  wl;
    opcode_context_ty *ocp;

    /*
     * see if the jobs variable is set
     */
    key = str_from_c("parallel_jobs");
    ocp = opcode_context_new(0, 0);
    idp = opcode_context_id_search(ocp, key);

    /*
     * extract the number of jobs
     */
    nproc = 1;
    if (idp)
    {
        id_variable_query(idp, &wl);
        if (wl.nstrings == 1)
        {
            nproc = atoi(wl.string[0]->str_text);
            if (nproc < 1)
                nproc = 1;
        }
        string_list_destructor(&wl);
    }

    /*
     * Set the jobs variable to precisely reflect what we are going
     * to do, should the recipes use it in some way.
     */
    string_list_constructor(&wl);
    s = str_format("%d", nproc);
    string_list_append(&wl, s);
    str_free(s);
    opcode_context_id_assign(ocp, key, id_variable_new(&wl), 0);
    string_list_destructor(&wl);
    str_free(key);
    opcode_context_delete(ocp);

    /*
     * walk the graph
     */
    return graph_walk_inner(gp, graph_recipe_run, nproc);
}


/*
 * NAME
 *      graph_walk_pairs
 *
 * SYNOPSIS
 *      graph_walk_status_ty graph_walk_pairs(graph_ty *);
 *
 * DESCRIPTION
 *      The graph_walk_pairs function is used to walk a file dependency
 *      graph printing file dependency pairs on the standard output.
 *      The output is similar to lorder(1).  This can be used as input
 *      to drawing tools to visualize dependency graphs.
 *
 * RETURNS
 *      graph_walk_status_ty
 *              error           something went wrong
 *              uptodate        success
 *
 * CAVEAT
 *      Also useful for debugging cookbooks.
 */

graph_walk_status_ty
graph_walk_pairs(graph_ty *gp)
{
    return graph_walk_inner(gp, graph_recipe_pairs, 1);
}


/*
 * NAME
 *      graph_walk_script
 *
 * SYNOPSIS
 *      graph_walk_status_ty graph_walk_script(graph_ty *);
 *
 * DESCRIPTION
 *      The graph_walk_script function is used to walk a file dependency
 *      graph printing a shell scrtipt on the standard output.  This
 *      shell script will simulate the build for the specified targets.
 *      It does simplistic checking to see if the files need building.
 *
 * RETURNS
 *      graph_walk_status_ty
 *              error           something went wrong
 *              uptodate        success
 *
 * CAVEAT
 *      Also useful for debugging cookbooks.
 */

graph_walk_status_ty
graph_walk_script(graph_ty *gp)
{
    graph_walk_status_ty status;
    char            *cp;

    cp = getenv("SHELL");
    if (!cp || !*cp)
        cp = CONF_SHELL;
    printf("#!%s\n", cp);
    status = graph_walk_inner(gp, graph_recipe_script, 1);
    printf("exit 0\n");
    return status;
}


/*
 * NAME
 *      graph_isit_uptodate
 *
 * SYNOPSIS
 *      int graph_isit_uptodate(graph_ty *);
 *
 * DESCRIPTION
 *      The graph_isit_uptodate function is used to walk a file dependency
 *      graph to determine if it is up-to-date.
 *
 * RETURNS
 *      int;    -1      something went wrong
 *              0       targets are out of date, no action taken
 *              1       targets are up-to-date
 */

int
graph_isit_uptodate(graph_ty *gp)
{
    graph_walk_status_ty result;

    result = graph_walk_inner(gp, graph_recipe_check, 1);
    switch (result)
    {
    case graph_walk_status_wait:
        assert(0);
        /* fall through... */

    case graph_walk_status_error:
        return -1;

    case graph_walk_status_uptodate:
    case graph_walk_status_uptodate_done:
        return 1;

    case graph_walk_status_done:
    case graph_walk_status_done_stop:
        /*
         * Fall out into the answer we want.
         * This allows GCC to warn if we missed any cases.
         */
        break;
    }
    return 0;
}
