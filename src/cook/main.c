/*
 *      cook - file construction tool
 *      Copyright (C) 1994-1999, 2001, 2003, 2004, 2006, 2007 Peter Miller;
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
 *
 *
 * If you are going to add a new recipe flag (set by the "set" statement,
 * or the "set" clause of a recipe) you need to change all of the
 * following places:
 *
 * cook/option.h
 *     to define the OPTION_ value
 * cook/option.c
 *     option_tidyup()
 *         if the option defaults to true
 *     option_set_errors()
 *         if the option should be turned off once cookbook errors
 *         are encountered.
 *     option_number_name()
 *         for the name of the option
 * cook/flag.h
 *     to define the RF_ values (RF stands for Recipe Flag)
 * cook/flag.c
 *     to define the RF_ names
 * lib/en/user-guide/langu.flags.so
 *     to document the recipe flag
 *
 * If you choose to make it a command line option,
 * you must also update these files:
 *
 * cook/main.c
 *     to define the new command line option and process it
 *     (only if it should also be a command line option)
 * cook/builtin/options.c
 *     to access the option from within the cookbook (typically used
 *     for recursive cook invokations)
 * lib/en/man1/cook.1
 *     to document it, if you added a new command line option
 */

#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/signal.h>

#include <common/arglex.h>
#include <common/error_intl.h>
#include <common/fflush_slow.h>
#include <common/help.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/star.h>
#include <common/trace.h>
#include <common/version.h>
#include <cook/builtin.h>
#include <cook/cook.h>
#include <cook/fingerprint.h>
#include <cook/id.h>
#include <cook/id/variable.h>
#include <cook/lex.h>
#include <cook/listing.h>
#include <cook/opcode/context.h>
#include <cook/option.h>
#include <cook/parse.h>


enum
{
    arglex_token_action,
    arglex_token_action_not,
    arglex_token_book,
    arglex_token_book_not,
    arglex_token_cascade,
    arglex_token_cascade_not,
    arglex_token_ctime,
    arglex_token_ctime_not,
    arglex_token_disassemble,
    arglex_token_disassemble_not,
    arglex_token_errok,
    arglex_token_errok_not,
    arglex_token_fingerprint,
    arglex_token_fingerprint_not,
    arglex_token_fingerprint_update,
    arglex_token_force,
    arglex_token_force_not,
    arglex_token_include,
    arglex_token_include_cooked,
    arglex_token_include_cooked_not,
    arglex_token_include_cooked_warning,
    arglex_token_include_cooked_warning_not,
    arglex_token_log,
    arglex_token_log_not,
    arglex_token_metering,
    arglex_token_metering_not,
    arglex_token_pairs,
    arglex_token_parallel,
    arglex_token_parallel_not,
    arglex_token_pedantic,
    arglex_token_pedantic_not,
    arglex_token_persevere,
    arglex_token_persevere_not,
    arglex_token_precious,
    arglex_token_precious_not,
    arglex_token_reason,
    arglex_token_reason_not,
    arglex_token_script,
    arglex_token_shallow,
    arglex_token_shallow_not,
    arglex_token_silent,
    arglex_token_silent_not,
    arglex_token_star,
    arglex_token_star_not,
    arglex_token_strip_dot,
    arglex_token_strip_dot_not,
    arglex_token_symlink_ingredients,
    arglex_token_symlink_ingredients_not,
    arglex_token_tell_position,
    arglex_token_tell_position_not,
    arglex_token_touch,
    arglex_token_touch_not,
    arglex_token_tty,
    arglex_token_tty_not,
    arglex_token_update,
    arglex_token_update_not,
    arglex_token_web
        /*
         * When you add an option to this list, you must
         * also add it to the list in cook/builtin/options.c
         */
};

