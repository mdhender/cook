/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2006, 2007 Peter Miller;
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

#include <common/ac/ctype.h>

#include <cook/builtin/wordlist.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/str_list.h>
#include <common/trace.h>


static int
number(char *s, long *n_p)
{
    long            n;
    int             neg;

    n = 0;
    neg = 0;
    while (isspace(*s))
        ++s;
    if (*s == '+')
    {
        ++s;
        if (!*s || !isdigit(*s))
            return 0;
    }
    else if (*s == '-')
    {
        neg = 1;
        ++s;
        if (!*s || !isdigit(*s))
            return 0;
    }
    while (isdigit(*s))
        n = n * 10 + *s++ - '0';
    while (isspace(*s))
        ++s;
    if (*s)
        return 0;
    *n_p = (neg ? -n : n);
    return 1;
}


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    long            n1;
    long            n2;

    trace(("word\n"));
    (void)ocp;
    if (arg->nstrings < 3)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires two or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    if (!number(arg->string[1]->str_text, &n1))
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        sub_var_set(scp, "Number", "1");
        error_with_position
        (
            pp,
            scp,
            i18n("$name: argument $number: must be a positive decimal number")
        );
        sub_context_delete(scp);
        return -1;
    }
    if (!number(arg->string[2]->str_text, &n2))
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        sub_var_set(scp, "Number", "2");
        error_with_position
        (
            pp,
            scp,
            i18n("$name: argument $number: must be a positive decimal number")
        );
        sub_context_delete(scp);
        return -1;
    }
    if (n1 > n2)
    {
        long            swap;

        swap = n1;
        n1 = n2;
        n2 = swap;
    }
    n1 += 2;
    n2 += 3;
    if (n1 < 3)
        n1 = 3;
    if (n2 > (long)arg->nstrings)
        n2 = arg->nstrings;
    while (n1 < n2)
        string_list_append(result, arg->string[n1++]);
    return 0;
}


builtin_ty builtin_wordlist =
{
    "wordlist",
    interpret,
    interpret,                  /* script */
};
