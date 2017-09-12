/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2003, 2004, 2006, 2007 Peter Miller;
 *      All rights reserved.
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

#include <common/ac/string.h>
#include <common/ac/unistd.h>   /* for getpid */

#include <common/error_intl.h>
#include <common/os_path_cat.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <cook/cook.h>
#include <cook/dir_part.h>
#include <cook/fingerprint.h>
#include <common/ts.h>
#include <cook/graph.h>
#include <cook/graph/file.h>
#include <cook/graph/file_list.h>
#include <cook/graph/file_pair.h>
#include <cook/graph/recipe.h>
#include <cook/graph/run.h>
#include <cook/id.h>
#include <cook/id/variable.h>
#include <cook/match.h>
#include <cook/opcode/context.h>
#include <cook/opcode/list.h>
#include <cook/option.h>
#include <cook/os_interface.h>
#include <cook/recipe.h>
#include <cook/stmt.h>


static int
make_target_directories(graph_file_list_nrc_ty *gflp)
{
    size_t          k;
    int             status;
    int             echo;
    int             errok;

    status = 0;
    echo = !option_test(OPTION_SILENT);
    errok = option_test(OPTION_ERROK);
    for (k = 0; k < gflp->nfiles; ++k)
    {
        graph_file_and_type_ty *gftp;
        graph_file_ty   *gfp;
        string_ty       *s;

        gftp = gflp->item + k;
        gfp = gftp->file;
        s = dir_part(gfp->filename);
        if (s)
        {
            if (os_mkdir(s, echo) && !errok)
                status = -1;
            str_free(s);
        }
    }
    return status;
}


static string_ty *
host_binding_round_robin(opcode_context_ty *ocp, string_list_ty *slp)
{
    static int      j;

    if (!j)
        j = getpid();
    if (!slp || !slp->nstrings)
    {
        static string_ty *key;
        id_ty           *idp;

        if (!key)
            key = str_from_c("parallel_hosts");
        idp = opcode_context_id_search(ocp, key);
        if (!idp)
            return 0;
        slp = id_variable_query2(idp);
        if (!slp || !slp->nstrings)
            return 0;
    }
    return slp->string[j++ % slp->nstrings];
}


/**
  * The relevate funtion is used to make a symbolic link relative to its
  * destintion, rather than relative to dot (the current directory).
  */
static string_ty *
relevate(string_ty *from, string_ty *to)
{
    const char      *cp;

    if (from->str_text[0] == '/')
        return str_copy(from);
    if (to->str_text[0] == '/')
        return os_path_cat(os_curdir(), from);
    cp = to->str_text;
    from = str_copy(from);
    for (;;)
    {
        string_ty       *s2;

        cp = strchr(cp, '/');
        if (!cp)
            return from;
        for (;;)
        {
            ++cp;
            if (*cp != '/')
                break;
        }
        if (!*cp)
            return from;

        s2 = str_format("../%s", from->str_text);
        str_free(from);
        from = s2;
    }
}