static arglex_table_ty argtab[] =
{
    { "-Action", (arglex_token_ty) arglex_token_action },
    { "-No_Action", (arglex_token_ty) arglex_token_action_not },
    { "-Book", (arglex_token_ty) arglex_token_book },
    { "-No_Book", (arglex_token_ty) arglex_token_book_not },
    { "-CAScade", (arglex_token_ty) arglex_token_cascade },
    { "-No_CAScade", (arglex_token_ty) arglex_token_cascade_not },
    { "-CTime", (arglex_token_ty) arglex_token_ctime },
    { "-No_CTime", (arglex_token_ty) arglex_token_ctime_not },
    { "-Continue", (arglex_token_ty) arglex_token_persevere },
    { "-No_Continue", (arglex_token_ty) arglex_token_persevere_not },
    { "-DISassemble", (arglex_token_ty) arglex_token_disassemble },
    { "-No_DISassemble", (arglex_token_ty) arglex_token_disassemble_not },
    { "-Errok", (arglex_token_ty) arglex_token_errok },
    { "-No_Errok", (arglex_token_ty) arglex_token_errok_not },
    { "-FingerPrint", (arglex_token_ty) arglex_token_fingerprint },
    { "-No_FingerPrint", (arglex_token_ty) arglex_token_fingerprint_not },
    { "-FingerPrint_Update",
        (arglex_token_ty) arglex_token_fingerprint_update },
    { "-Forced", (arglex_token_ty) arglex_token_force },
    { "-No_Forced", (arglex_token_ty) arglex_token_force_not },
    { "-HyperText_Markup_Language", (arglex_token_ty) arglex_token_web },
    { "-Include", (arglex_token_ty) arglex_token_include },
    { "-\\I*", (arglex_token_ty) arglex_token_include },
    { "-Include_Cooked", (arglex_token_ty) arglex_token_include_cooked },
    { "-Include_Cooked_Warning",
        (arglex_token_ty) arglex_token_include_cooked_warning },
    { "-Jobs", (arglex_token_ty) arglex_token_parallel },
    { "-No_Include_Cooked", (arglex_token_ty) arglex_token_include_cooked_not },
    { "-No_Include_Cooked_Warning",
        (arglex_token_ty) arglex_token_include_cooked_warning_not },
    { "-LOg", (arglex_token_ty) arglex_token_log },
    { "-List", (arglex_token_ty) arglex_token_log },
    { "-No_LOg", (arglex_token_ty) arglex_token_log_not },
    { "-No_List", (arglex_token_ty) arglex_token_log_not },
    { "-Meter", (arglex_token_ty) arglex_token_metering },
    { "-No_Meter", (arglex_token_ty) arglex_token_metering_not },
    { "-PAirs", (arglex_token_ty) arglex_token_pairs },
    { "-PARallel", (arglex_token_ty) arglex_token_parallel },
    { "-No_PARallel", (arglex_token_ty) arglex_token_parallel_not },
    { "-Precious", (arglex_token_ty) arglex_token_precious },
    { "-No_Precious", (arglex_token_ty) arglex_token_precious_not },
    { "-Reason", (arglex_token_ty) arglex_token_reason },
    { "-No_Reason", (arglex_token_ty) arglex_token_reason_not },
    { "-SCript", (arglex_token_ty) arglex_token_script },
    { "-Silent", (arglex_token_ty) arglex_token_silent },
    { "-No_Silent", (arglex_token_ty) arglex_token_silent_not },
    { "-SHallow", (arglex_token_ty) arglex_token_shallow },
    { "-No_SHallow", (arglex_token_ty) arglex_token_shallow_not },
    { "-STar", (arglex_token_ty) arglex_token_star },
    { "-No_STar", (arglex_token_ty) arglex_token_star_not },
    { "-Strip_Dot", (arglex_token_ty) arglex_token_strip_dot },
    { "-No_Strip_Dot", (arglex_token_ty) arglex_token_strip_dot_not },
    { "-Symbolic_Link_Ingredients",
        (arglex_token_ty)arglex_token_symlink_ingredients },
    { "-Not_Symbolic_Link_Ingredients",
        (arglex_token_ty)arglex_token_symlink_ingredients_not },
    { "-Tell_Position", (arglex_token_ty) arglex_token_tell_position },
    { "-No_Tell_Position", (arglex_token_ty) arglex_token_tell_position_not},
    { "-TErminal", (arglex_token_ty) arglex_token_tty },
    { "-No_TErminal", (arglex_token_ty) arglex_token_tty_not },
    { "-Touch", (arglex_token_ty) arglex_token_touch },
    { "-No_Touch", (arglex_token_ty) arglex_token_touch_not },
    { "-TRace", (arglex_token_ty) arglex_token_reason },
    { "-No_TRace", (arglex_token_ty) arglex_token_reason_not },
    { "-Update", (arglex_token_ty) arglex_token_update },
    { "-Time_Adjust", (arglex_token_ty) arglex_token_update },
    { "-No_Update", (arglex_token_ty) arglex_token_update_not },
    { "-No_Time_Adjust", (arglex_token_ty) arglex_token_update_not },
    { "-Web", (arglex_token_ty) arglex_token_web },
    { 0, (arglex_token_ty)0 },  /* end marker */
};


