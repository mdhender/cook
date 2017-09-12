/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2006-2009 Peter Miller
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
#include <common/ac/unistd.h>

#include <cook/builtin/print.h>
#include <common/error_intl.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_print - builtin function for reading symbolic links
 *
 * SYNOPSIS
 *      int builtin_print(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The builtin_print function is used to implement the
 *      "print" builtin function of cook to read symbolic links.
 *
 * RETURNS
 *      int; 0 on success, -1 on any error
 *
 * CAVEAT
 *      This function is designed to be used as a "builtin" function.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    string_ty       *s;
    sub_context_ty  *scp;

    trace(("print::interpret(result = %p, args = %p)\n{\n", result, args));
    (void)result;
    (void)pp;
    (void)ocp;
    s = wl2str(args, 1, args->nstrings, (char *)0);
    scp = sub_context_new();
    sub_var_set_string(scp, "MeSsaGe", s);
    str_free(s);
    error_intl(scp, i18n("$message"));
    sub_context_delete(scp);
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


static int
script(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    string_ty       *s;
    string_ty       *s2;

    trace(("print::script(result = %p, args = %p)\n{\n", result, args));
    (void)result;
    (void)pp;
    (void)ocp;
    s = wl2str(args, 1, args->nstrings, (char *)0);
    s2 = str_quote_shell(s);
    str_free(s);
    printf("echo %s\n", s2->str_text);
    str_free(s2);
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


builtin_ty builtin_print =
{
    "print",
    interpret,
    script,
};
