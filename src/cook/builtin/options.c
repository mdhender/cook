/*
 *      cook - file construction tool
 *      Copyright (C) 1996-1999, 2003, 2006, 2007 Peter Miller;
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
 * langu.flags.so
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

#include <cook/builtin/options.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/option.h>
#include <common/str_list.h>
#include <common/trace.h>


typedef struct table_ty table_ty;
struct table_ty
{
    int             option;
    char            *set;
    char            *unset;
};

static table_ty table[] =
{
    { OPTION_ACTION, "-action", "-noaction" },
    { OPTION_CASCADE, "-cascade", "-nocascade" },
    { OPTION_ERROK, "-errok", "-noerrok" },
    { OPTION_FINGERPRINT, "-fingerprint", "-nofingerprint" },
    { OPTION_FORCE, "-force", "-noforce" },
    { OPTION_METER, "-meter", "-nometer" },
    { OPTION_PERSEVERE, "-continue", "-nocontinue" },
    { OPTION_PRECIOUS, "-precious", "-noprecious" },
    { OPTION_REASON, "-reason", "-noreason" },
    { OPTION_SHALLOW, "-shallow", "-noshallow" },
    { OPTION_SILENT, "-silent", "-nosilent" },
    { OPTION_STAR, "-star", "-nostar" },
    { OPTION_STRIP_DOT, "-stripdot", "-nostripdot" },
    { OPTION_SYMLINK_INGREDIENTS, "-symlink-ingredi", "-no-symlink-ingredi" },
    { OPTION_TERMINAL, "-terminal", "-noterminal" },
    { OPTION_TOUCH, "-touch", "-notouch" },
    { OPTION_UPDATE, "-timeadjust", "-notimeadjust" },
    { OPTION_TELL_POSITION, "-tellposition", "-notellposition" },
};


/*
 * NAME
 *      builtin_options - describe operating system
 *
 * SYNOPSIS
 *      int builtin_options(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Operating_system is a built-in function of cook, described as
 *      follows: This function must have zero arguments.
 *
 * RETURNS
 *      The resulting wordlist contains the values of various cook
 *      options, suitable for use on a recursive cook command line.  All
 *      command line options are emitted, to override any environment
 *      variable or default settings.  The options emitted are the
 *      current union of the command line options given to cook, the
 *      settings in the COOK environment variable, and any current
 *      ``set'' clause actions.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    table_ty        *tp;
    string_ty       *s;
    size_t          j;

    trace(("options\n"));
    (void)ocp;
    if (args->nstrings != 1)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position(pp, scp, i18n("$name: requires no arguments"));
        sub_context_delete(scp);
        return -1;
    }

    /*
     * do the simple options
     */
    for (tp = table; tp < ENDOF(table); ++tp)
    {
        s = str_from_c(option_test(tp->option) ? tp->set : tp->unset);
        string_list_append(result, s);
        str_free(s);
    }

    /*
     * the include options next
     */
    for (j = 0; j < option.o_search_path.nstrings; ++j)
    {
        s = str_format("-I%s", option.o_search_path.string[j]->str_text);
        string_list_append(result, s);
        str_free(s);
    }

    /*
     * any recursive cook must have no logging, because it will
     * inherit its log file from the parent.
     */
    s = str_from_c("-nolog");
    string_list_append(result, s);
    str_free(s);
    return 0;
}


builtin_ty builtin_options =
{
    "options",
    interpret,
    interpret,                  /* script */
};
