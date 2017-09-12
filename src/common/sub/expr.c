/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006-2009 Peter Miller
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

#include <common/sub/expr.h>
#include <common/sub/expr_gram.h>
#include <common/sub/private.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/wstr_list.h>


wstring_ty *
sub_expression(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *ws;
    string_ty       *s1;
    string_ty       *s2;
    wstring_ty      *result;

    /*
     * Check the number of arguments
     */
    trace(("sub_expression()\n{\n"));
    if (arg->nitems < 2)
    {
        sub_context_error_set(scp, i18n("requires at least one argument"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }

    /*
     * Fold all of the arguments into a single string,
     * and parse that string for an expression.
     */
    ws = wstring_list_to_wstring(arg, 1, arg->nitems, " ");
    s1 = wstr_to_str(ws);
    trace(("s1 = \"%s\";\n", s1->str_text));
    wstr_free(ws);
    s2 = sub_expr_gram(scp, s1);
    str_free(s1);
    if (!s2)
    {
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }

    /*
     * Turn the result of the parse into a wide string.
     */
    trace(("result = \"%s\";\n", s2->str_text));
    result = str_to_wstr(s2);
    str_free(s2);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}
