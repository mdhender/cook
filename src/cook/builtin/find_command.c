/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997-1999, 2004, 2006-2008 Peter Miller
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
 * The builtin function all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 *
 * Only a limited set of this are candidates for builtin functions,
 * these are
 *      - string manipulation [dirname, stringset, ect ]
 *      - environment manipulation [getenv(3), etc]
 *      - stat(3) related functions [exists, mtime, pathname, etc]
 *      - launching OS commands [execute, collect]
 * The above list is though to be exhaustive.
 *
 * This explicitly and forever excluded from being a builtin function
 * is anything which known or understands the format of some secific
 * class of files.
 *
 * Access to stdio(3) has been thought of, and explicitly avoided.
 * Mostly because a specialist program used through [collect]
 * will almost always be far faster.
 */

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error_intl.h>
#include <common/exeext.h>
#include <common/os_path_cat.h>
#include <common/trace.h>
#include <cook/builtin/find_command.h>
#include <cook/expr/position.h>
#include <cook/os_interface.h>


static int      ret_value;


static string_ty *
remove_exe_suffix(string_ty *prog)
{
    int             n;

    n = exeext(prog->str_text);
    if (n <= 0)
        return str_copy(prog);
    return str_n_from_c(prog->str_text, n);
}


static string_ty *
look_for_exe(string_ty *prog)
{
    int             j;
    const char      *suffix;
    string_ty       *s;
    int             ok;

    for (j = 0;; ++j)
    {
        suffix = exeext_nth(j);
        if (!suffix)
            return 0;
        s = str_format("%s%s", prog->str_text, suffix);
        ok = os_executable(s);
        if (ok < 0)
            ret_value = -1;
        if (ok)
            return s;
        str_free(s);
    }
}


static string_ty *
look_for(string_ty *prog, string_list_ty *path)
{
    string_ty       *result;
    size_t          j;
    string_ty       *s;
    string_ty       *s2;

    prog = remove_exe_suffix(prog);

    result = 0;
    if (strchr(prog->str_text, '/'))
        result = look_for_exe(prog);
    else
    {
        for (j = 0; j < path->nstrings; ++j)
        {
            s = os_path_cat(path->string[j], prog);
            s2 = look_for_exe(s);
            str_free(s);
            if (s2)
            {
                result = s2;
                break;
            }
        }
    }
    str_free(prog);
    return result;
}


/*
 * NAME
 *      builtin_find_command - find pathname commands
 *
 * SYNOPSIS
 *      int builtin_find_command(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Find_command is a built-in function of cook, described as follows:
 *      This function requires one or more arguments.
 *
 * RETURNS
 *      A word list containing the expanded pathname of the named commands
 *      given as arguments.
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
    size_t          j;
    string_ty       *s;
    string_ty       *s2;
    char            *cp;
    string_list_ty  path;

    trace(("find command\n"));
    (void)ocp;
    ret_value = 0;
    assert(result);
    assert(args);
    assert(args->nstrings);

    /*
     * Find the command search path, and break it into colon-separated
     * pieces.
     */
    cp = getenv("PATH");
    if (!cp || !*cp)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set(scp, "Name", "PATH");
        error_with_position
        (
            pp,
            scp,
            i18n("the $name environment variable is not set")
        );
        sub_context_delete(scp);
        return -1;
    }
    s = str_from_c(cp);
    str2wl(&path, s, ":", 0);
    str_free(s);

    /*
     * for each argument, hunt down the search path for it.
     */
    for (j = 1; j < args->nstrings; ++j)
    {
        s2 = look_for(args->string[j], &path);
        if (s2)
        {
            string_list_append(result, s2);
            str_free(s2);
        }
        else
            string_list_append(result, str_false);
    }

    /*
     * clean up and go home
     */
    string_list_destructor(&path);
    return ret_value;
}


builtin_ty builtin_find_command =
{
    "find_command",
    interpret,
    interpret,                  /* script */
};
