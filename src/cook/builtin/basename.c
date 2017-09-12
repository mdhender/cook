/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1999, 2006-2008 Peter Miller
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

#include <cook/builtin/basename.h>
#include <common/str_list.h>


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    string_ty       *s;
    char            *cp;
    char            *ep;

    (void)pp;
    (void)ocp;
    for (j = 1; j < arg->nstrings; ++j)
    {
        s = arg->string[j];
        cp = strrchr(s->str_text, '/');
        if (!cp)
            cp = s->str_text;
        ep = strrchr(cp, '.');
        if (!ep || ep == cp)
            string_list_append(result, s);
        else
        {
            s = str_n_from_c(s->str_text, ep - s->str_text);
            string_list_append(result, s);
            str_free(s);
        }
    }
    return 0;
}


builtin_ty builtin_basename =
{
    "basename",
    interpret,
    interpret,                  /* script */
};
