/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 1999, 2006, 2007 Peter Miller;
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
 * The builtin function all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <cook/builtin/expr.h>
#include <cook/builtin/expr_parse.h>
#include <common/str_list.h>
#include <common/trace.h>


static int
interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    string_ty       *s;

    trace(("expr\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    builtin_expr_parse_begin(args, pp);
    if (builtin_expr_parse_parse())
        return -1;
    s = builtin_expr_parse_end();
    string_list_append(result, s);
    str_free(s);
    return 0;
}


builtin_ty builtin_expr =
{
    "expr",
    interpret,
    interpret,                  /* script */
};
