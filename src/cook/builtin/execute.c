/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997-1999, 2006, 2007 Peter Miller;
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
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <common/error.h>
#include <common/error_intl.h>
#include <common/star.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <cook/builtin/execute.h>
#include <cook/expr/position.h>
#include <cook/option.h>
#include <cook/os_interface.h>


/*
 * NAME
 *      builtin_execute - execute a command
 *
 * SYNOPSIS
 *      int builtin_execute(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Exec is a built-in function of cook, described as follows:
 *      This function requires at least one argument, and
 *      executes the command given by the arguments.
 *
 * RETURNS
 *      If the executed command returns an error code the resulting value
 *      is "" (false), otherwise it is "1" (true).
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty * pp, const struct opcode_context_ty *ocp)
{
    string_list_ty  wl;
    int             j;
    size_t          k;
    string_ty       *s;
    int             silent;

    trace(("execute\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings < 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires one or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    string_list_constructor(&wl);
    for (k = 1; k < args->nstrings; k++)
        string_list_append(&wl, args->string[k]);
    option_set(OPTION_SILENT, OPTION_LEVEL_EXECUTE, 1);
    silent = option_test(OPTION_SILENT);
    if (silent)
        star_bang();
    else
    {
        s = wl2str(&wl, 0, wl.nstrings - 1, (char *)0);
        error_raw("%s", s->str_text);
        str_free(s);
    }
    j = os_execute(&wl, (string_ty *) 0, 0);
    if (!silent)
        star_sync();
    option_undo_level(OPTION_LEVEL_EXECUTE);
    string_list_destructor(&wl);
    if (j < 0)
        return -1;
    s = (j ? str_false : str_true);
    string_list_append(result, s);
    return 0;
}


builtin_ty builtin_execute =
{
    "execute",
    interpret,
    interpret,                  /* script */
};