/*
 * NAME
 *      graph_recipe_run
 *
 * SYNOPSIS
 *      graph_walk_status_ty graph_recipe_run(graph_recipe_ty *);
 *
 * DESCRIPTION
 *      The graph_recipe_run function is used via graph_walk_inner to
 *      perform a recipe.  If the derived files are out-of-date, the
 *      recipe body will be run to bring them up-to-date.
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
graph_recipe_run(graph_recipe_ty *grp, graph_ty *gp)
{
    graph_walk_status_ty status;
    time_t          target_age;
    long            target_depth;
    string_ty       *target_absent;
    int             forced;
    string_list_ty  wl;
    string_list_ty  younger;
    int             show_reasoning;
    string_ty       *target1;
    time_t          need_age;
    size_t          j;
    size_t          k;
    int             phony;
    sub_context_ty  *scp;
    time_t          timestamp_granularity;

    trace(("graph_recipe_run(grp = %08lX)\n{\n", (long)grp));
    status = graph_walk_status_uptodate;
    timestamp_granularity = ts_granularity();

    if (grp->ocp)
    {
        need_age = grp->ocp->need_age;
        opcode_context_resume(grp->ocp);
        status = graph_walk_status_done;
        goto resume;
    }
    need_age = 0;
    phony = !grp->rp->out_of_date;

    /*
     * create the opcode context for this recipe
     */
    grp->ocp = opcode_context_new(0, grp->mp);
    grp->ocp->gp = gp;

    /*
     * Warn about essential information which is kept only in
     * derived files.
     */
    if (gp->file_pair)
    {
        for (j = 0; j < grp->output->nfiles; ++j)
        {
            target1 = grp->output->item[j].file->filename;
            for (k = 0; k < grp->input->nfiles; ++k)
            {
                graph_file_pair_check
                (
                    gp->file_pair,
                    target1,
                    grp->input->item[k].file->filename,
                    gp
                );
            }
        }
    }

    /*
     * construct the ``target'' variable
     */
    string_list_constructor(&wl);
    assert(grp->output);
    assert(grp->output->nfiles > 0);
    if (grp->output->nfiles > 0)
    {
        target1 = grp->output->item[0].file->filename;
        string_list_append(&wl, target1);
    }
    else
        target1 = str_from_c("\7bogus\7");      /* mem leak */
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
     */
    string_list_constructor(&wl);
    assert(grp->input);
    for (j = 0; j < grp->input->nfiles; ++j)
        string_list_append(&wl, grp->input->item[j].file->filename);
    opcode_context_id_assign(grp->ocp, id_need, id_variable_new(&wl), -1);
    string_list_destructor(&wl);

    /*
     * Constructing the ``younger'' variable takes quite a bit
     * longer because we need to consult the file modification
     * times.  [[Fake an assignment to avoid problems with errors.]]
     */
    string_list_constructor(&younger);
    opcode_context_id_assign
    (
        grp->ocp,
        id_younger,
        id_variable_new(&younger),
        -1
    );

    /*
     * Flags apply to the precondition and to the ingredients
     * evaluation.  That is why the grammar puts them first.
     */
    recipe_flags_set(grp->rp);
    show_reasoning = option_test(OPTION_REASON);

    /*
     * see of the recipe is forced to activate
     */
    forced = option_test(OPTION_FORCE) | grp->multi_forced;
    if (forced && show_reasoning)
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", target1);
        error_with_position
        (
            &grp->rp->pos,
            scp,
            i18n("\"$filename\" is out of date because the \"forced\" flag is "
                "set (reason)")
        );
        sub_context_delete(scp);
    }

    /*
     * By taking a fingerprint of the ingredients, we can cause
     * the recipe to trigger when the number of ingredients change,
     * or when one ingredient is substituted for another, EVEN IF
     * the time stamps are all consistent.  (This is especially
     * important when an ingredient is *removed* from a library.)
     */
    if (option_test(OPTION_INGREDIENTS_FINGERPRINT))
    {
        string_list_ty  ingr;
        string_ty       *ingr2;
        string_ty       *ingr_fp;

        string_list_constructor(&ingr);
        for (j = 0; j < grp->input->nfiles; ++j)
        {
            graph_file_ty   *gfp;

            gfp = grp->input->item[j].file;
            string_list_append(&ingr, gfp->filename);
        }
        string_list_sort(&ingr);
        ingr2 = wl2str(&ingr, 0, ingr.nstrings, "\n");
        string_list_destructor(&ingr);
        ingr_fp = fp_fingerprint_string(ingr2);
        str_free(ingr2);

        /*
         * Not only does the fp_ingredients_fingerprint_differs
         * function check to see if it is different, it also
         * updates the cache files, so that it will be written
         * out at the next opportunity.
         */
        if (fp_ingredients_fingerprint_differs(target1, ingr_fp))
        {
            forced = 1;

            if (show_reasoning)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", target1);
                error_with_position
                (
                    &grp->rp->pos,
                    scp,
                    i18n("\"$filename\" is out of date because the ingredients "
                        "changed (reason)")
                );
                sub_context_delete(scp);
            }
        }
        str_free(ingr_fp);
    }

    /*
     * age should be set to the worst case of all the targets
     *
     * The depth of the target search should be less than or equal
     * to the depth of the worst (shallowest) ingredients search.
     * This is to guarantee that when ingredients change they
     * result in targets shallower in the path being updated.
     */
    target_age = 0;
    target_absent = 0;
    target_depth = 32767;
    for (j = 0; j < grp->output->nfiles; ++j)
    {
        graph_file_ty   *gfp2;
        time_t          age2;
        long            depth2;

        gfp2 = grp->output->item[j].file;

        /*
         * Remember the oldest mtime for later, when we compare
         * them to see if it changed.  Only do this for
         * fingerprints - they will stay the same even when the
         * file is re-written.
         */
        if (option_test(OPTION_FINGERPRINT))
        {
            depth2 = 32767;
            age2 = cook_mtime_oldest(grp->ocp, gfp2->filename, &depth2, depth2);
            if (age2 < 0)
            {
                /*
                 * Error message already printed.
                 *
                 * cook_mtime_oldest calls os_mtime_oldest
                 * os_mtime_oldest calls stat_cache_oldest
                 * stat_cache_oldest calls stat_cache
                 * stat_cache_oldest prints the file time traces
                 *
                 * stat_cache calls lstat
                 * if there is an error, and the error isn't ENOENT or
                 * ENOTDIR, stat_cache calls error_intl_stat (which
                 * calls error_intl) to print it.
                 */
                trace(("Error message already printed?\n"));
                status = graph_walk_status_error;
                goto ret;
            }
            gfp2->mtime_oldest = age2;
        }

        depth2 = 32767;
        age2 = cook_mtime_newest(grp->ocp, gfp2->filename, &depth2, depth2);
        if (age2 < 0)
        {
            /*
             * Error message already printed.
             *
             * cook_time_newest calls os_mtime_newest
             * os_mtime_newest calls stat_cache_newest
             * stat_cache_newest calls stat_cache
             * stat_cache_newest prints the file time traces
             *
             * stat_cache calls lstat
             * if there is an error, and the error isn't ENOENT or
             * ENOTDIR, stat_cache calls error_intl_stat (which calls
             * error_intl) to print it.
             */
            trace(("Error message already printed?\n"));
            status = graph_walk_status_error;
            goto ret;
        }
        if (age2 == 0)
            target_absent = gfp2->filename;
        else
        {
            if (depth2 < target_depth)
                target_depth = depth2;
            if (!target_age || age2 < target_age)
                target_age = age2;
        }
    }
    if (!forced && target_absent && !phony)
    {
        if (show_reasoning)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", target_absent);
            error_with_position
            (
                &grp->rp->pos,
                scp,
                i18n("\"$filename\" is out of date because it does not exist "
                    "(reason)")
            );
            sub_context_delete(scp);
        }
        forced = 1;
    }
    if (!forced && target_depth > 0 && option_test(OPTION_SHALLOW))
    {
        if (show_reasoning)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", target1);
            error_with_position
            (
                &grp->rp->pos,
                scp,
                i18n("\"$filename\" is out of date because it is too deep "
                    "(reason)")
            );
            sub_context_delete(scp);
        }
        forced = 1;
    }
    if (forced)
    {
        /* make sure ``younger'' contains all ingredients */
        target_age = 0;
        target_depth = 0;
    }
    trace(("forced = %d;\n", forced));
    trace(("target_depth = %d;\n", target_depth));
    trace(("target_age = %ld;\n", (long)target_age));

    /*
     * Look at the mtimes for each of the ingredients.
     */
    need_age = 0;
    for (j = 0; j < grp->input->nfiles; ++j)
    {
        graph_file_and_type_ty *gftp2;
        graph_file_ty   *gfp2;
        time_t          age2;
        long            depth2;
        int             do_this_one;

        gftp2 = grp->input->item + j;
        gfp2 = gftp2->file;
        trace(("%d/%d \"%s\"\n", j, grp->input->nfiles,
                gfp2->filename->str_text));
        depth2 = 32767;
        age2 =
            cook_mtime_oldest
            (
                grp->ocp,
                gfp2->filename,
                &depth2,
                target_depth + 1
            );
        if (age2 < 0)
        {
            /*
             * Error message already printed.
             *
             *
             * cook_mtime_oldest calls os_mtime_oldest
             * os_mtime_oldest calls stat_cache_oldest
             * stat_cache_oldest calls stat_cache
             * stat_cache_oldest prints the file time traces
             *
             * stat_cache calls lstat
             * If there is an error, and the error isn't ENOENT or
             * ENOTDIR, stat_cache calls error_intl_stat (which calls
             * error_intl) to print it.
             */
            trace(("Error message already printed?\n"));
            status = graph_walk_status_error;
            goto ret;
        }

        /*
         * track the youngest ingredient,
         * in case we need to adjust the targets' times
         */
        if (age2 > need_age)
            need_age = age2;

        /*
         * This function is only called AFTER an ingredient has
         * been derived. It will exist (well, almost: it could
         * be a phony) and so the mtime in the stat cache will
         * have been set.
         *
        assert(age2 != 0);
         */

        /*
         * Check to see if this ingredient invalidates the
         * target, based on its age.  (Don't say anything if we
         * already know it's out of date.)
         */
        if (gfp2->done && gftp2->edge_type != edge_type_exists)
        {
            if (!forced)
            {
                if (show_reasoning)
                {
                    scp = sub_context_new();
                    sub_var_set_string(scp, "File_Name1", target1);
                    sub_var_set_string(scp, "File_Name2", gfp2->filename);
                    error_with_position
                    (
                        &grp->rp->pos,
                        scp,
                        i18n("$filename1 is out of date because $filename2 was "
                            "cooked and is now younger (reason)")
                    );
                    sub_context_delete(scp);
                }
                forced = 1;
            }
            string_list_append_unique(&younger, gfp2->filename);
        }

        /*
         * Check to see if this ingredient invalidates the
         * target, based on its age.  (Don't say anything if we
         * already know it's out of date.)
         * The edge type is relevant to the calculation.
         */
        do_this_one = 0;
        switch (gftp2->edge_type)
        {
        case edge_type_exists:
            /*
             * The "exists" edge type is merely an ordering
             * constraint.  It guarantees that the ingredient
             * will be up to date before this recipie's body
             * is evaluated.  The actual relative ages of the
             * ingredient and the target don't come into it.
             *
             * Also, the depth is not a consideration
             * (otherwise this could imply a `set shallow'
             * that the user did not intend.
             */
            depth2 = 32767;
            break;

        case edge_type_weak:
            /*
             * The "weak" edge type allows two files to have
             * the same time stamp, and still be up-to-date.
             */
            do_this_one = age2 > target_age;
            break;

        case edge_type_strict:
        case edge_type_default:
            /*
             * The "strict" edge type requires two files to
             * have distinct time stamps.
             */
            do_this_one = age2 >= target_age;
            break;
        }
        if (do_this_one && !phony)
        {
            if (!forced)
            {
                if (show_reasoning)
                {
                    scp = sub_context_new();
                    sub_var_set_string(scp, "File_Name1", target1);
                    sub_var_set_string(scp, "File_Name2", gfp2->filename);
                    error_with_position
                    (
                        &grp->rp->pos,
                        scp,
                        i18n("$filename1 is out of date because $filename2 is "
                            "younger (reason)")
                    );
                    sub_context_delete(scp);
                }
                forced = 1;
            }
            string_list_append_unique(&younger, gfp2->filename);
        }

        /*
         * Check to see if this ingredient invalidates the
         * target, based on its depth.  (Don't say anything if we
         * already know its out of date.)
         */
        if (depth2 < target_depth && !phony)
        {
            trace(("depth2 = %d;\n", depth2));
            if (!forced)
            {
                if (show_reasoning)
                {
                    scp = sub_context_new();
                    sub_var_set_string(scp, "File_Name1", target1);
                    sub_var_set_string(scp, "File_Name2", gfp2->filename);
                    error_with_position
                    (
                        &grp->rp->pos,
                        scp,
                        i18n("$filename1 is out of date because $filename2 is "
                            "shallower (reason)")
                    );
                    sub_context_delete(scp);
                }
                forced = 1;
            }
            string_list_append_unique(&younger, gfp2->filename);
        }
    }
    if (grp->input->nfiles == 0)
    {
        /*
         * If there are no input files, pretend that the
         * youngest ingredient is ``now'' if the file does not
         * exist, and a second older than the target if it does
         * exist.
         */
        if (forced)
            time(&need_age);
        else
            need_age = target_age - 1;
    }

    /*
     * Assign the ``younger'' variable.
     * (Use a normal assignment, we did the push already.)
     */
    trace(("mark\n"));
    opcode_context_id_assign
    (
        grp->ocp,
        id_younger,
        id_variable_new(&younger),
        -1
    );
    string_list_destructor(&younger);

    /*
     * See if we need to perform the actions attached to this recipe.
     */
    if (forced)
    {
        trace(("forced\n"));

        /*
         * Remember that we did something.
         */
        status = graph_walk_status_done;

        if (grp->rp->out_of_date)
        {
            /*
             * Make directories for the targets if asked to.
             */
            trace(("do recipe body\n"));
            if
            (
                option_test(OPTION_MKDIR)
            &&
                make_target_directories(grp->output) < 0
            )
            {
                /*
                 * Error message already printed.
                 *
                 * make_target_directories calls os_mkdir
                 * os_mkdir calls mkdir
                 *
                 * If there is an error, and the error isn't
                 * EEXISTS, os_mkdir calls error_intl to report the
                 * error.
                 */
                trace(("Error message already printed?\n"));
                status = graph_walk_status_error;
                goto ret;
            }

            /*
             * Unlink the targets if asked to.
             */
            if (option_test(OPTION_UNLINK))
            {
                for (k = 0; k < grp->output->nfiles; ++k)
                {
                    graph_file_ty  *gfp2;

                    gfp2 = grp->output->item[k].file;
                    if
                    (
                        os_delete(gfp2->filename, !option_test(OPTION_SILENT))
                    &&
                        !option_test(OPTION_ERROK)
                    )
                    {
                        /*
                         * Error message already printed.
                         *
                         * os_delete calls unlink
                         *
                         * If there is an error, and the error isn't
                         * ENOENT, os_delete calls error_intl to report
                         * the error.
                         */
                        trace(("Error message already printed?\n"));
                        status = graph_walk_status_error;
                        goto ret;
                    }
                }
            }

            /*
             * Create symbolic links to ingredients of they are not
             * present in the top level directory of the search path.
             */
            if (option_test(OPTION_SYMLINK_INGREDIENTS))
            {
                for (k = 0; k < grp->input->nfiles; ++k)
                {
                    graph_file_ty  *gfp2;
                    string_ty      *from;
                    string_ty      *to;

                    gfp2 = grp->input->item[k].file;
                    to = gfp2->filename;
                    from = cook_mtime_resolve1(grp->ocp, to);
                    if (from && !str_equal(from, to))
                    {
                        string_ty       *from2;
                        int             echo;

                        from2 = relevate(from, to);
                        echo = !option_test(OPTION_SILENT);
                        os_symlink(from2, to, echo);
                        str_free(from2);
                    }
                    str_free(from);
                }
            }

            if (option_test(OPTION_TOUCH))
            {
                /*
                 * Touch the targets, if asked to touch
                 * rather than to build.
                 */
                for (k = 0; k < grp->output->nfiles; ++k)
                {
                    graph_file_ty  *gfp2;

                    gfp2 = grp->output->item[k].file;
                    if (!option_test(OPTION_SILENT))
                    {
                        scp = sub_context_new();
                        sub_var_set_string(scp, "File_Name", gfp2->filename);
                        error_intl(scp, i18n("touch $filename"));
                        sub_context_delete(scp);
                    }
                    if (os_touch(gfp2->filename))
                    {
                        /*
                         * Error message already printed.
                         *
                         * os_touch calls utime
                         *
                         * If there is an errror os_touch calls
                         * error_intl to report the error.
                         */
                        trace(("Error message already printed?\n"));
                        status = graph_walk_status_error;
                    }
                }
            }
            else
            {
                opcode_status_ty result;
                string_ty       *hostname;

                /*
                 * run the recipe body
                 */
                trace(("doing it now\n"));
                opcode_context_call(grp->ocp, grp->rp->out_of_date);
                hostname =
                    host_binding_round_robin(grp->ocp, grp->host_binding);
                if (hostname)
                {
                    opcode_context_host_binding_set(grp->ocp, hostname);
                }
              resume:
                result = opcode_context_execute(grp->ocp);
                switch (result)
                {
                case opcode_status_wait:
                    grp->ocp->need_age = need_age;
                    opcode_context_suspend(grp->ocp);
                    trace(("wait...\n"));
                    trace(("}\n"));
                    return graph_walk_status_wait;

                case opcode_status_success:
                    status = graph_walk_status_done;
                    break;

                case opcode_status_error:
                    if (option_test(OPTION_ERROK))
                        status = graph_walk_status_done;
                    else
                    {
                        /*
                         * Error message already printed.
                         *
                         * opcode_context_execute calls
                         *     opcode_context_execute_inner.
                         * opcode_context_execute_inner calls opcode_execute.
                         * opcode_execute calls the execute method of
                         *     the relevant opcode.
                         * All opcodes print an error message if they
                         * return opcde_status_error.
                         */
                        trace(("Error message already printed?\n"));
                        status = graph_walk_status_error;
                    }
                    break;

                case opcode_status_interrupted:
                    /*
                     * Error message already printed.
                     * by the interrupt handler.
                     */
                    trace(("Error message already printed?\n"));
                    status = graph_walk_status_error;
                    break;
                }

                /*
                 * Remove recipe targets on errors, unless asked to keep
                 * them around.  This ensures they will be built again;
                 * hopefully without errors the next time.  (Perfect
                 * cookbooks with exact dependencies don't need this,
                 * but users often omit dependencies; removing the file
                 * forces a re-build.)
                 */
                if
                (
                    status == graph_walk_status_error
                &&
                    !option_test(OPTION_PRECIOUS)
                )
                {
                    for (k = 0; k < grp->output->nfiles; ++k)
                    {
                        graph_file_ty   *gfp2;

                        gfp2 = grp->output->item[k].file;
                        os_delete(gfp2->filename, !option_test(OPTION_SILENT));
                    }
                }
            }
        }
        else
        {
            if (show_reasoning)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", target1);
                error_intl(scp, i18n("$filename is phony (reason)"));
                sub_context_delete(scp);
            }

            /*
             * remember that this ``file'' has been ``changed''
             */
            for (j = 0; j < grp->output->nfiles; ++j)
            {
                graph_file_ty   *gfp;

                gfp = grp->output->item[j].file;
                gfp->done++;
            }
        }
    }
    else
    {
        trace(("not forced\n"));
        if (show_reasoning)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", target1);
            error_intl(scp, i18n("$filename is up to date (reason)"));
            sub_context_delete(scp);
        }
        if (grp->rp->up_to_date)
        {
            opcode_status_ty result;

            /*
             * Make directories for the targets if asked to.
             * Often redundant, but it wont hurt.
             */
            if
            (
                option_test(OPTION_MKDIR)
            &&
                make_target_directories(grp->output) < 0
            )
            {
                /*
                 * Error message already printed.
                 *
                 * make_target_directories calls os_mkdir
                 * os_mkdir calls mkdir
                 *
                 * If there is an error, and the error isn't EEXISTS,
                 * os_mkdir calls error_intl to report the error.
                 */
                trace(("Error message already printed?\n"));
                status = graph_walk_status_error;
                goto ret;
            }

            /*
             * This feature isn't used often, don't worry
             * about making it parallel.
             */
            trace(("perform ``use'' clause\n"));
            opcode_context_call(grp->ocp, grp->rp->up_to_date);
            result = opcode_context_execute_nowait(grp->ocp);
            if (result != opcode_status_success)
            {
                /*
                 * Error message already printed.
                 *
                 * opcode_context_execute_nowait calls opcode_context_execute.
                 * opcode_context_execute calls opcode_context_execute_inner.
                 * opcode_context_execute_inner calls opcode_execute.
                 * opcode_execute calls the execute method of the relevant
                 *     opcode.
                 * All opcodes print an error message if they return
                 * opcde_status_error.
                 */
                trace(("Error message already printed?\n"));
                status = graph_walk_status_error;
            }
        }
    }

    /*
     * adjust file modification times
     * (tracking fingerprints as we go)
     */
  ret:
    trace(("mark\n"));
    if (status == graph_walk_status_done && grp->rp->out_of_date)
    {
        time_t          mtime;

        /*
         * The output files need to be at least this date
         * stamp to be mtime-consistent with the inputs.
         */
        mtime = need_age + timestamp_granularity;
        if (option_test(OPTION_FINGERPRINT))
        {
            /*
             * Update the times, and see if the pringerprint
             * changed.  If the fingerprint did not change
             * on any target, use the uptodate_done result,
             * otherwise use the done result.
             */
            status = graph_walk_status_uptodate_done;
            for (j = 0; j < grp->output->nfiles; ++j)
            {
                graph_file_ty   *gfp;
                time_t          t;
                long            depth4;

                gfp = grp->output->item[j].file;
                os_clear_stat(gfp->filename);
                if (os_mtime_adjust(gfp->filename, mtime))
                {
                    /*
                     * Error message already printed.
                     *
                     * os_mtime_adjust calls utime.
                     * If there is an error, os_mtime_sdjust calls
                     * error_intl top report the error.
                     */
                    trace(("Error message already printed?\n"));
                    status = graph_walk_status_error;
                    /*
                     * don't break here, need to
                     * update all of them
                     */
                    continue;
                }
                depth4 = 32767;
                t = cook_mtime_oldest(grp->ocp, gfp->filename, &depth4, depth4);
                if (t < 0)
                {
                    /*
                     * Error message already printed.
                     *
                     * cook_mtime_oldest calls os_mtime_oldest
                     * os_mtime_oldest calls stat_cache_oldest
                     * stat_cache_oldest calls stat_cache
                     * stat_cache_oldest prints the file time traces
                     *
                     * stat_cache calls lstat
                     * if there is an error, and the error isn't ENOENT or
                     * ENOTDIR, stat_cache calls error_intl_stat (which
                     * calls error_intl) to print it.
                     */
                    trace(("Error message already printed?\n"));
                    status = graph_walk_status_error;

                    /*
                     * don't break here, need to
                     * update all of them
                     */
                    continue;
                }
                if (t == gfp->mtime_oldest)
                {
                    if (!option_test(OPTION_SILENT))
                    {
                        scp = sub_context_new();
                        sub_var_set_string(scp, "File_Name", gfp->filename);
                        error_intl
                        (
                            scp,
                            i18n("$filename fingerprint unchanged")
                        );
                        sub_context_delete(scp);
                    }
                }
                else if (status != graph_walk_status_error)
                {
                    status = graph_walk_status_done;
                    gfp->done++;
                }
            }
        }
        else
        {
            /*
             * update the times if it worked
             * and if it was not a phony recipe
             */
            for (j = 0; j < grp->output->nfiles; ++j)
            {
                graph_file_ty   *gfp;

                gfp = grp->output->item[j].file;
                gfp->done++;    /* for out-of-date calcs */
                if (os_mtime_adjust(gfp->filename, mtime))
                {
                    /*
                     * Error message already printed.
                     *
                     * os_mtime_adjust calls utime.
                     * If there is an error, os_mtime_sdjust calls
                     * error_intl top report the error.
                     */
                    trace(("Error message already printed?\n"));
                    status = graph_walk_status_error;
                    /*
                     * don't break here, need to
                     * update all of them
                     */
                }
            }
        }
    }

    /*
     * Make sure the target times are consistent with the
     * ingredients times, when there was nothing to do.  This is
     * usually only necessary when finger prints are in use, but can
     * sometimes be necessary when file server times are out of
     * sync.  This guarantees that a later fingerprint-less run will
     * not find huge amounts of work to do.
     */
    trace(("mark\n"));
    if
    (
        (
            status == graph_walk_status_uptodate
        ||
            status == graph_walk_status_uptodate_done
        )
    &&
        grp->rp->out_of_date
    &&
        (option_test(OPTION_UPDATE) || option_test(OPTION_FINGERPRINT))
    )
    {
        time_t          need_age_youngest;

        /*
         * find the youngest ingredient's age
         * (will be in the stat cache or the fingerprint cache)
         */
        need_age_youngest = 0;
        for (j = 0; j < grp->input->nfiles; ++j)
        {
            graph_file_and_type_ty *gftp2;
            graph_file_ty   *gfp2;
            time_t          age2;
            long            depth2;

            gftp2 = grp->input->item + j;
            gfp2 = gftp2->file;
            depth2 = 32767;
            age2 = cook_mtime_newest(grp->ocp, gfp2->filename, &depth2, depth2);
            if (age2 > need_age_youngest)
                need_age_youngest = age2;
        }

        /*
         * Advance one second younger.  This is the youngest a
         * target may be to be mtime-consistent with the
         * ingredients.
         *
         * Actually, on Cygwin on FAT filesystems, the timestamp
         * granularity is 2 seconds.  There is no pathconf query
         * to tell you the time stamp granularity (sigh) so we
         * have to assume that if you are on Cygwin, you needd
         * a 2 second granularity, even if you are on an NTFS
         * file system with 1 second granularity.
         */
        need_age_youngest += timestamp_granularity;

        /*
         * check each of the targets for consistency
         */
        for (j = 0; j < grp->output->nfiles; ++j)
        {
            graph_file_ty   *gfp;
            time_t          age3;
            long            depth3;

            gfp = grp->output->item[j].file;
            depth3 = 32767;
            age3 = cook_mtime_newest(grp->ocp, gfp->filename, &depth3, depth3);
            if (age3 > 0 && depth3 == 0 && age3 < need_age_youngest)
            {
                os_mtime_adjust(gfp->filename, need_age_youngest);
                /*
                 * Error message already printed.
                 *
                 * os_mtime_adjust calls utime.  If there is an error,
                 * os_mtime_sdjust calls error_intl top report the
                 * error.
                 */
            }
        }
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

    trace(("return %s;\n", graph_walk_status_name(status)));
    trace(("}\n"));
    return status;
}
