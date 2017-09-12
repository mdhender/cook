/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2001, 2006, 2007 Peter Miller;
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

#include <cook/builtin/substr.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/str_list.h>
#include <common/trace.h>


static int
number(char *s, long *np)
{
    long            n;
    int             neg;

    neg = 0;
    while (isspace(*s))
        ++s;
    if (*s == '+')
        ++s;
    else if (*s == '-')
    {
        neg = -1;
        ++s;
    }
    if (!isdigit(*s))
        return 0;
    n = 0;
    while (isdigit(*s))
        n = n * 10 + *s++ - '0';
    while (isspace(*s))
        ++s;
    if (*s)
        return 0;
    *np = (neg ? -n : n);
    return 1;
}


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    long            start;
    long            length;
    long            end;
    size_t          j;

    trace(("builtin::substr()\n{\n"));
    (void)ocp;
    if (arg->nstrings < 3)
    {
        sub_context_ty *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires two or more arguments")
        );
        sub_context_delete(scp);
        trace(("return -1;\n"));
        trace(("}\n"));
        return -1;
    }

    if (!number(arg->string[1]->str_text, &start))
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
        trace(("return -1;\n"));
        trace(("}\n"));
        return -1;
    }
    --start;
    trace(("start = %ld;\n", start));

    if (!number(arg->string[2]->str_text, &length))
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
        trace(("return -1;\n"));
        trace(("}\n"));
        return -1;
    }
    trace(("length = %ld;\n", length));
    if (start < 0)
    {
        length += start;
        start = 0;
        trace(("length = %ld;\n", length));
    }
    if (length < 0)
    {
        start = 0;
        length = 0;
    }
    end = start + length;

    for (j = 3; j < arg->nstrings; ++j)
    {
        string_ty       *s;
        string_ty       *s2;

        s = arg->string[j];
        trace(("s = \"%s\";\n", s->str_text));
        if (start >= (int)(s->str_length))
            s2 = str_from_c("");
        else if (end > (int)(s->str_length))
        {
            s2 = str_n_from_c(s->str_text + start, s->str_length - start);
        }
        else
            s2 = str_n_from_c(s->str_text + start, length);
        trace(("s2 = \"%s\";\n", s2->str_text));
        string_list_append(result, s2);
        str_free(s2);
    }
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


builtin_ty builtin_substr =
{
    "substr",
    interpret,
    interpret,                  /* script */
};

builtin_ty builtin_substring =
{
    "substring",
    interpret,
    interpret,                  /* script */
};
