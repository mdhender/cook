/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2006, 2007 Peter Miller;
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

#include <cook/builtin/strip.h>
#include <common/str_list.h>
#include <common/stracc.h>


static string_ty *
strip(string_ty *s)
{
    static stracc   sa;
    char            *sp;
    int             space;

    sa_open(&sa);
    sp = s->str_text;
    while (*sp && isspace(*sp))
        ++sp;
    space = 0;
    while (*sp)
    {
        if (isspace(*sp))
            space = 1;
        else
        {
            if (space)
                sa_char(&sa, ' ');
            sa_char(&sa, *sp);
            space = 0;
        }
        ++sp;
    }
    return sa_close(&sa);
}


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    string_ty       *s;

    (void)pp;
    (void)ocp;
    for (j = 1; j < arg->nstrings; ++j)
    {
        s = strip(arg->string[j]);
        string_list_append(result, s);
        str_free(s);
    }
    return 0;
}


builtin_ty builtin_strip =
{
    "strip",
    interpret,
    interpret,                  /* script */
};
