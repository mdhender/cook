/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2006-2008 Peter Miller
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

#include <cook/builtin/findstring.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/str_list.h>
#include <common/trace.h>


static int
contains(string_ty *s1, string_ty *s2)
{
    size_t          len;
    size_t          j;

    if (s1->str_length == 0)
        return 1;
    if (s1->str_length > s2->str_length)
        return 0;
    len = s2->str_length - s1->str_length;
    for (j = 0; j <= len; ++j)
    {
        if (!memcmp(s2->str_text + j, s1->str_text, s1->str_length))
            return 1;
    }
    return 0;
}


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("findstring\n"));
    (void)ocp;
    if (arg->nstrings < 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires one or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    for (j = 2; j < arg->nstrings; ++j)
    {
        if (contains(arg->string[1], arg->string[j]))
            string_list_append(result, arg->string[1]);
        else
            string_list_append(result, str_false);
    }
    return 0;
}


builtin_ty builtin_findstring =
{
    "findstring",
    interpret,
    interpret,                  /* script */
};
