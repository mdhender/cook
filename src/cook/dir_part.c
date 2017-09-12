/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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

#include <cook/dir_part.h>
#include <common/str.h>


string_ty *
dir_part(string_ty *s)
{
    char            *ep;

    /*
     * ignore trailing slashes
     */
    ep = s->str_text + s->str_length;
    while (ep > s->str_text && ep[-1] == '/')
        --ep;
    if (ep == s->str_text)
        return 0;

    /*
     * back up over that last word
     */
    while (ep > s->str_text && ep[-1] != '/')
        --ep;
    if (ep == s->str_text)
        return 0;

    /*
     * back up over the separating slash
     */
    while (ep > s->str_text && ep[-1] == '/')
        --ep;
    if (ep == s->str_text)
        return 0;

    /*
     * return the directory part
     */
    return str_n_from_c(s->str_text, ep - s->str_text);
}