/*
 *  NAME
 *      usage - options diagnostic
 *
 *  SYNOPSIS
 *      void usage(void);
 *
 *  DESCRIPTION
 *      Usage is called when the user has made a syntactic or semantic error
 *      on the command line.
 *
 *  CAVEAT
 *      This function does NOT return.
 */

static void
usage(void)
{
    char            *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ][ <filename>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    fprintf(stderr, "       %s -VERSion\n", progname);
    quit(1);
    trace(("to silence warnings\n"));
}


/*
 * NAME
 *      argparse - parse command line
 *
 * SYNOPSIS
 *      void argparse(option_level_ty);
 *
 * DESCRIPTION
 *      The argparse function is used to parse command lines.
 *
 * RETURNS
 *      void
 */

static void
argparse(option_level_ty level)
{
    option_number_ty type;
    string_ty       *s;
    sub_context_ty  *scp;
    int             fingerprint_update;

    type = -1;
    fingerprint_update = 0;
    switch (arglex())
    {
    case arglex_token_help:
        if (level != OPTION_LEVEL_COMMAND_LINE)
        {
          not_in_env:
            scp = sub_context_new();
            sub_var_set(scp, "Name", "%s", arglex_value.alv_string);
            fatal_intl(scp, i18n("may not use $name in environment variable"));
            /* NOTREACHED */
        }
        help((char *)0, usage);
        quit(0);

    case arglex_token_version:
        if (level != OPTION_LEVEL_COMMAND_LINE)
            goto not_in_env;
        version();
        quit(0);

    default:
        break;
    }
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_include:
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_include, usage);
                /* NOTREACHED */
            }
            s = str_from_c(arglex_value.alv_string);
            string_list_append_unique(&option.o_search_path, s);
            str_free(s);
            break;

        case arglex_token_reason:
            type = OPTION_REASON;
          normal_on:
            if (option_already(type, level))
            {
              too_many:
                arg_duplicate_cur(usage);
                /* NOTREACHED */
            }
            option_set(type, level, 1);
            break;

        case arglex_token_reason_not:
            type = OPTION_REASON;
          normal_off:
            if (option_already(type, level))
                goto too_many;
            option_set(type, level, 0);
            break;

        case arglex_token_cascade:
            type = OPTION_CASCADE;
            goto normal_on;

        case arglex_token_cascade_not:
            type = OPTION_CASCADE;
            goto normal_off;

        case arglex_token_ctime:
            type = OPTION_CTIME;
            goto normal_on;

        case arglex_token_ctime_not:
            type = OPTION_CTIME;
            goto normal_off;

        case arglex_token_disassemble:
            type = OPTION_DISASSEMBLE;
            goto normal_on;

        case arglex_token_disassemble_not:
            type = OPTION_DISASSEMBLE;
            goto normal_off;

        case arglex_token_tty:
            type = OPTION_TERMINAL;
            goto normal_on;

        case arglex_token_tty_not:
            type = OPTION_TERMINAL;
            goto normal_off;

        case arglex_token_precious:
            type = OPTION_PRECIOUS;
            goto normal_on;

        case arglex_token_precious_not:
            type = OPTION_PRECIOUS;
            goto normal_off;

        case arglex_token_log:
            if (option_already(OPTION_LOGGING, level))
                goto too_many;
            option_set(OPTION_LOGGING, level, 1);
            if (arglex() != arglex_token_string)
                continue;
            if (option.o_logfile)
                str_free(option.o_logfile);
            option.o_logfile = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_log_not:
            type = OPTION_LOGGING;
            goto normal_off;

        case arglex_token_book:
            if (option_already(OPTION_BOOK, level))
                goto too_many;
            option_set(OPTION_BOOK, level, 1);
            if (arglex() != arglex_token_string)
                continue;
            if (option.o_book)
                str_free(option.o_book);
            option.o_book = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_book_not:
            type = OPTION_BOOK;
            goto normal_off;

        case arglex_token_include_cooked:
            type = OPTION_INCLUDE_COOKED;
            goto normal_on;

        case arglex_token_include_cooked_not:
            type = OPTION_INCLUDE_COOKED;
            goto normal_off;

        case arglex_token_include_cooked_warning:
            type = OPTION_INCLUDE_COOKED_WARNING;
            goto normal_on;

        case arglex_token_include_cooked_warning_not:
            type = OPTION_INCLUDE_COOKED_WARNING;
            goto normal_off;

        case arglex_token_silent:
            type = OPTION_SILENT;
            goto normal_on;

        case arglex_token_silent_not:
            type = OPTION_SILENT;
            goto normal_off;

        case arglex_token_tell_position:
            type = OPTION_TELL_POSITION;
            goto normal_on;

        case arglex_token_tell_position_not:
            type = OPTION_TELL_POSITION;
            goto normal_off;

        case arglex_token_metering:
            type = OPTION_METER;
            goto normal_on;

        case arglex_token_metering_not:
            type = OPTION_METER;
            goto normal_off;

        case arglex_token_touch:
            type = OPTION_TOUCH;
            goto normal_on;

        case arglex_token_touch_not:
            type = OPTION_TOUCH;
            goto normal_off;

        case arglex_token_action:
            type = OPTION_ACTION;
            goto normal_on;

        case arglex_token_action_not:
            type = OPTION_ACTION;
            goto normal_off;

        case arglex_token_persevere:
            type = OPTION_PERSEVERE;
            goto normal_on;

        case arglex_token_persevere_not:
            type = OPTION_PERSEVERE;
            goto normal_off;

        case arglex_token_errok:
            type = OPTION_ERROK;
            goto normal_on;

        case arglex_token_errok_not:
            type = OPTION_ERROK;
            goto normal_off;

        case arglex_token_force:
            type = OPTION_FORCE;
            goto normal_on;

        case arglex_token_force_not:
            type = OPTION_FORCE;
            goto normal_off;

        case arglex_token_fingerprint:
            type = OPTION_FINGERPRINT;
            goto normal_on;

        case arglex_token_fingerprint_not:
            type = OPTION_FINGERPRINT;
            goto normal_off;

        case arglex_token_fingerprint_update:
            if (level != OPTION_LEVEL_COMMAND_LINE)
                goto not_in_env;
            if (option.fingerprint_update)
                goto too_many;
            option.fingerprint_update++;
            break;

        case arglex_token_pairs:
            if (level != OPTION_LEVEL_COMMAND_LINE)
                goto not_in_env;
            if (option.pairs)
                goto too_many;
            option.pairs++;
            break;

        case arglex_token_script:
            if (level != OPTION_LEVEL_COMMAND_LINE)
                goto not_in_env;
            if (option.script)
                goto too_many;
            option.script++;
            break;

        case arglex_token_web:
            if (level != OPTION_LEVEL_COMMAND_LINE)
                goto not_in_env;
            if (option.web)
                goto too_many;
            option.web++;
            break;

        case arglex_token_string:
            if (level != OPTION_LEVEL_COMMAND_LINE)
            {
                if (strchr(arglex_value.alv_string, '='))
                {
                    fatal_intl
                    (
                        0,
                        i18n("may not assign variables in environment variable")
                    );
                }
                else
                {
                    fatal_intl
                    (
                        0,
                        i18n("may not name targets in environment variable")
                    );
                }
            }
            else
            {
                char            *cp;

                cp = strchr(arglex_value.alv_string, '=');
                if (!cp)
                {
                    s = str_from_c(arglex_value.alv_string);
                    string_list_append(&option.o_target, s);
                    str_free(s);
                }
                else
                {
                    s = str_from_c(arglex_value.alv_string);
                    string_list_append(&option.o_vardef, s);
                    str_free(s);
                }
            }
            break;

        case arglex_token_star:
            type = OPTION_STAR;
            goto normal_on;

        case arglex_token_star_not:
            type = OPTION_STAR;
            goto normal_off;

        case arglex_token_strip_dot:
            type = OPTION_STRIP_DOT;
            goto normal_on;

        case arglex_token_strip_dot_not:
            type = OPTION_STRIP_DOT;
            goto normal_off;

        case arglex_token_symlink_ingredients:
            type = OPTION_SYMLINK_INGREDIENTS;
            goto normal_on;

        case arglex_token_symlink_ingredients_not:
            type = OPTION_SYMLINK_INGREDIENTS;
            goto normal_off;

        case arglex_token_update:
            type = OPTION_UPDATE;
            goto normal_on;

        case arglex_token_update_not:
            type = OPTION_UPDATE;
            goto normal_off;

        case arglex_token_parallel:
            if (arglex() != arglex_token_number)
            {
                s = str_from_c("parallel_jobs=4");
                string_list_append(&option.o_vardef, s);
                str_free(s);
                continue;
            }
            s = str_format("parallel_jobs=%d", (int)arglex_value.alv_number);
            string_list_append(&option.o_vardef, s);
            str_free(s);
            break;

        case arglex_token_parallel_not:
            s = str_from_c("parallel_jobs=1");
            string_list_append(&option.o_vardef, s);
            str_free(s);
            break;

        case arglex_token_shallow:
            type = OPTION_SHALLOW;
            goto normal_on;

        case arglex_token_shallow_not:
            type = OPTION_SHALLOW;
            goto normal_off;
        }
        arglex();
    }
}


