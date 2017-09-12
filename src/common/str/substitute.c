/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006-2008 Peter Miller
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

#include <common/str.h>
#include <common/stracc.h>


string_ty *
str_substitute(string_ty *a, string_ty *b, string_ty *c)
{
    char            *cp;
    char            *ep;
    static stracc   sa;

    sa_open(&sa);
    cp = c->str_text;
    ep = cp + c->str_length;
    while (cp < ep)
    {
        if (a->str_length == 0)
        {
            sa_chars(&sa, b->str_text, b->str_length);
            sa_char(&sa, *cp++);
        }
        else if
        (
            (size_t)(ep - cp) < a->str_length
        ||
            0 != memcmp(a->str_text, cp, a->str_length)
        )
        {
            sa_char(&sa, *cp++);
        }
        else
        {
            sa_chars(&sa, b->str_text, b->str_length);
            cp += a->str_length;
        }
    }
    return sa_close(&sa);
}
