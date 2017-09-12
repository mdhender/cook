/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2002, 2006-2008 Peter Miller
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

#include <cook/option.h>
#include <common/str_list.h>
#include <cook/strip_dot.h>


/*
 * NAME
 *      strip_dot_inner
 *
 * SYNOPSIS
 *      void strip_dot_inner(string_ty **);
 *
 * DESCRIPTION
 *      The strip_dot_inner function is used to check the filename for a
 *      leading dot path element.  If there is one, it is removed, the
 *      original string free()ed, and the new string put in its place.
 */

string_ty *
strip_dot_inner(string_ty *s)
{
    char            *cp;
    size_t          len;

    cp = s->str_text;
    len = s->str_length;
    while (len >= 3 && cp[0] == '.' && cp[1] == '/')
    {
        cp += 2;
        len -= 2;
    }
    if (len == s->str_length)
        return str_copy(s);

    return str_n_from_c(cp, len);
}


/*
 * NAME
 *      strip_dot
 *
 * SYNOPSIS
 *      void strip_dot(string_ty **);
 *
 * DESCRIPTION
 *      The strip_dot function is used to check the filename for a
 *      leading dot path element.  If there is one, it is removed, the
 *      original string free()ed, and the new string put in its place.
 *
 * CAVEAT
 *      This is only done if the STRIP_DOT option is enabled.  It is
 *      enabled by default, but the userr may choose to turn it off.
 */

string_ty *
strip_dot(string_ty *s)
{
    if (option_test(OPTION_STRIP_DOT))
        return strip_dot_inner(s);
    return str_copy(s);
}


/*
 * NAME
 *      strip_dot_list
 *
 * SYNOPSIS
 *      void strip_dot_list(string_list_ty *);
 *
 * DESCRIPTION
 *      The strip_dot_list function is used to check each filename in
 *      the list for a leading dot path element.  If there is one, it is
 *      removed, the original string free()ed, and the new string put in
 *      its place.
 *
 * CAVEAT
 *      This is only done if the STRIP_DOT option is enabled.  It is
 *      enabled by default, but the userr may choose to turn it off.
 */

void
strip_dot_list(string_list_ty *slp)
{
    size_t          j;

    if (option_test(OPTION_STRIP_DOT))
    {
        for (j = 0; j < slp->nstrings; ++j)
        {
            string_ty       *s;

            s = strip_dot_inner(slp->string[j]);
            str_free(slp->string[j]);
            slp->string[j] = s;
        }
    }
}