static void
set_command_line_goals(void)
{
    string_ty       *name;
    opcode_context_ty *ocp;

    name = str_from_c("command-line-goals");
    ocp = opcode_context_new(0, 0);
    opcode_context_id_assign(ocp, name, id_variable_new(&option.o_target), 0);
    opcode_context_delete(ocp);
    str_free(name);
}


/*
 * NAME
 *      main - initial entry point for cook
 *
 * SYNOPSIS
 *      void main(int argc, char **argv);
 *
 * DESCRIPTION
 *      Main is the initial entry point for cook.
 *
 * RETURNS
 *      Exit is always through exit().
 *      The exit code will be 0 for success, or 1 for some error.
 */

int
main(int argc, char **argv)
{
    int             retval;

    /*
     * Some versions of cron(8) and at(1) set SIGCHLD to SIG_IGN.
     * This is kinda dumb, because it breaks assumprions made in
     * libc (like pclose, for instance).  It also blows away most
     * of Cook's process handling.  We explicitly set the SIGCHLD
     * signal handling to SIG_DFL to make sure this signal does what
     * we expect no matter how we are invoked.
     */
#ifdef SIGCHLD
    signal(SIGCHLD, SIG_DFL);
#else
    signal(SIGCLD, SIG_DFL);
#endif

    /*
     * initialize things
     * (order is critical here)
     */
    progname_set(argv[0]);
    str_initialize();
    id_initialize();
    lex_initialize();

    /*
     * parse the COOK environment variable
     */
    arglex_init_from_env(argv[0], argtab);
    argparse(OPTION_LEVEL_ENVIRONMENT);

    /*
     * parse the command line
     */
    arglex_init(argc, argv, argtab);
    argparse(OPTION_LEVEL_COMMAND_LINE);

    option_tidy_up();

    log_open();

    /*
     * turn on progress stars if they asked for them
     */
    if (option_test(OPTION_STAR))
        star_enable();

    /*
     * If we were asked to update the fingerprints, do it here.
     * We don't actually ant to read in the cookbook.
     */
    if (option.fingerprint_update)
    {
        fp_tweak();
        quit(0);
    }

    /*
     * read in the cook book
     *
     * If there are #include-cooked directives,
     * we may need to do it more than once.
     */
    if (!option.o_book)
        fatal_intl(0, i18n("no book found"));
    for (;;)
    {
        int             status;
        size_t          j;

        builtin_initialize();

        /*
         * instanciate the command line variable assignments
         */
        for (j = 0; j < option.o_vardef.nstrings; ++j)
        {
            char            *s;
            char            *cp;
            string_ty       *name;
            string_ty       *value;
            string_list_ty  wl;
            opcode_context_ty *ocp;

            s = option.o_vardef.string[j]->str_text;
            cp = strchr(s, '=');
            assert(cp);
            if (!cp)
                continue;
            name = str_n_from_c(s, cp - s);
            value = str_from_c(cp + 1);
            str2wl(&wl, value, (char *)0, 0);
            str_free(value);
            ocp = opcode_context_new(0, 0);
            opcode_context_id_assign(ocp, name, id_variable_new(&wl), 0);
            opcode_context_delete(ocp);
            str_free(name);
            string_list_destructor(&wl);
        }

        set_command_line_goals();

        parse(option.o_book);
        status = cook_auto_required();
        if (status < 0)
            quit(1);
        if (!status)
            break;
        id_reset();
        cook_reset();
    }

    /*
     * work out what to cook.
     * If no targets have been given, use the first explicit recipe.
     */
    set_command_line_goals();
    if (!option.o_target.nstrings)
        cook_find_default(&option.o_target);
    assert(option.o_target.nstrings);

    /*
     * cook the target
     */
    if (option.pairs)
        retval = cook_pairs(&option.o_target);
    else if (option.script)
        retval = cook_script(&option.o_target);
    else if (option.web)
        retval = cook_web(&option.o_target);
    else
        retval = cook(&option.o_target);

#ifdef DEBUG
    fflush_slowly_report();
#endif

    quit(retval);
    /*NOTREACHED*/
    return 0;
}
